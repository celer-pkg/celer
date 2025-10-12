package cmds

import (
	"celer/configs"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"io/fs"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type dependCmd struct {
	celer *configs.Celer
	dev   bool
}

func (d dependCmd) Command(celer *configs.Celer) *cobra.Command {
	d.celer = celer
	command := &cobra.Command{
		Use:   "depend",
		Short: "Query the dependent libraries.",
		Args:  cobra.ExactArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			if err := d.celer.Init(); err != nil {
				configs.PrintError(err, "init celer error: %s.", err)
				os.Exit(1)
			}

			libraries, err := d.query(args[0])
			if err != nil {
				configs.PrintError(err, "failed to query dependent libraries.")
				return
			}

			color.Println(color.Cyan, "[Dependency Libraries]:")
			if len(libraries) > 0 {
				color.Println(color.Gray, strings.Join(libraries, "\n"))
			} else {
				color.Println(color.Red, "no dependent libraries found.")
			}
		},
		ValidArgsFunction: d.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&d.dev, "dev", "d", false, "query dependent libraries with dev mode.")
	return command
}

func (d dependCmd) query(target string) ([]string, error) {
	var libraries []string
	if fileio.PathExists(dirs.PortsDir) {
		if err := filepath.WalkDir(dirs.PortsDir, func(path string, entity fs.DirEntry, err error) error {
			if err != nil {
				return err
			}

			if !entity.IsDir() && entity.Name() == "port.toml" {
				libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
				libVersion := filepath.Base(filepath.Dir(path))
				nameVersion := libName + "@" + libVersion

				var port configs.Port
				if err := port.Init(d.celer, nameVersion, d.celer.BuildType()); err != nil {
					return err
				}

				if d.dev {
					for _, dependency := range port.MatchedConfig.DevDependencies {
						if dependency == target {
							libraries = append(libraries, nameVersion)
						}
					}
				} else {
					for _, dependency := range port.MatchedConfig.Dependencies {
						if dependency == target {
							libraries = append(libraries, nameVersion)
						}
					}
				}
			}

			return nil
		}); err != nil {
			return nil, err
		}
	}
	return libraries, nil
}

func (d dependCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
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

		// Support flags completion.
		for _, flag := range []string{"--dev", "-d"} {
			if strings.HasPrefix(flag, toComplete) {
				suggestions = append(suggestions, flag)
			}
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
