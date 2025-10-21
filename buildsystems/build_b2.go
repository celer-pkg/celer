package buildsystems

import (
	"bufio"
	"bytes"
	"celer/buildtools"
	"celer/context"
	"celer/pkgs/cmd"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strconv"
	"strings"
)

func NewB2(config *BuildConfig, optimize *context.Optimize) *b2 {
	return &b2{
		BuildConfig: config,
		Optimize:    optimize,
	}
}

type b2 struct {
	*BuildConfig
	*context.Optimize
}

func (b b2) Name() string {
	return "b2"
}

func (b b2) CheckTools() error {
	b.BuildConfig.BuildTools = append(b.BuildConfig.BuildTools, "git", "cmake")
	return buildtools.CheckTools(b.Ctx, b.BuildConfig.BuildTools...)
}

func (b b2) Clean() error {
	if fileio.PathExists(filepath.Join(b.PortConfig.SrcDir, "b2")) {
		title := fmt.Sprintf("[clean %s@%s]", b.PortConfig.LibName, b.PortConfig.LibVersion)
		executor := cmd.NewExecutor(title, "./b2", "clean")
		executor.SetWorkDir(b.PortConfig.SrcDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

func (b b2) configured() bool {
	b2Exist := filepath.Join(b.PortConfig.SrcDir, "b2"+expr.If(runtime.GOOS == "windows", ".exe", ""))
	configExist := filepath.Join(b.PortConfig.SrcDir, "project-config.jam")
	return fileio.PathExists(b2Exist) && fileio.PathExists(configExist)
}

func (b b2) Configure(options []string) error {
	// Clean build cache.
	if err := b.Clean(); err != nil {
		return err
	}

	// Join options into a string.
	configure := expr.If(runtime.GOOS == "windows", "./bootstrap.bat", "./bootstrap.sh")

	// Execute configure.
	logPath := b.getLogPath("configure")
	title := fmt.Sprintf("[configure %s]", b.PortConfig.nameVersionDesc())
	executor := cmd.NewExecutor(title, configure)
	executor.SetWorkDir(b.PortConfig.SrcDir)
	executor.SetLogPath(logPath)
	if err := executor.Execute(); err != nil {
		return err
	}

	// Modify project-config.jam to set cross-compiling tool for none-runtime library.
	if !b.DevDep {
		configPath := filepath.Join(b.PortConfig.SrcDir, "project-config.jam")
		file, err := os.OpenFile(configPath, os.O_RDONLY, os.ModePerm)
		if err != nil {
			return err
		}
		defer file.Close()

		// Override project-config.jam.
		var buffer bytes.Buffer
		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			line := scanner.Text()
			if strings.Contains(line, "using gcc ;") {
				line = fmt.Sprintf("using gcc : : %sg++ ;", b.PortConfig.Toolchain.CrosstoolPrefix)
			} else if strings.Contains(line, "using msvc ;") {
				line = fmt.Sprintf("using msvc : %s : %s ;", b.msvcVersion(), b.PortConfig.Toolchain.CXX)
			}
			buffer.WriteString(line + "\n")
		}

		// Write override `project-config.jam`.
		if err := os.WriteFile(configPath, buffer.Bytes(), os.ModePerm); err != nil {
			return err
		}
	}

	return nil
}

func (b b2) buildOptions() ([]string, error) {
	var options = slices.Clone(b.Options)

	// MSVC need to specify its version extactly.
	if b.PortConfig.Toolchain.Name == "msvc" {
		options = append(options, fmt.Sprintf("toolset=msvc-%s architecture=x86 address-model=64 install", b.msvcVersion()))
	} else {
		options = append(options, "toolset=gcc install")
	}

	// Set build type.
	switch b.BuildType {
	case "release":
		options = append(options, "variant=release")
	case "debug":
		options = append(options, "variant=debug")
	case "relwithdebinfo":
		options = append(options, "variant=release debug-symbols=on")
	case "minsizerel":
		options = append(options, "variant=release optimization=space")
	}

	// Set build cache dir.
	options = append(options, "--build-dir="+b.PortConfig.BuildDir)

	// Set build library type.
	libraryType := b.libraryType(
		"link=shared runtime-link=shared",
		"link=static runtime-link=static",
	)
	switch b.BuildConfig.LibraryType {
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

	// It's suggested for windows.
	options = append(options, "--abbreviate-paths")

	// Set install dir.
	options = append(options, fmt.Sprintf("--prefix=%s", b.PortConfig.PackageDir))

	// Replace placeholders.
	for index, value := range options {
		options[index] = b.replaceHolders(value)
	}

	return options, nil
}

func (b b2) Build(options []string) error {
	// Assemble command.
	joinedArgs := strings.Join(options, " ")
	command := fmt.Sprintf("%s/b2 %s -j %d", b.PortConfig.SrcDir, joinedArgs, b.PortConfig.Jobs)

	// Execute build.
	logPath := b.getLogPath("build")
	title := fmt.Sprintf("[build %s@%s]", b.PortConfig.LibName, b.PortConfig.LibVersion)
	executor := cmd.NewExecutor(title, command)
	executor.SetWorkDir(b.PortConfig.SrcDir)
	executor.SetLogPath(logPath)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (b b2) Install(options []string) error {
	// No stand-alone install process.
	return nil
}

func (b b2) msvcVersion() string {
	// Split by "." to get major, minor, patch
	parts := strings.Split(b.PortConfig.Toolchain.Version, ".")
	if len(parts) < 2 {
		panic(fmt.Errorf("invalid MSVC version format: %s", b.PortConfig.Toolchain.Version))
	}

	// Parse major and minor
	major, err := strconv.Atoi(parts[0])
	if err != nil {
		panic(fmt.Errorf("parse major version: %v", err))
	}

	minor, err := strconv.Atoi(parts[1])
	if err != nil {
		panic(fmt.Errorf("parse minor version: %v", err))
	}

	if minor >= 10 {
		minor = minor / 10
	}
	return fmt.Sprintf("%d.%d", major, minor)
}
