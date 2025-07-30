package buildsystems

import (
	"celer/pkgs/cmd"
	"celer/pkgs/fileio"
	"fmt"
	"path/filepath"
)

func NewNoBuild(config *BuildConfig) *nobuild {
	return &nobuild{BuildConfig: config}
}

type nobuild struct {
	*BuildConfig
}

func (n nobuild) CheckTools() error {
	return nil
}

func (n nobuild) CleanRepo() error {
	if fileio.PathExists(filepath.Join(n.PortConfig.RepoDir, ".git")) {
		title := fmt.Sprintf("[clean %s]", n.PortConfig.nameVersionDesc())
		executor := cmd.NewExecutor(title, "git clean -fdx && git reset --hard")
		executor.SetWorkDir(n.PortConfig.RepoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	} else if n.BuildInSource {
		if err := n.replaceSource(n.PortConfig.Archive, n.PortConfig.Url); err != nil {
			return err
		}
	}

	return nil
}

func (n nobuild) Name() string {
	return "nobuild"
}

func (n nobuild) configured() bool {
	return false
}

func (n nobuild) Configure(options []string) error {
	return nil
}

func (n nobuild) Build(options []string) error {
	return nil
}

func (n nobuild) Install(options []string) error {
	return nil
}
