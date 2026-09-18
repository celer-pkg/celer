package main

import (
	"errors"
	"os"

	"github.com/celer-pkg/celer/cmds"
	"github.com/celer-pkg/celer/pkgs/logger"
)

func main() {
	if err := cmds.Execute(); err != nil {
		if !errors.Is(err, logger.ErrSilent) {
			logger.PrintError(err, "failed to execute command, run 'celer help' for usage information.")
		}
		os.Exit(1)
	}
}
