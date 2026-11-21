package cmds

import (
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"strings"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/depcheck"
	"github.com/celer-pkg/celer/internal/dirs"
	"github.com/celer-pkg/celer/internal/errors"
	"github.com/celer-pkg/celer/internal/logger"

	"github.com/spf13/cobra"
)

type AutoremoveCmd struct {
	Celer       *configs.Celer
	Packages    []string
	DevPackages []string
	purge       bool
	buildCache  bool
}

func (a *AutoremoveCmd) Command(celer *configs.Celer) *cobra.Command {
	a.Celer = celer
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
			if err := a.Celer.Init(); err != nil {
				return fmt.Errorf("failed to init celer -> %w", err)
			}

			return a.autoremove()
		},
		ValidArgsFunction: a.Completion,
	}

	command.Flags().BoolVarP(&a.buildCache, "build-cache", "c", false, "autoremove packages along with build cache.")
	command.Flags().BoolVarP(&a.purge, "purge", "p", false, "autoremove packages along with its package file.")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (a *AutoremoveCmd) autoremove() error {
	// Reset collected state for every run.
	a.Packages = nil
	a.DevPackages = nil

	// Collect packages/devPackages that belongs to project.
	for _, nameVersion := range a.Celer.Project().GetPorts() {
		if err := a.collectPackages(nameVersion); err != nil {
			return fmt.Errorf("failed to collect packages -> %w", err)
		}
		if err := a.collectDevPackages(nameVersion); err != nil {
			return fmt.Errorf("failed to collect dev packages -> %w", err)
		}
	}

	// Query installed packages.
	depPackages, devDepPackages, err := a.installedPackages()
	if err != nil {
		return fmt.Errorf("failed to query installed packages -> %w", err)
	}

	// Remove packages that do not belongs to project.
	for _, nameVersion := range depPackages {
		if slices.Contains(a.Packages, nameVersion) {
			continue
		}
		if err := a.removePackage(nameVersion, false); err != nil {
			// Remove buildtrees for packages that are not found.
			if errors.Is(err, errors.ErrPortNotFound) {
				os.RemoveAll(filepath.Join(dirs.BuildtreesDir, nameVersion))
			} else {
				return fmt.Errorf("failed to autoremove %s -> %w", nameVersion, err)
			}
		}
	}

	// Remove dev_packages that do not belongs to current project.
	for _, nameVersion := range devDepPackages {
		if slices.Contains(a.DevPackages, nameVersion) {
			continue
		}
		if err := a.removePackage(nameVersion, true); err != nil {
			// Remove buildtrees for packages that are not found.
			if errors.Is(err, errors.ErrPortNotFound) {
				os.RemoveAll(filepath.Join(dirs.BuildtreesDir, nameVersion))
			} else {
				return fmt.Errorf("failed to autoremove %s -> %w", nameVersion, err)
			}
		}
	}

	logger.PrintSuccess("autoremove successfully for %s", a.Celer.Project().GetName())
	return nil
}

func (a *AutoremoveCmd) removePackage(nameVersion string, dev bool) error {
	var port configs.Port
	port.DevDep = dev
	port.HostDep = dev
	if err := port.Init(a.Celer, nameVersion); err != nil {
		return err
	}

	options := configs.RemoveOptions{
		Purge:      a.purge,
		Recursive:  false,
		BuildCache: a.buildCache,
	}
	return port.Remove(options)
}

func (a *AutoremoveCmd) collectPackages(nameVersion string) error {
	// Init port with name version.
	var port configs.Port
	if err := port.Init(a.Celer, nameVersion); err != nil {
		return err
	}

	// Check circular dependence and version conflict.
	depcheck := depcheck.NewDepCheck()
	if err := depcheck.CheckCircular(a.Celer, port); err != nil {
		return fmt.Errorf("found circular dependence %s -> %w", nameVersion, err)
	}
	if err := depcheck.CheckConflict(a.Celer, port); err != nil {
		return fmt.Errorf("found version conflict %s -> %w", nameVersion, err)
	}

	// Add if not added before.
	if !slices.Contains(a.Packages, nameVersion) {
		a.Packages = append(a.Packages, nameVersion)
	}

	for _, nameVersion := range port.MatchedConfig.Dependencies {
		if !slices.Contains(a.Packages, nameVersion) {
			a.Packages = append(a.Packages, nameVersion)
			if err := a.collectPackages(nameVersion); err != nil {
				return fmt.Errorf("failed to collect package for %s -> %w", nameVersion, err)
			}
		}
	}

	return nil
}

