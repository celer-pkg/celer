package cmds

import (
	"celer/configs"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"io/fs"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type searchCmd struct {
	celer *configs.Celer
}

func (s searchCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "search",
		Short: "Search matched ports.",
		Args:  cobra.ExactArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			s.celer = configs.NewCeler()
			if err := s.celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				return
			}

			libraries, err := s.search(args[0])
			if err != nil {
				configs.PrintError(err, "failed to search available ports.")
				return
			}

			color.Println(color.Cyan, "[Search result]:")
			if len(libraries) > 0 {
				color.Println(color.Gray, strings.Join(libraries, "\n"))
			} else {
				color.Println(color.Red, "no matched port found.")
			}
		},
		ValidArgsFunction: s.completion,
	}

	return command
}

func (s searchCmd) search(pattern string) ([]string, error) {
	var results []string
	if fileio.PathExists(dirs.PortsDir) {
		if err := filepath.WalkDir(dirs.PortsDir, func(path string, entity fs.DirEntry, err error) error {
			if err != nil {
				return err
			}

			if !entity.IsDir() && entity.Name() == "port.toml" {
				libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
				libVersion := filepath.Base(filepath.Dir(path))
				nameVersion := libName + "@" + libVersion

				switch {
				case !strings.Contains(pattern, "*"):
					if nameVersion == pattern {
						results = append(results, nameVersion)
					}

				case strings.HasPrefix(pattern, "*") && strings.Count(pattern, "*") == 1:
					if strings.HasSuffix(nameVersion, pattern[1:]) {
						results = append(results, nameVersion)
					}

				case strings.HasSuffix(pattern, "*") && strings.Count(pattern, "*") == 1:
					if strings.HasPrefix(nameVersion, pattern[:len(pattern)-1]) {
						results = append(results, nameVersion)
					}

				case strings.Count(pattern, "*") == 2 && strings.HasPrefix(pattern, "*") && strings.HasSuffix(pattern, "*"):
					content := pattern[1 : len(pattern)-1]
					if strings.Contains(nameVersion, content) {
						results = append(results, nameVersion)
					}

				default:
					return nil
				}
			}

			return nil
		}); err != nil {
			return nil, err
		}
	}
	return results, nil
}

func (s searchCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	if fileio.PathExists(dirs.PortsDir) {
		filepath.WalkDir(dirs.PortsDir, func(path string, d fs.DirEntry, err error) error {
			if err != nil {
				return err
			}

			if !d.IsDir() && d.Name() == "port.toml" {
				libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
				libVersion := filepath.Base(filepath.Dir(path))
				nameVersion := libName + "@" + libVersion

				if strings.HasPrefix(nameVersion, toComplete) {
					suggestions = append(suggestions, nameVersion)
				}
			}

			return nil
		})
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
