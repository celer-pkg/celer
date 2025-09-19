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

func NewGyp(config *BuildConfig, optimize Optimize) *gyp {
	return &gyp{
		BuildConfig: config,
		Optimize:    optimize,
	}
}

type gyp struct {
	*BuildConfig
	Optimize
}

func (g gyp) Name() string {
	return "gyp"
}

func (g gyp) CheckTools() error {
	g.BuildConfig.BuildTools = append(g.BuildConfig.BuildTools, "git", "cmake", "python3:gyp-next", "ninja")
	return buildtools.CheckTools(g.BuildConfig.BuildTools...)
}

func (g gyp) Clean() error {
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
	if g.DevDep {
		g.PortConfig.CrossTools.ClearEnvs()
	} else {
		g.PortConfig.CrossTools.SetEnvs(g.BuildConfig)
	}

	// Set optimization flags with build_type.
	cflags := strings.Split(os.Getenv("CFLAGS"), " ")
	cxxflags := strings.Split(os.Getenv("CXXFLAGS"), " ")
	if g.DevDep {
		if g.Optimize.Release != "" {
			cflags = append(cflags, g.Optimize.Release)
			cxxflags = append(cxxflags, g.Optimize.Release)
		}
	} else {
		buildType := strings.ToLower(g.BuildType)
		switch buildType {
		case "release":
			if g.Optimize.Release != "" {
				cflags = append(cflags, g.Optimize.Release)
				cxxflags = append(cxxflags, g.Optimize.Release)
			}
		case "debug":
			if g.Optimize.Debug != "" {
				cflags = append(cflags, g.Optimize.Debug)
				cxxflags = append(cxxflags, g.Optimize.Debug)
			}
		case "relwithdebinfo":
			if g.Optimize.RelWithDebInfo != "" {
				cflags = append(cflags, g.Optimize.RelWithDebInfo)
				cxxflags = append(cxxflags, g.Optimize.RelWithDebInfo)
			}
		case "minsizerel":
			if g.Optimize.MinSizeRel != "" {
				cflags = append(cflags, g.Optimize.MinSizeRel)
				cxxflags = append(cxxflags, g.Optimize.MinSizeRel)
			}
		}
	}
	os.Setenv("CFLAGS", strings.Join(cflags, " "))
	os.Setenv("CXXFLAGS", strings.Join(cxxflags, " "))

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
