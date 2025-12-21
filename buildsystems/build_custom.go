package buildsystems

import (
	"celer/context"
	"celer/pkgs/cmd"
	"celer/pkgs/expr"
	"fmt"
	"os"
	"runtime"
	"strings"
)

func NewCustom(config *BuildConfig, optimize *context.Optimize) *custom {
	return &custom{
		BuildConfig: config,
		Optimize:    optimize,
	}
}

type custom struct {
	*BuildConfig
	*context.Optimize
}

func (c custom) CheckTools() []string {
	c.BuildTools = append(c.BuildTools, "git", "cmake")
	return c.BuildConfig.BuildTools
}

func (c custom) Name() string {
	return "custom"
}

func (c custom) configured() bool {
	return false
}

func (c custom) Configure(options []string) error {
	toolchain := c.Ctx.Platform().GetToolchain()
	rootfs := c.Ctx.Platform().GetRootFS()

	if len(c.CustomConfigure) > 0 {
		// msvc and clang-cl need to set build environment event in dev mode.
		if c.DevDep && toolchain.GetName() != "msvc" && toolchain.GetName() != "clang-cl" {
			toolchain.ClearEnvs()
		} else {
			toolchain.SetEnvs(rootfs, c.Name())
		}

		c.setOptimizeFlags()

		// Create build dir if not exists.
		if !c.BuildInSource {
			if err := os.MkdirAll(c.PortConfig.BuildDir, os.ModePerm); err != nil {
				return err
			}
		}

		scripts := strings.Join(c.CustomConfigure, " && ")
		title := fmt.Sprintf("[configure %s]", c.PortConfig.nameVersionDesc())
		executor := cmd.NewExecutor(title, scripts)
		executor.SetLogPath(c.getLogPath("configure"))
		executor.SetWorkDir(expr.If(c.BuildInSource, c.PortConfig.SrcDir, c.PortConfig.BuildDir))
		if err := executor.Execute(); err != nil {
			return err
		}
	}
	return nil
}

func (c custom) Build(options []string) error {
	toolchain := c.Ctx.Platform().GetToolchain()
	rootfs := c.Ctx.Platform().GetRootFS()

	if len(c.CustomBuild) > 0 {
		// msvc and clang-cl need to set build environment event in dev mode.
		if c.DevDep && toolchain.GetName() != "msvc" && toolchain.GetName() != "clang-cl" {
			toolchain.ClearEnvs()
		} else {
			toolchain.SetEnvs(rootfs, c.Name())
		}

		c.setOptimizeFlags()

		scripts := strings.Join(c.CustomBuild, " && ")
		scripts = c.expandCommandsVariables(scripts)
		title := fmt.Sprintf("[build %s]", c.PortConfig.nameVersionDesc())
		executor := cmd.NewExecutor(title, scripts)
		executor.SetLogPath(c.getLogPath("build"))
		executor.SetWorkDir(expr.If(c.BuildInSource, c.PortConfig.SrcDir, c.PortConfig.BuildDir))
		if err := executor.Execute(); err != nil {
			return err
		}
	}
	return nil
}

func (c custom) Install(options []string) error {
	toolchain := c.Ctx.Platform().GetToolchain()
	rootfs := c.Ctx.Platform().GetRootFS()

	if len(c.CustomInstall) > 0 {
		// msvc and clang-cl need to set build environment event in dev mode.
		if c.DevDep && toolchain.GetName() != "msvc" && toolchain.GetName() != "clang-cl" {
			toolchain.ClearEnvs()
		} else {
			toolchain.SetEnvs(rootfs, c.Name())
		}

		c.setOptimizeFlags()

		scripts := strings.Join(c.CustomInstall, " && ")
		scripts = c.expandCommandsVariables(scripts)
		title := fmt.Sprintf("[install %s]", c.PortConfig.nameVersionDesc())
		executor := cmd.NewExecutor(title, scripts)
		executor.SetLogPath(c.getLogPath("install"))
		executor.SetWorkDir(expr.If(c.BuildInSource, c.PortConfig.SrcDir, c.PortConfig.BuildDir))
		if err := executor.Execute(); err != nil {
			return err
		}
	}
	return nil
}

func (c custom) setOptimizeFlags() {
	if c.Optimize != nil && runtime.GOOS != "windows" {
		cflags := strings.Fields(os.Getenv("CFLAGS"))
		cxxflags := strings.Fields(os.Getenv("CXXFLAGS"))
		if c.DevDep {
			if c.Optimize.Release != "" {
				cflags = append(cflags, c.Optimize.Release)
				cxxflags = append(cxxflags, c.Optimize.Release)
			}
		} else {
			switch c.BuildType {
			case "release":
				if c.Optimize.Release != "" {
					cflags = append(cflags, c.Optimize.Release)
					cxxflags = append(cxxflags, c.Optimize.Release)
				}
			case "debug":
				if c.Optimize.Debug != "" {
					cflags = append(cflags, c.Optimize.Debug)
					cxxflags = append(cxxflags, c.Optimize.Debug)
				}
			case "relwithdebinfo":
				if c.Optimize.RelWithDebInfo != "" {
					cflags = append(cflags, c.Optimize.RelWithDebInfo)
					cxxflags = append(cxxflags, c.Optimize.RelWithDebInfo)
				}
			case "minsizerel":
				if c.Optimize.MinSizeRel != "" {
					cflags = append(cflags, c.Optimize.MinSizeRel)
					cxxflags = append(cxxflags, c.Optimize.MinSizeRel)
				}
			}
		}
		os.Setenv("CFLAGS", strings.Join(cflags, " "))
		os.Setenv("CXXFLAGS", strings.Join(cxxflags, " "))
	}
}
