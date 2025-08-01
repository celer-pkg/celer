package main

import (
	"celer/cmds"
	"celer/configs"
	"celer/envs"
	"celer/pkgs/color"
)

func main() {
	// Init celer.
	celer := configs.NewCeler()
	if err := celer.Init(); err != nil {
		configs.PrintError(err, "failed to init celer.")
		return
	}

	envs.CleanEnv()

	if err := cmds.Execute(celer); err != nil {
		color.Println(color.Red, err.Error())
	}
}
