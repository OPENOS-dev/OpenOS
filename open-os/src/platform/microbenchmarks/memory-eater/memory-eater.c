// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

// Lack a common header that defines page size on x86 and ARM. So just
// explicitly define it here for convenience.
#define PAGE_SHIFT 12
#define PAGE_SIZE (1<<PAGE_SHIFT)

// IPC handshake signals
#define IPC_GOAHEAD "1"
#define IPC_DONE "2"
#define IPC_DONE_C '2'

// Add 's' after nouns
#define CHECK_PLURAL(x) (x > 1 ? "s" : "")

// Accumulator used to suppress compiler optimizations
int g;

// Walk though the pages with specified starting location and number of
// iterations. Return the next page index. Record the elapsed time too.
size_t walk_through_pages(char *buf, size_t start_idx, size_t iters,
                          size_t num_pages, unsigned int wait_time,
                          clock_t *elapsed_time)
{
  size_t idx = start_idx;

  clock_t start, end;
  start = clock();
  for (size_t i = 0; i < iters; i++) {
    g += buf[idx << PAGE_SHIFT];
    idx += 1;
    // Wrap around idx
    if (idx >= num_pages)
      idx = 0;
  }
  end = clock();
  if (elapsed_time)
    *elapsed_time = end - start;
  if (wait_time)
    sleep(wait_time);
  return idx;
}

// Initialize the buffer with contents
void initialize_pages(char *buf, size_t bytes)
{
  clock_t start, end;
  start = clock();
  // Writing every byte is too slow. We just need to populate some values and
  // every 4 byte seems OK.
  for (size_t i = 0; i < bytes; i += 4)
    buf[i] = i;
  end = clock();
  printf("Initializing %u pages took %.2f seconds\n",
         (int)(bytes / PAGE_SIZE), ((double)(end - start)/CLOCKS_PER_SEC));
}

// Iterate pages in the buffer for repeat_count times to measure the
// throughput. When do_fork is true, run two processes concurrently to
// generate enough memory pressure, in case a single process cannot consume
// enough memory (eg 32-bit userspace and 64-bit kernel).
// Instead of accessing all the pages in a non-stop way, try to slice the
// pages into chunks and use IPC to coordinate the accesses.
void measure_zram_throughput(char *buf, size_t bytes, size_t num_pages,
                             unsigned int pages_per_chunk,
                             unsigned int repeat_count, unsigned int wait_time,
                             int do_fork)
{
  clock_t elapsed_time, total_time = 0;
  int fd1[2], fd2[2];
  pid_t childpid;
  int receiver, sender;
  char sync;

  // For the child process, manage the pipes and let the child process to start
  // accessing pages.
  if (do_fork) {
    if (pipe(fd1) || pipe(fd2)) {
      perror("pipe failure");
      exit(1);
    }

    if ((childpid = fork()) == -1) {
      perror("fork failure");
      exit(1);
    }

    if (childpid == 0) {
      // Child process
      close(fd1[0]);
      close(fd2[1]);
      sender = fd1[1];
      receiver = fd2[0];

    } else {
      // Parent process
      close(fd2[0]);
      close(fd1[1]);
      sender = fd2[1];
      receiver = fd1[0];
      // Let the child process to fetch pages first
      if (write(sender, IPC_GOAHEAD, 1) == -1) {
        perror("write failure");
        exit(1);
      }
    }

    // Dirty the pages again to trigger copy-on-write so that we have enough
    // memory pressure from both processes.
    // NOTE: It is found that re-initializing for the parent process can reduce
    // the difference in results with the child process.
    initialize_pages(buf, bytes);
  }

  // Warmup: touch every page in the buffer to thrash memory first
  walk_through_pages(buf, 0, num_pages, num_pages, 0, &elapsed_time);

  size_t total_pages = num_pages * repeat_count;

  size_t page_idx = 0;
  bool need_to_sync = do_fork;

  // Itereate until the specified amount of pages are all accesses
  while (total_pages != 0) {
    // Synchronize parent and child page accesses
    if (need_to_sync) {
      if (read(receiver, &sync, 1) == -1) {
        perror("read failure");
        exit(1);
      }
      // The other process is done - no need to wait anymore.
      if (sync == IPC_DONE_C)
        need_to_sync = false;
    }

    // Determine the number of pages to access in this iteration
    size_t num_pages_chunk = total_pages >= pages_per_chunk ?
                             pages_per_chunk : total_pages;

    page_idx = walk_through_pages(buf, page_idx, pages_per_chunk, num_pages,
                                  wait_time, &elapsed_time);
    total_time += elapsed_time;
    total_pages -= num_pages_chunk;

    // Let the other process proceed
    if (need_to_sync) {
      int r;
      if (total_pages)
        r = write(sender, IPC_GOAHEAD, 1);
      else
        r = write(sender, IPC_DONE, 1);
      if (r == -1) {
        perror("write failure");
        exit(1);
      }
    }
  }

  printf("Average page access speed: %.0f pages/sec\n",
         (num_pages * repeat_count)/((double)(total_time)/CLOCKS_PER_SEC));
}

