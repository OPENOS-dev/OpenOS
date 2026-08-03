// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package linux

import (
	"context"
	"fmt"
	"regexp"
	"strings"

	"go.chromium.org/chromiumos/platform/btpeerd/core/exec"
)

// EthernetMacAddress runs "cat /sys/class/net/<networkInterfaceName>/address"
// and returns the output, which should be the mac address of the interface.
func EthernetMacAddress(ctx context.Context, runner exec.CmdRunner, networkInterfaceName string) (string, error) {
	addressFilePath := fmt.Sprintf("/sys/class/net/%s/address", networkInterfaceName)
	addressFileContents, err := runner.Output(ctx, exec.DefaultTimeout, "cat", addressFilePath)
	if err != nil {
		return "", fmt.Errorf("ethernet mac address: %w", err)
	}
	macAddress := strings.TrimSpace(string(addressFileContents))
	if macAddress == "" {
		return "", fmt.Errorf("ethernet mac address: no mac address found in file %q", addressFilePath)
	}
	return macAddress, nil
}

// IPv4Address runs "ifconfig <networkInterfaceName>" and returns the first IPv4
// address found in the output.
func IPv4Address(ctx context.Context, runner exec.CmdRunner, networkInterfaceName string) (string, error) {
	ifconfigStdout, err := runner.Output(ctx, exec.DefaultTimeout, "ifconfig", networkInterfaceName)
	if err != nil {
		return "", fmt.Errorf("ipv4 address: %w", err)
	}
	matcher := regexp.MustCompile(`inet (\d+\.\d+\.\d+\.\d+)`)
	match := matcher.FindStringSubmatch(string(ifconfigStdout))
	if len(match) != 2 {
		return "", fmt.Errorf("ipv4 address: failed parse ipv4 address from ifconfig output %q", string(ifconfigStdout))
	}
	return match[1], nil
}
