// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package linux

import (
	"context"
	"fmt"
	"strings"

	"go.chromium.org/chromiumos/platform/btpeerd/core/exec"
)

// PackageVersion returns the version of system package installed on the device
// using the dpkg-query command.
func PackageVersion(ctx context.Context, runner exec.CmdRunner, packageName string) (string, error) {
	dpkgQueryStdout, err := runner.Output(ctx, exec.DefaultTimeout, "dpkg-query", "-Wf", "${Version}", packageName)
	if err != nil {
		return "", fmt.Errorf("package version: failed to get version of system package %q with dpkg-query: %w", packageName, err)
	}
	version := strings.TrimSpace(string(dpkgQueryStdout))
	if version == "" {
		return "", fmt.Errorf("package version: got empty version of system package %q with dpkg-query", packageName)
	}
	return version, nil
}
