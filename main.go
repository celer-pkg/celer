package main

import (
	"celer/cmds"
	"celer/pkgs/color"
	"errors"
	"os"
)

func main() {
	if err := cmds.Execute(); err != nil {
		if !errors.Is(err, color.ErrSilent) {
			color.Printf(color.Error, "failed to execute command:\n %s.\n", err)
		}
		os.Exit(1)
	}
}
