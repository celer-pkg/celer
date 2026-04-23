package buildsystems

import (
	"fmt"
	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"os"
	"path/filepath"
	"slices"
	"strings"
)

func NewQMake(config *BuildConfig) *qmake {
	return &qmake{BuildConfig: config}
}

type qmake struct {
	*BuildConfig
}

func (qmake) Name() string {
	return "qmake"
}

func (q *qmake) CheckTools() []string {
	// Start with build_tools from port.toml
	tools := slices.Clone(q.BuildConfig.BuildTools)

	// Add default tools
	tools = append(tools, "cmake")
	return tools
}

func (q qmake) preConfigure() error {
	// Execute pre configure scripts.
	for _, script := range q.PreConfigure {
		script = strings.TrimSpace(script)
		if script == "" {
			continue
		}

		title := fmt.Sprintf("[post confiure %s]", q.PortConfig.nameVersion())
		script = q.expandVariables(script)
		executor := cmd.NewExecutor(title, script)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

func (q qmake) configureOptions() ([]string, error) {
	var options = slices.Clone(q.Options)

	// Remove common cross compile args for native build.
	rootfs := q.Ctx.Platform().GetRootFS()
	if q.PortConfig.HostDev || q.BuildConfig.DevDep {
		options = slices.DeleteFunc(options, func(element string) bool {
			return strings.Contains(element, "-sysroot")
		})
	} else if rootfs != nil {
		options = append(options, "-sysroot "+rootfs.GetAbsDir())
	}

	// Set installation directory.
	options = append(options, "-extprefix "+q.PortConfig.PackageDir)

	// Set build library type.
	libraryType := q.libraryType("-shared", "-static")
	switch q.BuildConfig.LibraryType {
	case "shared", "": // default is `shared`.
		options = append(options, libraryType.enableShared)
		if libraryType.disableStatic != "" {
			options = append(options, libraryType.disableStatic)
		}
	case "static":
		options = append(options, libraryType.enableStatic)
		if libraryType.disableShared != "" {
			options = append(options, libraryType.disableShared)
		}
	}

	options = append(options, fmt.Sprintf("--prefix=%s", q.PortConfig.PackageDir))

	// Replace placeholders.
	for index, value := range options {
		options[index] = q.expandVariables(value)
	}

	return options, nil
}

func (q qmake) configured() bool {
	makeFile := filepath.Join(q.PortConfig.BuildDir, "Makefile")
	return fileio.PathExists(q.PortConfig.RepoDir) && fileio.PathExists(makeFile)
}

func (q qmake) Configure(options []string) error {
	toolchain := q.Ctx.Platform().GetToolchain()
	rootfs := q.Ctx.Platform().GetRootFS()

	// Host-side dev dependencies must not inherit the target toolchain's
	// CC/CXX/AR/... from previously built cross packages. Otherwise QMake
	// may lock onto a target compiler (for example aarch64-linux-gnu-g++)
	// and produce an un-runnable host tool inside x86_64-linux-dev.
	if q.DevDep || toolchain.GetName() == "msvc" || toolchain.GetName() == "clang" {
		toolchain.ClearEnvs()
	} else {
		toolchain.SetEnvs(rootfs, q.Name())
	}

	// Create build dir if not exists.
	if !q.BuildInSource {
		if err := os.MkdirAll(q.PortConfig.BuildDir, os.ModePerm); err != nil {
			return err
		}
	}

	// Asssemble configure command.
	joinedOptions := strings.Join(options, " ")
	command := fmt.Sprintf("%s/configure %s", q.PortConfig.SrcDir, joinedOptions)
	title := fmt.Sprintf("[configure %s]", q.PortConfig.nameVersion())
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(q.getLogPath("configure"))
	executor.SetWorkDir(expr.If(q.BuildInSource, q.PortConfig.SrcDir, q.PortConfig.BuildDir))
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (q qmake) buildOptions() ([]string, error) {
	return nil, nil
}

func (q qmake) Build(options []string) error {
	// Assemble command.
	command := fmt.Sprintf("make -j %d", q.PortConfig.Jobs)

	// Execute build.
	title := fmt.Sprintf("[build %s]", q.PortConfig.nameVersion())
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(q.getLogPath("build"))
	executor.SetWorkDir(q.PortConfig.BuildDir)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (q qmake) Install(options []string) error {
	// Execute install.
	title := fmt.Sprintf("[install %s]", q.PortConfig.nameVersion())
	executor := cmd.NewExecutor(title, "make install")
	executor.SetLogPath(q.getLogPath("install"))
	executor.SetWorkDir(q.PortConfig.BuildDir)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}
