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

type installCmd struct {
	celer      *configs.Celer
	buildType  string
	dev        bool
	force      bool
	recurse    bool
	storeCache bool
	cacheToken string
	jobs       int
	verbose    bool
}

func (i installCmd) Command(celer *configs.Celer) *cobra.Command {
	i.celer = celer
	command := &cobra.Command{
		Use:   "install",
		Short: "Install a package.",
		Args:  cobra.ExactArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			if err := i.celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				os.Exit(1)
			}

			i.install(args[0])
		},
		ValidArgsFunction: i.completion,
	}

	// Register flags.
	flags := command.Flags()
	flags.StringVarP(&i.buildType, "build-type", "b", i.celer.Global.BuildType, "install with build type.")
	flags.BoolVarP(&i.dev, "dev", "d", false, "install in dev mode.")
	flags.BoolVarP(&i.force, "force", "f", false, "try to uninstall before installation.")
	flags.BoolVarP(&i.recurse, "recurse", "r", false, "combine with --force, recursively reinstall dependencies.")
	flags.BoolVarP(&i.storeCache, "store-cache", "s", false, "store artifact into cache after installation.")
	flags.StringVarP(&i.cacheToken, "cache-token", "t", "", "combine with --store-cache, specify cache token.")
	flags.IntVarP(&i.jobs, "jobs", "j", i.celer.Jobs(), "the number of jobs to run in parallel.")
	flags.BoolVarP(&i.verbose, "verbose", "v", false, "verbose detail information.")

	return command
}

func (i installCmd) install(nameVersion string) {
	// Overwrite global config.
	if i.buildType != "" {
		i.celer.Global.BuildType = i.buildType
	}
	if i.jobs != i.celer.Global.Jobs {
		i.celer.Global.Jobs = i.jobs
	}
	i.celer.Global.Verbose = i.verbose

	if err := i.celer.Platform().Setup(); err != nil {
		configs.PrintError(err, "failed to setup platform.")
		os.Exit(1)
	}

	// In Windows PowerShell, when handling completion,
	// "`" is automatically added as an escape character before the "@".
	// We need to remove this escape character.
	nameVersion = strings.ReplaceAll(nameVersion, "`", "")

	parts := strings.Split(nameVersion, "@")
	if len(parts) != 2 {
		configs.PrintError(fmt.Errorf("invalid name and version"), "failed to install %s.", nameVersion)
		os.Exit(1)
	}

	portInProject := filepath.Join(dirs.ConfProjectsDir, i.celer.Project().GetName(), parts[0], parts[1], "port.toml")
	portInPorts := filepath.Join(dirs.PortsDir, parts[0], parts[1], "port.toml")
	if !fileio.PathExists(portInProject) && !fileio.PathExists(portInPorts) {
		configs.PrintError(fmt.Errorf("port %s is not found", nameVersion), "failed to install %s.", nameVersion)
		os.Exit(1)
	}

	// Install the port.
	var port configs.Port
	port.DevDep = i.dev
	port.Reinstall = i.force
	port.Recurse = i.recurse
	port.StoreCache = i.storeCache
	port.CacheToken = i.cacheToken
	if err := port.Init(i.celer, nameVersion, i.buildType); err != nil {
		configs.PrintError(err, "failed to init %s.", nameVersion)
		os.Exit(1)
	}

	// Check circular dependence.
	depcheck := depcheck.NewDepCheck()
	if err := depcheck.CheckCircular(i.celer, port); err != nil {
		configs.PrintError(err, "failed to check circular dependence.")
		os.Exit(1)
	}

	// Check version conflict.
	if err := depcheck.CheckConflict(i.celer, port); err != nil {
		configs.PrintError(err, "failed to check version conflict.")
		os.Exit(1)
	}

	// Do install.
	fromWhere, err := port.Install()
	if err != nil {
		configs.PrintError(err, "failed to install %s.", nameVersion)
		os.Exit(1)
	}
	if fromWhere != "" {
		if port.DevDep {
			configs.PrintSuccess("install %s from %s as dev successfully.", nameVersion, fromWhere)
		} else {
			configs.PrintSuccess("install %s from %s successfully.", nameVersion, fromWhere)
		}
	} else {
		if port.DevDep {
			configs.PrintSuccess("install %s as dev successfully.", nameVersion)
		} else {
			configs.PrintSuccess("install %s successfully.", nameVersion)
		}
	}
}

func (i installCmd) buildSuggestions(suggestions *[]string, portDir string, toComplete string) {
	err := filepath.WalkDir(portDir, func(path string, entity fs.DirEntry, err error) error {
		if err != nil {
			return err
		}

		if !entity.IsDir() && entity.Name() == "port.toml" {
			libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
			libVersion := filepath.Base(filepath.Dir(path))
			nameVersion := libName + "@" + libVersion

			if strings.HasPrefix(nameVersion, toComplete) {
				*suggestions = append(*suggestions, nameVersion)
			}
		}

		return nil
	})
	if err != nil {
		configs.PrintError(err, "failed to read %s.\n %s.\n", portDir, err)
		os.Exit(1)
	}
}

func (i installCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	if fileio.PathExists(dirs.PortsDir) {
		i.buildSuggestions(&suggestions, dirs.PortsDir, toComplete)
	}

	projectPortsDir := filepath.Join(dirs.ConfProjectsDir, i.celer.Project().GetName())
	if fileio.PathExists(projectPortsDir) {
		i.buildSuggestions(&suggestions, projectPortsDir, toComplete)
	}

	// Support flags completion.
	commands := []string{
		"--dev", "-d",
		"--build-type", "-b",
		"--force", "-f",
		"--recurse", "-r",
		"--store-cache", "-s",
		"--jobs", "-j",
		"--verbose", "-v",
	}

	for _, flag := range commands {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
