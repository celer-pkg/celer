package main

import (
	"celer/cmds"
	"celer/configs"
	"celer/envs"
	"celer/pkgs/color"
	"os"
)

func main() {
	envs.CleanEnv()

	// Warn user if they're in offline mode.
	celer := configs.NewCeler()
	if err := celer.Init(); err != nil {
		color.Printf(color.Red, "Init celer error: %s.\n", err)
		os.Exit(1)
	} else if celer.Offline() {
		color.Println(color.Yellow, "\n================ Warning: currently you're in offline mode. ================")
	}

	// Execute celer command.
	if err := cmds.Execute(); err != nil {
		color.Printf(color.Red, "Execute command error: %s.\n", err)
		os.Exit(1)
	}
}
