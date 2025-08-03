package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/pkgs/cmd"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type updateCmd struct {
	ctx       configs.Context
	confRepo  bool
	portsRepo bool
	recurse   bool
	force     bool
}

func (u updateCmd) Command() *cobra.Command {
	command := &cobra.Command{
		Use:   "update",
		Short: "Update conf repo, ports repo and port source.",
		Run: func(cmd *cobra.Command, args []string) {
			// Init celer.
			celer := configs.NewCeler()
			if err := celer.Init(); err != nil {
				configs.PrintError(err, "failed to init celer.")
				return
			}
			u.ctx = celer

			// Make sure git is available.
			if err := buildtools.CheckTools("git"); err != nil {
				configs.PrintError(err, "failed to check git tools.")
				return
			}

			if u.confRepo {
				if err := u.updateConfRepo(); err != nil {
					configs.PrintError(err, "failed to update conf repo.")
				}
			} else if u.portsRepo {
				if err := u.updatePortsRepo(); err != nil {
					configs.PrintError(err, "failed to update ports repo.")
				}
			} else {
				if err := u.updatePorts(args); err != nil {
					configs.PrintError(err, "failed to update port repo.")
				}
			}
		},
		ValidArgsFunction: u.completion,
	}

	// Register flags.
	command.Flags().BoolVarP(&u.confRepo, "conf-repo", "c", false, "update conf repo")
	command.Flags().BoolVarP(&u.portsRepo, "ports-repo", "p", false, "update ports repo")
	command.Flags().BoolVarP(&u.force, "force", "f", false, "update forcibly")
	command.Flags().BoolVarP(&u.recurse, "recurse", "r", false, "update recursively")

	command.MarkFlagsMutuallyExclusive("conf-repo", "ports-repo")
	return command
}

func (u updateCmd) updateConfRepo() error {
	title := "[update conf repo]"
	repoDir := filepath.Join(dirs.WorkspaceDir, "conf")
	return cmd.UpdateRepo(title, repoDir, "", u.force)
}

func (u updateCmd) updatePortsRepo() error {
	title := "[update ports repo]"
	repoDir := filepath.Join(dirs.WorkspaceDir, "ports")
	return cmd.UpdateRepo(title, repoDir, "", u.force)
}

func (u updateCmd) updatePorts(targets []string) error {
	if len(targets) == 0 {
		return fmt.Errorf("no ports specified to update")
	}

	for _, target := range targets {
		target = strings.ReplaceAll(target, "`", "")
		if err := u.updatePortRepo(target); err != nil {
			return err
		}
	}

	return nil
}

func (u updateCmd) updatePortRepo(nameVersion string) error {
	// Read port file.
	var port configs.Port
	if err := port.Init(u.ctx, nameVersion, "release"); err != nil {
		return fmt.Errorf("%s: %w", nameVersion, err)
	}

	// Update repos of port's depedencies.
	if u.recurse {
		for _, nameVersion := range port.MatchedConfig.Dependencies {
			if err := u.updatePortRepo(nameVersion); err != nil {
				return err
			}
		}
	}

	// No need to update port if it's not git repo or its code is not exist.
	srcDir := filepath.Join(dirs.WorkspaceDir, "buildtrees", nameVersion, "src")
	if !fileio.PathExists(srcDir) {
		return fmt.Errorf("%s/%s/src is not found, update is skipped", filepath.ToSlash(dirs.BuildtreesDir), nameVersion)
	}
	if !strings.HasSuffix(port.Package.Url, ".git") {
		return fmt.Errorf("%s/%s/src is not git repo, update is skipped", filepath.ToSlash(dirs.BuildtreesDir), nameVersion)
	}

	// Update port.
	title := fmt.Sprintf("[update %s]", nameVersion)
	if err := cmd.UpdateRepo(title, srcDir, port.Package.Ref, u.force); err != nil {
		return err
	}

	return nil
}

func (u updateCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
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
	flags := []string{
		"--conf-repo", "-c",
		"--ports-repo", "-p",
		"--recurse", "-r",
		"--force", "-f",
	}
	for _, flag := range flags {
		if strings.HasPrefix(flag, toComplete) {
			suggestions = append(suggestions, flag)
		}
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
