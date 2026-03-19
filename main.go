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
			color.PrintError(err, "error occurred when exec command.")
		}
		os.Exit(1)
	}
}
