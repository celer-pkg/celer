package buildsystems

import (
	"celer/buildtools"
	"celer/pkgs/cmd"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

func NewGyp(config *BuildConfig) *gyp {
	return &gyp{BuildConfig: config}
}

type gyp struct {
	*BuildConfig
}

func (g gyp) Name() string {
	return "gyp"
}

func (g gyp) CheckTools() error {
	g.BuildConfig.BuildTools = append(g.BuildConfig.BuildTools, "git", "cmake", "python3:gyp-next", "ninja")
	return buildtools.CheckTools(g.BuildConfig.BuildTools...)
}

func (g gyp) CleanRepo() error {
	if fileio.PathExists(filepath.Join(g.PortConfig.RepoDir, ".git")) {
		title := fmt.Sprintf("[clean %s]", g.PortConfig.nameVersionDesc())
		executor := cmd.NewExecutor(title, "git clean -fdx && git reset --hard")
		executor.SetWorkDir(g.PortConfig.RepoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	} else {
		// gyp build in source, so we need to replace source with archive.
		if err := g.replaceSource(g.PortConfig.Archive, g.PortConfig.Url); err != nil {
			return err
		}
	}

	// Remove dist inside source folder.
	if err := os.RemoveAll(filepath.Join(g.PortConfig.RepoDir, "dist")); err != nil {
		return err
	}

	return nil
}

func (g gyp) configured() bool {
	return false
}

func (g gyp) Configure(options []string) error {
	// Set cross tool environment Variables.
	if g.DevDep {
		g.PortConfig.CrossTools.ClearEnvs()
	} else {
		g.PortConfig.CrossTools.SetEnvs(g.BuildConfig)
	}

	return nil
}

func (g gyp) Build(options []string) error {
	// Remove dist dir.
	distDir := filepath.Join(filepath.Dir(g.PortConfig.RepoDir), "dist")
	if err := os.RemoveAll(distDir); err != nil {
		return err
	}

	joinedOptions := strings.Join(g.Options, " ")

	// Execute build.
	logPath := g.getLogPath("build")
	title := fmt.Sprintf("[build %s@%s]", g.PortConfig.LibName, g.PortConfig.LibVersion)
	executor := cmd.NewExecutor(title, "./build.sh "+joinedOptions)
	executor.SetLogPath(logPath)
	executor.SetWorkDir(g.PortConfig.SrcDir)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (g gyp) Install(options []string) error {
	buildTreesRootDir := filepath.Dir(g.PortConfig.SrcDir)
	headerDir := filepath.Join(buildTreesRootDir, "dist", "public")
	libDir := filepath.Join(buildTreesRootDir, "dist", "Debug", "lib")
	binDir := filepath.Join(buildTreesRootDir, "dist", "Debug", "bin")

	if err := fileio.CopyDir(headerDir, filepath.Join(g.PortConfig.PackageDir, "include")); err != nil {
		return fmt.Errorf("install include error: %w", err)
	}

	if err := fileio.CopyDir(libDir, filepath.Join(g.PortConfig.PackageDir, "lib")); err != nil {
		return fmt.Errorf("install lib error: %w", err)
	}

	if err := fileio.CopyDir(binDir, filepath.Join(g.PortConfig.PackageDir, "bin")); err != nil {
		return fmt.Errorf("install bin error: %w", err)
	}

	return nil
}
