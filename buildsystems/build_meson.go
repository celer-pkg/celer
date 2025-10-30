package buildsystems

import (
	"bytes"
	"celer/buildtools"
	"celer/context"
	"celer/pkgs/cmd"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
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
}

func (meson) Name() string {
	return "meson"
}

func (m meson) CheckTools() error {
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

	return buildtools.CheckTools(m.Ctx, m.BuildTools...)
}

func (m meson) Clean() error {
	// We do not configure meson project in source folder.
	return nil
}

func (m meson) preConfigure() error {
	// For MSVC build, we need to set PATH, INCLUDE and LIB env vars.
	if runtime.GOOS == "windows" {
		if m.PortConfig.Toolchain.Name == "msvc" ||
			m.PortConfig.Toolchain.Name == "clang-cli" {
			msvcEnvs, err := m.readMSVCEnvs()
			if err != nil {
				return err
			}

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
		options[index] = m.replaceHolders(value)
	}

	return options, nil
}

func (m meson) configured() bool {
	buildFile := filepath.Join(m.PortConfig.BuildDir, "build.ninja")
	return fileio.PathExists(buildFile)
}

func (m meson) Configure(options []string) error {
	// msvc and clang-cl need to set build environment event in dev mode.
	if m.DevDep ||
		m.PortConfig.Toolchain.Name == "msvc" ||
		m.PortConfig.Toolchain.Name == "clang-cl" {
		m.PortConfig.Toolchain.ClearEnvs()
	} else {
		m.PortConfig.Toolchain.SetEnvs(m.BuildConfig)
	}

	// Create build dir if not exists.
	if err := os.MkdirAll(m.PortConfig.BuildDir, os.ModePerm); err != nil {
		return err
	}

	// Assemble command.
	crossFile, err := m.generateCrossFile(expr.If(m.BuildConfig.DevDep, m.nativeCrossTool(), *m.PortConfig.Toolchain))
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

	// Remove `nul` file in workspace_dir.
	os.Remove(filepath.Join(dirs.WorkspaceDir, "nul"))

	return nil
}

func (m meson) generateCrossFile(toolchain Toolchain) (string, error) {
	var buffers bytes.Buffer

	buffers.WriteString("[build_machine]\n")
	buffers.WriteString(fmt.Sprintf("system = '%s'\n", strings.ToLower(runtime.GOOS)))
	buffers.WriteString(fmt.Sprintf("cpu_family = '%s'\n", "x86_64"))
	buffers.WriteString(fmt.Sprintf("cpu = '%s'\n", "x86_64"))
	buffers.WriteString("endian = 'little'\n\n")

	buffers.WriteString("[host_machine]\n")
	buffers.WriteString(fmt.Sprintf("system = '%s'\n", strings.ToLower(toolchain.SystemName)))
	buffers.WriteString(fmt.Sprintf("cpu_family = '%s'\n", toolchain.SystemProcessor))
	buffers.WriteString(fmt.Sprintf("cpu = '%s'\n", toolchain.SystemProcessor))
	buffers.WriteString("endian = 'little'\n")

	buffers.WriteString("\n[binaries]\n")
	pkgconfPath := filepath.Join(dirs.InstalledDir, m.PortConfig.HostName+"-dev", "bin", "pkgconf")

	if m.PortConfig.LibName == "pkgconf" {
		buffers.WriteString("pkgconfig = 'false'\n")
		buffers.WriteString("pkg-config = 'false'\n")
	} else {
		buffers.WriteString(fmt.Sprintf("pkgconfig = '%s'\n", filepath.ToSlash(pkgconfPath)))
		buffers.WriteString(fmt.Sprintf("pkg-config = '%s'\n", filepath.ToSlash(pkgconfPath)))
	}

	buffers.WriteString("cmake = 'cmake'\n")
	buffers.WriteString(fmt.Sprintf("c = '%s'\n", toolchain.CC))
	buffers.WriteString(fmt.Sprintf("cpp = '%s'\n", toolchain.CXX))

	if toolchain.FC != "" {
		buffers.WriteString(fmt.Sprintf("fc = '%s'\n", toolchain.FC))
	}
	if toolchain.RANLIB != "" {
		buffers.WriteString(fmt.Sprintf("ranlib = '%s'\n", toolchain.RANLIB))
	}
	if toolchain.AR != "" {
		buffers.WriteString(fmt.Sprintf("ar = '%s'\n", toolchain.AR))
	}
	if toolchain.LD != "" {
		buffers.WriteString(fmt.Sprintf("ld = '%s'\n", toolchain.LD))
	}
	if toolchain.NM != "" {
		buffers.WriteString(fmt.Sprintf("nm = '%s'\n", toolchain.NM))
	}
	if toolchain.OBJDUMP != "" {
		buffers.WriteString(fmt.Sprintf("objdump = '%s'\n", toolchain.OBJDUMP))
	}
	if toolchain.STRIP != "" {
		buffers.WriteString(fmt.Sprintf("strip = '%s'\n", toolchain.STRIP))
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
		if m.PortConfig.Toolchain.Name == "gcc" || m.PortConfig.Toolchain.Name == "clang" {
			linkArgs = append(linkArgs, "'-Wl,-rpath=$ORIGIN/../lib'")
		}
	case "darwin":
		// TODO: it may supported in the future for darwin.
	}

	if !m.DevDep && toolchain.RootFS != "" {
		// In meson, `sys_root` will be joined as the suffix with
		// the prefix in .pc files to locate libraries.
		buffers.WriteString(fmt.Sprintf("sys_root = '%s'\n", toolchain.RootFS))

		for _, item := range toolchain.IncludeDirs {
			includeDir := filepath.Join(toolchain.RootFS, item)
			includeDir = filepath.ToSlash(includeDir)

			switch runtime.GOOS {
			case "windows":
				if len(includeArgs) == 0 {
					includeArgs = append(includeArgs, fmt.Sprintf("'/I %q'", includeDir))
				} else {
					includeArgs = append(includeArgs, fmt.Sprintf("    '/I %q'", includeDir))
				}

			default:
				if len(includeArgs) == 0 {
					includeArgs = append(includeArgs, fmt.Sprintf("'-isystem %s'", includeDir))
				} else {
					includeArgs = append(includeArgs, fmt.Sprintf("    '-isystem %s'", includeDir))
				}
			}
		}

		// Allow meson to locate libraries in rootfs.
		for _, item := range toolchain.LibDirs {
			libDir := filepath.Join(toolchain.RootFS, item)
			libDir = filepath.ToSlash(libDir)

			switch m.PortConfig.Toolchain.Name {
			case "gcc", "clang":
				if len(linkArgs) == 0 {
					linkArgs = append(linkArgs, fmt.Sprintf("'-L %s'", libDir))
					linkArgs = append(linkArgs, fmt.Sprintf("'-Wl,-rpath-link=%s'", libDir))
				} else {
					linkArgs = append(linkArgs, fmt.Sprintf("    '-L %s'", libDir))
					linkArgs = append(linkArgs, fmt.Sprintf("    '-Wl,-rpath-link=%s'", libDir))
				}

			case "msvc", "clang-cl":
				if len(linkArgs) == 0 {
					linkArgs = append(linkArgs, fmt.Sprintf("'/LIBPATH:%s'", libDir))
				} else {
					linkArgs = append(linkArgs, fmt.Sprintf("    '/LIBPATH:%s'", libDir))
				}

			default:
				panic(fmt.Sprintf("unexpected toolchain: %s", m.PortConfig.Toolchain.Name))
			}
		}
	}

	// Allow meson to locate headers of dependecies.
	depIncludeDir := filepath.Join(dirs.TmpDepsDir, m.PortConfig.LibraryFolder, "include")
	m.appendIncludeArgs(&includeArgs, depIncludeDir)

	// Allow meson to locate libraries of dependecies.
	switch runtime.GOOS {
	case "linux":
		if m.PortConfig.Toolchain.Name == "gcc" || m.PortConfig.Toolchain.Name == "clang" {
			depLibDir := filepath.Join(dirs.TmpDepsDir, m.PortConfig.LibraryFolder, "lib")
			depLibDir = filepath.ToSlash(depLibDir)

			if len(linkArgs) == 0 {
				linkArgs = append(linkArgs, fmt.Sprintf("'-L %s'", depLibDir))
				linkArgs = append(linkArgs, fmt.Sprintf(`'-Wl,-rpath-link,%s'`, depLibDir))
			} else {
				linkArgs = append(linkArgs, fmt.Sprintf("    '-L %s'", depLibDir))
				linkArgs = append(linkArgs, fmt.Sprintf("    '-Wl,-rpath-link,%s'", depLibDir))
			}
		}

	case "darwin":
		// TODO: it may supported in the future for darwin.
	}

	buffers.WriteString(fmt.Sprintf("c_args = [%s]\n", strings.Join(includeArgs, ",\n")))
	buffers.WriteString(fmt.Sprintf("cpp_args = [%s]\n", strings.Join(includeArgs, ",\n")))
	buffers.WriteString(fmt.Sprintf("c_link_args = [%s]\n", strings.Join(linkArgs, ",\n")))
	buffers.WriteString(fmt.Sprintf("cpp_link_args = [%s]\n", strings.Join(linkArgs, ",\n")))

	crossFilePath := filepath.Join(m.PortConfig.BuildDir, "cross_file.toml")
	if err := os.WriteFile(crossFilePath, buffers.Bytes(), os.ModePerm); err != nil {
		return "", err
	}

	return crossFilePath, nil
}

func (m meson) appendIncludeArgs(includeArgs *[]string, includeDir string) {
	includeDir = filepath.ToSlash(includeDir)

	switch m.PortConfig.Toolchain.Name {
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
		panic(fmt.Sprintf("unexpected cross tool: %s", m.PortConfig.Toolchain.Name))
	}
}

func (m meson) nativeCrossTool() Toolchain {
	var toolchain Toolchain
	toolchain.Native = true
	toolchain.SystemName = expr.UpperFirst(runtime.GOOS)
	toolchain.SystemProcessor = "x86_64"

	switch m.PortConfig.Toolchain.Name {
	case "gcc":
		toolchain.CC = "gcc"
		toolchain.CXX = "g++"
		toolchain.RANLIB = "ranlib"
		toolchain.AR = "ar"
		toolchain.LD = "ld"
		toolchain.NM = "nm"
		toolchain.OBJDUMP = "objdump"
		toolchain.STRIP = "strip"
		return toolchain

	case "msvc":
		toolchain.CC = "cl"
		toolchain.CXX = "cl"
		toolchain.AR = "lib"
		toolchain.LD = "link"
		toolchain.MSVC = m.PortConfig.Toolchain.MSVC
		toolchain.IncludeDirs = m.PortConfig.Toolchain.IncludeDirs
		toolchain.LibDirs = m.PortConfig.Toolchain.LibDirs
		toolchain.Fullpath = m.PortConfig.Toolchain.Fullpath
		return toolchain

	case "clang":
		toolchain.CC = "clang"
		toolchain.CXX = "clang++"
		toolchain.RANLIB = "llvm-ranlib"
		toolchain.AR = "llvm-ar"
		toolchain.LD = "clang"
		toolchain.NM = "llvm-nm"
		toolchain.OBJDUMP = "llvm-objdump"
		toolchain.STRIP = "llvm-strip"
		return toolchain

	case "clang-cl":
		toolchain.CC = "clang-cl"
		toolchain.CXX = "clang-cl"
		toolchain.RANLIB = "llvm-ranlib"
		toolchain.AR = "llvm-ar"
		toolchain.LD = "clang-cl"
		toolchain.NM = "llvm-nm"
		toolchain.OBJDUMP = "llvm-objdump"
		toolchain.STRIP = "llvm-strip"
		toolchain.IncludeDirs = m.PortConfig.Toolchain.IncludeDirs
		toolchain.LibDirs = m.PortConfig.Toolchain.LibDirs
		toolchain.Fullpath = m.PortConfig.Toolchain.Fullpath
		return toolchain

	default:
		panic("unsupported toolchain: " + m.PortConfig.Toolchain.Name)
	}
}
