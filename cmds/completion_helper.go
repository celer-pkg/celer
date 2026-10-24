package cmds

import (
	"os"
	"strings"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/spf13/cobra"
)

func boolCompletion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	return []string{"true", "false"}, cobra.ShellCompDirectiveNoFileComp
}

func buildTypeCompletion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	return []string{"Release", "Debug", "RelWithDebInfo", "MinSizeRel"}, cobra.ShellCompDirectiveNoFileComp
}

func projectCompletgion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	return tomlFileCompletion(dirs.ConfProjectsDir, toComplete)
}

func platformCompletgion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	// "native" is reserved: the toolchain of this machine, there is no platform file.
	var suggestions []string
	if strings.HasPrefix(configs.NativePlatform, toComplete) {
		suggestions = append(suggestions, configs.NativePlatform)
	}

	fileNames, _ := tomlFileCompletion(dirs.ConfPlatformsDir, toComplete)
	return append(suggestions, fileNames...), cobra.ShellCompDirectiveNoFileComp
}

func tomlFileCompletion(dir, toComplete string) ([]string, cobra.ShellCompDirective) {
	var fileNames []string
	if fileio.PathExists(dir) {
		entities, err := os.ReadDir(dir)
		if err != nil {
			return nil, cobra.ShellCompDirectiveNoFileComp
		}

		for _, entity := range entities {
			if !entity.IsDir() && strings.HasSuffix(entity.Name(), ".toml") {
				fileName := strings.TrimSuffix(entity.Name(), ".toml")
				if strings.HasPrefix(fileName, toComplete) {
					fileNames = append(fileNames, fileName)
				}
			}
		}

		return fileNames, cobra.ShellCompDirectiveNoFileComp
	}

	return nil, cobra.ShellCompDirectiveNoFileComp
}
