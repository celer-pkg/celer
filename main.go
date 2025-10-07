package main

import (
	"celer/cmds"
	"celer/configs"
	"celer/envs"
	"celer/pkgs/color"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
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
		color.Println(color.Yellow, "\n================ WARNING: You're in offline mode currently! ================")
	}

	// Set proxy.
	if celer.Proxy != nil {
		git.ProxyAddress = celer.Proxy.Address
		git.ProxyPort = celer.Proxy.Port
		fileio.ProxyAddress = celer.Proxy.Address
		fileio.ProxyPort = celer.Proxy.Port
	}

	// Execute celer command.
	if err := cmds.Execute(celer); err != nil {
		color.Printf(color.Red, "Execute command error: %s.\n", err)
		os.Exit(1)
	}
}
