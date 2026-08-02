# Copyright 2023 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Attempt to provide a reasonably correct accounting of system memory usage."""

import argparse
import os
import pprint
import pwd
import re
import stat
import struct
import time
import traceback


# Don't display any processes or mappings that use less real memory than this
MIN_BYTES_TO_DISPLAY = 10 * 1024**2


# Verbosity levels
SHORT_OUTPUT = 0
NORMAL_OUTPUT = 1
VERBOSE_OUTPUT = 2


class Printer:
    """Verbosity-aware print() function."""

    def __init__(self):
        self.verbosity = NORMAL_OUTPUT

    def should_print(self, verbosity):
        """Check if the given verbosity level is being printed."""
        return self.verbosity >= verbosity

    def print(self, verbosity, *args):
        """Print something, if we have a high enough verbosity."""
        if self.should_print(verbosity):
            print(*args)


printer = Printer()
p = printer.print


def find_kernel_page_size():
    """Read kernel page size out of our smaps file."""
    with open("/proc/self/smaps", encoding="utf-8") as smaps_file:
        for line in smaps_file:
            m = re.match(r"KernelPageSize:\s+(\d+) kB", line)
            if m:
                return int(m.group(1)) * 1024
    raise Exception("Unable to find KernelPageSize in /proc/self/smaps")


kpagesize = find_kernel_page_size()


def format_mem_size(sz):
    """Format a number of bytes as a human-readable string."""
    if abs(sz) < 1024:
        return str(sz)
    sz /= 1024
    if abs(sz) < 1024:
        return f"{sz:.1f}k"
    sz /= 1024
    if abs(sz) < 1024:
        return f"{sz:.1f}M"
    sz /= 1024
    return f"{sz:.1f}G"


