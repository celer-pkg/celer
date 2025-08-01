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

func NewMeson(config *BuildConfig) *meson {
	return &meson{BuildConfig: config}
}

type meson struct {
	*BuildConfig
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

func (m meson) CleanRepo() error {
	// We do not configure meson project in source folder.
	return nil
}

func (m meson) configureOptions() ([]string, error) {
	buildType := strings.ToLower(m.BuildType)

	var options = slices.Clone(m.Options)

	// Append 'BUILD_TYPE' if not contains it.
	if m.DevDep {
		options = slices.DeleteFunc(options, func(element string) bool {
			return strings.Contains(element, "--buildtype")
		})
		options = append(options, "--buildtype=release")
	} else {
		if !slices.ContainsFunc(options, func(arg string) bool {
			return strings.Contains(arg, "--buildtype")
		}) {
			options = append(options, "--buildtype="+buildType)
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
	if m.DevDep && m.PortConfig.CrossTools.Name != "msvc" {
		m.PortConfig.CrossTools.ClearEnvs()
	} else {
		m.PortConfig.CrossTools.SetEnvs(m.BuildConfig)
	}

	// Create build dir if not exists.
	if err := os.MkdirAll(m.PortConfig.BuildDir, os.ModePerm); err != nil {
		return err
	}

	// Assemble command.
	crossFile, err := m.generateCrossFile(expr.If(m.BuildConfig.DevDep, m.nativeCrossTool(), *m.PortConfig.CrossTools))
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
	command := fmt.Sprintf("meson compile -C %s -j %d", m.PortConfig.BuildDir, m.PortConfig.JobNum)

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

func (m meson) generateCrossFile(crosstool CrossTools) (string, error) {
	var buffers bytes.Buffer
	buffers.WriteString("[host_machine]\n")
	buffers.WriteString(fmt.Sprintf("system = '%s'\n", strings.ToLower(crosstool.SystemName)))
	buffers.WriteString(fmt.Sprintf("cpu_family = '%s'\n", crosstool.SystemProcessor))
	buffers.WriteString(fmt.Sprintf("cpu = '%s'\n", crosstool.SystemProcessor))
	buffers.WriteString("endian = 'little'\n")

	buffers.WriteString("\n[binaries]\n")
	pkgconfPath := filepath.Join(dirs.InstalledDir, m.PortConfig.HostName+"-dev", "bin", "pkgconf")
	buffers.WriteString(fmt.Sprintf("pkgconfig = '%s'\n", pkgconfPath))
	buffers.WriteString(fmt.Sprintf("pkg-config = '%s'\n", pkgconfPath))
	buffers.WriteString("cmake = 'cmake'\n")
	buffers.WriteString(fmt.Sprintf("c = '%s'\n", crosstool.CC))
	buffers.WriteString(fmt.Sprintf("cpp = '%s'\n", crosstool.CXX))

	if crosstool.FC != "" {
		buffers.WriteString(fmt.Sprintf("fc = '%s'\n", crosstool.FC))
	}
	if crosstool.RANLIB != "" {
		buffers.WriteString(fmt.Sprintf("ranlib = '%s'\n", crosstool.RANLIB))
	}
	if crosstool.AR != "" {
		buffers.WriteString(fmt.Sprintf("ar = '%s'\n", crosstool.AR))
	}
	if crosstool.LD != "" {
		buffers.WriteString(fmt.Sprintf("ld = '%s'\n", crosstool.LD))
	}
	if crosstool.NM != "" {
		buffers.WriteString(fmt.Sprintf("nm = '%s'\n", crosstool.NM))
	}
	if crosstool.OBJDUMP != "" {
		buffers.WriteString(fmt.Sprintf("objdump = '%s'\n", crosstool.OBJDUMP))
	}
	if crosstool.STRIP != "" {
		buffers.WriteString(fmt.Sprintf("strip = '%s'\n", crosstool.STRIP))
	}

	buffers.WriteString("\n[properties]\n")
	buffers.WriteString("cross_file = 'true'\n")

	var (
		includeArgs []string
		linkArgs    []string
	)

	// This allows the bin to locate the libraries in the relative lib dir.
	if runtime.GOOS == "linux" {
		linkArgs = append(linkArgs, "'-Wl,-rpath=$ORIGIN/../lib'")
	}

	if !m.DevDep && crosstool.RootFS != "" {
		// In meson, `sys_root` will be joined as the suffix with
		// the prefix in .pc files to locate libraries.
		buffers.WriteString(fmt.Sprintf("sys_root = '%s'\n", crosstool.RootFS))

		for _, item := range crosstool.IncludeDirs {
			includeDir := filepath.Join(crosstool.RootFS, item)

			switch runtime.GOOS {
			case "linux", "darwin":
				if len(includeArgs) == 0 {
					includeArgs = append(includeArgs, fmt.Sprintf("'-isystem %s'", includeDir))
				} else {
					includeArgs = append(includeArgs, fmt.Sprintf("\t"+"'-isystem %s'", includeDir))
				}

			case "windows":
				if len(includeArgs) == 0 {
					includeArgs = append(includeArgs, fmt.Sprintf("'/I%s'", includeDir))
				} else {
					includeArgs = append(includeArgs, fmt.Sprintf("\t"+"'/I%s'", includeDir))
				}

			default:
				panic(fmt.Sprintf("unexpected os: %s", runtime.GOOS))
			}
		}

		// Allow meson to locate libraries in rootfs.
		for _, item := range crosstool.LibDirs {
			libDir := filepath.Join(crosstool.RootFS, item)
			switch runtime.GOOS {
			case "linux", "darwin":
				if len(linkArgs) == 0 {
					linkArgs = append(linkArgs, fmt.Sprintf("'-L%s'", libDir))
					linkArgs = append(linkArgs, fmt.Sprintf(`'-Wl,-rpath-link=%s'`, libDir))
				} else {
					linkArgs = append(linkArgs, fmt.Sprintf("\t"+"'-L%s'", libDir))
					linkArgs = append(linkArgs, fmt.Sprintf("\t"+`'-Wl,-rpath-link=%s'`, libDir))
				}

			case "windows":
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
	if len(includeArgs) == 0 {
		includeArgs = append(includeArgs, fmt.Sprintf("'-isystem%s'", depIncludeDir))
	} else {
		includeArgs = append(includeArgs, fmt.Sprintf("\t"+"'-isystem%s'", depIncludeDir))
	}

	// Allow meson to locate libraries of dependecies.
	if runtime.GOOS == "linux" {
		depLibDir := filepath.Join(dirs.TmpDepsDir, m.PortConfig.LibraryFolder, "lib")
		if len(linkArgs) == 0 {
			linkArgs = append(linkArgs, fmt.Sprintf("'-L%s'", depLibDir))
			linkArgs = append(linkArgs, fmt.Sprintf(`'-Wl,-rpath-link=%s'`, depLibDir))
		} else {
			linkArgs = append(linkArgs, fmt.Sprintf("\t"+"'-L%s'", depLibDir))
			linkArgs = append(linkArgs, fmt.Sprintf("\t"+`'-Wl,-rpath-link=%s'`, depLibDir))
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

func (m meson) nativeCrossTool() CrossTools {
	switch runtime.GOOS {
	case "windows":
		return CrossTools{
			Native:          true,
			Name:            "msvc",
			SystemName:      "Windows",
			SystemProcessor: "x86_64",
			CC:              "cl.exe",
			CXX:             "cl.exe",
			AR:              "lib.exe",
			LD:              "link.exe",
		}

	case "linux":
		return CrossTools{
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
		panic("unsupported operating system: " + runtime.GOOS)
	}
}
