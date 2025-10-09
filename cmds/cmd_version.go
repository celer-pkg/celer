package cmds

import (
	"celer/configs"
	"celer/pkgs/color"
	"fmt"
	"path/filepath"

	"github.com/spf13/cobra"
)

type versionCmd struct {
	celer *configs.Celer
}

func (v versionCmd) Command(celer *configs.Celer) *cobra.Command {
	v.celer = celer
	return &cobra.Command{
		Use:   "version",
		Short: "Version",
		Run: func(cmd *cobra.Command, args []string) {
			// Handler celer error inside.
			if v.celer.HandleError() {
				return
			}

			v.version()
		},
	}
}

func (v versionCmd) version() {
	toolchainPath, _ := filepath.Abs("toolchain_file.cmake")
	toolchainPath = color.Sprintf(color.Magenta, "%s", toolchainPath)

	content := fmt.Sprintf("\nWelcome to celer (%s).\n"+
		"--------------------------------------------\n"+
		"This is a lightweight pkg-manager for C/C++.\n\n"+
		"How to apply it in your cmake project: \n"+
		"option1: %s\n"+
		"option2: %s\n\n",
		v.celer.Version(),
		color.Sprintf(color.Blue, "set(CMAKE_TOOLCHAIN_FILE \"%s\")", toolchainPath),
		color.Sprintf(color.Blue, "cmake .. -DCMAKE_TOOLCHAIN_FILE=%s", toolchainPath),
	)
	fmt.Print(content)
}
