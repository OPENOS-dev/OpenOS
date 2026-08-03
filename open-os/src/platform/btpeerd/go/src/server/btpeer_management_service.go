// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package server

import (
	"context"
	"fmt"

	"go.chromium.org/chromiumos/config/go/test/lab/api/btpeerd"
	"go.chromium.org/chromiumos/platform/btpeerd/bluetooth/chameleond"
	"go.chromium.org/chromiumos/platform/btpeerd/core"
	"go.chromium.org/chromiumos/platform/btpeerd/core/exec"
	"go.chromium.org/chromiumos/platform/btpeerd/core/linux"
	"go.chromium.org/chromiumos/platform/btpeerd/core/raspberrypi"
)

type BtpeerManagementServiceServer struct {
	runner exec.CmdRunner
}

func NewBtpeerManagementServiceServer(runner exec.CmdRunner) *BtpeerManagementServiceServer {
	return &BtpeerManagementServiceServer{
		runner: runner,
	}
}

func (s *BtpeerManagementServiceServer) DeviceInfo(ctx context.Context, request *btpeerd.DeviceInfoRequest) (*btpeerd.DeviceInfoResponse, error) {
	resp := &btpeerd.DeviceInfoResponse{}

	ethMac, err := linux.EthernetMacAddress(ctx, s.runner, raspberrypi.EthernetPortInterface)
	if err != nil {
		return nil, fmt.Errorf("device info: failed to get device ethernet mac address: %w", err)
	}
	resp.MacEth0 = ethMac

	addr, err := linux.IPv4Address(ctx, s.runner, raspberrypi.EthernetPortInterface)
	if err != nil {
		return nil, fmt.Errorf("device info: failed to get device ethernet ipv4 address: %w", err)
	}
	resp.Ipv4Address = addr

	osVersion, err := raspberrypi.OSVersion(ctx, s.runner)
	if err != nil {
		return nil, fmt.Errorf("device info: failed to get os version: %w", err)
	}
	resp.OsVersion = osVersion

	chameleondCommit, err := chameleond.Commit(ctx, s.runner)
	if err != nil {
		return nil, fmt.Errorf("device info: failed to get chameleond commit: %w", err)
	}
	resp.ChameleondCommit = chameleondCommit

	bluezVersion, err := linux.PackageVersion(ctx, s.runner, "bluez")
	if err != nil {
		return nil, fmt.Errorf("device info: failed to get bluez version: %w", err)
	}
	resp.BluezVersion = bluezVersion

	btpeerdCommit, err := core.BtpeerdCommit(ctx, s.runner)
	if err != nil {
		return nil, fmt.Errorf("device info: failed to get btpeerd commit: %w", err)
	}
	resp.BtpeerdCommit = btpeerdCommit

	model, err := raspberrypi.ModelName(ctx, s.runner)
	if err != nil {
		return nil, fmt.Errorf("device info: failed to get model: %w", err)
	}
	resp.Model = model

	return resp, nil
}

func (s *BtpeerManagementServiceServer) DeviceStatus(ctx context.Context, request *btpeerd.DeviceStatusRequest) (*btpeerd.DeviceStatusResponse, error) {
	//TODO implement me
	panic("implement me")
}

func (s *BtpeerManagementServiceServer) Reboot(ctx context.Context, request *btpeerd.RebootRequest) (*btpeerd.RebootResponse, error) {
	//TODO implement me
	panic("implement me")
}

func (s *BtpeerManagementServiceServer) GetActiveBluetoothStack(ctx context.Context, request *btpeerd.GetActiveBluetoothStackRequest) (*btpeerd.GetActiveBluetoothStackResponse, error) {
	//TODO implement me
	panic("implement me")
}

func (s *BtpeerManagementServiceServer) SetActiveBluetoothStack(ctx context.Context, request *btpeerd.SetActiveBluetoothStackRequest) (*btpeerd.SetActiveBluetoothStackResponse, error) {
	//TODO implement me
	panic("implement me")
}

func (s *BtpeerManagementServiceServer) GetActiveBluetoothStackAPI(ctx context.Context, request *btpeerd.GetActiveBluetoothStackAPIRequest) (*btpeerd.GetActiveBluetoothStackAPIResponse, error) {
	//TODO implement me
	panic("implement me")
}

func (s *BtpeerManagementServiceServer) SetActiveBluetoothStackAPI(ctx context.Context, request *btpeerd.SetActiveBluetoothStackAPIRequest) (*btpeerd.SetActiveBluetoothStackAPIResponse, error) {
	//TODO implement me
	panic("implement me")
}
