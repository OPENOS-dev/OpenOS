// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package linux

import (
	"context"
	"testing"

	"go.chromium.org/chromiumos/platform/btpeerd/core/exec"
	"go.chromium.org/chromiumos/platform/btpeerd/core/exec/mock"
)

func TestIPv4Address(t *testing.T) {
	type args struct {
		ifconfigStdout       string
		networkInterfaceName string
	}
	tests := []struct {
		name    string
		args    args
		want    string
		wantErr bool
	}{
		{
			"empty output",
			args{
				ifconfigStdout:       "",
				networkInterfaceName: "eth0",
			},
			"",
			true,
		},
		{
			"valid",
			args{
				ifconfigStdout: `eth0: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
inet 100.71.224.110  netmask 255.255.240.0  broadcast 100.71.239.255
inet6 fe80::dea6:32ff:fe86:b585  prefixlen 64  scopeid 0x20<link>
ether dc:a6:32:86:b5:85  txqueuelen 1000  (Ethernet)
RX packets 161260331  bytes 1558837028 (1.4 GiB)
RX errors 1049445  dropped 1049445  overruns 0  frame 0
TX packets 93333  bytes 40883200 (38.9 MiB)
TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

`,
				networkInterfaceName: "eth0",
			},
			"100.71.224.110",
			false,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			mockRunner := mock.NewCmdRunner(&exec.RunResult{Stdout: []byte(tt.args.ifconfigStdout)}, nil)
			got, err := IPv4Address(context.Background(), mockRunner, tt.args.networkInterfaceName)
			if (err != nil) != tt.wantErr {
				t.Errorf("IPv4Address() error = %v, wantErr %v", err, tt.wantErr)
				return
			}
			if got != tt.want {
				t.Errorf("IPv4Address() got = %v, want %v", got, tt.want)
			}
		})
	}
}
