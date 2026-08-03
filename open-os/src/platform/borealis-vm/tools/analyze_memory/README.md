# analyze_cros_memory

This tool digs through the various places where Linux reports memory usage
information, and attempts to produce a reasonably correct accounting of where
everything is going.

## Usage

If you have a device running a test image, so that you can run `ssh dut` and get
a root shell, use `run_on_dut.sh` to copy the script over and run it:

    ./run_on_dut.sh -h dut  # To analyze host memory
    ./run_on_dut.sh -b dut  # To analyze Borealis memory

Or to run both and save them as `result-host.txt` and `result-guest.txt`:

    ./collect.sh dut result

If you don't have SSH access to your device, copy `analyze_cros_memory.py` over
somehow, then run it from `crosh`.  These instructions assume it's in your
Downloads folder:

    # Run on host.
    python3 /home/chronos/user/MyFiles/Downloads/analyze_cros_memory.py

    # Run on Borealis guest.
    borealis-sh dd of=/home/chronos/analyze_cros_memory.py < \
        /home/chronos/user/MyFiles/Downloads/analyze_cros_memory.py \
    && borealis-sh --user=root python3 /home/chronos/analyze_cros_memory.py

## Example output

Running the script produces output like this (some sections snipped below for brevity!)

    RUNNING ON HOST ON mydevice
    looking through /proc/*/maps
    kernel page size is 4096
    {'Active': 700751872,
     'Active(anon)': 431222784,
     'Active(file)': 269529088,
    [...]
    kernel slabs are using 215.5M
    c.f. Slab 222.8M in meminfo
    gpu usage estimate from i915_gem_objects: 247.3M
    reading procs took 0.6 s
    kernel
     \_ 1 (root): [init] /sbin/init  maps=44
          pss/gem totals for pid 1 (init): 3.3M (pss 3.3M gem 0);
    [...]
    Largest mem usage by mapping:
    - /usr/lib64/libcrypto.so.1.1: 13.7M
    - [heap]: 45.8M
    - /opt/google/chrome/chrome: 222.8M
    - : 245.9M

    Swap used 0; c.f. sum of SwapPss everywhere 0
    Total pss 777.2M [includes uss 571.8M], (total 7.6G - balloon 0 - avail 6.0G = 1.5G; - zram 12.0k - gem 79.9M [c.f. gpu 247.3M] - kslab 222.8M - pss 777.2M = 494.8M)
    PSS 777.2M GPU 79.9M zram 12.0k swap 0 avail 6.0G

## Output sections

### Parsed /proc/meminfo and /proc/vmstat

The script first dumps out the contents of /proc/meminfo and /proc/vmstat.

### Digested /proc/PID/*

This is followed by a process tree showing all mappings using over 10 MiB of PSS or
graphics memory (listed as GEM, because the script only understands Intel GEM
memory right now.)

Example process with mappings:

    \_ 1640 (chronos): [steam] /home/chronos/.local/share/Steam/ubuntu12_32/steam -chromeos -chromeosnopreallocate  maps=711
            pss/gem totals for pid 1640 (steam): 138.0M (pss 138.0M gem 0); /usr/lib32/libLLVM-15.so=27.8M =18.7M /home/chronos/.local/share/Steam/ubuntu12_32/steamclient.so=18.0M /home/chronos/.local/share/Steam/ubuntu12_32/steamui.so=12.3M

In this case, Steam is using about 138 MiB overall, with 18.7M of anonymous
memory, and three mapped libraries using over 10 MiB each.

### Biggest mappings

This is followed by a list of the mappings using the most memory, summed across
all processes.  For example:

    Largest mem usage by mapping:
    - /usr/lib32/libLLVM-15.so: 27.8M
    - [heap]: 39.0M
    - /usr/lib/libLLVM-15.so: 48.9M
    - /home/chronos/.local/share/Steam/ubuntu12_64/libcef.so: 119.5M
    - : 146.6M

Here, the sum of memory used by mappings named `/usr/lib32/libLLVM-15.so` in all
processes on the system is 27.8 MiB.  `: 146.6M` means that a total of 146.6 MiB
of anonymous memory is in use.

On the host, you'll usually see something like `/memfd:crosvm_guest (deleted):
3.4G`, which indicates that crosvm guests (Borealis, ARCVM, Crostini) are using
a total of 3.4 GiB.  (`/memfd:` and `(deleted)` indicate that this memory is
part of a memfd, which is a special kind of temporary file.)

Note that this does not include swap: all of this is resident memory.

### Summary section

The last three output lines summarize the types of memory in use.

`Swap used 33.5M; c.f. sum of SwapPss everywhere 0`

- `Swap used` = what top reports.

- `Sum of SwapPss` = more or less how much process memory is swapped.

- So `Swap used` - `Sum of SwapPss` ~= how much graphics memory is swapped.

`total pss 5.2G [includes uss 2.4G], (total 7.4G - balloon 0 - avail 1.6G = 5.8G; - zram 716.0k - gem 385.3M [c.f. gpu 452.4M] - kslab 267.9M - pss 5.2G = 22.8M)`

- "pss" is proportional set size, which is like RSS, except when multiple
  processes share a mapping or some memory, it's divided across them, so it sums
  to the actual amount used total pss ~= total process memory (anon memory,
  mapped files that are actually using ram, etc)

- uss = unshared pages, i.e. pages that are only used by one process

Followed by an accuracy check at the end:

- 7.4G total ram on the system

- balloon not inflated at all

- 1.6G available (i.e. free or evictable, like file-backed mappings)

- so that means 5.8G is truly used

- the zram swap device is using 716k,

- adding up all GEM mappings from processes gives 385.3M,

- i915_gem_objects is reporting 452.4,

- kernel slabs are 267.9M,

- and process memory is 5.2G,

- leaving 22.8M of mystery memory that I can't account for.  Swapped GPU memory,
  maybe?

`PSS 5.2G GPU 385.3M zram 716.0k swap 33.5M avail 1.6G`

This last line is a quick summary of some of the more important numbers from the previous line.
