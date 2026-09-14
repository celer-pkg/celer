package cmds

import (
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/color"

	"github.com/spf13/cobra"
)

type stripCmd struct {
	celer *configs.Celer
}

func (s *stripCmd) Command(celer *configs.Celer) *cobra.Command {
	s.celer = celer
	command := cobra.Command{
		Use:   "strip",
		Short: "Strip installed target files.",
		Long: `Build a runtime tree under workspace/stripped/<platform>/<project>/<buildType>/.

Keeps ELF/PE binaries and shared libraries (symbol-stripped), shell scripts,
configs, and other runtime data. Drops headers, static libraries, CMake/pkg-config
files, PDBs, and related build-only trees.

Requires toolchain.strip in the platform (e.g. GNU strip or llvm-strip on Windows).`,
		Args: cobra.NoArgs,
		RunE: func(cmd *cobra.Command, args []string) error {
			return s.strip()
		},
	}

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true

	return &command
}

func (s *stripCmd) strip() error {
	if err := s.celer.Init(); err != nil {
		return color.PrintError(err, "failed to initialize celer.")
	}

	if err := s.celer.Strip(); err != nil {
		return color.PrintError(err, "failed to strip installed files.")
	}

	color.PrintSuccess("stripped files written under workspace/stripped.")
	return nil
}
