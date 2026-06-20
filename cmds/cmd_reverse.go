package cmds

import (
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"regexp"
	"sort"
	"strings"

	"github.com/BurntSushi/toml"
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/fileio"

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
  
  # Include dev dependencies in search
  celer reverse nasm@2.16.03 --dev
  
  # Check reverse dependencies before removing a package
  celer reverse boost@1.87.0`,
		Args: cobra.ExactArgs(1),
		RunE: func(cmd *cobra.Command, args []string) error {
			return r.doExecute(args)
		},
		ValidArgsFunction: r.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&r.dev, "dev", "d", false, "include dev dependencies in reverse lookup.")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (r *reverseCmd) doExecute(args []string) error {
	// Validate package name format
	if err := r.validatePackageName(args[0]); err != nil {
		return fmt.Errorf("invalid package name -> %w", err)
	}

	if err := r.celer.Init(); err != nil {
		return fmt.Errorf("failed to init celer -> %w", err)
	}

	libraries, err := r.query(args[0])
	if err != nil {
		return fmt.Errorf("failed to query %s -> %w", args[0], err)
	}

	r.displayResults(args[0], libraries)
	return nil
}

func (r *reverseCmd) query(target string) ([]string, error) {
	var libraries []string
	visited := map[string]bool{}

	walkPorts := func(root string) error {
		return filepath.WalkDir(root, func(path string, d fs.DirEntry, err error) error {
			if err != nil {
				return nil
			}
			if d.IsDir() && strings.HasPrefix(d.Name(), ".") {
				return filepath.SkipDir
			}
			if d.IsDir() || d.Name() != "port.toml" {
				return nil
			}

			portDir := filepath.Dir(path)
			libVersion := filepath.Base(portDir)
			libName := filepath.Base(filepath.Dir(portDir))
			nameVersion := libName + "@" + libVersion

			if visited[nameVersion] {
				return nil
			}
			visited[nameVersion] = true

			if r.tomlHasDependency(path, target) {
				libraries = append(libraries, nameVersion)
			}
			return nil
		})
	}

	if fileio.PathExists(dirs.PortsDir) {
		walkPorts(dirs.PortsDir)
	}
	if r.celer.Project().GetName() != "" {
		projectDir := filepath.Join(dirs.ConfProjectsDir, r.celer.Project().GetName())
		if fileio.PathExists(projectDir) {
			walkPorts(projectDir)
		}
	}

	sort.Strings(libraries)
	return libraries, nil
}

// tomlDeps is a minimal struct for extracting dependencies from port.toml
// without the overhead of a full Port.Init().
type tomlDeps struct {
	BuildConfigs []struct {
		Dependencies    []string `toml:"dependencies"`
		DevDependencies []string `toml:"dev_dependencies"`
	} `toml:"build_configs"`
}

// tomlHasDependency reads port.toml and checks if target appears in any
// build_config's dependencies or dev_dependencies. Much faster than Port.Init().
func (r *reverseCmd) tomlHasDependency(portTomlPath, target string) bool {
	data, err := os.ReadFile(portTomlPath)
	if err != nil {
		return false
	}

	var deps tomlDeps
	if err := toml.Unmarshal(data, &deps); err != nil {
		return false
	}
	for _, config := range deps.BuildConfigs {
		for _, dependency := range config.Dependencies {
			if dependency == target {
				return true
			}
		}
		if r.dev {
			for _, dependency := range config.DevDependencies {
				if dependency == target {
					return true
				}
			}
		}
	}
	return false
}

func (r *reverseCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	suggestions := make([]string, 0)
	visited := map[string]bool{}

	walkPorts := func(root string) {
		filepath.WalkDir(root, func(path string, d fs.DirEntry, err error) error {
			if err != nil {
				return nil
			}
			if d.IsDir() && strings.HasPrefix(d.Name(), ".") {
				return filepath.SkipDir
			}
			if d.IsDir() || d.Name() != "port.toml" {
				return nil
			}
			portDir := filepath.Dir(path)
			libVersion := filepath.Base(portDir)
			libName := filepath.Base(filepath.Dir(portDir))
			nameVersion := libName + "@" + libVersion
			if !visited[nameVersion] && strings.HasPrefix(nameVersion, toComplete) {
				visited[nameVersion] = true
				suggestions = append(suggestions, nameVersion)
			}
			return nil
		})
	}

	if fileio.PathExists(dirs.PortsDir) {
		walkPorts(dirs.PortsDir)
	}
	if r.celer != nil && r.celer.Project().GetName() != "" {
		projectDir := filepath.Join(dirs.ConfProjectsDir, r.celer.Project().GetName())
		if fileio.PathExists(projectDir) {
			walkPorts(projectDir)
		}
	}

	for _, flag := range []string{"--dev", "-d"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
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

// displayResults shows the reverse dependency results
func (r *reverseCmd) displayResults(target string, libraries []string) {
	var title string
	if r.dev {
		title = fmt.Sprintf("reverse dependencies of %s as dev", target)
	} else {
		title = fmt.Sprintf("reverse dependencies of %s", target)
	}
	color.Println(color.Title, title)
	color.Println(color.Title, strings.Repeat("-", len(title)))

	if len(libraries) > 0 {
		for _, lib := range libraries {
			fmt.Println(lib)
		}
		color.Println(color.Line, strings.Repeat("-", len(title)))
		color.Printf(color.Summary, "total: %d package(s)\n", len(libraries))
	} else {
		color.Println(color.Error, "no reverse dependencies found.")
	}
}
