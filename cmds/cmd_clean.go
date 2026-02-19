package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/errors"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"os"
	"path/filepath"
	"slices"
	"strings"

	"github.com/spf13/cobra"
)

type cleanCmd struct {
	celer     *configs.Celer
	recursive bool
	dev       bool
	all       bool
	cleaned   []string
}

func (c *cleanCmd) Command(celer *configs.Celer) *cobra.Command {
	c.celer = celer
	command := &cobra.Command{
		Use:   "clean",
		Short: "Remove build cache and clean source repository.",
		Long: `Remove build cache and clean source repository.

This command removes build artifacts and caches for specified packages or
projects. It can optionally clean dependencies recursively and handle both
release and development builds.

Examples:
  celer clean x264@stable                           	# Clean specific package
  celer clean my-project                            	# Clean project
  celer clean x264@stable --dev                     	# Clean dev build cache
  celer clean automake@1.18 --recursive              	# Clean with dependencies
  celer clean --all                                 	# Clean all packages
  celer clean x264@stable ffmpeg@3.4.13 --recursive   	# Clean multiple packages`,
		RunE: func(cmd *cobra.Command, args []string) error {
			return c.execute(args)
		},
		ValidArgsFunction: c.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&c.recursive, "recursive", "r", false, "clean package/project along with its depedencies.")
	command.Flags().BoolVarP(&c.dev, "dev", "d", false, "clean package/project for dev mode.")
	command.Flags().BoolVarP(&c.all, "all", "a", false, "clean all packages.")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (c *cleanCmd) execute(args []string) error {
	if err := buildtools.CheckTools(c.celer, "git"); err != nil {
		return err
	}

	if err := c.celer.Init(); err != nil {
		return configs.PrintError(err, "failed to init celer.")
	}

	if c.all {
		if err := c.cleanAll(); err != nil {
			return configs.PrintError(err, "failed to clean all packages.")
		}
		configs.PrintSuccess("all packages cleaned.")
	} else {
		if err := c.validateTargets(args); err != nil {
			return configs.PrintError(err, "invalid arguments.")
		}

		if err := c.clean(args...); err != nil {
			return configs.PrintError(err, "failed to clean %s.", strings.Join(args, ", "))
		}
	}

	return nil
}

func (c *cleanCmd) validateTargets(targets []string) error {
	if len(targets) == 0 {
		return errors.New("no package or project specified")
	}
	return nil
}

func (c *cleanCmd) clean(targets ...string) error {
	// git is required when clean port.
	if err := buildtools.CheckTools(c.celer, "git"); err != nil {
		return err
	}

	var summaries []string
	for _, target := range targets {
		if strings.Contains(target, "@") {
			// Init port.
			var port configs.Port
			port.DevDep = false
			if err := port.Init(c.celer, target); err != nil {
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
			if err := port.MatchedConfig.Clean(); err != nil {
				return err
			}
		} else {
			var project configs.Project
			if err := project.Init(c.celer, target); err != nil {
				return err
			}

			for _, nameVersion := range project.Ports {
				// Init port.
				var port configs.Port
				port.DevDep = false
				if err := port.Init(c.celer, nameVersion); err != nil {
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
				if err := port.MatchedConfig.Clean(); err != nil {
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

	var cleaned bool
	entities, err := os.ReadDir(dirs.BuildtreesDir)
	if err != nil {
		return err
	}

	for _, entity := range entities {
		cleaned = false
		nameVersion := entity.Name()
		buildDir := filepath.Join(dirs.BuildtreesDir, entity.Name())
		entities, err := os.ReadDir(buildDir)
		if err != nil {
			return err
		}

	leaveLoop: // Remove all except src.
		for _, entity := range entities {
			// Remove build dir and log files.
			if entity.Name() != "src" {
				if err := os.RemoveAll(filepath.Join(buildDir, entity.Name())); err != nil {
					return err
				}
			}

			// Clean repo.
			var port configs.Port
			if err := port.Init(c.celer, nameVersion); err != nil {
				if errors.Is(err, errors.ErrPortNotFound) {
					color.Printf(color.Warning, "[clean %s]: cannot find it in ports, clean is skipped.\n", port.NameVersion())
					break leaveLoop
				}
				return err
			}
			if err := port.MatchedConfig.Clean(); err != nil {
				return err
			}
			cleaned = true
		}

		if cleaned {
			color.Printf(color.Hint, "✔ %s\n", entity.Name())
		}
	}

	return nil
}

func (c *cleanCmd) doClean(port configs.Port) error {
	// Ignore already cleaned ports.
	if slices.Contains(c.cleaned, port.NameVersion()+expr.If(port.DevDep || port.HostDep, " [dev]", "")) {
		return nil
	}

	// Remove build cache for dev build or platform build.
	matchedConfig := port.MatchedConfig
	if port.DevDep || port.HostDep {
		devBuildDir := filepath.Join(filepath.Dir(matchedConfig.PortConfig.BuildDir), matchedConfig.PortConfig.HostName+"-dev")
		if err := os.RemoveAll(devBuildDir); err != nil {
			return err
		}
	} else {
		if err := os.RemoveAll(matchedConfig.PortConfig.BuildDir); err != nil {
			return err
		}
	}

	// Remove build logs current platform build.
	if err := port.RemoveLogs(); err != nil {
		return err
	}

	// Clean dependencies.
	if c.recursive {
		for _, nameVersion := range matchedConfig.Dependencies {
			var depPort configs.Port
			depPort.HostDep = port.DevDep || port.HostDep
			if err := depPort.Init(c.celer, nameVersion); err != nil {
				return err
			}

			if err := c.doClean(depPort); err != nil {
				return err
			}
		}

		for _, nameVersion := range matchedConfig.DevDependencies {
			// Same name, version as parent and they are booth build with native toolchain, so skip.
			if (port.DevDep || port.HostDep) && port.NameVersion() == nameVersion {
				continue
			}

			var devDepPort configs.Port
			devDepPort.DevDep = true
			devDepPort.HostDep = port.DevDep || port.HostDep
			if err := devDepPort.Init(c.celer, nameVersion); err != nil {
				return err
			}

			if err := c.doClean(devDepPort); err != nil {
				return err
			}
		}
	}

	c.cleaned = append(c.cleaned, port.NameVersion()+expr.If(port.DevDep || port.HostDep, " [dev]", ""))
	color.Printf(color.Hint, "✔ %-25s%s\n", port.NameVersion(), expr.If(port.DevDep || port.HostDep, " -- [dev]", ""))

	return nil
}

func (c *cleanCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	// Support port completion.
	if fileio.PathExists(dirs.BuildtreesDir) {
		entities, err := os.ReadDir(dirs.BuildtreesDir)
		if err == nil {
			for _, entity := range entities {
				if entity.IsDir() && strings.HasPrefix(entity.Name(), toComplete) {
					suggestions = append(suggestions, entity.Name())
				}
			}
		}
	}

	// Support project completion.
	if fileio.PathExists(dirs.ConfProjectsDir) {
		entities, err := os.ReadDir(dirs.ConfProjectsDir)
		if err == nil {
			for _, entity := range entities {
				if !entity.IsDir() && strings.HasSuffix(entity.Name(), ".toml") {
					fileName := strings.TrimSuffix(entity.Name(), ".toml")
					if strings.HasPrefix(fileName, toComplete) {
						suggestions = append(suggestions, fileName)
					}
				}
			}
		}
	}

	// Support flags completion.
	for _, flag := range []string{"--dev", "-d", "--recursive", "-r", "--all", "-a"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
