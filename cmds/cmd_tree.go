package cmds

import (
	"celer/configs"
	"celer/context"
	"celer/depcheck"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type portInfo struct {
	parent          *portInfo
	nameVersion     string
	depedencies     []*portInfo
	devDependencies []*portInfo
	devDep          bool
	depth           int
}

type treeCmd struct {
	celer      *configs.Celer
	hideDevDep bool
}

func (t *treeCmd) Command(celer *configs.Celer) *cobra.Command {
	t.celer = celer
	command := &cobra.Command{
		Use:   "tree",
		Short: "Show the dependency tree of a package or project.",
		Long: `Show the dependency tree of a package or project.

This command shows a hierarchical view of all dependencies for a given
package or project, including both regular and development dependencies.
It also performs dependency validation including circular dependency checks
and version conflict detection.

Examples:
  celer tree boost@1.87.0              # Show dependencies for a specific package
  celer tree my_project                # Show dependencies for a project
  celer tree opencv@4.11.0 --hide-dev  # Hide development dependencies`,
		Args: cobra.ExactArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			if err := t.tree(args[0]); err != nil {
				configs.PrintError(err, "Failed to show dependency tree.")
				return
			}
		},
		ValidArgsFunction: t.completion,
	}

	// Register flags.
	command.Flags().BoolVar(&t.hideDevDep, "hide-dev", false, "hide dev dep in dependencies tree.")
	return command
}

func (t *treeCmd) tree(target string) error {
	// Initialize celer.
	if err := t.celer.Init(); err != nil {
		return fmt.Errorf("failed to initialize celer: %w", err)
	}

	// Validate target.
	if err := t.validateTarget(target); err != nil {
		return fmt.Errorf("invalid target: %w", err)
	}

	depchecker := depcheck.NewDepCheck()
	if strings.Contains(target, "@") {
		return t.showPortTree(target, depchecker)
	}
	return t.showProjectTree(target, depchecker)
}

// validateTarget validates the target parameter.
func (t *treeCmd) validateTarget(target string) error {
	target = strings.TrimSpace(target)
	if target == "" {
		return fmt.Errorf("target cannot be empty")
	}
	return nil
}

// showPortTree displays dependency tree for a port.
func (t *treeCmd) showPortTree(target string, depchecker any) error {
	var port configs.Port
	if err := port.Init(t.celer, target); err != nil {
		return fmt.Errorf("failed to initialize port %s: %w", target, err)
	}

	// Check circular dependence and version conflicts.
	// We know depchecker is *depcheck from NewDepCheck()
	type depChecker interface {
		CheckCircular(context.Context, configs.Port) error
		CheckConflict(context.Context, ...configs.Port) error
	}
	checker := depchecker.(depChecker)

	if err := checker.CheckCircular(t.celer, port); err != nil {
		return fmt.Errorf("circular dependency detected: %w", err)
	}

	if err := checker.CheckConflict(t.celer, port); err != nil {
		return fmt.Errorf("version conflict detected: %w", err)
	}

	rootInfo := portInfo{
		nameVersion: target,
		depth:       0,
		devDep:      false,
	}
	if err := t.collectPortInfos(&rootInfo, target); err != nil {
		return fmt.Errorf("failed to collect port information: %w", err)
	}

	color.Printf(color.Blue, "Display dependencies in tree view\n---------------------------------------\n")
	t.printTree(&rootInfo)
	return nil
}

// showProjectTree displays dependency tree for a project.
func (t *treeCmd) showProjectTree(target string, depchecker any) error {
	var project configs.Project
	if err := project.Init(t.celer, target); err != nil {
		return fmt.Errorf("failed to initialize project %s: %w", target, err)
	}

	rootInfo := portInfo{
		nameVersion: target,
		depth:       0,
		devDep:      false,
	}
	nextDepth := rootInfo.depth + 1

	// Define checker interface.
	type depChecker interface {
		CheckCircular(context.Context, configs.Port) error
		CheckConflict(context.Context, ...configs.Port) error
	}
	checker := depchecker.(depChecker)

	// Check circular dependence and version conflicts.
	var ports []configs.Port
	for _, nameVersion := range project.Ports {
		var port configs.Port
		if err := port.Init(t.celer, nameVersion); err != nil {
			return fmt.Errorf("failed to initialize port %s: %w", nameVersion, err)
		}

		if err := checker.CheckCircular(t.celer, port); err != nil {
			return fmt.Errorf("circular dependency detected in %s: %w", nameVersion, err)
		}

		ports = append(ports, port)
	}
	if err := checker.CheckConflict(t.celer, ports...); err != nil {
		return fmt.Errorf("version conflicts detected: %w", err)
	}

	// Collect port info.
	for _, port := range project.Ports {
		portInfo := portInfo{
			nameVersion: port,
			depth:       nextDepth,
			devDep:      false,
		}
		if err := t.collectPortInfos(&portInfo, port); err != nil {
			return fmt.Errorf("failed to collect port information for %s: %w", port, err)
		}

		rootInfo.depedencies = append(rootInfo.depedencies, &portInfo)
	}

	color.Printf(color.Blue, "Display dependencies in tree view\n--------------------------------------------\n")
	t.printTree(&rootInfo)
	return nil
}

