package buildsystems

import (
	"bytes"
	"celer/context"
	"celer/pkgs/cmd"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
)

func NewMeson(config *BuildConfig, optimize *context.Optimize) *meson {
	return &meson{
		BuildConfig: config,
		Optimize:    optimize,
	}
}

type meson struct {
	*BuildConfig
	*context.Optimize
	msvcEnvs map[string]string
}

func (meson) Name() string {
	return "meson"
}

func (m meson) CheckTools() []string {
	m.BuildTools = append(m.BuildTools, "git", "ninja", "cmake")

	switch runtime.GOOS {
	case "windows":
		m.BuildTools = append(m.BuildTools,
			"python3",
			"python3:meson",
		)
	case "linux":
		m.BuildTools = append(m.BuildTools, "python3:meson")
	}

	return m.BuildConfig.BuildTools
}

func (m *meson) preConfigure() error {
	toolchain := m.Ctx.Platform().GetToolchain()

	// For MSVC build, we need to set PATH, INCLUDE and LIB env vars.
	if runtime.GOOS == "windows" {
		if toolchain.GetName() == "msvc" || toolchain.GetName() == "clang-cl" {
			msvcEnvs, err := m.readMSVCEnvs()
			if err != nil {
				return err
			}
			m.msvcEnvs = msvcEnvs

			os.Setenv("PATH", msvcEnvs["PATH"])
			os.Setenv("INCLUDE", msvcEnvs["INCLUDE"])
			os.Setenv("LIB", msvcEnvs["LIB"])
		}
	}

	return nil
}

