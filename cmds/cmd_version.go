package cmds

import (
	"fmt"
	"path/filepath"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/logger"

	"github.com/spf13/cobra"
)

type versionCmd struct {
	celer *configs.Celer
	short bool
}

func (v *versionCmd) Command(celer *configs.Celer) *cobra.Command {
	v.celer = celer
	command := &cobra.Command{
		Use:   "version",
		Short: "Show version info of celer.",
		Long: `Show version info of celer.

Use --short to print nothing but the version number, which is what
scripts and CI jobs compare against 'celer_version' in a schedule.`,
		Args: cobra.NoArgs,
		Run: func(cmd *cobra.Command, args []string) {
			v.version()
		},
	}

	command.Flags().BoolVarP(&v.short, "short", "s", false, "print only the version number")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (v *versionCmd) version() {
	if v.short {
		fmt.Println(v.celer.Version())
		return
	}

	toolchainPath, _ := filepath.Abs("toolchain_file.cmake")
	toolchainPath = logger.Sprintf(logger.Important, "%s", toolchainPath)

	content := fmt.Sprintf("Welcome to Celer (%s)\n"+
		"--------------------------------------------\n"+
		"This is a lightweight pkg-manager for C/C++.\n\n"+
		"How to apply it in your cmake project: \n"+
		"option1: %s\n"+
		"option2: %s\n\n",
		v.celer.Version(),
		logger.Sprintf(logger.Title, "set(CMAKE_TOOLCHAIN_FILE \"%s\")", toolchainPath),
		logger.Sprintf(logger.Title, "cmake .. -DCMAKE_TOOLCHAIN_FILE=\"%s\"", toolchainPath),
	)
	fmt.Print(content)
}
