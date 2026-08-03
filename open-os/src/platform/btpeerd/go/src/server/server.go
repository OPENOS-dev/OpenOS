// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Package server provides functionality for running a gRPC server that serves
// the BtpeerManagementService.
package server

import (
	"context"
	"fmt"
	"log/slog"
	"net"

	"go.chromium.org/chromiumos/config/go/test/lab/api/btpeerd"
	"go.chromium.org/chromiumos/platform/btpeerd/core/exec"
	"google.golang.org/grpc"
)

// RunGRPCServer starts a gRPC server on the specified system port.
// The server is stopped if the context expires. Will only return if the server
// has stopped. Returns a non-nil error if the server stopped on its own.
func RunGRPCServer(ctx context.Context, port int) error {
	// Configure server.
	var serverOpts []grpc.ServerOption
	server := grpc.NewServer(serverOpts...)
	runner := &exec.SystemCmdRunner{}
	btpeerd.RegisterBtpeerManagementServiceServer(server, NewBtpeerManagementServiceServer(runner))

	// Start server, stopping it if the ctx expires.
	listenAddr := fmt.Sprintf("localhost:%d", port)
	slog.Info("Starting gRPC server", "listenAddr", listenAddr)
	lis, err := net.Listen("tcp", listenAddr)
	if err != nil {
		return fmt.Errorf("failed to listen on tcp port %d: %w", port, err)
	}
	slog.Debug("gRPC service info", "serviceInfo", server.GetServiceInfo())
	serverChannel := make(chan error, 1)
	go func() {
		serverChannel <- server.Serve(lis)
	}()
	select {
	case err := <-serverChannel:
		return fmt.Errorf("grpc server error: %w", err)
	case <-ctx.Done():
		slog.Info("Stopping gRPC server gracefully")
		server.GracefulStop()
		return nil
	}
}
