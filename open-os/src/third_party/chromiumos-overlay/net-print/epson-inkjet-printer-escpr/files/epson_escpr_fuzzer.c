// Copyright 2025 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/param.h>
#include <unistd.h>

#include "stdin_util.h"
#include "wrapper.h"
#include <cups/ppd.h>
#include <cups/raster.h>

// This PPD file is a randomly selected PPD file that uses the
// epson-escpr-wrapper filter.
const char ppdFile[] = "/usr/share/cups/model/epson.ppd";

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  static cups_page_header2_t header;
  static bool once = true;
  if (once) {
    ppd_file_t *file = ppdOpenFile(ppdFile);
    if (!file) {
      fprintf(stderr, "ppdOpenFile() failed\n");
      abort();
    }
    if (cupsRasterInterpretPPD(&header, file, 0, NULL, NULL) < 0) {
      fprintf(stderr, "cupsRasterInterpretPPD() failed\n");
      ppdClose(file);
      abort();
    }
    ppdClose(file);
    header.HWResolution[0] = 360;
    header.HWResolution[1] = 360;
    once = false;
  }

  if (setenv("PPD", ppdFile, true)) {
    fprintf(stderr, "setenv failed: %s\n", strerror(errno));
    abort();
  }

  // Sets stdin to a memory-backed file.
  int error = fuzzer_set_stdin(NULL, 0);
  if (error) {
    fprintf(stderr, "set_stdin() failed: error code %d\n", error);
    abort();
  }

  // Determine the max page size based on our cups header.
  const size_t maxPageSize = header.cupsHeight * header.cupsBytesPerLine;

  // Grab some bytes from the fuzz data for the options to be passed into the
  // filter.
  size_t offset = 0;
  const size_t max_options_size = 64;
  const size_t options_size = MIN(size, max_options_size);
  char options[max_options_size + 1];
  strncpy(options, data, options_size);
  options[options_size] = '\0';
  size -= options_size;
  offset += options_size;

  // Writes the raster content to stdin for the filter to consume.
  // Stdin is a memory-backed file.
  cups_raster_t *raster = cupsRasterOpen(STDIN_FILENO, CUPS_RASTER_WRITE);
  while (size > 0) {
    // Don't write more than the max size specified by our header.
    const size_t numToWrite = MIN(size, maxPageSize);
    cupsRasterWriteHeader2(raster, &header);
    cupsRasterWritePixels(raster, (unsigned char *)(data + offset), numToWrite);
    size -= numToWrite;
    offset += numToWrite;
  }
  cupsRasterClose(raster);
  fuzzer_rewind_stdin();

  char *argv[] = {/*uri*/ "",         /*job id*/ "1",
                  /*user*/ "chronos", /*title*/ "Untitled",
                  /*copies*/ "1",     /*options*/ options};
  // This command may fail and return a non-zero error code
  // if the input raster header is malformed.
  EpsonEscprPrintJob(sizeof(argv) / sizeof(argv[0]), argv);

  return 0;
}
