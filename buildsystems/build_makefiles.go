package buildsystems

import (
	"celer/buildtools"
	"celer/context"
	"celer/pkgs/cmd"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
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

func (m *makefiles) CheckTools() error {
	if runtime.GOOS == "windows" {
		configureWithPerl := m.shouldConfigureWithPerl()
		tool := expr.If(configureWithPerl, "strawberry-perl", "msys2")
		m.BuildTools = append(m.BuildTools, tool)
	}

	m.BuildTools = append(m.BuildTools, "git", "cmake")
	return buildtools.CheckTools(m.Ctx, m.BuildTools...)
}

func (m makefiles) Clean() error {
	if fileio.PathExists(filepath.Join(m.PortConfig.RepoDir, ".git")) {
		title := fmt.Sprintf("[clean %s]", m.PortConfig.nameVersionDesc())
		if err := git.Clean(title, m.PortConfig.RepoDir); err != nil {
			return err
		}
	} else if m.BuildInSource {
		if err := m.replaceSource(m.PortConfig.Archive, m.PortConfig.Url); err != nil {
			return err
		}
	}

	return nil
}

func (m *makefiles) preConfigure() error {
	// `clang` inside visual studio cannot be used to compile makefiles project.
	if runtime.GOOS == "windows" && strings.Contains(m.PortConfig.Toolchain.Fullpath, "Microsoft Visual Studio") {
		if m.PortConfig.Toolchain.Name != "msvc" {
			return fmt.Errorf("clang-cl or clang inside visual studio cannot be used to compile makefiles project, msvc is required")
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
		command = m.replaceHolders(command)
		executor := cmd.NewExecutor(title, command)
		executor.MSYS2Env(runtime.GOOS == "windows")
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	// If `configure` exists, then `autogen.sh` is unnecessary.
	haveAutogen := fileio.PathExists(filepath.Join(m.PortConfig.SrcDir, "autogen.sh"))
	haveConfigure := fileio.PathExists(filepath.Join(m.PortConfig.SrcDir, "configure"))
	if haveAutogen && !haveConfigure {
		// Disable auto configure by autogen.sh.
		os.Setenv("NOCONFIGURE", "1")

		var autogenCommand = "./autogen.sh"
		if len(m.AutogenOptions) > 0 {
			autogenCommand += " " + strings.Join(m.AutogenOptions, " ")
		}

		configureWithPerl := m.shouldConfigureWithPerl()

		title := fmt.Sprintf("[autogen %s]", m.PortConfig.nameVersionDesc())
		executor := cmd.NewExecutor(title, autogenCommand)
		executor.MSYS2Env(runtime.GOOS == "windows" && !configureWithPerl)
		executor.SetLogPath(m.getLogPath("autogen"))
		executor.SetWorkDir(m.PortConfig.SrcDir)

		// Use msys2 and msvc envs only when in windows and not using perl.
		if runtime.GOOS == "windows" && !configureWithPerl {
			executor.MSYS2Env(true)
			executor.SetMsvcEnvs(m.msvcEnvs)
		}

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
		release := m.DevDep ||
			(m.BuildType == "release" || m.BuildType == "relwithdebinfo" || m.BuildType == "minsizerel")
		options = append(options, expr.If(release, "--release", "--debug"))
	}

	// Remove common cross compile args for native build.
	if m.PortConfig.Toolchain.Native || m.BuildConfig.DevDep {
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
			options = append(options, fmt.Sprintf("--host=%s", m.PortConfig.Toolchain.Host))
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

	// In msys2 or linux, the package path should be fixed to `/c/path1/path2`.
	if runtime.GOOS == "windows" && configureWithPerl {
		options = append(options, fmt.Sprintf("--prefix=%s", m.PortConfig.PackageDir))
	} else {
		options = append(options, fmt.Sprintf("--prefix=%s", fileio.ToCygpath(m.PortConfig.PackageDir)))
	}

	// Replace placeholders.
	for index, value := range options {
		options[index] = m.replaceHolders(value)
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
	makeFile := filepath.Join(m.PortConfig.BuildDir, "Makefile")
	return fileio.PathExists(makeFile)
}

func (m makefiles) Configure(options []string) error {
	// Some libraries may not need to configure.
	configureRequired := m.configureRequired()
	if !configureRequired {
		return nil
	}

	// msvc and clang-cl need to set build environment event in dev mode.
	if m.DevDep &&
		m.PortConfig.Toolchain.Name != "msvc" &&
		m.PortConfig.Toolchain.Name != "clang-cl" {
		m.PortConfig.Toolchain.ClearEnvs()
	} else {
		m.PortConfig.Toolchain.SetEnvs(m.BuildConfig)
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
