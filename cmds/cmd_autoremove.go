package cmds

import (
	"celer/configs"
	"celer/depcheck"
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

func (a *autoremoveCmd) Command(celer *configs.Celer) *cobra.Command {
	a.celer = celer
	command := &cobra.Command{
		Use:   "autoremove",
		Short: "Remove libraries that do not belong to current project.",
		Long: `Remove libraries that do not belong to current project.

This command scans installed runtime and buildtime packages, compares them
against the dependency graph required by the current project, and removes
packages that are no longer needed.

Use --purge to also remove cached package archives, and --build-cache to
remove build cache together with removed packages.

Examples:
  celer autoremove                      	# Remove unused installed packages
  celer autoremove --purge              	# Also remove package archives
  celer autoremove --build-cache        	# Also remove build cache
  celer autoremove --purge --build-cache	# Remove packages, archives, and build cache`,
		Args: cobra.NoArgs,
		RunE: func(cmd *cobra.Command, args []string) error {
			if err := a.celer.Init(); err != nil {
				return configs.PrintError(err, "failed to init celer.")
			}

			if err := a.autoremove(); err != nil {
				return configs.PrintError(err, "failed to autoremove.")
			}

			configs.PrintSuccess("autoremove successfully.")
			return nil
		},
		ValidArgsFunction: a.completion,
	}

	command.Flags().BoolVarP(&a.buildCache, "build-cache", "c", false, "autoremove packages along with build cache.")
	command.Flags().BoolVarP(&a.purge, "purge", "p", false, "autoremove packages along with its package file.")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (a *autoremoveCmd) autoremove() error {
	// Reset collected state for every run.
	a.packages = nil
	a.devPackages = nil

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
		if slices.Contains(a.packages, nameVersion) {
			continue
		}
		if err := a.removePackage(nameVersion, false); err != nil {
			return err
		}
	}

	// Remove dev_packages that do not belongs to current project.
	for _, nameVersion := range devDepPackages {
		if slices.Contains(a.devPackages, nameVersion) {
			continue
		}
		if err := a.removePackage(nameVersion, true); err != nil {
			return err
		}
	}

	return nil
}

func (a *autoremoveCmd) removePackage(nameVersion string, dev bool) error {
	var port configs.Port
	port.DevDep = dev
	port.HostDep = dev
	if err := port.Init(a.celer, nameVersion); err != nil {
		return err
	}

	options := configs.RemoveOptions{
		Purge:      a.purge,
		Recursive:  false,
		BuildCache: a.buildCache,
	}
	return port.Remove(options)
}

func (a *autoremoveCmd) collectPackages(nameVersion string) error {
	// Init port with name version.
	var port configs.Port
	if err := port.Init(a.celer, nameVersion); err != nil {
		return err
	}

	// Check circular dependence and version conflict.
	depcheck := depcheck.NewDepCheck()
	if err := depcheck.CheckCircular(a.celer, port); err != nil {
		return fmt.Errorf("found circular dependence %s \n%w", nameVersion, err)
	}
	if err := depcheck.CheckConflict(a.celer, port); err != nil {
		return fmt.Errorf("found version conflict %s \n%w", nameVersion, err)
	}

	// Add if not added before.
	if !slices.Contains(a.packages, nameVersion) {
		a.packages = append(a.packages, nameVersion)
	}

	for _, nameVersion := range port.MatchedConfig.Dependencies {
		if !slices.Contains(a.packages, nameVersion) {
			a.packages = append(a.packages, nameVersion)
			if err := a.collectPackages(nameVersion); err != nil {
				return err
			}
		}
	}

	return nil
}

func (a *autoremoveCmd) collectDevPackages(nameVersion string) error {
	// Init port with name version.
	var port configs.Port
	if err := port.Init(a.celer, nameVersion); err != nil {
		return err
	}

	// Check circular dependence and version conflict.
	depcheck := depcheck.NewDepCheck()
	if err := depcheck.CheckCircular(a.celer, port); err != nil {
		return fmt.Errorf("found circular dependence when collecting dev package %s -> %w", nameVersion, err)
	}
	if err := depcheck.CheckConflict(a.celer, port); err != nil {
		return fmt.Errorf("found version conflict when collecting dev package %s -> %w", nameVersion, err)
	}

	// Collect dev dependencies.
	for _, nameVersion := range port.MatchedConfig.DevDependencies {
		if !slices.Contains(a.devPackages, nameVersion) {
			a.devPackages = append(a.devPackages, nameVersion)
			if err := a.collectDevPackages(nameVersion); err != nil {
				return err
			}
		}
	}

	// Dev package chains may include regular dependencies as host dependencies.
	// They are also installed under host-dev and should not be removed.
	for _, nameVersion := range port.MatchedConfig.Dependencies {
		if !slices.Contains(a.devPackages, nameVersion) {
			a.devPackages = append(a.devPackages, nameVersion)
			if err := a.collectDevPackages(nameVersion); err != nil {
				return err
			}
		}
	}

	return nil
}

func (a *autoremoveCmd) installedPackages() (packages []string, devPackages []string, err error) {
	// Collect installed packages and cached packages.
	libraryFolder := fmt.Sprintf("%s@%s@%s",
		a.celer.Platform().GetName(),
		a.celer.Project().GetName(),
		a.celer.BuildType(),
	)
	packages, err = a.readInstalledPackages(libraryFolder)
	if err != nil {
		return nil, nil, err
	}
	cachedPackages, err := a.readCachedPackages(libraryFolder)
	if err != nil {
		return nil, nil, err
	}
	packages = mergePackages(packages, cachedPackages)

	// Collect installed dev packages and cached dev packages.
	devLibraryFolder := a.celer.Platform().GetHostName() + "-dev"
	devPackages, err = a.readInstalledPackages(devLibraryFolder)
	if err != nil {
		return nil, nil, err
	}
	cachedDevPackages, err := a.readCachedPackages(devLibraryFolder)
	if err != nil {
		return nil, nil, err
	}
	devPackages = mergePackages(devPackages, cachedDevPackages)
	return
}

func (a *autoremoveCmd) readInstalledPackages(libraryFolder string) ([]string, error) {
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

func (a *autoremoveCmd) readCachedPackages(libraryFolder string) ([]string, error) {
	entries, err := os.ReadDir(dirs.PackagesDir)
	if err != nil {
		if os.IsNotExist(err) {
			return nil, nil
		}
		return nil, err
	}

	suffix := "@" + libraryFolder
	var packages []string

	for _, entry := range entries {
		if !entry.IsDir() {
			continue
		}

		folder := entry.Name()
		if cutted, ok := strings.CutSuffix(folder, suffix); ok {
			packages = append(packages, cutted)
		}
	}

	return packages, nil
}

func mergePackages(base []string, extras []string) []string {
	for _, item := range extras {
		if !slices.Contains(base, item) {
			base = append(base, item)
		}
	}
	return base
}

func (a *autoremoveCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	// Support flags completion.
	for _, flag := range []string{"--build-cache", "-c", "--purge", "-p"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
