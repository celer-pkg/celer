package buildsystems

import (
	"celer/buildtools"
	"celer/context"
	"celer/pkgs/cmd"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strconv"
	"strings"
)

const (
	visualStudio_17_2022 = "Visual Studio 17 2022"
	visualStudio_16_2019 = "Visual Studio 16 2019"
	visualStudio_15_2017 = "Visual Studio 15 2017 Win64"
	visualStudio_14_2015 = "Visual Studio 14 2015 Win64"
)

func NewCMake(config *BuildConfig, optimize *context.Optimize) *cmake {
	return &cmake{
		BuildConfig: config,
		Optimize:    optimize,
	}
}

type cmake struct {
	*BuildConfig
	*context.Optimize
}

func (c cmake) Name() string {
	return "cmake"
}

func (c cmake) CheckTools() []string {
	// Start with build_tools from port.toml
	tools := slices.Clone(c.BuildConfig.BuildTools)

	// Add default tools
	tools = append(tools, "git", "cmake")
	if c.CMakeGenerator == "Ninja" {
		tools = append(tools, "ninja")
	}

	// In windows, the default toolchain is Visual Studio,
	// so vswhere is required to find installed Visual Studio.
	if runtime.GOOS == "windows" && c.CMakeGenerator == "" {
		tools = append(tools, "vswhere")
	}

	return tools
}

func (c *cmake) preConfigure() error {
	toolchain := c.Ctx.Platform().GetToolchain()

	// For MSVC build with Ninja generator, we need to set INCLUDE and LIB env vars.
	// Visual Studio generator handles these automatically via MSBuild.
	// Ninja generator requires explicit environment variables for RC.exe and link.exe to find system headers/libs.
	// Note: Environment variables set in toolchain_file.cmake only affect CMake's configure phase,
	// not the build phase when Ninja invokes RC.exe and link.exe.
	if runtime.GOOS == "windows" && c.CMakeGenerator == "Ninja" {
		if toolchain.GetName() == "msvc" || toolchain.GetName() == "clang-cl" {
			msvcEnvs, err := c.readMSVCEnvs()
			if err != nil {
				return err
			}

			os.Setenv("INCLUDE", msvcEnvs["INCLUDE"])
			os.Setenv("LIB", msvcEnvs["LIB"])
		}
	}

	return nil
}

