package cmds

import (
	"celer/configs"
	"celer/pkgs/color"
	"fmt"
	"path/filepath"

	"github.com/spf13/cobra"
)

type aboutCmd struct {
	celer *configs.Celer
}

func (a aboutCmd) Command() *cobra.Command {
	return &cobra.Command{
		Use:   "about",
		Short: "About celer.",
		Run: func(cmd *cobra.Command, args []string) {
			fmt.Print(a.about())
		},
	}
}

func (a aboutCmd) about() string {
	toolchainPath, _ := filepath.Abs("toolchain_file.cmake")
	toolchainPath = color.Sprintf(color.Magenta, "%s", toolchainPath)

	return fmt.Sprintf("\nWelcome to celer (%s).\n"+
		"---------------------------------------\n"+
		"This is a simple pkg-manager for C/C++.\n\n"+
		"How to use it to build cmake project: \n"+
		"option1: %s\n"+
		"option2: %s\n\n",
		a.celer.Version(),
		color.Sprintf(color.Blue, "set(CMAKE_TOOLCHAIN_FILE \"%s\")", toolchainPath),
		color.Sprintf(color.Blue, "cmake .. -DCMAKE_TOOLCHAIN_FILE=%s", toolchainPath),
	)
}
