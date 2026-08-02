#!/usr/bin/env python3

# Copyright 2017 The ChromiumOS Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Mad Memory Muncher - Dynamically Organizing Non-Uniprocess Tester.

This program attempts to exercise low memory situations by munching memory
in a coordinated way across several processes.

Specifically, there is a single controlling process that keeps communication
channels open to subprocesses so that all processes can start and stop various
parts of the test at the same time.  This tester also has various clever ways
to access memory.

The main modes are: munch (allocate memory), taste (re-read already
allocated memory), and chew (modify already allocated memory).  Whenever
possible we try to put some sane values into memory so that any memory
compression will behave in a real-world-like way.

At the moment this program always makes sure that all of the child
sub-processes are set to have an OOM score of 1000 (easy to kill them) and
the parent has a default OOM score (unkillable on Chrome OS).  At various
checkpoints in the test the parent looks for dead children and stops the test.

NOTES:
- The way this program works is subject to chagne depending on the needs
  of people stressing memory.  Don't rely on command line arguments staying
  consistent.  If we need a consistent test, we could fork this or add a
  consistent subcommand.
- You should probably have KASAN and slub_debug turned off when running this.
  If you have those on then you're not doing a real test of the memory system
  and shouldn't be surprised that it can't keep up.

Examples:
    1. Launch one process per CPU and aim for 500 MB swap left.  Re-access
       memory for 70 seconds and then access/modify memory for 90 seconds:

         mmm_donut --free_swap=500 --taste=70 --chew=90
    2. Like #1 but use 200 processes.  Note that since by default each
       process will access 1MB at a time we'll probably really end up stopping
       at closer to 300 MB free swap or less (the muncher stops telling
       sub-processes to allocate when free swap is 500 MB, but then any
       outstanding allocatoins will finish.

         mmm_donut -n200 --free_swap=500 --taste=70 --chew=90
    3. Like #1 but have children allocate 20MB chunks.  This will act to
       more quickly allocate memory but will also over-allocate a bit more.
       On a 6-CPU system you might overallocate by 120MB.

         mmm_donut --free_swap=500 --munch_mbs=20 --taste=70 --chew=90
"""

import argparse
import ctypes
import multiprocessing
import numpy
import os
import queue
import subprocess
import sys
import time

libc = ctypes.CDLL('libc.so.6')
libc.free.argtypes = [ctypes.c_void_p]
libc.free.restype = None
libc.valloc.argtypes = [ctypes.c_size_t]
libc.valloc.restype = ctypes.c_void_p

# By default, we'll fill memory with data based on the contents of this
# file.  Ideally it should be a big file and fairly representative of
# what we expect memory to contain.
_DEFAULT_FILE_TO_MAP = '/opt/google/chrome/chrome'

_KB = 1024
_MB = _KB * _KB

# For the purpose of this program, a 'word' is 32-bits.
_WORDS_PER_MB = _MB // 4

_PAGESIZE = os.sysconf('SC_PAGESIZE')


class _MemoryMuncher(object):
    """A class for eating memory.

    This class has functions in it for efficiently munching up memory.
    Specifically, it has a few things it can do:

    munch: This will allocate more memory and fill it with data copied from
        a prototype datasource.  It will attempt to make this data 'unique'
        by adding a value to each word based on the current PID.  Allocating
        is done 1 MB at a time and done with valloc() so we get page-sized
        allocations.  Copying / making unique is done with numpy to get
        reasonably efficiency.
    taste: This will re-read memory (1MB at a time) that's already been munched,
        which ought to cause it to get paged in.  We read 1 word from each page.
        Again we use numpy which ought to make it somewhat efficient.
    chew: This will attempt to read-modify-write memory (1MB at a time) that's
        already been munched.  This ought to have no huge performance difference
        than taste.
    spit: This will release memory allocated by munch.

    Attributes:
        num_mbs_allocated: Number of MB that are currently allocated.
        num_mbs_munched: Number of MB that have been munched in total.  Note
            that if you munch something and then spit it out it still counts in
            this number, so munch(30); spit(10); munch(20) => 50.
        num_mbs_tasted: Number of MB that have been tasted in total.
        num_mbs_chewed: Number of MB that have been chewed in total.
    """

    def __init__(self, proto_data=None):
        """Create a MemoryMuncher object.

        Args:
            proto_data: A numpy.memmap array, or None for the default.  We'll
                use this as prototype data to copy to our allocated pages.
        """
        if not proto_data:
            proto_data = numpy.memmap(_DEFAULT_FILE_TO_MAP,
                                      dtype='uint32', mode='r')
        self._proto_data = proto_data
        self._num_proto_mbs = len(self._proto_data) // _WORDS_PER_MB
        self._at_proto_mb = 0

        # Every time we munch through a chunk we'll add this to each integer to
        # make the chunk look unique, then increment it.
        self._unique = os.getpid() << 16

        self._mbs = []
        self._last_accessed_mb = -1

        self.num_mbs_munched = 0
        self.num_mbs_tasted = 0
        self.num_mbs_chewed = 0

    @property
    def num_mbs_allocated(self):
        return len(self._mbs)

    def _alloc_array(self, n, element_type=ctypes.c_uint8):
        """Allocate a numpy array using libc.valloc (page aligned allocation).

        Args:
            n: Number of elements in the array
            element_type: The type of the element (a ctypes type)
        """
        ptr = libc.valloc(n * ctypes.sizeof(element_type))
        ptr = ctypes.cast(ptr, ctypes.POINTER(element_type))

        return numpy.ctypeslib.as_array(ptr, shape=(n,))

    def _free_array(self, arr):
        """Free a numpy array allocated with _alloc_array

        Args:
            arr: The return value from _alloc_array.
        """
        ptr = ctypes.cast(arr, ctypes.c_void_p)
        libc.free(ptr)

    def munch(self, mbs_to_munch, quick_alloc=False):
        """Allocate the given number of mbs, filling the memory with data.

        Args:
            mbs_to_munch: The number of MBs to allocate.
            quick_alloc: If true, we'll try to allocate quicker by not using
                the proto data; we'll just put a unique value in the first
                word of the page.
        """
        for _ in range(mbs_to_munch):
            # Allocate some memory using libc; give back a numpy object
            mb = self._alloc_array(_WORDS_PER_MB, ctypes.c_uint32)

            # Copy data from our proto data making it unique by adding a
            # unique integer to each word.
            mb[0] = self._unique

            if quick_alloc:
                # Don't even bother to zero memory, but put at least something
                # unique per page
                mb.reshape((_PAGESIZE, -1)).T[0] = self._unique
            else:
                # Copy from the next spot in the prototype
                # As we copy, add the unique data based on our PID.
                mb[:] = (self._proto_data[self._at_proto_mb *
                                          _WORDS_PER_MB:
                                          (self._at_proto_mb + 1) *
                                          _WORDS_PER_MB] + self._unique)

            # Update so we're ready for the next time
            self._at_proto_mb += 1
            self._at_proto_mb %= self._num_proto_mbs
            self._unique += 1

            self._mbs.append(mb)
            self.num_mbs_munched += 1

    def spit(self, mbs_to_spit):
        """Spit (free) out the oldest munched memory.

        Args:
            mbs_to_spit: Number of MBs to spit.
        """
        for _ in range(mbs_to_spit):
            if not self._mbs:
                raise RuntimeError('No more memory to spit out')
            self._free_array(self._mbs.pop(0))

    def taste(self, mbs_to_taste):
        """Access memory that we've munched through, reading 1 word per page.

        Args:
            mbs_to_taste: Number of MBs that we'd like to try to access
        """
        if not self._mbs:
            raise RuntimeError('No memory')

        mb_num = self._last_accessed_mb
        for mb_num in range(mb_num + 1, mb_num + 1 + mbs_to_taste):
            mb_num %= len(self._mbs)
            mb = self._mbs[mb_num]
            self.num_mbs_tasted += 1
            # Fancy numpy to access 1 word from each page
            _ = sum(mb.reshape((-1, _PAGESIZE)).T[0])
        self._last_accessed_mb = mb_num

    def chew(self, mbs_to_chew):
        """Modify memory that we've munched through, tweaking 1 word per page.

        Args:
            mbs_to_chew: Number of MBs that we'd like to try to access
        """
        if not self._mbs:
            raise RuntimeError('No memory')

        mb_num = self._last_accessed_mb
        for mb_num in range(mb_num + 1, mb_num + 1 + mbs_to_chew):
            mb_num %= len(self._mbs)
            mb = self._mbs[mb_num]
            self.num_mbs_chewed += 1

            # Fancy numpy to access 1 word from each page; we'll invert each
            # time as our modification
            _ = sum(mb.reshape((-1, _PAGESIZE)).T[0])
        self._last_accessed_mb = mb_num


class _MemInfo(object):
    """An object that makes accessing /proc/meminfo easy.

    When this object is created it will read /proc/meminfo and store all the
    attributes it finds as integer properties.  All memory quantities are
    expressed in bytes, so if /proc/meminfo said 'MemFree' was 100 kB then our
    MemFree attribute will be 102400.
    """

    def __init__(self):
        with open('/proc/meminfo', 'r') as f:
            for line in f.readlines():
                name, _, val = line.partition(':')
                num, _, unit = val.strip().partition(' ')
                num = int(num)

                if unit == 'kB':
                    num *= 1024
                elif unit != '':
                    raise RuntimeError('Unexpected meminfo: %s' % line)

                setattr(self, name, num)


def _make_self_oomable():
    """Makes sure that the current process is easily OOMable."""
    with open('/proc/self/oom_score_adj', 'w') as f:
        f.write('1000\n')


def _thread_main(task_num, options, cmd_queue, done_queue):
    """The main entry point of the worker threads.

    Threads communicate with the main thread through two queues.  They get
    commands from the cmd_queue and communicate that they're done by putting
    their task_num on the done_queue.

    Args:
        task_num: The integer ID of this task.
        options: Options created by _parse_options()
        cmd_queue: String commands will be put here by the main thread.
        done_queue: We'll put our task_num on this queue when we're done with
            our command.
    """
    _make_self_oomable()

    muncher = _MemoryMuncher()

    munch_mbs = options.munch_mbs
    taste_mbs = options.taste_mbs
    chew_mbs = options.chew_mbs

    try:
        cmd = None
        while cmd != 'done':
            cmd = cmd_queue.get()
            if cmd == 'status':
                print(('Task %d: allocated %d MB, munched %d MB, ' +
                       'tasted %d MB, chewed %d MB') %
                      (task_num, muncher.num_mbs_allocated,
                       muncher.num_mbs_munched, muncher.num_mbs_tasted,
                       muncher.num_mbs_chewed))
            elif cmd == 'munch':
                muncher.munch(munch_mbs)
            elif cmd == 'taste':
                muncher.taste(taste_mbs)
            elif cmd == 'chew':
                muncher.chew(chew_mbs)

            done_queue.put(task_num)
    except KeyboardInterrupt:
        # Don't yell about keyboard interrupts
        pass
    finally:
        print('Task %d is done' % task_num)
        done_queue.close()
        cmd_queue.close()


class WorkerDeadError(RuntimeError):
    """We throw this when we see that a worker has died."""
    def __init__(self, task_num):
        super(WorkerDeadError, self).__init__('Task %d is dead' % task_num)
        self.task_num = task_num


def _wait_everyone_done(tasks, done_queue, refill_done_queue=True):
    """Wait until all of our workers are done.

    This will wait until all tasks have put their task_num in the done_queue.
    We'll also check to see if any tasks are dead and we'll raise an exception
    if we notice this.

    Args:
        tasks: The list of our worker tasks.
        done_queue: Our done queue
        refill_done_queue: If True then we'll make sure that the done_queue
            has each task number in it when we're done; if False then we'll
            leave the done_queue empty.

    Raises:
        WorkerDeadError: If we notice something has died.
    """
    num_tasks = len(tasks)

    # We want to see every task number report it's done via the done_queue; if
    # things are taking too long we'll poll for dead children.
    done_tasks = set()
    while len(done_tasks) != num_tasks:
        try:
            task_num = done_queue.get(timeout=.5)
            done_tasks.add(task_num)
        except queue.Empty:
            for task_num, task in enumerate(tasks):
                if not task.is_alive():
                    raise WorkerDeadError(task_num)

    assert done_queue.empty()
    if not refill_done_queue:
        return

    # Add everyone back to the done_queue.
    for task_num in range(num_tasks):
        done_queue.put(task_num)


def _end_stage(old_stage_name, tasks, done_queue, cmd_queues):
    """End the given stage and ask wokers to print status.

    Args:
        old_stage_name: We'll print this to tell the user we finished this.
        tasks: The list of our worker tasks.
        done_queue: Our done queue
        cmd_queues: A list of all task command queues.
    """
    num_tasks = len(tasks)

    # Wait, but don't refill the queue since since we'll get the queue
    # refilled after the workers finish printing their status.
    _wait_everyone_done(tasks, done_queue, refill_done_queue=False)

    print('Done with stage %s' % old_stage_name)

    # Give the system a second to quiesce (TODO: needed?)
    time.sleep(1)

    # We'll throw an extra status update; this will refill the done_queue
    for task_num in range(num_tasks):
        assert cmd_queues[task_num].empty()
        cmd_queues[task_num].put('status')
    _wait_everyone_done(tasks, done_queue)


def _parse_options(args):
    """Parse command line options.

    Args:
        args: sys.argv[1:]

    Returns:
        An argparse.ArgumentParser object.
    """
    p = subprocess.Popen(['nproc'], stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)
    stdout, _ = p.communicate()
    nproc = int(stdout)

    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        '-n', '--num_tasks', type=int, default=nproc,
        help='Number of tasks to use (default: %(default)s)'
    )
    parser.add_argument(
        '-z', '--munch_mbs', type=int, default=1,
        help='Munch this many MB at a time (default: %(default)s)'
    )
    parser.add_argument(
        '-s', '--free_swap', type=int, default=500,
        help='Stop munching when free swap <= this many MB ' +
        '(default: %(default)s)'
    )
    parser.add_argument(
        '-f', '--free_total', type=int, required=False,
        help='Stop munching when free memory <= this many MB ' +
        '(default: disabled)'
    )
    parser.add_argument(
        '-a', '--available', type=int, required=False,
        help='Stop munching when available memory <= this many MB ' +
        '(default: disabled)'
    )
    parser.add_argument(
        '-t', '--taste', type=int, default=30,
        help='Taste for this many seconds (default: %(default)s)'
    )
    parser.add_argument(
        '-T', '--taste_mbs', type=int, default=-1,
        help='Taste this many MB at a time (default: use munch_mbs)'
    )
    parser.add_argument(
        '-c', '--chew', type=int, default=30,
        help='Chew for this many seconds (default: %(default)s)'
    )
    parser.add_argument(
        '-C', '--chew_mbs', type=int, default=-1,
        help='Chew this many MB at a time (default: use munch_mbs)'
    )
    parser.add_argument(
        '-F', '--memfree_sleep', type=int, default=0,
        help='Sleep when memfree is < this many MB (default: %(default)s)'
    )

    options = parser.parse_args(args)

    if options.taste_mbs == -1:
        options.taste_mbs = options.munch_mbs
    if options.chew_mbs == -1:
        options.chew_mbs = options.munch_mbs

    return options


def main(args):
    options = _parse_options(args)

    num_tasks = options.num_tasks

    done_queue = multiprocessing.Queue()
    cmd_queues = [multiprocessing.Queue() for task_num in range(num_tasks)]
    tasks = [
        multiprocessing.Process(
            target=_thread_main,
            args=(task_num, options, cmd_queues[task_num], done_queue)
        )
        for task_num in range(num_tasks)
    ]
    for task in tasks:
        task.start()

    print('Starting test.')
    for task_num in range(num_tasks):
        cmd_queues[task_num].put('status')
    _wait_everyone_done(tasks, done_queue)

    try:
        print("Munching till one of these is met:")
        print("- swap < %d MB free" % options.free_swap)
        if options.free_total is not None:
            print("- total memory < %d MB free" % options.free_total)
        if options.available is not None:
            print("- available memory < %d MB" % options.available)
        while True:
            meminfo = _MemInfo()
            if meminfo.SwapFree < options.free_swap * _MB:
                break
            if options.free_total is not None and meminfo.MemFree < options.free_total * _MB:
                break
            if options.available is not None and meminfo.MemAvailable < options.available * _MB:
                break
            if meminfo.MemFree < options.memfree_sleep * _MB:
                print('MemFree only %d MB; sleeping' % (meminfo.MemFree / _MB))
                time.sleep(1)
                continue
            task_num = done_queue.get()
            cmd_queues[task_num].put('munch')

            for task_num, task in enumerate(tasks):
                if not task.is_alive():
                    raise WorkerDeadError(task_num)

        _end_stage('munch', tasks, done_queue, cmd_queues)

        print('Tasting for %d seconds; taste %d MB at a time.' %
              (options.taste, options.taste_mbs))
        end_time = time.time() + options.taste
        while time.time() < end_time:
            task_num = done_queue.get()
            cmd_queues[task_num].put('taste')
        _end_stage('taste', tasks, done_queue, cmd_queues)

        print('Chewing for %d seconds; chew %d MB at a time.' %
              (options.chew, options.chew_mbs))
        end_time = time.time() + options.chew
        while time.time() < end_time:
            task_num = done_queue.get()
            cmd_queues[task_num].put('chew')
        _end_stage('chew', tasks, done_queue, cmd_queues)

    except KeyboardInterrupt:
        pass
    except WorkerDeadError as error:
        print('ERROR: %s' % str(error))
    finally:
        print('All done I guess; trying to end things nicely.')

        # Throw in a command to try to get them to quit
        for cmd_queue in cmd_queues:
            cmd_queue.put('done')
        for task in tasks:
            task.join(10)
            task.terminate()

        done_queue.close()
        for cmd_queue in cmd_queues:
            cmd_queue.close()

        print('Quitting')

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
