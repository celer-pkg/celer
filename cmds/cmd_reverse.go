package cmds

import (
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"regexp"
	"slices"
	"sort"
	"strings"

	"github.com/BurntSushi/toml"
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/internal/dirs"
	"github.com/celer-pkg/celer/internal/errors"
	"github.com/celer-pkg/celer/internal/fileio"
	"github.com/celer-pkg/celer/internal/logger"

	"github.com/spf13/cobra"
)

type ReverseCmd struct {
	Celer *configs.Celer
	Dev   bool
}

func (r *ReverseCmd) Command(celer *configs.Celer) *cobra.Command {
	r.Celer = celer
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
		ValidArgsFunction: r.Completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&r.Dev, "dev", "d", false, "include dev dependencies in reverse lookup.")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (r *ReverseCmd) doExecute(args []string) error {
	// Validate package name format
	if err := r.ValidatePackageName(args[0]); err != nil {
		return fmt.Errorf("invalid package name -> %w", err)
	}

	if err := r.Celer.Init(); err != nil {
		return fmt.Errorf("failed to init celer -> %w", err)
	}

	libraries, err := r.Query(args[0])
	if err != nil {
		return fmt.Errorf("failed to query %s -> %w", args[0], err)
	}

	r.displayResults(args[0], libraries)
	return nil
}

func (r *ReverseCmd) Query(target string) ([]string, error) {
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

			if r.TomlHasDependency(path, target) {
				libraries = append(libraries, nameVersion)
			}
			return nil
		})
	}

	if fileio.PathExists(dirs.PortsDir) {
		walkPorts(dirs.PortsDir)
	}
	projectName := r.Celer.GetProjectName()
	if projectName != "" {
		projectDir := dirs.GetProjectDir(projectName)
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
func (r *ReverseCmd) TomlHasDependency(portTomlPath, target string) bool {
	data, err := os.ReadFile(portTomlPath)
	if err != nil {
		return false
	}

	var deps tomlDeps
	if err := toml.Unmarshal(data, &deps); err != nil {
		return false
	}
	for _, config := range deps.BuildConfigs {
		if slices.Contains(config.Dependencies, target) {
			return true
		}
		if r.Dev {
			if slices.Contains(config.DevDependencies, target) {
				return true
			}
		}
	}
	return false
}

func (r *ReverseCmd) Completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
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
	projectName := r.Celer.GetProjectName()
	if projectName != "" {
		projectDir := dirs.GetProjectDir(projectName)
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
func (r *ReverseCmd) ValidatePackageName(packageName string) error {
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
func (r *ReverseCmd) displayResults(target string, libraries []string) {
	var title string
	if r.Dev {
		title = fmt.Sprintf("reverse dependencies of %s as dev", target)
	} else {
		title = fmt.Sprintf("reverse dependencies of %s", target)
	}
	logger.Println(logger.Title, title)
	logger.Println(logger.Title, strings.Repeat("-", len(title)))

	if len(libraries) > 0 {
		for _, lib := range libraries {
			fmt.Println(lib)
		}
		logger.Println(logger.Line, strings.Repeat("-", len(title)))
		logger.Printf(logger.Summary, "total: %d package(s)\n", len(libraries))
	} else {
		logger.Println(logger.Error, "no reverse dependencies found.")
	}
}
