package cmds

import (
	"celer/configs"
	"celer/depcheck"
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

func (t treeCmd) Command(celer *configs.Celer) *cobra.Command {
	t.celer = celer
	command := &cobra.Command{
		Use:   "tree",
		Short: "Show the dependencies of a port or a project.",
		Args:  cobra.ExactArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			// Handler celer error inside.
			if t.celer.HandleError() {
				os.Exit(1)
			}

			t.tree(args[0])
		},
		ValidArgsFunction: t.completion,
	}

	// Register flags.
	command.Flags().BoolVar(&t.hideDevDep, "hide-dev", false, "hide dev dep in dependencies tree.")
	return command
}

func (t *treeCmd) tree(target string) {
	depcheck := depcheck.NewDepCheck()
	if strings.Contains(target, "@") {
		var port configs.Port
		if err := port.Init(t.celer, target, t.celer.BuildType()); err != nil {
			configs.PrintError(err, "failed to init port.")
			return
		}

		// Check circular dependence and version conflicts.
		if err := depcheck.CheckCircular(t.celer, port); err != nil {
			configs.PrintError(err, "failed to check circular dependence.")
			return
		}

		if err := depcheck.CheckConflict(t.celer, port); err != nil {
			configs.PrintError(err, "failed to check version conflict.")
			return
		}

		rootInfo := portInfo{
			nameVersion: target,
			depth:       0,
			devDep:      false,
		}
		if err := t.collectPortInfos(&rootInfo, target); err != nil {
			configs.PrintError(err, "failed to collect port infos.")
			return
		}

		t.printTree(&rootInfo)
	} else {
		var project configs.Project
		if err := project.Init(t.celer, target); err != nil {
			configs.PrintError(err, "failed to init project.")
			return
		}

		rootInfo := portInfo{
			nameVersion: target,
			depth:       0,
			devDep:      false,
		}
		nextDepth := rootInfo.depth + 1

		// Check circular dependence and version conflicts.
		var ports []configs.Port
		for _, nameVersion := range project.Ports {
			var port configs.Port
			if err := port.Init(t.celer, nameVersion, t.celer.BuildType()); err != nil {
				configs.PrintError(err, "failed to init port: %s", nameVersion)
				return
			}

			if err := depcheck.CheckCircular(t.celer, port); err != nil {
				configs.PrintError(err, "failed to check circular dependence.")
				return
			}

			ports = append(ports, port)
		}
		if err := depcheck.CheckConflict(t.celer, ports...); err != nil {
			configs.PrintError(err, "failed to check version conflicts.")
			return
		}

		// Collect port info.
		for _, port := range project.Ports {
			portInfo := portInfo{
				nameVersion: port,
				depth:       nextDepth,
				devDep:      false,
			}
			if err := t.collectPortInfos(&portInfo, port); err != nil {
				configs.PrintError(err, "failed to collect port info.")
				return
			}

			rootInfo.depedencies = append(rootInfo.depedencies, &portInfo)
		}

		t.printTree(&rootInfo)
	}
}

func (t *treeCmd) collectPortInfos(parent *portInfo, nameVersion string) error {
	var port configs.Port
	if err := port.Init(t.celer, nameVersion, t.celer.BuildType()); err != nil {
		return err
	}

	matchedConfig := port.MatchedConfig
	nextDepth := parent.depth + 1

	// Collect dependency ports.
	for _, depNameVersion := range matchedConfig.Dependencies {
		var depPort configs.Port
		if err := depPort.Init(t.celer, depNameVersion, t.celer.BuildType()); err != nil {
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
		if err := devDepPort.Init(t.celer, devDepNameVersion, t.celer.BuildType()); err != nil {
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
	fmt.Println(prefix + line)

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

func (t treeCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	// Support port completion.
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

	// Support project completion.
	if fileio.PathExists(dirs.ConfProjectsDir) {
		entities, err := os.ReadDir(dirs.ConfProjectsDir)
		if err != nil {
			configs.PrintError(err, "failed to read %s: %s.\n", dirs.ConfProjectsDir, err)
			os.Exit(1)
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
