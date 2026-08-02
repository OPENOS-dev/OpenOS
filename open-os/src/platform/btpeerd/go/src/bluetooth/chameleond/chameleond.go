// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package chameleond

import (
	"context"
	"fmt"
	"path"
	"strings"

	"go.chromium.org/chromiumos/platform/btpeerd/core/exec"
)

// installDir is where the chameleond bundle, which is effectively a snapshot
// of the chameleon repository, is installed on custom btpeer images.
const installDir = "/etc/chromiumos/src/platform/chameleon"

func Commit(ctx context.Context, runner exec.CmdRunner) (string, error) {
	commitFilePath := path.Join(installDir, "/dist/commit")
	commitFileContents, err := runner.Output(ctx, exec.DefaultTimeout, "cat", commitFilePath)
	if err != nil {
		return "", fmt.Errorf("commit: failed to read chameleond commit file at %q: %w", commitFilePath, err)
	}
	return strings.TrimSpace(string(commitFileContents)), nil
}
