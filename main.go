package main

import (
	"celer/cmds"
	"celer/configs"
	"celer/envs"
	"celer/pkgs/color"
)

func main() {
	envs.CleanEnv()

	celer := configs.NewCeler()
	if err := celer.Init(); err != nil {
		color.Println(color.Red, err.Error())
		return
	}

	if celer.Offline() {
		color.Println(color.Yellow, "\n================ Warning: currently you're in offline mode. ================")
	}

	if err := cmds.Execute(); err != nil {
		color.Println(color.Red, err.Error())
	}
}
