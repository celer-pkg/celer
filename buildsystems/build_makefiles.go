package buildsystems

import (
	"celer/context"
	"celer/pkgs/cmd"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
)

func NewMakefiles(config *BuildConfig, optimize *context.Optimize) *makefiles {
	return &makefiles{
		BuildConfig: config,
		Optimize:    optimize,
	}
}

type makefiles struct {
	*BuildConfig
	*context.Optimize
	msvcEnvs string
}

func (makefiles) Name() string {
	return "makefiles"
}

func (m *makefiles) CheckTools() []string {
	if runtime.GOOS == "windows" {
		configureWithPerl := m.shouldConfigureWithPerl()
		tool := expr.If(configureWithPerl, "strawberry-perl", "msys2")
		m.BuildTools = append(m.BuildTools, tool)
	}

	m.BuildTools = append(m.BuildTools, "git", "cmake")
	return m.BuildConfig.BuildTools
}

func (m *makefiles) preConfigure() error {
	toolchain := m.Ctx.Platform().GetToolchain()

	// `clang` inside visual studio cannot be used to compile makefiles project.
	if runtime.GOOS == "windows" && strings.Contains(toolchain.GetFullPath(), "Microsoft Visual Studio") {
		if toolchain.GetName() != "msvc" {
			return fmt.Errorf("visual studio's clang-cl or clang cannot be used to compile makefiles project, only msvc is supported")
		}
	}

	// Cache MSVC envs.
	if runtime.GOOS == "windows" {
		msvcEnvs, err := m.BuildConfig.msvcEnvs()
		if err != nil {
			return err
		}
		m.msvcEnvs = msvcEnvs
	}

	// Execute pre configure scripts.
	for _, command := range m.PreConfigure {
		command = strings.TrimSpace(command)
		if command == "" {
			continue
		}

		title := fmt.Sprintf("[post confiure %s]", m.PortConfig.nameVersionDesc())
		command = m.expandCommandsVariables(command)
		executor := cmd.NewExecutor(title, command)
		executor.SetWorkDir(m.PortConfig.RepoDir)
		executor.MSYS2Env(runtime.GOOS == "windows")
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

func (m makefiles) configureOptions() ([]string, error) {
	var options = slices.Clone(m.Options)
	configureWithPerl := m.shouldConfigureWithPerl()
	if configureWithPerl {
		release := m.DevDep || (m.BuildType == "release" || m.BuildType == "relwithdebinfo" || m.BuildType == "minsizerel")
		options = append(options, expr.If(release, "--release", "--debug"))
	}

	// Remove common cross compile args for native build.
	if m.PortConfig.Native || m.BuildConfig.DevDep {
		options = slices.DeleteFunc(options, func(element string) bool {
			return strings.HasPrefix(element, "--host=") ||
				strings.HasPrefix(element, "--sysroot=") ||
				strings.HasPrefix(element, "--arch=") ||
				strings.HasPrefix(element, "--cross-prefix=") ||
				strings.HasPrefix(element, "--target-os") ||
				strings.HasPrefix(element, "--build=") ||
				strings.HasPrefix(element, "--with-build-python=") ||
				strings.HasPrefix(element, "--enable-cross-compile")
		})
	} else {
		if m.shouldAddHost(options) {
			toolchain := m.Ctx.Platform().GetToolchain()
			options = append(options, fmt.Sprintf("--host=%s", toolchain.GetHost()))
		}
	}

	// Set build library type.
	switch m.BuildConfig.BuildShared {
	case "no":
		m.BuildConfig.BuildShared = ""
	case "":
		m.BuildConfig.BuildShared = "--enable-shared"
	}

	switch m.BuildConfig.BuildStatic {
	case "no":
		m.BuildConfig.BuildStatic = ""
	case "":
		m.BuildConfig.BuildStatic = "--enable-static"
	}

	libraryType := m.libraryType(
		m.BuildConfig.BuildShared,
		m.BuildConfig.BuildStatic,
	)
	switch m.BuildConfig.LibraryType {
	case "shared", "": // default is `shared`.
		if libraryType.enableShared != "" {
			options = append(options, libraryType.enableShared)
		}
		if libraryType.disableStatic != "" {
			options = append(options, libraryType.disableStatic)
		}
	case "static":
		if libraryType.enableStatic != "" {
			options = append(options, libraryType.enableStatic)
		}
		if libraryType.disableShared != "" {
			options = append(options, libraryType.disableShared)
		}
	}

	// Add ccache support for projects that need explicit --cc parameter, like ffmpeg.
	if m.Ctx.CCacheEnabled() {
		for index, option := range options {
			if after, ok := strings.CutPrefix(option, "--cc="); ok {
				options[index] = fmt.Sprintf("--cc='ccache %s'", after)
			}
		}
		for index, option := range options {
			if after, ok := strings.CutPrefix(option, "--cxx="); ok {
				options[index] = fmt.Sprintf("--cxx='ccache %s'", after)
			}
		}
	}

	// In msys2 or linux, the package path should be fixed to `/c/path1/path2`.
	if runtime.GOOS == "windows" && configureWithPerl {
		options = append(options, fmt.Sprintf("--prefix=%s", m.PortConfig.PackageDir))
	} else {
		options = append(options, fmt.Sprintf("--prefix=%s", fileio.ToCygpath(m.PortConfig.PackageDir)))
	}

	// Replace placeholders.
	for index, value := range options {
		options[index] = m.expandCommandsVariables(value)
	}

	return options, nil
}

func (m makefiles) shouldAddHost(options []string) bool {
	if m.shouldConfigureWithPerl() {
		return false
	}

	if slices.ContainsFunc(options, func(element string) bool {
		return strings.HasPrefix(element, "--host=")
	}) {
		return false
	}

	var (
		hasArch     bool
		hasTargetOS bool
	)

	// `--arch`` and `--target-os`` have the same function as `--host`
	for _, option := range options {
		if strings.HasPrefix(option, "--arch=") {
			hasArch = true
			if hasArch && hasTargetOS {
				return false
			}
		}
		if strings.HasPrefix(option, "--target-os=") {
			hasTargetOS = true
			if hasArch && hasTargetOS {
				return false
			}
		}
	}

	return true
}

func (m makefiles) configured() bool {
	buildDir := expr.If(m.BuildInSource, m.PortConfig.RepoDir, m.PortConfig.BuildDir)
	makeFile := filepath.Join(buildDir, "Makefile")
	return fileio.PathExists(m.PortConfig.RepoDir) &&
		fileio.PathExists(makeFile)
}

func (m makefiles) Configure(options []string) error {
	// Some libraries may not need to configure.
	configureRequired := m.configureRequired()
	if !configureRequired {
		return nil
	}

	// msvc and clang-cl need to set build environment event in dev mode.
	toolchain := m.Ctx.Platform().GetToolchain()
	rootfs := m.Ctx.Platform().GetRootFS()
	if m.DevDep && toolchain.GetName() != "msvc" && toolchain.GetName() != "clang-cl" {
		toolchain.ClearEnvs()
	} else {
		toolchain.SetEnvs(rootfs, m.Name())
	}

	// Set optimization flags with build_type.
	if m.Optimize != nil && runtime.GOOS != "windows" {
		cflags := strings.Fields(os.Getenv("CFLAGS"))
		cxxflags := strings.Fields(os.Getenv("CXXFLAGS"))
		if m.DevDep {
			if m.Optimize.Release != "" {
				cflags = append(cflags, m.Optimize.Release)
				cxxflags = append(cxxflags, m.Optimize.Release)
			}
		} else {
			switch m.BuildType {
			case "release":
				if m.Optimize.Release != "" {
					cflags = append(cflags, m.Optimize.Release)
					cxxflags = append(cxxflags, m.Optimize.Release)
				}
			case "debug":
				if m.Optimize.Debug != "" {
					cflags = append(cflags, m.Optimize.Debug)
					cxxflags = append(cxxflags, m.Optimize.Debug)
				}
			case "relwithdebinfo":
				if m.Optimize.RelWithDebInfo != "" {
					cflags = append(cflags, m.Optimize.RelWithDebInfo)
					cxxflags = append(cxxflags, m.Optimize.RelWithDebInfo)
				}
			case "minsizerel":
				if m.Optimize.MinSizeRel != "" {
					cflags = append(cflags, m.Optimize.MinSizeRel)
					cxxflags = append(cxxflags, m.Optimize.MinSizeRel)
				}
			}
		}
		os.Setenv("CFLAGS", strings.Join(cflags, " "))
		os.Setenv("CXXFLAGS", strings.Join(cxxflags, " "))
	}

	// Create build dir if not exists.
	if !m.BuildInSource {
		if err := os.MkdirAll(m.PortConfig.BuildDir, os.ModePerm); err != nil {
			return err
		}
	}

	configureWithPerl := m.shouldConfigureWithPerl()

	// Find `configure` or `Configure`.
	configureFile := expr.If(configureWithPerl, "Configure", "configure")

	// Asssemble configure command.
	joinedOptions := strings.Join(options, " ")
	command := fmt.Sprintf("%s/%s %s", m.PortConfig.SrcDir, configureFile, joinedOptions)
	if runtime.GOOS == "windows" {
		command = expr.If(configureWithPerl, fmt.Sprintf("perl %s", command), fileio.ToCygpath(command))
	}

	title := fmt.Sprintf("[configure %s]", m.PortConfig.nameVersionDesc())
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(m.getLogPath("configure"))
	executor.SetWorkDir(expr.If(m.BuildInSource, m.PortConfig.SrcDir, m.PortConfig.BuildDir))

	// Use msys2 and msvc env only when in windows and not using perl.
	if runtime.GOOS == "windows" && !configureWithPerl {
		executor.MSYS2Env(true)
		executor.SetMsvcEnvs(m.msvcEnvs)
	}
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (m makefiles) buildOptions() ([]string, error) {
	return nil, nil
}

func (m makefiles) Build(options []string) error {
	configureWithPerl := m.shouldConfigureWithPerl()

	// Assemble command.
	var command string
	if runtime.GOOS == "windows" {
		command = expr.If(configureWithPerl, "nmake", fmt.Sprintf("make -j %d", m.PortConfig.Jobs))
	} else {
		command = fmt.Sprintf("make -j %d", m.PortConfig.Jobs)
	}

	// Execute build.
	title := fmt.Sprintf("[build %s]", m.PortConfig.nameVersionDesc())
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(m.getLogPath("build"))

	// Use msys2 and msvc envs only when in windows and not using perl.
	if runtime.GOOS == "windows" && !configureWithPerl {
		executor.MSYS2Env(true)
		executor.SetMsvcEnvs(m.msvcEnvs)
	}

	if !m.configureRequired() || m.BuildInSource {
		executor.SetWorkDir(m.PortConfig.SrcDir)
	} else {
		executor.SetWorkDir(m.PortConfig.BuildDir)
	}

	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (m makefiles) Install(options []string) error {
	configureWithPerl := m.shouldConfigureWithPerl()
	makeCommand := expr.If(runtime.GOOS == "windows" && configureWithPerl, "nmake", "make")

	// Assemble command.
	var command string
	if m.configureRequired() {
		command = fmt.Sprintf("%s install", makeCommand)
	} else {
		command = fmt.Sprintf("make install -C %s prefix=%s", m.PortConfig.SrcDir, m.PortConfig.PackageDir)
	}

	// Execute install.
	title := fmt.Sprintf("[install %s]", m.PortConfig.nameVersionDesc())
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(m.getLogPath("install"))

	// Use msys2 and msvc envs only when in windows and not using perl.
	if runtime.GOOS == "windows" && !configureWithPerl {
		executor.MSYS2Env(true)
		executor.SetMsvcEnvs(m.msvcEnvs)
	}

	if !m.configureRequired() || m.BuildInSource {
		executor.SetWorkDir(m.PortConfig.SrcDir)
	} else {
		executor.SetWorkDir(m.PortConfig.BuildDir)
	}

	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (m makefiles) configureRequired() bool {
	return fileio.PathExists(m.PortConfig.SrcDir+"/configure") ||
		fileio.PathExists(m.PortConfig.SrcDir+"/Configure") ||
		fileio.PathExists(m.PortConfig.SrcDir+"/autogen.sh")
}

func (m makefiles) shouldConfigureWithPerl() bool {
	// Some libraries should be configured with perl, such as openssl.
	entities, err := os.ReadDir(m.PortConfig.SrcDir)
	if err != nil {
		return false
	}
	for _, entity := range entities {
		if entity.IsDir() {
			continue
		}
		if entity.Name() == "Configure" {
			return true
		}
	}

	return false
}
