package buildsystems

import (
	"celer/buildtools"
	"celer/pkgs/cmd"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
)

const (
	visualStudio_17_2022 = "Visual Studio 17 2022"
	visualStudio_16_2019 = "Visual Studio 16 2019"
	visualStudio_15_2017 = "Visual Studio 15 2017 Win64"
	visualStudio_14_2015 = "Visual Studio 14 2015 Win64"
	visualStudio_12_2013 = "Visual Studio 12 2013 Win64"
)

func NewCMake(config *BuildConfig, optimize *Optimize, generator string) *cmake {
	// Set default generator if not specified.
	if generator == "" {
		switch runtime.GOOS {
		case "darwin":
			generator = "Xcode"
		case "linux":
			generator = "Unix Makefiles"
		case "windows":
			msvcGenerator, err := detectMSVCGenerator()
			if err != nil {
				color.Printf(color.Red, err.Error())
				return nil
			}
			generator = msvcGenerator
		}
	}

	// Format generator name.
	switch strings.ToLower(generator) {
	case "ninja":
		generator = "Ninja"
	case "makefiles":
		generator = "Unix Makefiles"
	case "xcode":
		generator = "Xcode"
	}

	return &cmake{
		BuildConfig: config,
		Optimize:    optimize,
		generator:   generator,
	}
}

type cmake struct {
	*BuildConfig
	*Optimize
	generator string // e.g. Ninja, Unix Makefiles, Visual Studio 16 2019, etc.
}

func (c cmake) Name() string {
	return "cmake"
}

func (c cmake) CheckTools() error {
	c.BuildConfig.BuildTools = append(c.BuildConfig.BuildTools, "git", "cmake")
	return buildtools.CheckTools(c.BuildConfig.BuildTools...)
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

	if c.PortConfig.CrossTools.Name == "msvc" {
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

	// Set CMAKE_PREFIX_PATH and CMAKE_INSTALL_PREFIX.
	options = append(options, "-DCMAKE_PREFIX_PATH="+filepath.Join(dirs.TmpDepsDir, c.PortConfig.LibraryFolder))
	options = append(options, "-DCMAKE_INSTALL_PREFIX="+c.PortConfig.PackageDir)

	// Replace placeholders.
	for index, value := range options {
		options[index] = c.replaceHolders(value)
	}

	return options, nil
}

func (c cmake) configured() bool {
	switch c.PortConfig.CrossTools.Name {
	case "msvc":
		cmakeCache := filepath.Join(c.PortConfig.BuildDir, "CMakeCache.txt")
		slnFile := filepath.Join(c.PortConfig.BuildDir, c.PortConfig.LibName+".sln")
		vcxprojFile := filepath.Join(c.PortConfig.BuildDir, c.PortConfig.LibName+".vcxproj")
		return fileio.PathExists(cmakeCache) && fileio.PathExists(slnFile) && fileio.PathExists(vcxprojFile)

	case "gcc":
		cmakeCache := filepath.Join(c.PortConfig.BuildDir, "CMakeCache.txt")
		makefile := filepath.Join(c.PortConfig.BuildDir, "Makefile")
		return fileio.PathExists(cmakeCache) && fileio.PathExists(makefile)
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
	joinedArgs := strings.Join(options, " ")
	var command string
	if c.generator == "" {
		command = fmt.Sprintf("cmake -S%s -B%s %s", c.PortConfig.SrcDir, c.PortConfig.BuildDir, joinedArgs)
	} else {
		command = fmt.Sprintf(`cmake -G"%s" -S%s -B%s %s`, c.generator, c.PortConfig.SrcDir, c.PortConfig.BuildDir, joinedArgs)
	}

	// Execute configure.
	logPath := c.getLogPath("configure")
	title := fmt.Sprintf("[configure %s]", c.PortConfig.nameVersionDesc())
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(logPath)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (c cmake) buildOptions() ([]string, error) {
	// CMAKE_BUILD_TYPE is useless for MSVC, use --config Debug/Relase instead.
	var options []string
	if c.PortConfig.CrossTools.Name == "msvc" {
		c.BuildType = c.formatBuildType()
		options = append(options, "--config", c.BuildType)
	}

	return options, nil
}

func (c cmake) Build(options []string) error {
	// Assemble command.
	joinedOptions := strings.Join(options, " ")
	command := fmt.Sprintf("cmake --build %s %s --parallel %d", c.PortConfig.BuildDir, joinedOptions, c.PortConfig.JobNum)

	// Execute build.
	logPath := c.getLogPath("build")
	title := fmt.Sprintf("[build %s@%s]", c.PortConfig.LibName, c.PortConfig.LibVersion)
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(logPath)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (c cmake) installOptions() ([]string, error) {
	// CMAKE_BUILD_TYPE is useless for MSVC, use --config Debug/Relase instead.
	var options []string
	if c.PortConfig.CrossTools.Name == "msvc" {
		c.BuildType = c.formatBuildType()
		options = append(options, "--config", c.BuildType)
	}

	return options, nil
}

func (c cmake) Install(options []string) error {
	// Assemble command.
	joinedOptions := strings.Join(options, " ")
	command := fmt.Sprintf("cmake --install %s %s", c.PortConfig.BuildDir, joinedOptions)

	// Execute install.
	logPath := c.getLogPath("install")
	title := fmt.Sprintf("[install %s@%s]", c.PortConfig.LibName, c.PortConfig.LibVersion)
	executor := cmd.NewExecutor(title, command)
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

func detectMSVCGenerator() (string, error) {
	if err := buildtools.CheckTools("vswhere"); err != nil {
		return "", fmt.Errorf("check tool vswhere error: %w", err)
	}

	// Query all available msvc installation paths.
	command := "vswhere -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath"
	exector := cmd.NewExecutor("", command)
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
