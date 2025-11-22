package buildsystems

import (
	"celer/context"
	"celer/pkgs/cmd"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
)

func NewGyp(config *BuildConfig, optimize *context.Optimize) *gyp {
	return &gyp{
		BuildConfig: config,
		Optimize:    optimize,
	}
}

type gyp struct {
	*BuildConfig
	*context.Optimize
}

func (g gyp) Name() string {
	return "gyp"
}

func (g gyp) CheckTools() []string {
	g.BuildTools = append(g.BuildTools, "git", "cmake", "python3:gyp-next", "ninja")
	return g.BuildConfig.BuildTools
}

func (g gyp) configured() bool {
	return false
}

func (g gyp) Configure(options []string) error {
	if g.DevDep {
		g.PortConfig.Toolchain.ClearEnvs()
	} else {
		g.PortConfig.Toolchain.SetEnvs(g.BuildConfig)
	}

	// Set optimization flags with build_type.
	if g.Optimize != nil && runtime.GOOS != "windows" {
		cflags := strings.Fields(os.Getenv("CFLAGS"))
		cxxflags := strings.Fields(os.Getenv("CXXFLAGS"))
		if g.DevDep {
			if g.Optimize.Release != "" {
				cflags = append(cflags, g.Optimize.Release)
				cxxflags = append(cxxflags, g.Optimize.Release)
			}
		} else {
			switch g.BuildType {
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
		return fmt.Errorf("failed to install include.\n %w", err)
	}

	if err := fileio.CopyDir(libDir, filepath.Join(g.PortConfig.PackageDir, "lib")); err != nil {
		return fmt.Errorf("failed to install lib.\n %w", err)
	}

	if err := fileio.CopyDir(binDir, filepath.Join(g.PortConfig.PackageDir, "bin")); err != nil {
		return fmt.Errorf("failed to install bin.\n %w", err)
	}

	return nil
}
