package buildsystems

import (
	"celer/pkgs/cmd"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"strings"
)

func NewGyp(config *BuildConfig) *gyp {
	return &gyp{
		BuildConfig: config,
		buildSystem: config.BuildSystem,
	}
}

type gyp struct {
	*BuildConfig
	buildSystem string
}

func (g gyp) Name() string {
	return "gyp"
}

func (g gyp) CheckTools() []string {
	// Start with build_tools from port.toml
	tools := slices.Clone(g.BuildConfig.BuildTools)

	// Add build tools dynamically.
	_, version, hasVersion := strings.Cut(g.buildSystem, "@")
	if hasVersion && strings.TrimSpace(version) != "" {
		tools = append(tools, "git", "cmake", "ninja", "python3:gyp-next=="+strings.TrimSpace(version))
	} else {
		tools = append(tools, "git", "cmake", "ninja", "python3:gyp-next")
	}
	return tools
}

func (g gyp) configured() bool {
	return false
}

func (g gyp) Configure(options []string) error {
	toolchain := g.Ctx.Platform().GetToolchain()
	rootfs := g.Ctx.Platform().GetRootFS()

	if g.DevDep {
		toolchain.ClearEnvs()
	} else {
		toolchain.SetEnvs(rootfs, g.Name())
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
		return fmt.Errorf("failed to install include -> %w", err)
	}

	if err := fileio.CopyDir(libDir, filepath.Join(g.PortConfig.PackageDir, "lib")); err != nil {
		return fmt.Errorf("failed to install lib -> %w", err)
	}

	if err := fileio.CopyDir(binDir, filepath.Join(g.PortConfig.PackageDir, "bin")); err != nil {
		return fmt.Errorf("failed to install bin -> %w", err)
	}

	return nil
}