func (m meson) configureOptions() ([]string, error) {
	var options = slices.Clone(m.Options)

	// Set build type.
	if m.DevDep {
		options = append(options, "--buildtype=release")
	} else {
		switch m.BuildType {
		case "release":
			options = append(options, "--buildtype=release")
		case "debug":
			options = append(options, "--buildtype=debug")
		case "relwithdebinfo":
			options = append(options, "--buildtype=debugoptimized")
		case "minsizerel":
			options = append(options, "--buildtype=minsize")
		default:
			options = append(options, "--buildtype=plain")
		}
	}

	// Set install library output dir as "dir" always.
	options = append(options, "-Dlibdir=lib")

	// Set build library type.
	libraryType := m.libraryType(
		"--default-library=shared",
		"--default-library=static",
	)
	switch m.BuildConfig.LibraryType {
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

	// Set install dir.
	options = append(options, "--prefix="+m.PortConfig.PackageDir)

	// Replace placeholders.
	for index, value := range options {
		options[index] = m.expandCommandsVariables(value)
	}

	return options, nil
}

func (m meson) configured() bool {
	buildFile := filepath.Join(m.PortConfig.BuildDir, "build.ninja")
	return fileio.PathExists(m.PortConfig.RepoDir) && fileio.PathExists(buildFile)
}

func (m meson) Configure(options []string) error {
	toolchain := m.Ctx.Platform().GetToolchain()
	rootfs := m.Ctx.Platform().GetRootFS()

	// In windows, we set msvc related environments.
	if m.DevDep && (toolchain.GetName() != "msvc" && toolchain.GetName() != "clang-cli") {
		toolchain.ClearEnvs()
	} else {
		toolchain.SetEnvs(rootfs, m.Name(), m.Ctx.CCacheEnabled())
	}

	// Create build dir if not exists.
	if err := os.MkdirAll(m.PortConfig.BuildDir, os.ModePerm); err != nil {
		return err
	}

	// Assemble command.
	var targetToolchain context.Toolchain
	if m.BuildConfig.DevDep {
		targetToolchain = nativeToolchain{
			msvc: toolchain.GetMSVC(),
		}
	} else {
		targetToolchain = toolchain
	}
	crossFile, err := m.generateCrossFile(targetToolchain, rootfs, m.Ctx.CCacheEnabled())
	if err != nil {
		return fmt.Errorf("generate cross_file.toml for meson: %v", err)
	}
	joinedArgs := strings.Join(options, " ")
	command := fmt.Sprintf("meson setup %s %s --cross-file %s", m.PortConfig.BuildDir, joinedArgs, crossFile)

	// Execute configure.
	logPath := m.getLogPath("configure")
	title := fmt.Sprintf("[configure %s]", m.PortConfig.nameVersionDesc())
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(logPath)
	executor.SetWorkDir(m.PortConfig.SrcDir)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (m meson) Build(options []string) error {
	// Assemble command.
	command := fmt.Sprintf("meson compile -C %s -j %d", m.PortConfig.BuildDir, m.PortConfig.Jobs)

	// Execute build.
	logPath := m.getLogPath("build")
	title := fmt.Sprintf("[build %s]", m.PortConfig.nameVersionDesc())
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(logPath)
	executor.SetWorkDir(m.PortConfig.BuildDir)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (m meson) Install(options []string) error {
	// Assemble command.
	command := fmt.Sprintf("meson install -C %s", m.PortConfig.BuildDir)

	// Execute install.
	logPath := m.getLogPath("install")
	title := fmt.Sprintf("[install %s]", m.PortConfig.nameVersionDesc())
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(logPath)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (m meson) generateCrossFile(toolchain context.Toolchain, rootfs context.RootFS, ccacheEnabled bool) (string, error) {
	var buffers bytes.Buffer

	fmt.Fprintf(&buffers, "[build_machine]\n")
	fmt.Fprintf(&buffers, "system = '%s'\n", strings.ToLower(runtime.GOOS))
	fmt.Fprintf(&buffers, "cpu_family = '%s'\n", "x86_64")
	fmt.Fprintf(&buffers, "cpu = '%s'\n", "x86_64")
	fmt.Fprintf(&buffers, "endian = 'little'\n\n")

	fmt.Fprintf(&buffers, "[host_machine]\n")
	fmt.Fprintf(&buffers, "system = '%s'\n", strings.ToLower(toolchain.GetSystemName()))
	fmt.Fprintf(&buffers, "cpu_family = '%s'\n", toolchain.GetSystemProcessor())
	fmt.Fprintf(&buffers, "cpu = '%s'\n", toolchain.GetSystemProcessor())
	fmt.Fprintf(&buffers, "endian = 'little'\n")

	fmt.Fprintf(&buffers, "\n[binaries]\n")
	pkgconfPath := filepath.Join(dirs.InstalledDir, m.PortConfig.HostName+"-dev", "bin", "pkgconf")

	if m.PortConfig.LibName != "pkgconf" {
		fmt.Fprintf(&buffers, "pkgconfig = '%s'\n", filepath.ToSlash(pkgconfPath))
		fmt.Fprintf(&buffers, "pkg-config = '%s'\n", filepath.ToSlash(pkgconfPath))
	}

	buffers.WriteString("cmake = 'cmake'\n")
	if ccacheEnabled {
		// Meson requires array format for commands with arguments
		fmt.Fprintf(&buffers, "c = ['ccache', '%s']\n", toolchain.GetCC())
		fmt.Fprintf(&buffers, "cpp = ['ccache', '%s']\n", toolchain.GetCXX())
	} else {
		fmt.Fprintf(&buffers, "c = '%s'\n", toolchain.GetCC())
		fmt.Fprintf(&buffers, "cpp = '%s'\n", toolchain.GetCXX())
	}

	if toolchain.GetFC() != "" {
		fmt.Fprintf(&buffers, "fc = '%s'\n", toolchain.GetFC())
	}
	if toolchain.GetRANLIB() != "" {
		fmt.Fprintf(&buffers, "ranlib = '%s'\n", toolchain.GetRANLIB())
	}
	if toolchain.GetAR() != "" {
		fmt.Fprintf(&buffers, "ar = '%s'\n", toolchain.GetAR())
	}
	if toolchain.GetLD() != "" {
		fmt.Fprintf(&buffers, "ld = '%s'\n", toolchain.GetLD())
	}
	if toolchain.GetNM() != "" {
		fmt.Fprintf(&buffers, "nm = '%s'\n", toolchain.GetNM())
	}
	if toolchain.GetOBJDUMP() != "" {
		fmt.Fprintf(&buffers, "objdump = '%s'\n", toolchain.GetOBJDUMP())
	}
	if toolchain.GetSTRIP() != "" {
		fmt.Fprintf(&buffers, "strip = '%s'\n", toolchain.GetSTRIP())
	}

	buffers.WriteString("\n[properties]\n")
	buffers.WriteString("cross_file = 'true'\n")

	var (
		includeArgs []string
		linkArgs    []string
	)

	// This allows the bin to locate the libraries in the relative lib dir.
	switch runtime.GOOS {
	case "linux":
		if toolchain.GetName() == "gcc" || toolchain.GetName() == "clang" {
			linkArgs = append(linkArgs, "'-Wl,-rpath=$ORIGIN/../lib'")
		}
	case "darwin":
		// TODO: it may supported in the future for darwin.
	}

	if !m.DevDep && rootfs != nil {
		// In meson, `sys_root` will be joined as the suffix with
		// the prefix in .pc files to locate libraries.
		sysrootDir := rootfs.GetFullPath()
		fmt.Fprintf(&buffers, "sys_root = '%s'\n", sysrootDir)

		for _, item := range rootfs.GetIncludeDirs() {
			includeDir := filepath.Join(sysrootDir, item)
			includeDir = filepath.ToSlash(includeDir)
			m.appendIncludeArgs(&includeArgs, includeDir)
		}

		for _, item := range rootfs.GetLibDirs() {
			libDir := filepath.Join(sysrootDir, item)
			libDir = filepath.ToSlash(libDir)
			m.appendLinkArgs(&linkArgs, libDir)
		}
	}

	// Allow meson to locate libraries of dependecies.
	depIncludeDir := filepath.Join(dirs.TmpDepsDir, m.PortConfig.LibraryFolder, "include")
	depLinkDir := filepath.Join(dirs.TmpDepsDir, m.PortConfig.LibraryFolder, "lib")
	m.appendIncludeArgs(&includeArgs, depIncludeDir)
	m.appendLinkArgs(&linkArgs, depLinkDir)

	// Allow meson to locate libraries of MSVC.
	if runtime.GOOS == "windows" && (toolchain.GetName() == "msvc" || toolchain.GetName() == "clang-cl") {
		// Expose MSVC includes and libs.
		msvcIncludes := strings.SplitSeq(m.msvcEnvs["INCLUDE"], ";")
		msvcLibs := strings.SplitSeq(m.msvcEnvs["LIB"], ";")
		for include := range msvcIncludes {
			m.appendIncludeArgs(&includeArgs, include)
		}
		for lib := range msvcLibs {
			m.appendLinkArgs(&linkArgs, lib)
		}

		// Expose WindowsKit includes and libs.
		for _, include := range toolchain.GetMSVC().Includes {
			m.appendIncludeArgs(&includeArgs, include)
		}
		for _, lib := range toolchain.GetMSVC().Libs {
			m.appendLinkArgs(&linkArgs, lib)
		}
	}

	fmt.Fprintf(&buffers, "c_args = [%s]\n", strings.Join(includeArgs, ",\n"))
	fmt.Fprintf(&buffers, "cpp_args = [%s]\n", strings.Join(includeArgs, ",\n"))
	fmt.Fprintf(&buffers, "c_link_args = [%s]\n", strings.Join(linkArgs, ",\n"))
	fmt.Fprintf(&buffers, "cpp_link_args = [%s]\n", strings.Join(linkArgs, ",\n"))

	crossFilePath := filepath.Join(m.PortConfig.BuildDir, "cross_file.toml")
	if err := os.WriteFile(crossFilePath, buffers.Bytes(), os.ModePerm); err != nil {
		return "", err
	}

	return crossFilePath, nil
}

func (m meson) appendIncludeArgs(includeArgs *[]string, includeDir string) {
	toolchain := m.Ctx.Platform().GetToolchain()
	includeDir = filepath.ToSlash(includeDir)

	switch toolchain.GetName() {
	case "gcc", "clang":
		if len(*includeArgs) == 0 {
			*includeArgs = append(*includeArgs, fmt.Sprintf("'-isystem %s'", includeDir))
		} else {
			*includeArgs = append(*includeArgs, fmt.Sprintf("    '-isystem %s'", includeDir))
		}

	case "msvc", "clang-cl":
		if len(*includeArgs) == 0 {
			*includeArgs = append(*includeArgs, fmt.Sprintf("'/I %q'", includeDir))
		} else {
			*includeArgs = append(*includeArgs, fmt.Sprintf("    '/I %q'", includeDir))
		}

	default:
		panic(fmt.Sprintf("unexpected cross tool: %s", toolchain.GetName()))
	}
}

func (m meson) appendLinkArgs(linkArgs *[]string, linkDir string) {
	toolchain := m.Ctx.Platform().GetToolchain()
	linkDir = filepath.ToSlash(linkDir)

	switch runtime.GOOS {
	case "linux":
		if toolchain.GetName() == "gcc" || toolchain.GetName() == "clang" {
			if len(*linkArgs) == 0 {
				*linkArgs = append(*linkArgs, fmt.Sprintf("'-L %s'", linkDir))
				*linkArgs = append(*linkArgs, fmt.Sprintf(`'-Wl,-rpath-link,%s'`, linkDir))
			} else {
				*linkArgs = append(*linkArgs, fmt.Sprintf("    '-L %s'", linkDir))
				*linkArgs = append(*linkArgs, fmt.Sprintf("    '-Wl,-rpath-link,%s'", linkDir))
			}
		}

	case "windows":
		if toolchain.GetName() == "msvc" || toolchain.GetName() == "clang-cl" {
			if len(*linkArgs) == 0 {
				*linkArgs = append(*linkArgs, fmt.Sprintf("'/LIBPATH:\"%s\"'", linkDir))
			} else {
				*linkArgs = append(*linkArgs, fmt.Sprintf("    '/LIBPATH:\"%s\"'", linkDir))
			}
		}

	case "darwin":
		// TODO: it may supported in the future for darwin.
	}
}
