package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"errors"
	"os"
	"path/filepath"
	"slices"
	"strings"

	"github.com/spf13/cobra"
)

type cleanCmd struct {
	ctx     configs.Context
	recurse bool
	dev     bool
	all     bool
	cleaned []string
}

func (c cleanCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "clean",
		Short: "Clean build cache for package or project",
		Run: func(cmd *cobra.Command, args []string) {
			// Init celer.
			celer := configs.NewCeler()
			if err := celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				return
			}
			c.ctx = celer

			if c.all {
				if err := c.cleanAll(); err != nil {
					configs.PrintError(err, "failed to clean all packages.")
					return
				}
				configs.PrintSuccess("all packages cleaned.")
			} else {
				if len(args) == 0 {
					configs.PrintError(errors.New("no package or project specified"), "failed to collect port infos.")
					return
				}

				if err := c.clean(args); err != nil {
					configs.PrintError(err, "failed to clean %s.", strings.Join(args, ", "))
					return
				}
			}
		},
		ValidArgsFunction: c.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&c.recurse, "recurse", "r", false, "clean package/project along with its depedencies.")
	command.Flags().BoolVarP(&c.dev, "dev", "d", false, "clean package/project for dev mode.")
	command.Flags().BoolVarP(&c.all, "all", "a", false, "clean all packages.")
	return command
}

func (c *cleanCmd) clean(targets []string) error {
	// git is required when clean port.
	if err := buildtools.CheckTools("git"); err != nil {
		return err
	}

	var summaries []string
	for _, target := range targets {
		if strings.Contains(target, "@") {
			// Init port.
			var port configs.Port
			port.DevDep = false
			if err := port.Init(c.ctx, target, c.ctx.BuildType()); err != nil {
				return err
			}

			if c.dev {
				// Clean dev build cache.
				port.DevDep = true
				if err := c.doClean(port); err != nil {
					return err
				}
			} else {
				// Clean current platform build cache.
				if err := c.doClean(port); err != nil {
					return err
				}
			}

			// Clean source.
			if err := port.MatchedConfig.CleanRepo(); err != nil {
				return err
			}
		} else {
			var project configs.Project
			if err := project.Init(c.ctx, target); err != nil {
				return err
			}

			for _, nameVersion := range project.Ports {
				// Init port.
				var port configs.Port
				port.DevDep = false
				if err := port.Init(c.ctx, nameVersion, c.ctx.BuildType()); err != nil {
					return err
				}

				// Clean current platform build cache.
				if err := c.doClean(port); err != nil {
					return err
				}

				// Clean dev build cache.
				port.DevDep = true
				if err := c.doClean(port); err != nil {
					return err
				}

				// Clean repo.
				if err := port.MatchedConfig.CleanRepo(); err != nil {
					return err
				}
			}
		}

		summaries = append(summaries, target)
	}

	configs.PrintSuccess("clean %s successfully.", strings.Join(summaries, ", "))
	return nil
}

func (c *cleanCmd) cleanAll() error {
	if !fileio.PathExists(dirs.BuildtreesDir) {
		return nil
	}

	entities, err := os.ReadDir(dirs.BuildtreesDir)
	if err != nil {
		return err
	}

	for _, entity := range entities {
		nameVersion := entity.Name()
		buildDir := filepath.Join(dirs.BuildtreesDir, entity.Name())
		entities, err := os.ReadDir(buildDir)
		if err != nil {
			return err
		}

		// Remove all except src.
		for _, entity := range entities {
			if entity.Name() != "src" {
				if err := os.RemoveAll(filepath.Join(buildDir, entity.Name())); err != nil {
					return err
				}
			}

			// Clean repo.
			var port configs.Port
			if err := port.Init(c.ctx, nameVersion, c.ctx.BuildType()); err != nil {
				return err
			}
			if err := port.MatchedConfig.CleanRepo(); err != nil {
				return err
			}
		}

		color.Printf(color.Gray, "✔ %s\n", entity.Name())
	}

	return nil
}

func (c *cleanCmd) doClean(port configs.Port) error {
	if slices.Contains(c.cleaned, port.NameVersion()+expr.If(port.DevDep || port.Native, "@dev", "")) {
		return nil
	}

	// Remove build dir.
	matchedConfig := port.MatchedConfig
	if port.DevDep || port.Native {
		devBuildDir := filepath.Join(filepath.Dir(matchedConfig.PortConfig.BuildDir), matchedConfig.PortConfig.HostName+"-dev")
		if err := os.RemoveAll(devBuildDir); err != nil {
			return err
		}
	} else {
		if err := os.RemoveAll(matchedConfig.PortConfig.BuildDir); err != nil {
			return err
		}
	}

	// Remove platform and dev build logs.
	if err := port.RemoveLogs(); err != nil {
		return err
	}

	// Clean dependencies.
	if c.recurse {
		for _, nameVersion := range matchedConfig.Dependencies {
			var depPort configs.Port
			depPort.DevDep = port.DevDep
			depPort.Native = port.Native
			if err := depPort.Init(c.ctx, nameVersion, c.ctx.BuildType()); err != nil {
				return err
			}

			if err := c.doClean(depPort); err != nil {
				return err
			}
		}

		for _, nameVersion := range matchedConfig.DevDependencies {
			// Skip self.
			if port.DevDep && port.Native && port.NameVersion() == nameVersion {
				continue
			}

			var devDepPort configs.Port
			devDepPort.DevDep = true
			devDepPort.Native = true
			if err := devDepPort.Init(c.ctx, nameVersion, c.ctx.BuildType()); err != nil {
				return err
			}

			if err := c.doClean(devDepPort); err != nil {
				return err
			}
		}
	}

	c.cleaned = append(c.cleaned, port.NameVersion()+expr.If(port.DevDep || port.Native, "@dev", ""))
	color.Printf(color.Gray, "✔ %-25s%s\n", port.NameVersion(), expr.If(port.DevDep || port.Native, " -- [dev]", ""))

	return nil
}

func (c cleanCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	var buildtreesDir = dirs.BuildtreesDir

	// Support port completion.
	if fileio.PathExists(buildtreesDir) {
		entities, err := os.ReadDir(buildtreesDir)
		if err != nil {
			return suggestions, cobra.ShellCompDirectiveNoFileComp
		}

		for _, entity := range entities {
			if entity.IsDir() && strings.HasPrefix(entity.Name(), toComplete) {
				suggestions = append(suggestions, entity.Name())
			}
		}
	}

	// Support project completion.
	if fileio.PathExists(dirs.ConfProjectsDir) {
		entities, err := os.ReadDir(dirs.ConfProjectsDir)
		if err != nil {
			configs.PrintError(err, "failed to read %s: %s.\n", dirs.ConfProjectsDir, err)
			os.Exit(1)
		}

		for _, entity := range entities {
			if !entity.IsDir() && strings.HasSuffix(entity.Name(), ".toml") {
				fileName := strings.TrimSuffix(entity.Name(), ".toml")
				if strings.HasPrefix(fileName, toComplete) {
					suggestions = append(suggestions, fileName)
				}
			}
		}
	}

	// Support flags completion.
	for _, flag := range []string{"--dev", "-d", "--recurse", "-r", "--all", "-a"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
