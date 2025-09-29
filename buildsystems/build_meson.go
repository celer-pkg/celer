package buildsystems

import (
	"bytes"
	"celer/buildtools"
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

func NewMeson(config *BuildConfig, optimize *Optimize) *meson {
	return &meson{
		BuildConfig: config,
		Optimize:    optimize,
	}
}

type meson struct {
	*BuildConfig
	*Optimize
}

func (meson) Name() string {
	return "meson"
}

func (m meson) CheckTools() error {
	m.BuildConfig.BuildTools = append(m.BuildConfig.BuildTools, "git", "ninja", "cmake")

	switch runtime.GOOS {
	case "windows":
		m.BuildConfig.BuildTools = append(m.BuildConfig.BuildTools,
			"python3",
			"python3:meson",
		)
	case "linux":
		m.BuildConfig.BuildTools = append(m.BuildConfig.BuildTools, "python3:meson")
	}

	return buildtools.CheckTools(m.BuildConfig.BuildTools...)
}

func (m meson) Clean() error {
	// We do not configure meson project in source folder.
	return nil
}

func (m meson) preConfigure() error {
	// For MSVC build, we need to set PATH, INCLUDE and LIB env vars.
	if m.PortConfig.Toolchain.Name == "msvc" {
		msvcEnvs, err := m.readMSVCEnvs()
		if err != nil {
			return err
		}

		os.Setenv("PATH", msvcEnvs["PATH"])
		os.Setenv("INCLUDE", msvcEnvs["INCLUDE"])
		os.Setenv("LIB", msvcEnvs["LIB"])
	}

	return nil
}

func (m meson) configureOptions() ([]string, error) {
	var options = slices.Clone(m.Options)

	// Set build type.
	if m.DevDep {
		options = append(options, "--buildtype=release")
	} else {
		buildType := strings.ToLower(m.BuildType)
		switch buildType {
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
	// In windows, we set msvc related environments.
	if m.DevDep && m.PortConfig.Toolchain.Name != "msvc" {
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
	if m.PortConfig.Toolchain.Name == "gcc" {
		linkArgs = append(linkArgs, "'-Wl,-rpath=$ORIGIN/../lib'")
	}

	if !m.DevDep && toolchain.RootFS != "" {
		// In meson, `sys_root` will be joined as the suffix with
		// the prefix in .pc files to locate libraries.
		buffers.WriteString(fmt.Sprintf("sys_root = '%s'\n", toolchain.RootFS))

		for _, item := range toolchain.IncludeDirs {
			includeDir := filepath.Join(toolchain.RootFS, item)

			switch runtime.GOOS {
			case "windows":
				if len(includeArgs) == 0 {
					includeArgs = append(includeArgs, fmt.Sprintf("'/I %q'", includeDir))
				} else {
					includeArgs = append(includeArgs, fmt.Sprintf("\t"+"'/I %q'", includeDir))
				}

			default:
				if len(includeArgs) == 0 {
					includeArgs = append(includeArgs, fmt.Sprintf("'-isystem %s'", includeDir))
				} else {
					includeArgs = append(includeArgs, fmt.Sprintf("\t"+"'-isystem %s'", includeDir))
				}
			}
		}

		// Allow meson to locate libraries in rootfs.
		for _, item := range toolchain.LibDirs {
			libDir := filepath.Join(toolchain.RootFS, item)
			switch m.PortConfig.Toolchain.Name {
			case "gcc":
				if len(linkArgs) == 0 {
					linkArgs = append(linkArgs, fmt.Sprintf("'-L%s'", libDir))
					linkArgs = append(linkArgs, fmt.Sprintf(`'-Wl,-rpath-link=%s'`, libDir))
				} else {
					linkArgs = append(linkArgs, fmt.Sprintf("\t"+"'-L%s'", libDir))
					linkArgs = append(linkArgs, fmt.Sprintf("\t"+`'-Wl,-rpath-link=%s'`, libDir))
				}

			case "msvc":
				if len(linkArgs) == 0 {
					linkArgs = append(linkArgs, fmt.Sprintf("'/LIBPATH:%s'", libDir))
				} else {
					linkArgs = append(linkArgs, fmt.Sprintf("\t"+"'/LIBPATH:%s'", libDir))
				}

			default:
				panic(fmt.Sprintf("unexpected os: %s", runtime.GOOS))
			}
		}
	}

	// Allow meson to locate headers of dependecies.
	depIncludeDir := filepath.Join(dirs.TmpDepsDir, m.PortConfig.LibraryFolder, "include")
	m.appendIncludeArgs(&includeArgs, depIncludeDir)

	// Allow meson to locate libraries of dependecies.
	if m.PortConfig.Toolchain.Name == "gcc" {
		depLibDir := filepath.Join(dirs.TmpDepsDir, m.PortConfig.LibraryFolder, "lib")
		if len(linkArgs) == 0 {
			linkArgs = append(linkArgs, fmt.Sprintf("'-L%s'", depLibDir))
			linkArgs = append(linkArgs, fmt.Sprintf(`'-Wl,-rpath-link,%s'`, depLibDir))
		} else {
			linkArgs = append(linkArgs, fmt.Sprintf("\t"+"'-L%s'", depLibDir))
			linkArgs = append(linkArgs, fmt.Sprintf("\t"+`'-Wl,-rpath-link,%s'`, depLibDir))
		}
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
	switch m.PortConfig.Toolchain.Name {
	case "gcc":
		if len(*includeArgs) == 0 {
			*includeArgs = append(*includeArgs, fmt.Sprintf("'-isystem %s'", includeDir))
		} else {
			*includeArgs = append(*includeArgs, fmt.Sprintf("\t"+"'-isystem %s'", includeDir))
		}

	case "msvc":
		if len(*includeArgs) == 0 {
			*includeArgs = append(*includeArgs, fmt.Sprintf("'/I %q'", filepath.ToSlash(includeDir)))
		} else {
			*includeArgs = append(*includeArgs, fmt.Sprintf("\t"+"'/I %q'", filepath.ToSlash(includeDir)))
		}

	default:
		panic(fmt.Sprintf("unexpected cross tool: %s", m.PortConfig.Toolchain.Name))
	}
}

func (m meson) nativeCrossTool() Toolchain {
	switch m.PortConfig.Toolchain.Name {
	case "msvc":
		return Toolchain{
			Native:          true,
			Name:            "msvc",
			SystemName:      "Windows",
			SystemProcessor: "x86_64",
			CC:              "cl.exe",
			CXX:             "cl.exe",
			AR:              "lib.exe",
			LD:              "link.exe",
			MSVC:            m.PortConfig.Toolchain.MSVC,
			IncludeDirs:     m.PortConfig.Toolchain.IncludeDirs,
			LibDirs:         m.PortConfig.Toolchain.LibDirs,
			Fullpath:        m.PortConfig.Toolchain.Fullpath,
		}

	case "gcc":
		return Toolchain{
			Native:          true,
			Name:            "gcc",
			SystemName:      "Linux",
			SystemProcessor: "x86_64",
			CC:              "x86_64-linux-gnu-gcc",
			CXX:             "x86_64-linux-gnu-g++",
			AR:              "x86_64-linux-gnu-gcc-ar",
			LD:              "x86_64-linux-gnu-gcc-ld",
		}

	default:
		panic("unsupported cross tool: " + m.PortConfig.Toolchain.Name)
	}
}
