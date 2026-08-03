// Copyright 2024 The ChromiumOS Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package raspberrypi

import (
	"context"
	"testing"

	"go.chromium.org/chromiumos/platform/btpeerd/core/exec"
	"go.chromium.org/chromiumos/platform/btpeerd/core/exec/mock"
)

func TestOSVersion(t *testing.T) {
	type args struct {
		osReleaseFileContents string
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
				osReleaseFileContents: "",
			},
			"",
			true,
		},
		{
			"valid",
			args{
				osReleaseFileContents: `PRETTY_NAME="Raspbian GNU/Linux 10 (buster)"
NAME="Raspbian GNU/Linux"
VERSION_ID="10"
VERSION="10 (buster)"
VERSION_CODENAME=buster
ID=raspbian
ID_LIKE=debian
HOME_URL="http://www.raspbian.org/"
SUPPORT_URL="http://www.raspbian.org/RaspbianForums"
BUG_REPORT_URL="http://www.raspbian.org/RaspbianBugs"

`,
			},
			"Raspbian GNU/Linux 10 (buster)",
			false,
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			mockRunner := mock.NewCmdRunner(&exec.RunResult{Stdout: []byte(tt.args.osReleaseFileContents)}, nil)
			got, err := OSVersion(context.Background(), mockRunner)
			if (err != nil) != tt.wantErr {
				t.Errorf("OSVersion() error = %v, wantErr %v", err, tt.wantErr)
				return
			}
			if got != tt.want {
				t.Errorf("OSVersion() got = %v, want %v", got, tt.want)
			}
		})
	}
}
