package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strings"

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
		Run: func(cmd *cobra.Command, args []string) {
			if err := r.execute(args); err != nil {
				configs.PrintError(err, "removal failed")
				return
			}
		},
		ValidArgsFunction: r.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&r.buildCache, "build-cache", "c", false, "remove build cache along with the package")
	command.Flags().BoolVarP(&r.recursive, "recursive", "r", false, "remove package dependencies recursively")
	command.Flags().BoolVarP(&r.purge, "purge", "p", false, "purge package files completely")
	command.Flags().BoolVarP(&r.dev, "dev", "d", false, "remove from development dependencies")

	return command
}

// execute performs the main logic for package removal.
func (r *removeCmd) execute(args []string) error {
	// Initialize celer
	if err := r.celer.Init(); err != nil {
		return fmt.Errorf("failed to initialize celer: %w", err)
	}

	// Validate input arguments.
	if err := r.validatePackageNames(args); err != nil {
		return fmt.Errorf("invalid package names: %w", err)
	}

	// Remove packages.
	if err := r.removePackages(args); err != nil {
		return fmt.Errorf("failed to remove packages: %w", err)
	}

	// Print success message.
	configs.PrintSuccess("Successfully removed %s", strings.Join(args, ", "))
	return nil
}

// validatePackageNames validates package name format.
func (r *removeCmd) validatePackageNames(nameVersions []string) error {
	regex := regexp.MustCompile(`^[a-zA-Z0-9_-]+@[a-zA-Z0-9._-]+$`)
	for _, nameVersion := range nameVersions {
		if strings.TrimSpace(nameVersion) == "" {
			return fmt.Errorf("empty package name")
		}

		if !regex.MatchString(nameVersion) {
			return fmt.Errorf("invalid package name format: %s (expected format: name@version)", nameVersion)
		}
	}

	return nil
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
			return fmt.Errorf("failed to initialize package %s: %w", nameVersion, err)
		}

		if err := port.Remove(removeOptions); err != nil {
			return fmt.Errorf("failed to remove package %s: %w", nameVersion, err)
		}
	}

	return nil
}

func (r *removeCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	// Add installed package suggestions.
	suggestions = append(suggestions, r.getInstalledPackages(toComplete)...)

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

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}

// getInstalledPackages returns list of installed packages matching the completion prefix.
func (r *removeCmd) getInstalledPackages(toComplete string) []string {
	var packages []string

	traceDir := filepath.Join(dirs.InstalledDir, "celer", "trace")
	if !fileio.PathExists(traceDir) {
		return packages
	}

	entities, err := os.ReadDir(traceDir)
	if err != nil {
		return packages
	}

	for _, entity := range entities {
		if entity.IsDir() {
			continue
		}

		// Parse package name from trace file.
		// Expected format: package@version@platform.trace.
		fileName := entity.Name()
		if !strings.HasSuffix(fileName, ".trace") {
			continue
		}

		// Remove .trace extension.
		baseeName := strings.TrimSuffix(fileName, ".trace")
		parts := strings.Split(baseeName, "@")

		// Need at least package@version.
		if len(parts) >= 2 {
			nameVersion := parts[0] + "@" + parts[1]
			if strings.HasPrefix(nameVersion, toComplete) {
				packages = append(packages, nameVersion)
			}
		}
	}

	return packages
}
