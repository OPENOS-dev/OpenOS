// Copyright 2023 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <fstream>
#include <string>

// epoxy/gl.h needs to be included first:
// clang-format off
#include "epoxy/gl.h"
#include "GLFW/glfw3.h"
// clang-format on

// Amount to allocate, if not specified by the user.
#define DEFAULT_ALLOC_AMOUNT_MIB 1024

void error_callback(int error, const char *description) {
  fprintf(stderr, "Error: %s\n", description);
}

void help() {
  printf("Usage: gamemodeglalloc [options]\n"
         "\n"
         "Valid options:\n"
         "\n"
         "  --help:\n"
         "    Show this message.\n"
         "\n"
         "  --exit-immediately:\n"
         "    Exit as soon as the requested amount has been allocated.\n"
         "    Default behavior: keep looping until the window is closed.\n"
         "\n"
         "  --alloc-amount-mib=<amount>:\n"
         "    Allocate the specified amount of memory.  Default: %d MiB.\n"
         "\n"
         "  --report-file=<filename>:\n"
         "    If specified, write the amount allocated to the given file.\n"
         "\n",
         DEFAULT_ALLOC_AMOUNT_MIB);
}

int main(int argc, char *argv[]) {
  printf("gamemodeglalloc: Allocate graphics memory.\n\n");

  bool exit_immediately = false;

  // MiB to allocate; overridable with --alloc-amount-mib.
  uint64_t alloc_amount_mib = DEFAULT_ALLOC_AMOUNT_MIB;

  // If non-empty, log filename to report allocated amounts.
  std::string report_filename, report_tmp_filename;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "--help") {
      help();
      return EXIT_SUCCESS;
    } else if (arg == "--exit-immediately") {
      exit_immediately = true;
    } else if (arg.rfind("--alloc-amount-mib=", 0) == 0) {
      int requested_amount = atoi(arg.c_str() + arg.find("=") + 1);
      if (requested_amount <= 0) {
        printf("Invalid alloc-amount-mib value.\n\n");
        help();
        return EXIT_FAILURE;
      }
      alloc_amount_mib = requested_amount;
    } else if (arg.rfind("--report-file=", 0) == 0) {
      report_filename = arg.substr(arg.find("=") + 1);
      report_tmp_filename = report_filename + ".tmp";
    } else {
      printf("Unknown argument: %s\n\n", arg.c_str());
      help();
      return EXIT_FAILURE;
    }
  }

  if (!report_filename.empty()) {
    printf("Writing allocated amounts to file: %s\n", report_filename.c_str());
  }

  printf("%s after allocation is finished.\n",
         exit_immediately ? "Exiting immediately" : "Continuing to loop");
  printf("Allocating %lu MiB total.\n\n", alloc_amount_mib);

  glfwSetErrorCallback(error_callback);

  if (!glfwInit()) {
    printf("GLFW init failed.\n");
    return EXIT_FAILURE;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  GLFWwindow *window =
      glfwCreateWindow(100, 100, "gamemodeglalloc", NULL, NULL);
  if (!window) {
    printf("GLFW main window creation failed.\n");
    glfwTerminate();
    return EXIT_FAILURE;
  }

  printf("Make context current.\n");
  glfwMakeContextCurrent(window);

  printf("epoxy_gl_version: %d\n", epoxy_gl_version());
  printf("OpenGL version %s\n", glGetString(GL_VERSION));

  const uint64_t max_bytes_per_alloc = 4 * 1024 * 1024;
  uint8_t *src_data =
      reinterpret_cast<uint8_t *>(calloc(1, max_bytes_per_alloc));
  if (src_data == nullptr) {
    printf("Failed to allocate %lu bytes.\n", max_bytes_per_alloc);
    return EXIT_FAILURE;
  }
  uint64_t alloc_amount_bytes = alloc_amount_mib * 1024 * 1024;

  printf("Enter loop.\n\n");
  int n = 0;
  uint64_t amount_allocated = 0;
  while (!glfwWindowShouldClose(window)) {
    printf("Next frame.\n");
    fflush(stdout);
    glClear(GL_COLOR_BUFFER_BIT);
    fflush(stdout);
    glfwSwapBuffers(window);
    fflush(stdout);
    glfwPollEvents();

    if (amount_allocated < alloc_amount_bytes) {
      ++n;
      uint64_t amount_to_alloc_bytes =
          std::min(max_bytes_per_alloc, alloc_amount_bytes - amount_allocated);
      printf("Allocation #%d: %lu bytes\n", n, amount_to_alloc_bytes);
      fflush(stdout);

      GLuint buffer;
      glGenBuffers(1, &buffer);

      // b/263414738: As of 2023-05-26, when tested under Borealis on brya and
      // volteer devices, using mesa-iris on the host and GL on the guest (not
      // Zink), all of the following three bind/data calls allocate the
      // requested amount of graphics memory on the host, plus the same amount
      // of memory on the guest.  On the guest, /proc/meminfo shows the
      // allocated amount as Unevictable but not Mlocked.

      // glBindBuffer(GL_TEXTURE_BUFFER, buffer);
      // glBufferData(GL_TEXTURE_BUFFER, amount_to_alloc_bytes, src_data,
      //              GL_STATIC_DRAW);

      // glBindBuffer(GL_TEXTURE_BUFFER, buffer);
      // glBufferStorage(GL_TEXTURE_BUFFER, amount_to_alloc_bytes,
      //                 src_data, GL_MAP_WRITE_BIT);

      glBindBuffer(GL_ARRAY_BUFFER, buffer);
      glBufferData(GL_ARRAY_BUFFER, amount_to_alloc_bytes, src_data,
                   GL_DYNAMIC_DRAW);

      // Keep track of total allocated and update the report file.
      amount_allocated += amount_to_alloc_bytes;
      // This output line is used by the borealis.GraphicsAlloc tast
      // test; it must always contain "allocated: (\d+) bytes".
      printf("Total allocated: %" PRIu64 " bytes\n", amount_allocated);
      if (!report_filename.empty()) {
        // Write to temp file then rename, to avoid race conditions when
        // reading the report file in tests.
        {
          std::ofstream report_file;
          report_file.open(report_tmp_filename.c_str());
          if (report_file.fail()) {
            printf("Couldn't open %s\n", report_tmp_filename.c_str());
            return EXIT_FAILURE;
          }
          report_file << amount_allocated << std::endl;
          // report_file is closed when we fall out of scope.
        }
        if (rename(report_tmp_filename.c_str(), report_filename.c_str()) < 0) {
          printf("When renaming %s to %s, an error occurred.\n",
                 report_tmp_filename.c_str(), report_filename.c_str());
          perror("Renaming");
          return EXIT_FAILURE;
        }
      }

      // This deletes the buffer, and frees it on host and guest:
      // glDeleteBuffers(1, &buffer);
    } else if (exit_immediately) {
      // --exit-immediately specified: exit as soon as allocation is done.
      // This useful if you have Mesa compiled to dump stats on exit, and want
      // to see what it thought happened.

      // The tast borealis.GraphicsAlloc test expects to see this string;
      // don't change it.
      printf("\nSuccessfully allocated all requested memory; exiting!\n");
      break;
    }
  }

  printf("Terminate.\n");
  glfwTerminate();

  return EXIT_SUCCESS;
}
