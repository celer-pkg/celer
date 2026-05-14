package cmds

import (
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strings"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/spf13/cobra"
)

type removeCmd struct {
	celer      *configs.Celer
	dev        bool
	purge      bool
	recursive  bool
	buildCache bool
}

func (r *removeCmd) Command(celer *configs.Celer) *cobra.Command {
	r.celer = celer
	command := &cobra.Command{
		Use:   "remove",
		Short: "Remove installed packages, optionally with their dependencies, build cache, and package files.",
		Long: `Remove installed packages, optionally with their dependencies, build cache, and package files.

This command will uninstall the specified packages from your workspace. You can control
the removal behavior using various flags.

Examples:
  celer remove boost@1.87.0          # Remove boost package
  celer remove boost@1.87.0 -r       # Remove boost and its dependencies
  celer remove boost@1.87.0 -p       # Remove boost and purge package files
  celer remove boost@1.87.0 -c       # Remove boost and clear build cache
  celer remove boost@1.87.0 -d       # Remove boost from dev dependencies
  celer remove boost@1.87.0 -rpc     # Remove with all cleanup options`,
		Args: cobra.MinimumNArgs(1),
		RunE: func(cmd *cobra.Command, args []string) error {
			return r.execute(args)
		},
		ValidArgsFunction: r.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&r.buildCache, "build-cache", "c", false, "remove build cache along with the package")
	command.Flags().BoolVarP(&r.recursive, "recursive", "r", false, "remove package dependencies recursively")
	command.Flags().BoolVarP(&r.purge, "purge", "p", false, "purge package files completely")
	command.Flags().BoolVarP(&r.dev, "dev", "d", false, "remove from development dependencies")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

// execute performs the main logic for package removal.
func (r *removeCmd) execute(args []string) error {
	// Validate and normalize input arguments first.
	nameVersions, err := r.validatePackageNames(args)
	if err != nil {
		return fmt.Errorf("invalid package names -> %w", err)
	}

	// Initialize celer
	if err := r.celer.Init(); err != nil {
		return fmt.Errorf("failed to initialize celer -> %w", err)
	}

	// Remove packages.
	if err := r.removePackages(nameVersions); err != nil {
		return fmt.Errorf("failed to remove packages -> %w", err)
	}

	// Print success message.
	color.PrintSuccess("Successfully removed %s", strings.Join(nameVersions, ", "))
	return nil
}

func (r *removeCmd) validatePackageNames(nameVersions []string) ([]string, error) {
	regex := regexp.MustCompile(`^[a-zA-Z0-9_-]+@[a-zA-Z0-9._-]+$`)
	normalized := make([]string, 0, len(nameVersions))

	for _, nameVersion := range nameVersions {
		// In Windows PowerShell completion context, "`" can be added before "@",
		// keep behavior aligned with install/update commands.
		nameVersion = strings.ReplaceAll(nameVersion, "`", "")
		nameVersion = strings.TrimSpace(nameVersion)

		if nameVersion == "" {
			return nil, fmt.Errorf("empty package name")
		}

		if !regex.MatchString(nameVersion) {
			return nil, fmt.Errorf("invalid package name format: %s (expected format: name@version)", nameVersion)
		}

		normalized = append(normalized, nameVersion)
	}

	return normalized, nil
}

// removePackages handles the actual package removal.
func (r *removeCmd) removePackages(nameVersions []string) error {
	removeOptions := configs.RemoveOptions{
		Purge:      r.purge,
		Recursive:  r.recursive,
		BuildCache: r.buildCache,
	}

	for _, nameVersion := range nameVersions {
		var port configs.Port
		port.DevDep = r.dev

		if err := port.Init(r.celer, nameVersion); err != nil {
			return fmt.Errorf("failed to initialize package %s -> %w", nameVersion, err)
		}

		if err := port.Remove(removeOptions); err != nil {
			return fmt.Errorf("failed to remove package %s -> %w", nameVersion, err)
		}
	}

	return nil
}

func (r *removeCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	// Add installed package suggestions.
	suggestions = append(suggestions, r.getSuggestions(toComplete)...)

	// Add flag suggestions.
	flags := []string{
		"--build-cache", "-c",
		"--recursive", "-r",
		"--purge", "-p",
		"--dev", "-d",
	}

	for _, flag := range flags {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	// Keep completion list stable and deduplicated.
	seen := make(map[string]bool, len(suggestions))
	unique := make([]string, 0, len(suggestions))
	for _, suggestion := range suggestions {
		if seen[suggestion] {
			continue
		}
		seen[suggestion] = true
		unique = append(unique, suggestion)
	}

	return unique, cobra.ShellCompDirectiveNoFileComp
}

// getSuggestions returns list of installed packages matching the completion prefix.
func (r *removeCmd) getSuggestions(toComplete string) []string {
	// Initialize celer to get current platform/project/buildType configuration.
	if err := r.celer.Init(); err != nil {
		return []string{} // Ignore error.
	}

	// Build the trace directory path for current platform/project/buildType configuration.
	libraryDir := filepath.Join(
		r.celer.Platform().GetName(),
		r.celer.Project().GetName(),
		r.celer.BuildType(),
	)

	traceDir := filepath.Join(dirs.InstalledDir, "celer", "trace", libraryDir)
	if !fileio.PathExists(traceDir) {
		return []string{} // Ignore error.
	}

	var packages []string
	entries, err := os.ReadDir(traceDir)
	if err != nil {
		if os.IsNotExist(err) {
			return []string{} // Ignore error.
		}
		return packages
	}

	for _, entry := range entries {
		if entry.IsDir() {
			continue
		}
		if cutted, ok := strings.CutSuffix(entry.Name(), ".trace"); ok {
			if strings.HasPrefix(cutted, toComplete) {
				packages = append(packages, cutted)
			}
		}
	}

	return packages
}
