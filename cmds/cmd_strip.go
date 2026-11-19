package cmds

import (
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/logger"

	"github.com/spf13/cobra"
)

type StripCmd struct {
	Celer *configs.Celer
}

func (s *StripCmd) Command(celer *configs.Celer) *cobra.Command {
	s.Celer = celer
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

func (s *StripCmd) strip() error {
	if err := s.Celer.Init(); err != nil {
		return logger.PrintError(err, "failed to initialize celer.")
	}

	if err := s.Celer.Strip(); err != nil {
		return logger.PrintError(err, "failed to strip installed files.")
	}

	logger.PrintSuccess("stripped files written under workspace/stripped.")
	return nil
}