class Mapping:
    """A single memory mapping, from /proc/PID/smaps."""

    def __init__(
        self,
        startaddr,
        endaddr,
        perms,
        offset,
        dev,
        inode,
        pathname,
        pss,
        uss,
        swap,
    ):
        (
            self.startaddr,
            self.endaddr,
            self.perms,
            self.offset,
            self.dev,
            self.inode,
            self.pathname,
            self.pss,
            self.uss,
            self.swap,
        ) = (
            startaddr,
            endaddr,
            perms,
            offset,
            dev,
            inode,
            pathname,
            pss,
            uss,
            swap,
        )

        if self.pathname in ("anon_inode:i915.gem", "/dev/dri/renderD128"):
            total = self.total()
            self.gfx = total
            # I've never seen nonzero Pss numbers for graphics mappings; assert that
            # this continues to be the case.
            assert (
                not self.pss
            ), f"TODO check if pss {pss} should be total {total} for mapping {self.pathname}"
        else:
            self.gfx = 0

        assert (
            self.startaddr % kpagesize == 0
        ), f"startaddr {self.startaddr:x} is not page-aligned ({kpagesize:d} bytes)"
        assert self.endaddr % kpagesize == 0, "endaddr is not page-aligned"

    def __repr__(self):
        return (
            f"<Mapping: {self.startaddr:x}-{self.endaddr:x}"
            f" ({format_mem_size(self.total())})"
            f" pss={format_mem_size(self.pss)}"
            f" uss={format_mem_size(self.uss)}"
            f" swap={format_mem_size(self.swap)}"
            f" perms={self.perms }"
            f" offset={self.offset}"
            f" dev={self.dev}"
            f" inode={self.inode}"
            f" pathname={self.pathname}"
            ">"
        )

    def total(self):
        """Calculate total mapping size."""
        return self.endaddr - self.startaddr

    def read_pagemap_values(self, map_file):
        """Read a series of mappings from /proc/{pid}/pagemap.

        Args:
            map_file: Already-open /proc/{pid}/pagemap file.

        Returns:
            A list of 64-bit pagemap values, as described in
            https://www.kernel.org/doc/Documentation/vm/pagemap.txt
        """
        map_file.seek((self.startaddr // kpagesize) * 8)
        pagecount = (self.endaddr - self.startaddr) // kpagesize
        pmap_data = map_file.read(pagecount * 8)
        return struct.unpack(f"{pagecount:d}Q", pmap_data)


class Process:
    """Process details from a PID directory under /proc."""

    def __init__(self, pid):
        self.pid = pid
        self.children = set()
        self.maps = []
        self.comm = None

        if self.pid:
            self.load_details()

    def load_details(self):
        """Read various files under /proc/[pid] and extract memory usage info."""
        self.root = f"/proc/{self.pid:d}"

        def o(leaf, mode="rt"):
            # pylint: disable=consider-using-with
            return open(os.path.join(self.root, leaf), mode, encoding="utf-8")
            # pylint: enable=consider-using-with

        def r(leaf):
            return o(leaf).read().strip()

        self.uid = os.stat(self.root)[stat.ST_UID]
        try:
            self.user = pwd.getpwuid(self.uid).pw_name
        except KeyError:
            self.user = str(self.uid)  # not in /etc/passwd

        self.comm = r("comm")
        self.cmdline = r("cmdline").replace("\0", " ")
        self.status = dict(
            re.match(r"(\w+)\:\t(.*)", line).groups()
            for line in r("status").split("\n")
        )
        self.ppid = int(self.status["PPid"])

        def read_smaps():
            """Parse /proc/PID/smaps."""
            with o("smaps") as f:
                lines = []
                for line in f:
                    bits = line.split()
                    if lines and "-" in bits[0]:
                        # start of next group
                        yield lines
                        lines = []
                    lines.append(line)
                if lines:
                    yield lines

        # Make a Mapping object from every block in /proc/PID/smaps.
        self.uss = self.pss = self.gfx = self.swap = 0
        totalmem = 0
        self.maps = []
        for entry in read_smaps():
            # Parse mapping header.
            startaddr, endaddr, perms, offset, dev, inode, pathname = re.match(
                r"([0-9a-f]+)-([0-9a-f]+) ([rwxsp-]+) ([0-9a-f]+) ([0-9a-f:]+) (\d+) +(.*)",
                entry[0],
            ).groups()
            # Parse key/value data that follows the header.
            entry_uss = 0
            for line in entry[1:]:
                m = re.search(r"^(\w+):\s+(\d+) kB\s*$", line)
                if not m:
                    continue
                name = m.group(1)
                kb = int(m.group(2))
                b = kb * 1024
                if name == "Pss":
                    entry_pss = b
                elif name in ("Private_Clean", "Private_Dirty"):
                    entry_uss += b
                elif name == "SwapPss":
                    entry_swap = b
            mapping = Mapping(
                int(startaddr, base=16),
                int(endaddr, base=16),
                perms,
                int(offset, base=16),
                dev,
                inode,
                pathname,
                entry_pss,
                entry_uss,
                entry_swap,
            )
            self.maps.append(mapping)
            totalmem += mapping.total()
            self.gfx += mapping.gfx
            self.pss += mapping.pss
            self.uss += mapping.uss
            self.swap += mapping.swap

    def vm_name(self):
        """Detect VM based on contents of process command line."""
        if not self.comm:
            return None
        if not self.cmdline.startswith("/usr/bin/crosvm "):
            return None
        if "--syslog-tag ARCVM" in self.cmdline:
            return "arcvm"
        if "Ym9yZWFsaXM=" in self.cmdline:  # 'borealis' in base64
            return "borealis"
        # TODO Add termina.
        return "unknown_vm"

    def print_tree(self, prefix="", last_child=False):
        """Dump the entire tree of processes, with output similar to `ps auxf`."""
        sorted_children = sorted(
            (c for c in self.children if c.big_enough_to_print()),
            key=lambda c: c.pid,
        )
        if not self.pid:
            print("kernel")
            prefix = ""
            # Reverse order, so kernel processes show up before user
            # processes (children of pid 1), and the user won't need
            # to scroll up as much to get to the interesting bits.
            sorted_children.reverse()
        else:
            formatted_maps = (
                str(len(self.maps)) if self.maps is not None else "inaccessible"
            )
            print(
                f"{prefix} \\_ {self.pid:d} ({self.user}):"
                f" [{self.comm}] {self.cmdline} maps={formatted_maps}"
            )
            prefix += "    " if last_child else " |  "
            self.show_maps(prefix + (" " * len(str(self.pid))))
        if sorted_children:
            for child in sorted_children[:-1]:
                child.print_tree(prefix)
            sorted_children[-1].print_tree(prefix, last_child=True)

    def mem_by_mapping(self):
        """Return dict of pathname -> bytes of pss, summed up over all mappings."""
        totals = {}
        for m in self.maps:
            totals[m.pathname] = totals.get(m.pathname, 0) + m.pss
        return totals

    def show_maps(self, prefix=""):
        """Dump mappings using significant memory."""
        if self.maps:
            # Accumulate memory usage by pathname.
            totals = self.mem_by_mapping()
            # Filter out mappings that are too small to bother showing.
            totals = {
                path: size
                for path, size in totals.items()
                if size > MIN_BYTES_TO_DISPLAY
            }
            # ... and dump it!
            print(
                prefix,
                f"pss/gfx totals for pid {self.pid:d} ({self.comm}):"
                f" {format_mem_size(self.pss + self.gfx)}"
                f" (pss {format_mem_size(self.pss)}"
                f" gfx {format_mem_size(self.gfx)}); "
                + " ".join(
                    f"{k}={format_mem_size(v)}"
                    for k, v in sorted(
                        totals.items(), key=lambda i: i[1], reverse=True
                    )
                ),
            )

    def big_enough_to_print(self):
        """Return true if at least one subprocess is bigger than MIN_BYTES_TO_DISPLAY."""
        if self.pss + self.gfx > MIN_BYTES_TO_DISPLAY:
            return True
        if self.children:
            for child in self.children:
                if child.big_enough_to_print():
                    return True
        return False


def read_meminfo():
    """Parse everything out of /proc/meminfo."""
    minfo = {}
    with open("/proc/meminfo", encoding="utf-8") as f:
        for line in f:
            k, v, kb = re.match(r"([\w()]+):\s+(\d+)( kB)?\s*", line).groups()
            minfo[k] = int(v) * (1024 if kb else 1)
    return minfo


def read_vmstat():
    """Parse everything out of /proc/vmstat."""
    vmstat = {}
    with open("/proc/vmstat", encoding="utf-8") as f:
        for line in f:
            k, v = re.match(r"(\w+)\s*(\d+)\s*", line).groups()
            vmstat[k] = int(v)
    return vmstat


def read_slab_usage():
    """Read kernel slab usage."""
    slab_usage = 0
    with open("/proc/slabinfo", encoding="utf-8") as f:
        f.readline()  # skip header
        for line in f:
            line = line.strip()
            if not line:
                continue
            if line.startswith("#"):
                continue
            bits = line.split()
            pagesperslab = int(bits[5])
            num_slabs = int(bits[14])
            slab_usage += pagesperslab * num_slabs * 4096
    return slab_usage


def read_zram_usage():
    """Read zram usage (if running on host)."""
    try:
        # https://www.kernel.org/doc/Documentation/blockdev/zram.txt
        with open("/sys/block/zram0/mm_stat", encoding="utf-8") as f:
            return int(f.readline().strip().split()[2])
    except FileNotFoundError:
        p(VERBOSE_OUTPUT, "no zram")
    return 0


def read_gpu_usage():
    """Detect GPU memory used."""
    try:
        # On machines with Intel GPUs, i915_gem_objects gives some info about
        # where GPU memory is going.  This seems to give us the total memory
        # allocated, including swapped-out pages.  For resident pages, looking
        # for anon_inode:i915.gem mappings in individual processes seems to
        # work well.
        with open(
            "/sys/kernel/debug/dri/0/i915_gem_objects", encoding="utf-8"
        ) as f:
            gpu_usage = int(
                re.search(
                    r"shrinkable.*?(\d+) bytes$", f.readline().strip()
                ).group(1)
            )
            while 1:
                # get past the header
                if not f.readline().strip():
                    break
            for line in f:
                p(VERBOSE_OUTPUT, line.strip())
                m = re.search(r"virtio_gpu: \d+ objects, (\d+) bytes", line)
                if m:
                    gpu_usage = max(gpu_usage, int(m.group(1)))
        p(
            VERBOSE_OUTPUT,
            f"gpu usage estimate from i915_gem_objects: {format_mem_size(gpu_usage)}",
        )
        return gpu_usage
    except FileNotFoundError:
        p(
            VERBOSE_OUTPUT,
            "no i915_gem_objects; either not an intel gpu or we're in the guest",
        )
    return 0


def main():
    """Main entry point.

    Read everything and attempt to consolidate all data
    sources to provide an accounting of memory usage over
    the whole system.
    """

    parser = argparse.ArgumentParser(description=__doc__)

    # By default, we show the process list, a summary of mappings
    # ordered by PSS, and a short cut-and-pasteable memory report.
    # Pass --short to hide the process list and mappings under 0.5
    # GiB.  Pass --verbose to add extra debug info: the contents of
    # /proc/meminfo, etc.

    parser.add_argument(
        "--short",
        help="Only output the short memory report and processes using >0.5GiB memory",
        action="store_true",
    )
    parser.add_argument(
        "--verbose",
        help="Output extra debug information as well as the process list and short memory report",
        action="store_true",
    )

    args = parser.parse_args()
    if args.short:
        printer.verbosity = SHORT_OUTPUT
    elif args.verbose:
        printer.verbosity = VERBOSE_OUTPUT

    p(VERBOSE_OUTPUT, "looking through /proc/*/maps")
    p(VERBOSE_OUTPUT, f"kernel page size is {kpagesize}")

    # Read /proc/meminfo.
    meminfo = read_meminfo()
    p(VERBOSE_OUTPUT, pprint.pformat(meminfo))

    # Read /proc/vmstat.
    vmstat = read_vmstat()
    p(VERBOSE_OUTPUT, pprint.pformat(vmstat))

    # Sum up /proc/slabinfo.
    slab_usage = read_slab_usage()
    if args.verbose:
        p(
            VERBOSE_OUTPUT,
            # Mem used inside the slabs.
            f"kernel slabs are using {format_mem_size(slab_usage)}",
        )
        p(
            VERBOSE_OUTPUT,
            # Actual mem reserved for slabs.
            f"c.f. Slab {format_mem_size(meminfo['Slab'])} in meminfo",
        )

    # Find how much memory we're using for zram.
    zram_usage = read_zram_usage()

    # Find how much memory we're using for graphics.
    gpu_usage = read_gpu_usage()

    # Load details for each process under /proc.
    start_read_procs = time.time()
    procs = {0: Process(0)}
    totalpss = totaluss = totalgfx = totalswap = 0
    for pid in os.listdir("/proc"):
        if not re.match(r"\d+", pid):
            continue  # not a pid
        pid = int(pid)
        try:
            proc = Process(pid)
        except FileNotFoundError:
            traceback.print_exc()
            continue
        procs[pid] = proc
        totalpss += proc.pss
        totaluss += proc.uss
        totalgfx += proc.gfx
        totalswap += proc.swap

    for proc in procs.values():
        if proc.pid:
            procs[proc.ppid].children.add(proc)

    read_procs_time = time.time() - start_read_procs
    if args.verbose:
        p(VERBOSE_OUTPUT, f"reading procs took {read_procs_time:.1f} s")

    # Dump out tree of processes and their significant mappings.
    if printer.should_print(NORMAL_OUTPUT):
        procs[0].print_tree()

    # Size of the virtio balloon.
    balloon_size = (
        vmstat.get("balloon_inflate", 0) - vmstat.get("balloon_deflate", 0)
    ) * kpagesize

    # Add up memory by pathname across all processes.
    totals_by_pathname = {}
    for proc in procs.values():
        for pathname, size in proc.mem_by_mapping().items():
            if "crosvm_guest" in pathname:
                vm_name = proc.vm_name()
                if vm_name:
                    pathname += f" {vm_name}"
            totals_by_pathname[pathname] = (
                totals_by_pathname.get(pathname, 0) + size
            )
    totals_by_pathname = sorted(totals_by_pathname.items(), key=lambda e: e[1])

    p(NORMAL_OUTPUT, "\nLargest mem usage by mapping:")
    for pathname, size in totals_by_pathname[-10:]:
        p(
            SHORT_OUTPUT if size > 512 * 1024**2 else NORMAL_OUTPUT,
            f"- {pathname}: {format_mem_size(size)}",
        )
    p(NORMAL_OUTPUT)

    swap_used = meminfo["SwapTotal"] - meminfo["SwapFree"]
    p(
        SHORT_OUTPUT,
        f"Swap used {format_mem_size(swap_used)}; "
        f"c.f. sum of SwapPss everywhere {format_mem_size(totalswap)}",
    )

    unaccounted_memory = (
        meminfo["MemTotal"]
        - balloon_size
        - meminfo["MemAvailable"]
        - zram_usage
        - gpu_usage
        - meminfo["Slab"]
        - totalpss
    )
    p(
        SHORT_OUTPUT,
        f"Total pss {format_mem_size(totalpss)}"
        f" [includes uss {format_mem_size(totaluss)}],"
        f" (total {format_mem_size(meminfo['MemTotal'])}"
        f" - balloon {format_mem_size(balloon_size)}"
        f" - avail {format_mem_size(meminfo['MemAvailable'])}"
        f" = {format_mem_size(meminfo['MemTotal'] - balloon_size - meminfo['MemAvailable'])};"
        f" - zram {format_mem_size(zram_usage)}"
        f" - gpu {format_mem_size(gpu_usage)}"
        f" [c.f. process gfx {format_mem_size(totalgfx)}]"
        f" - kslab {format_mem_size(meminfo['Slab'])}"
        f" - pss {format_mem_size(totalpss)}"
        f" = {format_mem_size(unaccounted_memory)})",
    )

    # Print it again in a copy and paste friendly way
    p(
        SHORT_OUTPUT,
        f"PSS {format_mem_size(totalpss)}"
        f" GFX {format_mem_size(totalgfx)}"
        f" GPU {format_mem_size(gpu_usage)}"
        f" zram {format_mem_size(zram_usage)}"
        f" swap {format_mem_size(meminfo['SwapTotal'] - meminfo['SwapFree'])}"
        f" avail {format_mem_size(meminfo['MemAvailable'])}",
    )


if __name__ == "__main__":
    main()
