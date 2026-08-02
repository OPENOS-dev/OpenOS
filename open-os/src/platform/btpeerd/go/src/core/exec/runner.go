// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package exec

import (
	"bytes"
	"context"
	"fmt"
	"log/slog"
	"os/exec"
	"time"
)

// DefaultTimeout can be used as the timeout for most quick command runs.
const DefaultTimeout = 10 * time.Second

// RunResult provides results from a CmdRunner run.
type RunResult struct {
	// Command is the full command that was run to produce this result.
	Command string

	// Stdout is the stdout produced by running the command.
	Stdout []byte

	// Stderr is the stderr produced by running the command.
	Stderr []byte

	// ExitCode is the exit code the command returned.
	ExitCode int
}

// String returns the RunResult as a string for logging purposes.
func (r *RunResult) String() string {
	return fmt.Sprintf(
		"RunResult{Command=%q ExitCode=%d Stdout=%q Stderr=%q}",
		r.Command,
		r.ExitCode,
		string(r.Stdout),
		string(r.Stderr),
	)
}

// LogValue implements implementing slog.LogValuer.
func (r *RunResult) LogValue() slog.Value {
	return slog.StringValue(r.String())
}

// CmdRunner is an interface for running commands.
type CmdRunner interface {
	// Run will run the command with the specified name and optional arguments.
	//
	// Returns when the run is complete or the timeout is reached.
	// Returns a non-nil error if the run was unsuccessful.
	Run(ctx context.Context, timeout time.Duration, name string, args ...string) error

	// Output will run the command with the specified name and optional arguments
	// and then return the output of the command.
	//
	// Returns when the run is complete or the timeout is reached.
	// Returns a non-nil error if the run was unsuccessful.
	Output(ctx context.Context, timeout time.Duration, name string, args ...string) ([]byte, error)

	// RunForResult will run the command with the specified name and optional
	// arguments and then return the RunResult of the command for further
	// evaluation by the user.
	//
	// Returns when the run is complete or the timeout is reached.
	// Returns a non-nil error if the command failed to complete its execution.
	// Does not return a non-nil error for non-zero exit codes.
	RunForResult(ctx context.Context, timeout time.Duration, name string, args ...string) (*RunResult, error)
}

// SystemCmdRunner implements CmdRunner and will run commands on the local system.
type SystemCmdRunner struct {
}

func (r *SystemCmdRunner) Run(ctx context.Context, timeout time.Duration, name string, args ...string) error {
	_, err := r.Output(ctx, timeout, name, args...)
	return err
}

func (r *SystemCmdRunner) Output(ctx context.Context, timeout time.Duration, name string, args ...string) ([]byte, error) {
	result, err := r.RunForResult(ctx, timeout, name, args...)
	if err != nil {
		return result.Stdout, err
	}
	if result.ExitCode != 0 {
		return nil, fmt.Errorf(
			"failed to run command %q: command returned non-zero exit code %d: %s",
			result.Command,
			result.ExitCode,
			result,
		)
	}
	return result.Stdout, nil
}

func (r *SystemCmdRunner) RunForResult(ctx context.Context, timeout time.Duration, name string, args ...string) (*RunResult, error) {
	ctx, cancel := context.WithTimeout(ctx, timeout)
	defer cancel()
	cmd := exec.CommandContext(ctx, name, args...)
	result := &RunResult{
		Command:  cmd.String(),
		ExitCode: -1,
	}
	stderrBuffer := bytes.Buffer{}
	cmd.Stderr = &stderrBuffer
	slog.Info("Running system command", "cmd", result.Command)
	stdout, runErr := cmd.Output()
	result.Stdout = stdout
	result.Stderr = stderrBuffer.Bytes()
	if runErr != nil {
		if exitErr, ok := runErr.(*exec.ExitError); ok {
			// Normal exit error, extract exit code.
			result.ExitCode = exitErr.ExitCode()
			slog.Info("Successfully ran system command", "result", result)
			return result, nil
		}
		// Abnormal exit error or timout reached.
		slog.Warn("Failed to run system command", "result", result, "err", runErr)
		return result, fmt.Errorf("failed to run command %s: %w", result, runErr)
	}
	// Successful execution, assume zero exit code.
	result.ExitCode = 0
	slog.Info("Successfully ran system command", "result", result)
	return result, nil
}
