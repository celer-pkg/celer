package cmds

import (
	"os"
	"strings"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/internal/dirs"
	"github.com/celer-pkg/celer/internal/fileio"
	"github.com/spf13/cobra"
)

func boolCompletion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	return []string{"true", "false"}, cobra.ShellCompDirectiveNoFileComp
}

func buildTypeCompletion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	return []string{"Release", "Debug", "RelWithDebInfo", "MinSizeRel"}, cobra.ShellCompDirectiveNoFileComp
}

// projectNamesCompletion returns the project names under conf/projects, i.e. the
// directories that contain a project.toml.
func projectNamesCompletion(toComplete string) []string {
	if !fileio.PathExists(dirs.ConfProjectsDir) {
		return nil
	}

	entities, err := os.ReadDir(dirs.ConfProjectsDir)
	if err != nil {
		return nil
	}

	var projectNames []string
	for _, entity := range entities {
		if !entity.IsDir() || !strings.HasPrefix(entity.Name(), toComplete) {
			continue
		}
		if fileio.PathExists(dirs.GetProjectPath(entity.Name())) {
			projectNames = append(projectNames, entity.Name())
		}
	}
	return projectNames
}

func projectCompletgion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	return projectNamesCompletion(toComplete), cobra.ShellCompDirectiveNoFileComp
}

func platformCompletgion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	// "native" is reserved: the toolchain of this machine, there is no platform file.
	var suggestions []string
	if strings.HasPrefix(configs.NativePlatform, toComplete) {
		suggestions = append(suggestions, configs.NativePlatform)
	}

	var fileNames []string
	if !fileio.PathExists(dirs.ConfPlatformsDir) {
		return suggestions, cobra.ShellCompDirectiveNoFileComp
	}

	// The file with suffix ".toml" would be the platform name.
	entities, err := os.ReadDir(dirs.ConfPlatformsDir)
	if err != nil {
		return suggestions, cobra.ShellCompDirectiveNoFileComp
	}
	for _, entity := range entities {
		if !entity.IsDir() && strings.HasSuffix(entity.Name(), ".toml") {
			fileName := strings.TrimSuffix(entity.Name(), ".toml")
			if strings.HasPrefix(fileName, toComplete) {
				fileNames = append(fileNames, fileName)
			}
		}
	}

	return append(suggestions, fileNames...), cobra.ShellCompDirectiveNoFileComp
}