func (a *AutoremoveCmd) collectDevPackages(nameVersion string) error {
	// Init port with name version.
	var port configs.Port
	if err := port.Init(a.Celer, nameVersion); err != nil {
		return err
	}

	// Check circular dependence and version conflict.
	depcheck := depcheck.NewDepCheck()
	if err := depcheck.CheckCircular(a.Celer, port); err != nil {
		return fmt.Errorf("found circular dependence when collecting dev package %s -> %w", nameVersion, err)
	}
	if err := depcheck.CheckConflict(a.Celer, port); err != nil {
		return fmt.Errorf("found version conflict when collecting dev package %s -> %w", nameVersion, err)
	}

	// Collect dev dependencies.
	for _, nameVersion := range port.MatchedConfig.DevDependencies {
		if !slices.Contains(a.DevPackages, nameVersion) {
			a.DevPackages = append(a.DevPackages, nameVersion)
			if err := a.collectDevPackages(nameVersion); err != nil {
				return err
			}
		}
	}

	// Dev package chains may include regular dependencies as host dependencies.
	// They are also installed under host-dev and should not be removed.
	for _, nameVersion := range port.MatchedConfig.Dependencies {
		if !slices.Contains(a.DevPackages, nameVersion) {
			a.DevPackages = append(a.DevPackages, nameVersion)
			if err := a.collectDevPackages(nameVersion); err != nil {
				return err
			}
		}
	}

	return nil
}

func (a *AutoremoveCmd) installedPackages() (packages []string, devPackages []string, err error) {
	// Collect installed packages and cached packages.
	libraryDir := filepath.Join(
		a.Celer.Platform().GetName(),
		a.Celer.Project().GetName(),
		a.Celer.BuildType(),
	)
	packages, err = a.readInstalledPackages(libraryDir)
	if err != nil {
		return nil, nil, err
	}
	cachedPackages, err := a.readCachedPackages(libraryDir)
	if err != nil {
		return nil, nil, err
	}
	packages = mergePackages(packages, cachedPackages)

	// Collect installed dev packages and cached dev packages.
	devLibraryDir := a.Celer.Platform().GetHostName() + "-dev"
	devPackages, err = a.readInstalledPackages(devLibraryDir)
	if err != nil {
		return nil, nil, err
	}
	cachedDevPackages, err := a.readCachedPackages(devLibraryDir)
	if err != nil {
		return nil, nil, err
	}
	devPackages = mergePackages(devPackages, cachedDevPackages)
	return
}

func (a *AutoremoveCmd) readInstalledPackages(libraryDir string) ([]string, error) {
	traceDir := filepath.Join(dirs.InstalledDir, "infos", "traces", libraryDir)

	entries, err := os.ReadDir(traceDir)
	if err != nil {
		if os.IsNotExist(err) {
			return nil, nil
		}
		return nil, err
	}

	var packages []string
	for _, entry := range entries {
		if entry.IsDir() {
			continue
		}
		if cutted, ok := strings.CutSuffix(entry.Name(), ".trace"); ok {
			packages = append(packages, cutted)
		}
	}

	return packages, nil
}

func (a *AutoremoveCmd) readCachedPackages(libraryDir string) ([]string, error) {
	cacheDir := filepath.Join(dirs.PackagesDir, libraryDir)

	entries, err := os.ReadDir(cacheDir)
	if err != nil {
		if os.IsNotExist(err) {
			return nil, nil
		}
		return nil, err
	}

	var packages []string
	for _, entry := range entries {
		if !entry.IsDir() {
			continue
		}
		packages = append(packages, entry.Name())
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

func (a *AutoremoveCmd) Completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	// Support flags completion.
	for _, flag := range []string{"--build-cache", "-c", "--purge", "-p"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
