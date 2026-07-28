package buildsystems

import (
	"fmt"
	"os"
	"slices"
	"strings"

	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/expr"
)

func NewCustom(config *BuildConfig) *custom {
	return &custom{
		BuildConfig: config,
	}
}

type custom struct {
	*BuildConfig
}

func (c custom) CheckTools() []string {
	// Start with build_tools from port.toml
	tools := slices.Clone(c.BuildConfig.BuildTools)

	// Add default tools
	tools = append(tools, "cmake")

	// Auto-detect Python usage from custom commands to ensure the venv is set up.
	if c.needsPython() && !slices.Contains(tools, "python3") {
		tools = append(tools, "python3")
	}
	return tools
}

// needsPython checks if any custom command references Python placeholders.
func (c custom) needsPython() bool {
	commands := append(c.CustomConfigure, c.CustomBuild...)
	commands = append(commands, c.CustomInstall...)

	for _, command := range commands {
		if strings.Contains(command, "${PYTHON_VENV_EXE}") ||
			strings.Contains(command, "${PYTHON_VENV_DIR}") ||
			strings.Contains(command, "${PYTHON_PATH}") {
			return true
		}
	}
	return false
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
		if (c.DevDep || c.HostDev) && toolchain.GetName() != "msvc" && toolchain.GetName() != "clang-cl" {
			toolchain.ClearEnvs()
		} else {
			toolchain.SetEnvs(rootfs, c.Name(), c.Envs)
		}

		// Create build dir if not exists.
		if !c.BuildInSource {
			if err := os.MkdirAll(c.PortConfig.BuildDir, os.ModePerm); err != nil {
				return err
			}
		}

		scripts := strings.Join(c.CustomConfigure, " && ")
		scripts = c.expandVariables(scripts)
		title := fmt.Sprintf("[configure %s]", c.PortConfig.nameVersion())
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
		if (c.DevDep || c.HostDev) && toolchain.GetName() != "msvc" && toolchain.GetName() != "clang-cl" {
			toolchain.ClearEnvs()
		} else {
			toolchain.SetEnvs(rootfs, c.Name(), c.Envs)
		}

		scripts := strings.Join(c.CustomBuild, " && ")
		scripts = c.expandVariables(scripts)
		title := fmt.Sprintf("[build %s]", c.PortConfig.nameVersion())
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
		if (c.DevDep || c.HostDev) && toolchain.GetName() != "msvc" && toolchain.GetName() != "clang-cl" {
			toolchain.ClearEnvs()
		} else {
			toolchain.SetEnvs(rootfs, c.Name(), c.Envs)
		}

		scripts := strings.Join(c.CustomInstall, " && ")
		scripts = c.expandVariables(scripts)
		title := fmt.Sprintf("[install %s]", c.PortConfig.nameVersion())
		executor := cmd.NewExecutor(title, scripts)
		executor.SetLogPath(c.getLogPath("install"))
		executor.SetWorkDir(expr.If(c.BuildInSource, c.PortConfig.SrcDir, c.PortConfig.BuildDir))
		if err := executor.Execute(); err != nil {
			return err
		}
	}
	return nil
}
