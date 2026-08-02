// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package mock

import (
	"context"
	"time"

	"go.chromium.org/chromiumos/platform/btpeerd/core/exec"
)

// CmdRunner mocks exec.CmdRunner for use in unit tests.
type CmdRunner struct {
	mockResult *exec.RunResult
	mockErr    error
}

// NewCmdRunner creates a new CmdRunner with the provided mock results and err.
func NewCmdRunner(mockResult *exec.RunResult, mockError error) exec.CmdRunner {
	return &CmdRunner{
		mockResult: mockResult,
		mockErr:    mockError,
	}
}

// Run simply returns the mock error.
func (m CmdRunner) Run(ctx context.Context, timeout time.Duration, name string, args ...string) error {
	return m.mockErr
}

// Output returns the stdout of the mock result and the mock error.
func (m CmdRunner) Output(ctx context.Context, timeout time.Duration, name string, args ...string) ([]byte, error) {
	return m.mockResult.Stdout, m.mockErr
}

// RunForResult returns the mock result and mock error.
func (m CmdRunner) RunForResult(ctx context.Context, timeout time.Duration, name string, args ...string) (*exec.RunResult, error) {
	return m.mockResult, m.mockErr
}
