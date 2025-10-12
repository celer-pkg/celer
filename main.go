package main

import (
	"celer/cmds"
	"celer/envs"
	"celer/pkgs/color"
	"os"
)

func main() {
	// Remove uncessary envs.
	envs.CleanEnv()

	// Execute command.
	if err := cmds.Execute(); err != nil {
		color.Printf(color.Red, "failed to execute command:\n %s.\n", err)
		os.Exit(1)
	}
}
