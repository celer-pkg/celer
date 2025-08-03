package main

import (
	"celer/cmds"
	"celer/envs"
	"celer/pkgs/color"
)

func main() {
	envs.CleanEnv()

	if err := cmds.Execute(); err != nil {
		color.Println(color.Red, err.Error())
	}
}
