package main

import (
	"celer/cmds"
	"celer/pkgs/color"
	"os"
)

func main() {
	if err := cmds.Execute(); err != nil {
		color.Printf(color.Red, "failed to execute command:\n %s.\n", err)
		os.Exit(1)
	}
}
