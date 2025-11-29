package buildsystems

import (
	"bufio"
	"bytes"
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

func (b b2) CheckTools() []string {
	b.BuildConfig.BuildTools = append(b.BuildConfig.BuildTools, "git", "cmake")
	return b.BuildConfig.BuildTools
}

func (b *b2) preConfigure() error {
	// `clang` inside visual studio cannot be used to compile b2 project.
	if runtime.GOOS == "windows" && strings.Contains(b.PortConfig.Toolchain.Fullpath, "Microsoft Visual Studio") {
		if b.PortConfig.Toolchain.Name == "clang" {
			return fmt.Errorf("visual studio's clang cannot be used to compile b2 project, msvc or clang-cl is required")
		}
	}

	return nil
}

func (b b2) configured() bool {
	b2file := expr.If(runtime.GOOS == "windows", "b2.exe", "b2")
	b2Exist := filepath.Join(b.PortConfig.SrcDir, b2file)
	configExist := filepath.Join(b.PortConfig.SrcDir, "project-config.jam")
	return fileio.PathExists(b.PortConfig.RepoDir) &&
		fileio.PathExists(b2Exist) &&
		fileio.PathExists(configExist)
}

func (b b2) Configure(options []string) error {
	// Clean build cache.
	if err := b.Clean(); err != nil {
		return err
	}

	// Execute configure.
	logPath := b.getLogPath("configure")
	title := fmt.Sprintf("[configure %s]", b.PortConfig.nameVersionDesc())
	configure := expr.If(runtime.GOOS == "windows", "bootstrap.bat", "./bootstrap.sh")
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
		cxx := filepath.Join(b.PortConfig.Toolchain.Fullpath, b.PortConfig.Toolchain.CXX)

		var buffer bytes.Buffer
		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			line := scanner.Text()
			if strings.Contains(line, "using gcc ;") {
				if b.PortConfig.Toolchain.CCacheEnabled {
					line = fmt.Sprintf(`using gcc : : "ccache" "%s" ;`, filepath.ToSlash(cxx))
				} else {
					line = fmt.Sprintf(`using gcc : : "%s" ;`, filepath.ToSlash(cxx))
				}
			} else if strings.Contains(line, "using msvc ;") {
				switch b.PortConfig.Toolchain.Name {
				case "clang-cl":
					if b.PortConfig.Toolchain.CCacheEnabled {
						line = fmt.Sprintf(`using clang-win : : "ccache" "%s" ;`, filepath.ToSlash(cxx))
					} else {
						line = fmt.Sprintf(`using clang-win : : "%s" ;`, filepath.ToSlash(cxx))
					}

				case "msvc":
					if b.PortConfig.Toolchain.CCacheEnabled {
						line = fmt.Sprintf(`using msvc : %s : "ccache" "%s" ;`, b.msvcVersion(), filepath.ToSlash(cxx))
					} else {
						line = fmt.Sprintf(`using msvc : %s : "%s" ;`, b.msvcVersion(), filepath.ToSlash(cxx))
					}

				default:
					return fmt.Errorf("unsupported toolchain: %s for b2", b.PortConfig.Toolchain.Name)
				}
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

	// Set build toolset.
	switch runtime.GOOS {
	case "windows":
		switch b.PortConfig.Toolchain.Name {
		case "msvc":
			options = append(options, "toolset=msvc-"+b.msvcVersion())
		case "clang-cl":
			options = append(options, "toolset=clang-win")
		default:
			return nil, fmt.Errorf("unsupported toolchain: %s for b2", b.PortConfig.Toolchain.Name)
		}
	case "linux":
		// Set build toolset with toolchain name.
		options = append(options, "toolset="+b.PortConfig.Toolchain.Name)
	default:
		return nil, fmt.Errorf("unsupported platform: %s", runtime.GOOS)
	}

	// Set build architecture.
	switch b.PortConfig.Toolchain.SystemProcessor {
	case "x86_64", "amd64":
		options = append(options, "address-model=64", "architecture=x86")
	case "x86":
		options = append(options, "address-model=32", "architecture=x86")
	case "arm64", "aarch64":
		options = append(options, "address-model=64", "architecture=arm")
	case "arm":
		options = append(options, "address-model=32", "architecture=arm")
	default:
		return nil, fmt.Errorf("unsupported architecture: %s", b.PortConfig.Toolchain.SystemProcessor)
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

	// Note: `threading=multi` will make boost link pthread in linux.
	// In embedded system, `threading=multi` is not supported,
	// then you can set `threading=single` in port.toml.
	if !slices.ContainsFunc(b.Options, func(opt string) bool {
		return strings.HasPrefix(opt, "threading=")
	}) {
		options = append(options, "threading=multi")
	}

	// Set install dir.
	options = append(options, fmt.Sprintf("--prefix=%s", b.PortConfig.PackageDir))
	options = append(options, "install")

	// Replace placeholders.
	for index, value := range options {
		options[index] = b.replaceHolders(value)
	}

	return options, nil
}

func (b b2) Build(options []string) error {
	// Assemble command.
	b2file := expr.If(runtime.GOOS == "windows", "b2.exe", "b2")
	joinedArgs := strings.Join(options, " ")
	command := fmt.Sprintf("%s/%s %s -j %d", b.PortConfig.SrcDir, b2file, joinedArgs, b.PortConfig.Jobs)

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
