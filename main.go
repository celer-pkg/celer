package main

import (
	"celer/cmds"
	"celer/configs"
	"celer/pkgs/color"
	"errors"
	"os"
)

func main() {
	if err := cmds.Execute(); err != nil {
		if !errors.Is(err, configs.ErrSilent) {
			color.Printf(color.Error, "failed to execute command:\n %s.\n", err)
		}
		os.Exit(1)
	}
}
