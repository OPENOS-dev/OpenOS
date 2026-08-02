// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package core

import (
	"context"
	"fmt"
	"path"
	"strings"

	"go.chromium.org/chromiumos/platform/btpeerd/core/exec"
)

// installDir is where btpeerd is installed on the btpeer.
const installDir = "/etc/chromiumos/src/platform/btpeerd"

func BtpeerdCommit(ctx context.Context, runner exec.CmdRunner) (string, error) {
	commitFilePath := path.Join(installDir, "/COMMIT")
	commitFileContents, err := runner.Output(ctx, exec.DefaultTimeout, "cat", commitFilePath)
	if err != nil {
		return "", fmt.Errorf("btpeerd commit: failed to read btpeerd commit file at %q: %w", commitFilePath, err)
	}
	return strings.TrimSpace(string(commitFileContents)), nil
}