func (t *treeCmd) collectPortInfos(parent *portInfo, nameVersion string) error {
	var port configs.Port
	if err := port.Init(t.celer, nameVersion); err != nil {
		return err
	}

	matchedConfig := port.MatchedConfig
	nextDepth := parent.depth + 1

	// Collect dependency ports.
	for _, depNameVersion := range matchedConfig.Dependencies {
		var depPort configs.Port
		if err := depPort.Init(t.celer, depNameVersion); err != nil {
			return err
		}

		portInfo := portInfo{
			parent:      parent,
			nameVersion: depNameVersion,
			depth:       nextDepth,
			devDep:      parent.devDep,
		}
		parent.depedencies = append(parent.depedencies, &portInfo)
		t.collectPortInfos(&portInfo, depNameVersion)
	}

	// Collect dev_dependency ports.
	for _, devDepNameVersion := range matchedConfig.DevDependencies {
		// Ignore itself.
		if parent.devDep && devDepNameVersion == nameVersion {
			continue
		}

		var devDepPort configs.Port
		if err := devDepPort.Init(t.celer, devDepNameVersion); err != nil {
			return err
		}

		portInfo := portInfo{
			parent:      parent,
			nameVersion: devDepNameVersion,
			depth:       nextDepth,
			devDep:      true,
		}
		parent.devDependencies = append(parent.devDependencies, &portInfo)
		t.collectPortInfos(&portInfo, devDepNameVersion)
	}

	return nil
}

func (t *treeCmd) printTree(info *portInfo) {
	t.printTreeWithPrefix(info, "", true)

	// Count dependencies.
	depCount, devDepCount := t.countDependencies(info)

	// Print statistics.
	color.Printf(color.Blue, "---------------------------------------------\n")
	color.Printf(color.Blue, "Summary: dependencies: %d  dev_dependencies: %d\n", depCount, devDepCount)
}

func (t *treeCmd) countDependencies(info *portInfo) (int, int) {
	depCount := 0
	devDepCount := 0
	visited := make(map[string]bool)

	t.countDependenciesRecursive(info, visited, &depCount, &devDepCount)
	return depCount, devDepCount
}

func (t *treeCmd) countDependenciesRecursive(info *portInfo, visited map[string]bool, depCount *int, devDepCount *int) {
	// Skip root node.
	if info.depth == 0 {
		for _, nameVersion := range info.depedencies {
			t.countDependenciesRecursive(nameVersion, visited, depCount, devDepCount)
		}
		for _, nameVersion := range info.devDependencies {
			t.countDependenciesRecursive(nameVersion, visited, depCount, devDepCount)
		}
		return
	}

	// Check if already visited.
	if visited[info.nameVersion] {
		return
	}
	visited[info.nameVersion] = true

	// Count this dependency.
	if info.devDep {
		*devDepCount++
	} else {
		*depCount++
	}

	// Recursively count children.
	for _, child := range info.depedencies {
		t.countDependenciesRecursive(child, visited, depCount, devDepCount)
	}
	for _, child := range info.devDependencies {
		t.countDependenciesRecursive(child, visited, depCount, devDepCount)
	}
}

func (t *treeCmd) printTreeWithPrefix(info *portInfo, prefix string, isLast bool) {
	var branch string
	if info.depth == 0 {
		branch = ""
	} else if isLast {
		branch = "└── "
	} else {
		branch = "├── "
	}

	// Compose the line to print.
	line := branch + info.nameVersion
	if info.devDep {
		line += " -- [dev]"
	}
	color.Println(color.Gray, prefix+line)

	// Prepare the prefix for the next level.
	var nextPrefix string
	if info.depth == 0 {
		nextPrefix = ""
	} else if isLast {
		nextPrefix = prefix + "    " // No vertical line if this is the last child.
	} else {
		nextPrefix = prefix + "│   " // Keep vertical line for siblings.
	}

	// Merge normal and dev dependencies (if not hidden).
	children := info.depedencies
	if !t.hideDevDep {
		children = append(children, info.devDependencies...)
	}

	// Recursively print children
	for i, child := range children {
		t.printTreeWithPrefix(child, nextPrefix, i == len(children)-1)
	}
}

func (t *treeCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	// Support port completion from global ports.
	if fileio.PathExists(dirs.PortsDir) {
		filepath.WalkDir(dirs.PortsDir, func(path string, d fs.DirEntry, err error) error {
			if err != nil {
				return err
			}

			if !d.IsDir() && strings.HasSuffix(d.Name(), ".toml") {
				libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
				libVersion := filepath.Base(filepath.Dir(path))
				nameVersion := libName + "@" + libVersion

				if strings.HasPrefix(nameVersion, toComplete) {
					suggestions = append(suggestions, nameVersion)
				}
			}

			return nil
		})
	}

	// Support port completion from project-specific ports.
	projectPortsDir := filepath.Join(dirs.ConfProjectsDir, t.celer.Project().GetName())
	if fileio.PathExists(projectPortsDir) {
		filepath.WalkDir(projectPortsDir, func(path string, d fs.DirEntry, err error) error {
			if err != nil {
				return err
			}

			if !d.IsDir() && strings.HasSuffix(d.Name(), ".toml") {
				libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
				libVersion := filepath.Base(filepath.Dir(path))
				nameVersion := libName + "@" + libVersion

				if strings.HasPrefix(nameVersion, toComplete) {
					suggestions = append(suggestions, nameVersion)
				}
			}

			return nil
		})
	}

	// Support project completion.
	if fileio.PathExists(dirs.ConfProjectsDir) {
		entities, err := os.ReadDir(dirs.ConfProjectsDir)
		if err != nil {
			return suggestions, cobra.ShellCompDirectiveNoFileComp
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
	for _, flag := range []string{"--hide-dev"} {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
