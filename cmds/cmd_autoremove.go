package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"strings"

	"github.com/spf13/cobra"
)

type autoremoveCmd struct {
	celer       *configs.Celer
	packages    []string
	devPackages []string
	purge       bool
	buildCache  bool
}

func (a autoremoveCmd) Command(celer *configs.Celer) *cobra.Command {
	a.celer = celer
	command := &cobra.Command{
		Use:   "autoremove",
		Short: "Tidy up installation directory - removing project's unnecessary library files.",
		Run: func(cmd *cobra.Command, args []string) {
			if err := a.celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				os.Exit(1)
			}

			if err := a.autoremove(a.buildCache, a.purge); err != nil {
				configs.PrintError(err, "failed to autoremove.")
				os.Exit(1)
			}

			configs.PrintSuccess("autoremove successfully.")
		},
		ValidArgsFunction: a.completion,
	}

	command.Flags().BoolVarP(&a.buildCache, "build-cache", "c", false, "autoremove packages along with build cache.")
	command.Flags().BoolVarP(&a.purge, "purge", "p", false, "autoremove packages along with its package file.")

	return command
}

func (a *autoremoveCmd) autoremove(buildCache, purge bool) error {
	// Collect packages/devPackages that belongs to project.
	for _, nameVersion := range a.celer.Project().GetPorts() {
		if err := a.collectPackages(nameVersion); err != nil {
			return err
		}
		if err := a.collectDevPackages(nameVersion); err != nil {
			return err
		}
	}

	// Query installed packages.
	depPackages, devDepPackages, err := a.installedPackages()
	if err != nil {
		return err
	}

	// Remove packages that do not belongs to project.
	for _, nameVersion := range depPackages {
		// Skip if required by current project.
		if slices.Contains(a.packages, nameVersion) {
			continue
		}

		// Do remove package.
		var port configs.Port
		if err := port.Init(a.celer, nameVersion, a.celer.BuildType()); err != nil {
			return err
		}
		if err := port.Remove(false, purge, buildCache); err != nil {
			return err
		}
	}

	// Remove dev_packages not required by current project any more.
	for _, nameVersion := range devDepPackages {
		// Skip if required by current project.
		if slices.Contains(a.devPackages, nameVersion) {
			continue
		}

		// Do remove dev_package.
		var port configs.Port
		port.DevDep = true
		if err := port.Init(a.celer, nameVersion, a.celer.BuildType()); err != nil {
			return err
		}
		if err := port.Remove(false, purge, buildCache); err != nil {
			return err
		}
	}

	return nil
}

func (a *autoremoveCmd) collectPackages(nameVersion string) error {
	var port configs.Port
	if err := port.Init(a.celer, nameVersion, a.celer.BuildType()); err != nil {
		return err
	}

	// Add if not added before.
	if !slices.Contains(a.packages, nameVersion) {
		a.packages = append(a.packages, nameVersion)
	}

	for _, depNameVersion := range port.MatchedConfig.Dependencies {
		if !slices.Contains(a.packages, depNameVersion) {
			a.packages = append(a.packages, depNameVersion)
			if err := a.collectPackages(depNameVersion); err != nil {
				return err
			}
		}
	}

	return nil
}

func (a *autoremoveCmd) collectDevPackages(nameVersion string) error {
	var port configs.Port
	if err := port.Init(a.celer, nameVersion, a.celer.BuildType()); err != nil {
		return err
	}

	for _, devDepNameVersion := range port.MatchedConfig.DevDependencies {
		if !slices.Contains(a.devPackages, devDepNameVersion) {
			a.devPackages = append(a.devPackages, devDepNameVersion)
			if err := a.collectDevPackages(devDepNameVersion); err != nil {
				return err
			}
		}
	}

	return nil
}

func (a autoremoveCmd) installedPackages() (packages []string, devPackages []string, err error) {
	libraryFolder := fmt.Sprintf("%s@%s@%s", a.celer.Platform().GetName(),
		a.celer.Project().GetName(), strings.ToLower(a.celer.BuildType()))
	packages, err = a.readPackages(libraryFolder)
	if err != nil {
		return nil, nil, err
	}

	devLibraryFolder := a.celer.Platform().GetHostName() + "-dev"
	devPackages, err = a.readPackages(devLibraryFolder)
	if err != nil {
		return nil, nil, err
	}

	return
}

func (a autoremoveCmd) readPackages(libraryFolder string) ([]string, error) {
	traceDir := filepath.Join(dirs.InstalledDir, "celer", "trace")
	pattern := filepath.Join(traceDir, "*@"+libraryFolder+".trace")
	suffix := "@" + libraryFolder + ".trace"

	matches, err := filepath.Glob(pattern)
	if err != nil {
		return nil, err
	}

	var packages []string
	for _, match := range matches {
		if cutted, ok := strings.CutSuffix(filepath.Base(match), suffix); ok {
			packages = append(packages, cutted)
		}
	}

	return packages, nil
}

func (a autoremoveCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	// Support flags completion.
	for _, flag := range []string{"--build-cache", "-c", "--purge", "-p"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
