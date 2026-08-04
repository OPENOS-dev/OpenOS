package skylab_local_state

import (
	"fmt"
	"strings"
)

// Checks that a MultiBotHostInfo is well-formed and complete
func (hi *MultiBotHostInfo) Validate() error {
	if hi == nil {
		return fmt.Errorf("nil message")
	}

	var missingArgs []string

	if hi.HostInfo == nil {
		missingArgs = append(missingArgs, "host info")
	}

	if hi.DutName == "" {
		missingArgs = append(missingArgs, "DUT hostname")
	}

	if len(missingArgs) > 0 {
		return fmt.Errorf("no %s provided", strings.Join(missingArgs, ", "))
	}

	return nil
}