func (c cmake) configureOptions() ([]string, error) {
	var (
		toolchain = c.Ctx.Platform().GetToolchain()
		rootfs    = c.Ctx.Platform().GetRootFS()
		options   = slices.Clone(c.Options)
	)

	// When use clang-cl with visual studio, we must to set toolset by "-T".
	if runtime.GOOS == "windows" && strings.HasPrefix(c.CMakeGenerator, "Visual Studio") {
		switch toolchain.GetName() {
		case "clang-cl":
			options = append(options, "-T ClangCL")
		case "clang":
			return nil, fmt.Errorf("visual studio's clang is not supported with visual studio generator")
		}
	}

	if !c.BuildConfig.DevDep {
		options = append(options, fmt.Sprintf("-DCMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir))
	} else {
		options = append(options, "-DCMAKE_INSTALL_RPATH=$ORIGIN/../lib")
	}

	// Set CMAKE_INSTALL_PREFIX.
	options = append(options, "-DCMAKE_INSTALL_PREFIX="+c.PortConfig.PackageDir)

	// Set CMAKE_BUILD_TYPE.
	if c.multiConfigGenerator() {
		// CMAKE_BUILD_TYPE is not supported in multi-config generator.
		options = slices.DeleteFunc(options, func(element string) bool {
			return strings.HasPrefix(element, "-DCMAKE_BUILD_TYPE=")
		})
	} else {
		// Append `CMAKE_BUILD_TYPE` if not contains it.
		hasCMakeBuildType := slices.ContainsFunc(options, func(opt string) bool {
			return strings.HasPrefix(opt, "-DCMAKE_BUILD_TYPE=")
		})
		if !hasCMakeBuildType {
			if c.DevDep {
				options = append(options, "-DCMAKE_BUILD_TYPE=Release")
			} else {
				options = append(options, "-DCMAKE_BUILD_TYPE="+c.formatBuildType())
			}
		}
	}

	// Set build library type.
	libraryType := c.libraryType("-DBUILD_SHARED_LIBS=ON", "-DBUILD_SHARED_LIBS=OFF")
	switch c.BuildConfig.LibraryType {
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

	// Set C standard.
	if c.CStandard != "" {
		options = append(options, "-DCMAKE_C_STANDARD="+strings.TrimPrefix(c.CStandard, "c"))
		options = append(options, "-DCMAKE_C_STANDARD_REQUIRED=ON")
	}

	// Set C++ standard.
	if c.CXXStandard != "" {
		options = append(options, "-DCMAKE_CXX_STANDARD="+strings.TrimPrefix(c.CXXStandard, "c++"))
		options = append(options, "-DCMAKE_CXX_STANDARD_REQUIRED=ON")
	}

	// Override `CMAKE_FIND_ROOT_PATH` defined in toolchain file.
	// For DevDep or Native (host tools), don't include rootfs to avoid finding target arch binaries.
	tmpDepDir := filepath.Join(dirs.TmpDepsDir, c.PortConfig.LibraryFolder)
	rootPaths := []string{filepath.ToSlash(tmpDepDir)}
	if rootfs != nil && !c.BuildConfig.DevDep && !c.BuildConfig.Native {
		rootPaths = append(rootPaths, rootfs.GetAbsPath())
	}
	options = append(options, "-DCMAKE_FIND_ROOT_PATH="+strings.Join(rootPaths, ";"))
	options = append(options, "-DTMP_DEP_DIR="+filepath.ToSlash(tmpDepDir))

	// Explicitly set Python3_EXECUTABLE to use host system's Python instead of target arch Python.
	if buildtools.Python3 != nil && buildtools.Python3.Path != "" {
		pythonPath := filepath.ToSlash(buildtools.Python3.Path)
		options = append(options, "-DPython3_EXECUTABLE="+pythonPath)
	}

	// Enable verbose makefile.
	if c.Ctx.Verbose() {
		options = append(options, "-DCMAKE_VERBOSE_MAKEFILE=ON")
	}

	// Replace placeholders.
	for index, value := range options {
		options[index] = c.expandVariables(value)
	}

	// Convert Windows paths to forward slashes in CMake options to avoid escape sequence issues.
	// This is especially important for paths passed via -D options like -DVAR=path or -DVAR="path".
	if runtime.GOOS == "windows" {
		for index, value := range options {
			if strings.HasPrefix(value, "-D") {
				parts := strings.SplitN(value, "=", 2)
				if len(parts) == 2 {
					parts[1] = filepath.ToSlash(parts[1])
					options[index] = strings.Join(parts, "=")
				}
			}
		}
	}

	return options, nil
}

func (c cmake) configured() bool {
	if err := c.detectGenerator(); err != nil {
		color.Printf(color.Error, "failed to detect generator: %s", err)
		return false
	}

	switch c.CMakeGenerator {
	case "Ninja":
		cmakeCache := filepath.Join(c.PortConfig.BuildDir, "CMakeCache.txt")
		buildFile := filepath.Join(c.PortConfig.BuildDir, "build.ninja")
		ruluesFile := filepath.Join(c.PortConfig.BuildDir, "rules.ninja")
		return fileio.PathExists(c.PortConfig.RepoDir) &&
			fileio.PathExists(cmakeCache) &&
			fileio.PathExists(buildFile) &&
			fileio.PathExists(ruluesFile)

	case "Unix Makefiles":
		cmakeCache := filepath.Join(c.PortConfig.BuildDir, "CMakeCache.txt")
		makefile := filepath.Join(c.PortConfig.BuildDir, "Makefile")
		return fileio.PathExists(c.PortConfig.RepoDir) &&
			fileio.PathExists(cmakeCache) &&
			fileio.PathExists(makefile)

	case visualStudio_17_2022, visualStudio_16_2019, visualStudio_15_2017, visualStudio_14_2015:
		cmakeCache := filepath.Join(c.PortConfig.BuildDir, "CMakeCache.txt")
		slnFile := filepath.Join(c.PortConfig.BuildDir, c.PortConfig.LibName+".sln")
		vcxprojFile := filepath.Join(c.PortConfig.BuildDir, c.PortConfig.LibName+".vcxproj")
		return fileio.PathExists(c.PortConfig.RepoDir) &&
			fileio.PathExists(cmakeCache) &&
			fileio.PathExists(slnFile) &&
			fileio.PathExists(vcxprojFile)
	}

	return false
}

func (c cmake) Configure(options []string) error {
	// Remove build dir and create it for configure.
	if err := os.RemoveAll(c.PortConfig.BuildDir); err != nil {
		return err
	}

	// Create build dir if not exists.
	if err := os.MkdirAll(c.PortConfig.BuildDir, os.ModePerm); err != nil {
		return err
	}

	// Assemble args into a single command string.
	var args []string
	if c.CMakeGenerator == "" {
		args = append(args, "-S", c.PortConfig.SrcDir)
		args = append(args, "-B", c.PortConfig.BuildDir)
	} else {
		args = append(args, "-G", c.CMakeGenerator)
		args = append(args, "-S", c.PortConfig.SrcDir)
		args = append(args, "-B", c.PortConfig.BuildDir)
	}
	args = append(args, options...)

	// Execute configure.
	logPath := c.getLogPath("configure")
	title := fmt.Sprintf("[configure %s]", c.PortConfig.nameVersionDesc())
	executor := cmd.NewExecutor(title, "cmake", args...)
	executor.SetWorkDir(c.PortConfig.BuildDir)
	executor.SetLogPath(logPath)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (c cmake) buildOptions() ([]string, error) {
	// CMAKE_BUILD_TYPE is useless for MSVC, use --config Debug/Relase instead.
	var options []string
	if c.multiConfigGenerator() {
		options = append(options, "--config", c.formatBuildType())
	}

	return options, nil
}

func (c cmake) Build(options []string) error {
	// Assemble args.
	var args []string
	args = append(args, "--build", c.PortConfig.BuildDir)
	args = append(args, options...)
	args = append(args, "--parallel", strconv.Itoa(c.PortConfig.Jobs))

	// Execute build.
	logPath := c.getLogPath("build")
	title := fmt.Sprintf("[build %s@%s]", c.PortConfig.LibName, c.PortConfig.LibVersion)
	executor := cmd.NewExecutor(title, "cmake", args...)
	executor.SetWorkDir(c.PortConfig.BuildDir)
	executor.SetLogPath(logPath)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (c cmake) installOptions() ([]string, error) {
	// CMAKE_BUILD_TYPE is useless for MSVC, use --config Debug/Relase instead.
	var options []string
	if c.multiConfigGenerator() {
		options = append(options, "--config", c.formatBuildType())
	}

	return options, nil
}

func (c cmake) Install(options []string) error {
	// Assemble args.
	var args []string
	args = append(args, "--install", c.PortConfig.BuildDir)
	args = append(args, options...)

	// Execute install.
	logPath := c.getLogPath("install")
	title := fmt.Sprintf("[install %s@%s]", c.PortConfig.LibName, c.PortConfig.LibVersion)
	executor := cmd.NewExecutor(title, "cmake", args...)
	executor.SetWorkDir(c.PortConfig.BuildDir)
	executor.SetLogPath(logPath)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (c cmake) formatBuildType() string {
	switch c.BuildType {
	case "release":
		return "Release"

	case "debug":
		return "Debug"

	case "relwithdebinfo":
		return "RelWithDebInfo"

	case "minsizerel":
		return "MinSizeRel"

	default:
		return "Release"
	}
}

func (c *cmake) detectGenerator() error {
	if c.CMakeGenerator == "" {
		switch runtime.GOOS {
		case "darwin":
			c.CMakeGenerator = "Xcode"
		case "linux":
			c.CMakeGenerator = "Unix Makefiles"
		case "windows":
			msvcGenerator, err := detectMSVCGenerator()
			if err != nil {
				return err
			}
			c.CMakeGenerator = msvcGenerator
		}
	} else if c.CMakeGenerator != "Ninja" &&
		c.CMakeGenerator != "Unix Makefiles" &&
		c.CMakeGenerator != "Xcode" {
		return fmt.Errorf("unsupported cmake generator: %q", c.CMakeGenerator)
	}

	return nil
}

func (c cmake) multiConfigGenerator() bool {
	toolchain := c.Ctx.Platform().GetToolchain()

	if runtime.GOOS == "windows" {
		return toolchain.GetName() == "msvc" ||
			toolchain.GetName() == "clang-cl" ||
			toolchain.GetName() == "clang"
	}

	return false
}

func detectMSVCGenerator() (string, error) {
	// Query all available msvc installation paths.
	args := []string{
		"-products", "*",
		"-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
		"-property", "installationPath",
	}
	exector := cmd.NewExecutor("", "vswhere", args...)
	output, err := exector.ExecuteOutput()
	if err != nil {
		return "", err
	}

	// Trim the output.
	msvcDir := strings.TrimSpace(output)
	if msvcDir == "" {
		return "", fmt.Errorf("msvc not found, please install msvc first")
	}

	// return msvc name.
	switch {
	case strings.Contains(msvcDir, "2019"):
		return visualStudio_16_2019, nil
	case strings.Contains(msvcDir, "2022"):
		return visualStudio_17_2022, nil
	case strings.Contains(msvcDir, "2017"):
		return visualStudio_15_2017, nil
	case strings.Contains(msvcDir, "2015"):
		return visualStudio_14_2015, nil
	default:
		return "", fmt.Errorf("unsupported visual studio version: %s", msvcDir)
	}
}
