package cmds

import (
	"celer/configs"
	"celer/depcheck"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/errors"
	"celer/pkgs/fileio"
	"fmt"
	"io/fs"
	"path/filepath"
	"regexp"
	"slices"
	"sort"
	"strings"

	"github.com/spf13/cobra"
)

type reverseCmd struct {
	celer *configs.Celer
	dev   bool
}

func (r *reverseCmd) Command(celer *configs.Celer) *cobra.Command {
	r.celer = celer
	command := &cobra.Command{
		Use:   "reverse",
		Short: "Query libraries that depend on the specified package.",
		Long: `Query libraries that depend on the specified package (reverse dependency lookup).

This command searches through all packages to find which packages depend on
the specified package. Useful for impact analysis and dependency management.

Examples:
  # Find all packages that depend on Eigen
  celer reverse eigen@3.4.0
  
  # Include development dependencies in search
  celer reverse nasm@2.16.03 --dev
  
  # Check reverse dependencies before removing a package
  celer reverse boost@1.87.0`,
		Args: cobra.ExactArgs(1),
		RunE: func(cmd *cobra.Command, args []string) error {
			// Validate package name format
			if err := r.validatePackageName(args[0]); err != nil {
				return fmt.Errorf("invalid package name: %w", err)
			}

			if err := r.celer.Init(); err != nil {
				return fmt.Errorf("failed to init celer: %w", err)
			}

			libraries, err := r.query(args[0])
			if err != nil {
				return fmt.Errorf("failed to query reverse dependencies: %w", err)
			}

			r.displayResults(args[0], libraries)
			return nil
		},
		ValidArgsFunction: r.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&r.dev, "dev", "d", false, "include development dependencies in reverse lookup.")
	return command
}

func (r *reverseCmd) query(target string) ([]string, error) {
	var libraries []string
	if !fileio.PathExists(dirs.PortsDir) {
		return libraries, nil
	}

	if err := filepath.WalkDir(dirs.PortsDir, func(path string, entity fs.DirEntry, err error) error {
		if err != nil {
			return err
		}

		if !entity.IsDir() && entity.Name() == "port.toml" {
			libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
			libVersion := filepath.Base(filepath.Dir(path))
			nameVersion := libName + "@" + libVersion

			var port configs.Port
			if err := port.Init(r.celer, nameVersion); err != nil {
				if errors.Is(err, errors.ErrNoMatchedConfigFound) {
					return nil
				}
				return err
			}

			// Check circular dependence.
			depcheck := depcheck.NewDepCheck()
			if err := depcheck.CheckCircular(r.celer, port); err != nil {
				return fmt.Errorf("found circular dependence: %w", err)
			}

			// Check version conflict.
			if err := depcheck.CheckConflict(r.celer, port); err != nil {
				return fmt.Errorf("found version conflict: %w", err)
			}

			// Check dependencies based on mode
			if r.hasDependency(port, target) {
				libraries = append(libraries, nameVersion)
			}
		}

		return nil
	}); err != nil {
		return nil, err
	}

	// Sort results for consistent output.
	sort.Strings(libraries)
	return libraries, nil
}

func (r *reverseCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
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

// validatePackageName validates the package name format (name@version)
func (r *reverseCmd) validatePackageName(packageName string) error {
	if packageName == "" {
		return errors.New("package name cannot be empty")
	}

	// Check if package name contains '@' separator
	if !strings.Contains(packageName, "@") {
		return errors.New("package name must be in format 'name@version'")
	}

	// Validate format using regex
	packageRegex := regexp.MustCompile(`^[a-zA-Z0-9_-]+@[a-zA-Z0-9._-]+$`)
	if !packageRegex.MatchString(packageName) {
		return errors.New("invalid package name format, expected 'name@version'")
	}

	return nil
}

// hasDependency checks if a port has the target package as dependency
func (r *reverseCmd) hasDependency(port configs.Port, target string) bool {
	// Check regular dependencies
	if slices.Contains(port.MatchedConfig.Dependencies, target) {
		return true
	}

	// Check dev dependencies if dev mode is enabled
	if r.dev {
		if slices.Contains(port.MatchedConfig.DevDependencies, target) {
			return true
		}
	}

	return false
}

// displayResults shows the reverse dependency results
func (r *reverseCmd) displayResults(target string, libraries []string) {
	var title string
	if r.dev {
		title = fmt.Sprintf("Reverse dependencies of %s as dev:", target)
	} else {
		title = fmt.Sprintf("Reverse dependencies of %s:", target)
	}
	color.Println(color.Title, title)
	color.Println(color.Title, strings.Repeat("-", len(title)))
	if len(libraries) > 0 {
		for _, lib := range libraries {
			fmt.Println(lib)
		}
		color.Println(color.Line, strings.Repeat("-", len(title)))
		color.Printf(color.Bottom, "Total: %d package(s)\n", len(libraries))
	} else {
		color.Println(color.Error, "no reverse dependencies found.")
	}
}
