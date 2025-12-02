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

func NewFreeStyle(config *BuildConfig, optimize *context.Optimize) *freeStyle {
	return &freeStyle{
		BuildConfig: config,
		Optimize:    optimize,
	}
}

type freeStyle struct {
	*BuildConfig
	*context.Optimize
}

func (f freeStyle) CheckTools() []string {
	f.BuildTools = append(f.BuildTools, "git", "cmake")
	return f.BuildConfig.BuildTools
}

func (f freeStyle) Name() string {
	return "freestyle"
}

func (f freeStyle) configured() bool {
	return false
}

func (f freeStyle) Configure(options []string) error {
	if len(f.FreeStyleConfigure) > 0 {
		// msvc and clang-cl need to set build environment event in dev mode.
		if f.DevDep &&
			f.PortConfig.Toolchain.Name != "msvc" &&
			f.PortConfig.Toolchain.Name != "clang-cl" {
			f.PortConfig.Toolchain.ClearEnvs()
		} else {
			f.PortConfig.Toolchain.SetEnvs(f.BuildConfig)
		}

		f.setOptimizeFlags()

		// Create build dir if not exists.
		if !f.BuildInSource {
			if err := os.MkdirAll(f.PortConfig.BuildDir, os.ModePerm); err != nil {
				return err
			}
		}

		scripts := strings.Join(f.FreeStyleConfigure, " && ")
		title := fmt.Sprintf("[configure %s]", f.PortConfig.nameVersionDesc())
		executor := cmd.NewExecutor(title, scripts)
		executor.SetLogPath(f.getLogPath("configure"))
		executor.SetWorkDir(expr.If(f.BuildInSource, f.PortConfig.SrcDir, f.PortConfig.BuildDir))
		if err := executor.Execute(); err != nil {
			return err
		}
	}
	return nil
}

func (f freeStyle) Build(options []string) error {
	if len(f.FreeStyleBuild) > 0 {
		// msvc and clang-cl need to set build environment event in dev mode.
		if f.DevDep &&
			f.PortConfig.Toolchain.Name != "msvc" &&
			f.PortConfig.Toolchain.Name != "clang-cl" {
			f.PortConfig.Toolchain.ClearEnvs()
		} else {
			f.PortConfig.Toolchain.SetEnvs(f.BuildConfig)
		}

		// Set optimize flags if Configure wasn't called
		if len(f.FreeStyleConfigure) == 0 {
			f.setOptimizeFlags()
		}

		scripts := strings.Join(f.FreeStyleBuild, " && ")
		scripts = f.expandCommandsVariables(scripts)
		title := fmt.Sprintf("[build %s]", f.PortConfig.nameVersionDesc())
		executor := cmd.NewExecutor(title, scripts)
		executor.SetLogPath(f.getLogPath("build"))
		executor.SetWorkDir(expr.If(f.BuildInSource, f.PortConfig.SrcDir, f.PortConfig.BuildDir))
		if err := executor.Execute(); err != nil {
			return err
		}
	}
	return nil
}

func (f freeStyle) Install(options []string) error {
	if len(f.FreeStyleInstall) > 0 {
		scripts := strings.Join(f.FreeStyleInstall, " && ")
		scripts = f.expandCommandsVariables(scripts)
		title := fmt.Sprintf("[install %s]", f.PortConfig.nameVersionDesc())
		executor := cmd.NewExecutor(title, scripts)
		executor.SetLogPath(f.getLogPath("install"))
		executor.SetWorkDir(expr.If(f.BuildInSource, f.PortConfig.SrcDir, f.PortConfig.BuildDir))
		if err := executor.Execute(); err != nil {
			return err
		}
	}
	return nil
}

func (f freeStyle) setOptimizeFlags() {
	if f.Optimize != nil && runtime.GOOS != "windows" {
		cflags := strings.Fields(os.Getenv("CFLAGS"))
		cxxflags := strings.Fields(os.Getenv("CXXFLAGS"))
		if f.DevDep {
			if f.Optimize.Release != "" {
				cflags = append(cflags, f.Optimize.Release)
				cxxflags = append(cxxflags, f.Optimize.Release)
			}
		} else {
			switch f.BuildType {
			case "release":
				if f.Optimize.Release != "" {
					cflags = append(cflags, f.Optimize.Release)
					cxxflags = append(cxxflags, f.Optimize.Release)
				}
			case "debug":
				if f.Optimize.Debug != "" {
					cflags = append(cflags, f.Optimize.Debug)
					cxxflags = append(cxxflags, f.Optimize.Debug)
				}
			case "relwithdebinfo":
				if f.Optimize.RelWithDebInfo != "" {
					cflags = append(cflags, f.Optimize.RelWithDebInfo)
					cxxflags = append(cxxflags, f.Optimize.RelWithDebInfo)
				}
			case "minsizerel":
				if f.Optimize.MinSizeRel != "" {
					cflags = append(cflags, f.Optimize.MinSizeRel)
					cxxflags = append(cxxflags, f.Optimize.MinSizeRel)
				}
			}
		}
		os.Setenv("CFLAGS", strings.Join(cflags, " "))
		os.Setenv("CXXFLAGS", strings.Join(cxxflags, " "))
	}
}
