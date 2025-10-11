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

func (c cmake) CheckTools() error {
	c.BuildConfig.BuildTools = append(c.BuildConfig.BuildTools, "git", "cmake")
	if c.CMakeGenerator == "Ninja" {
		c.BuildConfig.BuildTools = append(c.BuildConfig.BuildTools, "ninja")
	}
	return buildtools.CheckTools(c.Offline, c.Proxy, c.BuildTools...)
}

func (c cmake) Clean() error {
	// We do not configure cmake project in source folder.
	return nil
}

func (c cmake) configureOptions() ([]string, error) {
	// Format as cmake build type.
	c.BuildType = c.formatBuildType()

	var options = slices.Clone(c.Options)

	if !c.BuildConfig.DevDep {
		options = append(options, fmt.Sprintf("-DCMAKE_TOOLCHAIN_FILE=%s/toolchain_file.cmake", dirs.WorkspaceDir))
	}

	// Set CMAKE_INSTALL_PREFIX.
	options = append(options, "-DCMAKE_INSTALL_PREFIX="+c.PortConfig.PackageDir)

	if c.PortConfig.Toolchain.Name == "msvc" {
		// MSVC doesn't support set `CMAKE_BUILD_TYPE` or `--config` during configure.
		options = slices.DeleteFunc(options, func(element string) bool {
			return strings.Contains(element, "CMAKE_BUILD_TYPE") || strings.Contains(element, "--config")
		})
	} else {
		// Append `CMAKE_BUILD_TYPE` if not contains it.
		if c.DevDep {
			options = append(options, "-DCMAKE_BUILD_TYPE=Release")
		} else {
			options = append(options, "-DCMAKE_BUILD_TYPE="+c.BuildType)
		}
	}

	// Set build library type.
	libraryType := c.libraryType(
		"-DBUILD_SHARED_LIBS=ON",
		"-DBUILD_SHARED_LIBS=OFF",
	)
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

	// Override `CMAKE_FIND_ROOT_PATH` defined in toolchain file.
	findRootPaths := []string{filepath.Join(dirs.TmpDepsDir, c.PortConfig.LibraryFolder)}
	if c.PortConfig.Toolchain.RootFS != "" {
		findRootPaths = append(findRootPaths, c.PortConfig.Toolchain.RootFS)
	}
	options = append(options, fmt.Sprintf("-DCMAKE_FIND_ROOT_PATH=%q", filepath.ToSlash(strings.Join(findRootPaths, ";"))))

	// Enable verbose makefile.
	if c.PortConfig.Toolchain.Verbose {
		options = append(options, "-DCMAKE_VERBOSE_MAKEFILE=ON")
	}

	// Replace placeholders.
	for index, value := range options {
		options[index] = c.replaceHolders(value)
	}

	return options, nil
}

func (c cmake) configured() bool {
	if err := c.detectGenerator(); err != nil {
		color.Printf(color.Red, "Detect generator error: %s\n", err)
		return false
	}

	switch c.CMakeGenerator {
	case "Ninja":
		cmakeCache := filepath.Join(c.PortConfig.BuildDir, "CMakeCache.txt")
		buildFile := filepath.Join(c.PortConfig.BuildDir, "build.ninja")
		ruluesFile := filepath.Join(c.PortConfig.BuildDir, "rules.ninja")
		return fileio.PathExists(cmakeCache) && fileio.PathExists(buildFile) && fileio.PathExists(ruluesFile)

	case "Unix Makefiles":
		cmakeCache := filepath.Join(c.PortConfig.BuildDir, "CMakeCache.txt")
		makefile := filepath.Join(c.PortConfig.BuildDir, "Makefile")
		return fileio.PathExists(cmakeCache) && fileio.PathExists(makefile)

	case visualStudio_17_2022, visualStudio_16_2019, visualStudio_15_2017, visualStudio_14_2015:
		cmakeCache := filepath.Join(c.PortConfig.BuildDir, "CMakeCache.txt")
		slnFile := filepath.Join(c.PortConfig.BuildDir, c.PortConfig.LibName+".sln")
		vcxprojFile := filepath.Join(c.PortConfig.BuildDir, c.PortConfig.LibName+".vcxproj")
		return fileio.PathExists(cmakeCache) && fileio.PathExists(slnFile) && fileio.PathExists(vcxprojFile)
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
	executor.SetLogPath(logPath)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (c cmake) buildOptions() ([]string, error) {
	// CMAKE_BUILD_TYPE is useless for MSVC, use --config Debug/Relase instead.
	var options []string
	if c.PortConfig.Toolchain.Name == "msvc" {
		c.BuildType = c.formatBuildType()
		options = append(options, "--config", c.BuildType)
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
	executor.SetLogPath(logPath)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (c cmake) installOptions() ([]string, error) {
	// CMAKE_BUILD_TYPE is useless for MSVC, use --config Debug/Relase instead.
	var options []string
	if c.PortConfig.Toolchain.Name == "msvc" {
		c.BuildType = c.formatBuildType()
		options = append(options, "--config", c.BuildType)
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
	executor.SetLogPath(logPath)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (c cmake) formatBuildType() string {
	switch strings.ToLower(c.BuildType) {
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
			msvcGenerator, err := detectMSVCGenerator(c.BuildConfig.Offline, c.BuildConfig.Proxy)
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

func detectMSVCGenerator(offline bool, proxy *context.Proxy) (string, error) {
	if err := buildtools.CheckTools(offline, proxy, "vswhere"); err != nil {
		return "", fmt.Errorf("check tool vswhere error: %w", err)
	}

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
