// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Package main includes the main function for running service as an executable.
package main

import (
	"context"
	"fmt"
	"log/slog"
	"os"
	"os/signal"
	"strconv"
	"strings"

	"go.chromium.org/chromiumos/platform/btpeerd/server"
)

const defaultGRPCServerPort = 8100
const defaultLogLevel = slog.LevelInfo

// runArgs are runtime arguments that can be set with positional arguments in
// the format <key>=<value> and is case-insensitive.
type runArgs struct {
	// ListenPort is the port the gRPC server shall listen on (ListenPort=8100).
	ListenPort int

	// LogLevel is the level for the logger (LogLevel=INFO).
	LogLevel slog.Level
}

func main() {
	parsedArgs, err := parseRunArgs()
	if err != nil {
		fmt.Printf("Failed to parse run args: %v\n", err)
		os.Exit(1)
	}
	configureDefaultLogger(parsedArgs.LogLevel)
	slog.Info("Starting btpeerd", "runArgs", parsedArgs)
	// Create context that will cancel when a SIGINT signal is received.
	ctx, cancel := context.WithCancel(context.Background())
	interruptSignalChannel := make(chan os.Signal, 1)
	signal.Notify(interruptSignalChannel, os.Interrupt)
	defer func() {
		signal.Stop(interruptSignalChannel)
		cancel()
	}()
	go func() {
		select {
		case <-interruptSignalChannel:
			slog.Warn("Received SIGINT, shutting down server")
			cancel()
		case <-ctx.Done():
		}
	}()

	// Start the gRPC server.
	if err := server.RunGRPCServer(ctx, parsedArgs.ListenPort); err != nil {
		slog.Error("Failed to run gRPC server", "err", err)
	}
}

// parseRunArgs parses simple CLI run arguments in the format <key>=<value>.
func parseRunArgs() (*runArgs, error) {
	parsedArgs := &runArgs{
		ListenPort: defaultGRPCServerPort,
		LogLevel:   defaultLogLevel,
	}
	for _, arg := range os.Args[1:] {
		argParts := strings.Split(arg, "=")
		if len(argParts) != 2 {
			return nil, fmt.Errorf("invalid argument %q, must be in format <key>=<value>", arg)
		}
		argKey := strings.ToLower(argParts[0])
		argKey = strings.TrimPrefix(argKey, "--")
		argValue := argParts[1]
		switch argKey {
		case "listenport":
			port, err := strconv.Atoi(argValue)
			if err != nil {
				return nil, fmt.Errorf("invalid argument %q: value %q is not an integer", argKey, argValue)
			}
			parsedArgs.ListenPort = port
		case "loglevel":
			argValue = strings.ToUpper(argValue)
			switch argValue {
			case "DEBUG":
				parsedArgs.LogLevel = slog.LevelDebug
			case "INFO":
				parsedArgs.LogLevel = slog.LevelInfo
			case "WARN":
				parsedArgs.LogLevel = slog.LevelWarn
			case "ERROR":
				parsedArgs.LogLevel = slog.LevelError
			default:
				return nil, fmt.Errorf("invalid argument %q: value %q is not one of the supported log levels (DEBUG, INFO, WARN, or ERROR)", argKey, argValue)
			}
		}
	}
	return parsedArgs, nil
}

func configureDefaultLogger(logLevel slog.Level) {
	handler := slog.NewTextHandler(os.Stdout, &slog.HandlerOptions{
		Level: logLevel,
	})
	slog.SetDefault(slog.New(handler))
}