void print_usage(char *name)
{
  printf(
    "Usage: %s --size MB [--speed] [--fork] [--repeat N] [--chunk P]\n"
    "  [--wait S]\n", name);
  printf(
    "  --size MB: required to specify the buffer size\n");
  printf(
    "  --fork: fork a child process to double the memory usage\n");
  printf(
    "  --repeat N: access the pages in the buffer N times\n");
  printf(
    "  --chunk P: access P pages in the buffer then wait for S seconds\n");
  printf(
    "  --wait S: wait S seconds between chunks of page accesses\n");
}

int main(int argc, char* argv[])
{
  // Control flags
  int opt_do_fork = 0;
  int opt_measure_speed = 0;
  size_t opt_size_mb = 0;
  unsigned opt_repeat_count = 0;
  unsigned opt_pages_per_chunk = 0;
  unsigned opt_wait_time = 0;


  if (argc < 2) {
    print_usage(argv[0]);
    exit(1);
  }

  while (1) {
    static struct option long_options[] = {
      {"help",  no_argument, 0, 'h'},
      {"fork",  no_argument, 0, 'f'},
      {"speed",  no_argument, 0, 'd'},
      {"size",  required_argument, 0, 's'},
      {"repeat", required_argument, 0, 'r'},
      {"chunk",  required_argument, 0, 'c'},
      {"wait",  required_argument, 0, 'w'},
      {0, 0, 0, 0}
    };

    /* getopt_long stores the option index here. */
    int option_index = 0;

    int c = getopt_long(argc, argv, "s:r:c:w:hfd", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
      case 0:
        break;
      case 'd':
        opt_measure_speed = 1;
        break;
      case 'f':
        opt_do_fork = 1;
        break;
      case 's':
        opt_size_mb = atol(optarg);
        break;
      case 'r':
        opt_repeat_count = atol(optarg);
        break;
      case 'c':
        opt_pages_per_chunk = atol(optarg);
        break;
      case 'w':
        opt_wait_time = atol(optarg);
        break;
      case 'h':
      default:
        print_usage(argv[0]);
        exit(1);
    }
  }

  if (opt_size_mb == 0) {
    fprintf(stderr, "Buffer size cannot be zero\n");
    print_usage(argv[0]);
    exit(1);
  }
  printf("Test is configured as below:\n");
  printf("- Size of buffer: %zu MB\n", opt_size_mb);
  printf("- The test will sleep for %d second%s after accessing %d page%s\n",
         opt_wait_time, CHECK_PLURAL(opt_wait_time),
         opt_pages_per_chunk, CHECK_PLURAL(opt_pages_per_chunk));
  if (opt_measure_speed)
    printf("- Pages in the buffer will be accessed %d time%s\n",
           opt_repeat_count, CHECK_PLURAL(opt_repeat_count));
  if (opt_do_fork)
    printf("- The test will fork a child process to double the memory usage\n");

  // As a result, bytes will always be multiples of PAGE_SIZE
  size_t bytes = opt_size_mb * 1024 * 1024;
  size_t num_pages = bytes / PAGE_SIZE;

  char *buf = (char*) malloc(bytes);
  if (buf == NULL) {
    fprintf(stderr, "Buffer allocation failed\n");
    exit(1);
  }

  // First, populate the memory buffer
  initialize_pages(buf, bytes);

  // Measure the throughput (pages/sec)
  if (opt_measure_speed) {
    fflush(stdout);
    measure_zram_throughput(buf, bytes, num_pages, opt_pages_per_chunk,
                            opt_repeat_count, opt_wait_time, opt_do_fork);
  }
  else {
    // Otherwise the program will continuously run in the background.
    printf("%d waiting for kill\n", getpid());
    printf("consuming memory\n");
    size_t page_idx = 0;
    printf("Will touch pages covering %zd KB of data per %d second%s\n",
           (size_t) opt_pages_per_chunk * (PAGE_SIZE / 1024), opt_wait_time,
           CHECK_PLURAL(opt_wait_time));
    fflush(stdout);
    // Access the specified page chunks then wait. Repeat forever.
    if (opt_pages_per_chunk) {
      while (1) {
        // Bring in PAGE_ACCESS_MB_PER_SEC of data every second
        page_idx = walk_through_pages(buf, page_idx, opt_pages_per_chunk,
                                      num_pages, opt_wait_time, NULL);
      }
    }
    // If opt_pages_per_chunk is 0, just hold on to the pages without accessing
    // them after allocation.
    else {
      pause();
    }
  }
  return 0;
}
