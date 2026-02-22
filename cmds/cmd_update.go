package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type updateCmd struct {
	celer     *configs.Celer
	confRepo  bool
	portsRepo bool
	recursive bool
	force     bool
}

func (u *updateCmd) Command(celer *configs.Celer) *cobra.Command {
	u.celer = celer
	command := &cobra.Command{
		Use:   "update",
		Short: "Update conf repo, ports config repo or project repo.",
		Long: `Update conf repo, ports config repo or project repo.

This command supports three types of updates:
  1. Update conf repository (configuration files)
  2. Update ports repository (port configuration files)
  3. Update source code repositories of third-party libraries

Examples:
  celer update --conf-repo                      # Update conf repository
  celer update --ports-repo                     # Update ports repository
  celer update zlib@1.3.1                       # Update single port
  celer update entt@3.16.0 fakeit@2.5.0         # Update multiple ports
  celer update --recursive ffmpeg@3.4.13        # Update port and all its dependencies
  celer update --force boost@1.82.0             # Force update (overwrites local changes)`,
		Args: u.validateArgs,
		RunE: func(cmd *cobra.Command, args []string) error {
			return u.doUpdate(args)
		},
		ValidArgsFunction: u.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&u.confRepo, "conf-repo", "c", false, "update conf repo")
	command.Flags().BoolVarP(&u.portsRepo, "ports-repo", "p", false, "update ports repo")
	command.Flags().BoolVarP(&u.force, "force", "f", false, "update forcibly")
	command.Flags().BoolVarP(&u.recursive, "recursive", "r", false, "update recursively")

	command.MarkFlagsMutuallyExclusive("conf-repo", "ports-repo")

	// Silence cobra's error and usage output to avoid duplicate messages.
	command.SilenceErrors = true
	command.SilenceUsage = true
	return command
}

func (u *updateCmd) validateArgs(cmd *cobra.Command, args []string) error {
	confRepo, err := cmd.Flags().GetBool("conf-repo")
	if err != nil {
		return err
	}

	portsRepo, err := cmd.Flags().GetBool("ports-repo")
	if err != nil {
		return err
	}

	recursive, err := cmd.Flags().GetBool("recursive")
	if err != nil {
		return err
	}

	if confRepo || portsRepo {
		if len(args) > 0 {
			return fmt.Errorf("positional arguments are not allowed with --conf-repo or --ports-repo")
		}
		if recursive {
			return fmt.Errorf("--recursive is only valid when updating specific ports")
		}
		return nil
	}

	if len(args) == 0 {
		return fmt.Errorf("requires at least one port argument when neither --conf-repo nor --ports-repo is set")
	}

	return nil
}

func (u *updateCmd) doUpdate(args []string) error {
	// Initialize celer configuration.
	if err := u.celer.Init(); err != nil {
		return configs.PrintError(err, "Failed to initialize celer.")
	}

	// Make sure git is available.
	if err := buildtools.CheckTools(u.celer, "git"); err != nil {
		return configs.PrintError(err, "Failed to check if git is available.")
	}

	// Perform update based on flags.
	if u.confRepo {
		if err := u.updateConfRepo(); err != nil {
			return configs.PrintError(err, "Failed to update conf repository.")
		}
		configs.PrintSuccess("Successfully updated conf repository.")
	} else if u.portsRepo {
		if err := u.updatePortsRepo(); err != nil {
			return configs.PrintError(err, "Failed to update ports repository.")
		}
		configs.PrintSuccess("Successfully updated ports repository.")
	} else {
		if err := u.updateProjectRepos(args); err != nil {
			return configs.PrintError(err, "Failed to update port repository.")
		}
		if len(args) == 1 {
			configs.PrintSuccess("Successfully updated %s.", args[0])
		} else {
			configs.PrintSuccess("Successfully updated %d ports.", len(args))
		}
	}

	return nil
}

func (u *updateCmd) updateConfRepo() error {
	title := "[update conf repo]"
	repoDir := filepath.Join(dirs.WorkspaceDir, "conf")
	return git.UpdateRepo(title, "", repoDir, u.force)
}

func (u *updateCmd) updatePortsRepo() error {
	title := "[update ports repo]"
	repoDir := filepath.Join(dirs.WorkspaceDir, "ports")
	return git.UpdateRepo(title, "", repoDir, u.force)
}

func (u *updateCmd) updateProjectRepos(nameVersions []string) error {
	if len(nameVersions) == 0 {
		return fmt.Errorf("no ports specified to update")
	}

	// Use visited map to prevent infinite recursion in case of circular dependencies.
	visited := make(map[string]bool)

	for _, nameVersion := range nameVersions {
		nameVersion = strings.ReplaceAll(nameVersion, "`", "")
		if err := u.updatePortRepo(nameVersion, visited); err != nil {
			return err
		}
	}

	return nil
}

func (u *updateCmd) updatePortRepo(nameVersion string, visited map[string]bool) error {
	// Check for circular dependencies.
	if visited[nameVersion] {
		return nil // Already processed, skip.
	}
	visited[nameVersion] = true

	// Read port file.
	var port configs.Port
	if err := port.Init(u.celer, nameVersion); err != nil {
		return fmt.Errorf("failed to init %s -> %w", nameVersion, err)
	}

	// Update repos of port's dependencies.
	if u.recursive {
		for _, nameVersion := range port.MatchedConfig.Dependencies {
			if err := u.updatePortRepo(nameVersion, visited); err != nil {
				return err
			}
		}
	}

	// No need to update port if it's not git repo or its code doesn't exist.
	srcDir := filepath.Join(dirs.WorkspaceDir, "buildtrees", nameVersion, "src")
	if !fileio.PathExists(srcDir) {
		return fmt.Errorf("source directory not found: %s/%s/src (has the port been cloned?)",
			filepath.ToSlash(dirs.BuildtreesDir), nameVersion)
	}
	// It may not happen, even archive repo is init as local git repo by celer.
	if !strings.HasSuffix(port.Package.Url, ".git") {
		return fmt.Errorf("%s/%s/src is not a git repository, update is skipped",
			filepath.ToSlash(dirs.BuildtreesDir), nameVersion)
	}

	// Update port.
	title := fmt.Sprintf("[update %s]", nameVersion)
	if err := git.UpdateRepo(title, port.Package.Ref, srcDir, u.force); err != nil {
		return err
	}

	return nil
}

func (u *updateCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string
	var buildtreesDir = dirs.BuildtreesDir

	// Support port completion.
	if fileio.PathExists(buildtreesDir) {
		entities, err := os.ReadDir(buildtreesDir)
		if err != nil {
			return suggestions, cobra.ShellCompDirectiveNoFileComp
		}

		for _, entity := range entities {
			if entity.IsDir() && strings.HasPrefix(entity.Name(), toComplete) {
				suggestions = append(suggestions, entity.Name())
			}
		}
	}

	// Support project completion.
	if fileio.PathExists(dirs.ConfProjectsDir) {
		entities, err := os.ReadDir(dirs.ConfProjectsDir)
		if err != nil {
			// Don't fail completion, just return what we have.
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
	flags := []string{
		"--conf-repo", "-c",
		"--ports-repo", "-p",
		"--recursive", "-r",
		"--force", "-f",
	}
	for _, flag := range flags {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
