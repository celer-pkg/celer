package buildsystems

import (
	"bufio"
	"bytes"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strconv"
	"strings"

	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

func NewB2(config *BuildConfig) *b2 {
	return &b2{
		BuildConfig: config,
	}
}

type b2 struct {
	*BuildConfig
}

func (b b2) Name() string {
	return "b2"
}

func (b b2) CheckTools() []string {
	// Start with build_tools from port.toml
	tools := slices.Clone(b.BuildConfig.BuildTools)

	// Add default tools
	tools = append(tools, "cmake")
	return tools
}

func (b *b2) preConfigure() error {
	toolchain := b.Ctx.Platform().GetToolchain()

	// Boost b2 does not support clang on Windows.
	// b2's clang toolset uses MSVC-style link flags (/LIBPATH) incompatible
	// with clang's MinGW mode. Use msvc or clang-cl toolchain instead.
	if runtime.GOOS == "windows" && toolchain.GetName() == "clang" {
		return fmt.Errorf("boost b2 does not support clang on Windows, use msvc or clang-cl toolchain")
	}

	// `clang` inside visual studio cannot be used to compile b2 project.
	if runtime.GOOS == "windows" && strings.Contains(toolchain.GetAbsDir(), "Microsoft Visual Studio") {
		if toolchain.GetName() == "clang" {
			return fmt.Errorf("visual studio's clang cannot be used to compile b2 project, msvc or clang-cl is required")
		}
	}

	// For MSVC build, we need to set PATH, INCLUDE and LIB env vars.
	if runtime.GOOS == "windows" {
		if toolchain.GetName() == "msvc" || toolchain.GetName() == "clang-cl" {
			msvcEnvs, err := b.readMSVCEnvs()
			if err != nil {
				return err
			}

			b.envBackup.setenv("PATH", msvcEnvs["PATH"])
			b.envBackup.setenv("INCLUDE", msvcEnvs["INCLUDE"])
			b.envBackup.setenv("LIB", msvcEnvs["LIB"])
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
	toolchain := b.Ctx.Platform().GetToolchain()

	// Clean build cache.
	if err := b.Clean(); err != nil {
		return err
	}

	// Execute configure.
	logPath := b.getLogPath("configure")
	title := fmt.Sprintf("[configure %s]", b.PortConfig.nameVersion())
	configure := expr.If(runtime.GOOS == "windows", "bootstrap.bat", "./bootstrap.sh")

	// For cross-compilation, set --prefix to dependency directory.
	rootfs := b.Ctx.RootFS()
	if !b.DevDep && rootfs != nil {
		depsDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryDir)
		configure = fmt.Sprintf("%s --prefix=%s", configure, depsDir)
	}

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
		cxx := filepath.Join(toolchain.GetAbsDir(), toolchain.GetCXX())

		// For cross-compilation, use version identifier to distinguish toolchain.
		var toolchainVersion string
		rootfs := b.Ctx.RootFS()
		if !b.DevDep && rootfs != nil {
			toolchainVersion = toolchain.GetVersion()
		} else {
			toolchainVersion = ""
		}

		// Determine whether the toolchain is Clang.
		// In windows, "clang-cl" is not a real clang toolchain, it's a wrapper of MSVC.
		isClang := strings.Contains(toolchain.GetName(), "clang") && toolchain.GetName() != "clang-cl"
		isQNX := toolchain.GetName() == "qcc"

		var buffer bytes.Buffer
		scanner := bufio.NewScanner(file)
		for scanner.Scan() {
			line := scanner.Text()
			if strings.Contains(line, "using gcc ;") {
				switch {
				case isQNX:
					fmt.Fprintf(&buffer, "%s\n", line)
					fmt.Fprintf(&buffer, "%s\n", b.formatUsingToolset("qcc", toolchainVersion, cxx))
					continue
				case isClang:
					toolchainRoot := filepath.Dir(toolchain.GetAbsDir())
					platformTriple, err := b.detectPlatformTriple(toolchainRoot)
					if err != nil {
						return fmt.Errorf("failed to detect platform triple -> %w", err)
					}

					libcxxInclude := filepath.Join(toolchainRoot, "include", "c++", "v1")
					platformInclude := filepath.Join(toolchainRoot, "include", platformTriple, "c++", "v1")
					platformLib := filepath.Join(toolchainRoot, "lib", platformTriple)

					var compilerCmd string
					if b.Ctx.CCacheEnabled() {
						compilerCmd = fmt.Sprintf(`"ccache" "%s"`, filepath.ToSlash(cxx))
					} else {
						compilerCmd = fmt.Sprintf(`"%s"`, filepath.ToSlash(cxx))
					}

					compilerOptions := fmt.Sprintf(`<cxxflags>"-stdlib=libc++ -isystem %s -isystem %s" <linkflags>"-stdlib=libc++ -L%s"`,
						filepath.ToSlash(libcxxInclude), filepath.ToSlash(platformInclude), filepath.ToSlash(platformLib))
					if toolchainVersion != "" {
						line = fmt.Sprintf(`using clang : %s : %s : %s ;`, toolchainVersion, compilerCmd, compilerOptions)
					} else {
						line = fmt.Sprintf(`using clang : : %s : %s ;`, compilerCmd, compilerOptions)
					}
				default:
					line = b.formatUsingToolset("gcc", toolchainVersion, cxx)
				}
			} else if strings.Contains(line, "using msvc ;") || strings.Contains(line, "using clang-win") {
				switch toolchain.GetName() {
				case "clang-cl":
					line = b.formatUsingToolset("clang-win", "", cxx)
				case "msvc":
					line = b.formatUsingToolset("msvc", b.msvcVersion(), cxx)
				default:
					return fmt.Errorf("unsupported toolchain: %s for b2", toolchain.GetName())
				}
			}
			fmt.Fprintf(&buffer, "%s\n", line)
		}
		if err := scanner.Err(); err != nil {
			return err
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
	toolchain := b.Ctx.Platform().GetToolchain()
	toolchainName := toolchain.GetName()

	// Set build toolset with version for cross-compilation.
	var toolsetName string
	switch toolchainName {
	case "msvc":
		toolsetName = "msvc-" + b.msvcVersion()
	case "clang-cl":
		toolsetName = "clang-win"
	case "gcc":
		toolsetName = "gcc"
	case "clang":
		toolsetName = "clang"
	case "qcc":
		toolsetName = "qcc"
	default:
		return nil, fmt.Errorf("unsupported toolchain: %s for b2", toolchain.GetName())
	}
	rootfs := b.Ctx.RootFS()
	if !b.DevDep && rootfs != nil {
		options = append(options, "toolset="+toolsetName+"-"+toolchain.GetVersion())
	} else {
		options = append(options, "toolset="+toolsetName)
	}

	// Set target-os and architecture for QNX.
	isQNX := toolchainName == "qcc"
	if isQNX {
		options = append(options, "target-os=qnxnto")
	}

	// Set compiler and linker flags for cross-compilation.
	if !b.DevDep && rootfs != nil {
		sysroot := rootfs.GetAbsDir()
		if sysroot != "" && !strings.Contains(toolchain.GetName(), "clang") && !isQNX {
			options = append(options, fmt.Sprintf(`cflags="--sysroot=%s"`, sysroot))
			options = append(options, fmt.Sprintf(`cxxflags="--sysroot=%s"`, sysroot))
			options = append(options, fmt.Sprintf(`linkflags="--sysroot=%s"`, sysroot))
		}
	}

	// Boost build system (b2) requires architecture and ABI specifications.
	//
	// ABI (Application Binary Interface) Selection Strategy:
	// - x86/x86_64: Boost Jamfile has default rules for SYSV ABI (Linux) and MS ABI (Windows).
	//   No explicit ABI needed; b2 will auto-select based on platform.
	// - ARM32/ARM64: Boost Jamfile ONLY has AAPCS ABI rules, NO SYSV rules exist.
	//   Without explicit "abi=aapcs", b2 defaults to SYSV which has no ARM rules,
	//   causing build failure: "No best alternative for asm_sources".
	//   Solution: Always specify "abi=aapcs" for ARM architectures.
	switch toolchain.GetSystemProcessor() {
	case "x86_64", "amd64":
		options = append(options, "address-model=64", "architecture=x86")
	case "x86":
		options = append(options, "address-model=32", "architecture=x86")
	case "arm64", "aarch64":
		options = append(options, "address-model=64", "architecture=arm", "abi=aapcs")
	case "arm":
		options = append(options, "address-model=32", "architecture=arm", "abi=aapcs")
	default:
		return nil, fmt.Errorf("unsupported architecture: %s", toolchain.GetSystemProcessor())
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

	// Set build library type(remove user defined first).
	options = slices.DeleteFunc(options, func(element string) bool {
		return strings.HasPrefix(element, "link=") ||
			strings.HasPrefix(element, "runtime-link=")
	})
	libraryType := b.BuildConfig.buildLibraryType()
	switch {
	case libraryType.shared && libraryType.static:
		options = append(options, "link=shared,static runtime-link=shared,static")
	case libraryType.static:
		options = append(options, "link=static runtime-link=static")
	default:
		options = append(options, "link=shared runtime-link=shared")
	}

	// Note: `threading=multi` is the default config for boost.
	// In embedded system, if `threading=multi` is not supported,
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
		options[index] = b.expandVariables(value)
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
	toolchain := b.Ctx.Platform().GetToolchain()

	// Split by "." to get major, minor, patch
	parts := strings.Split(toolchain.GetVersion(), ".")
	if len(parts) < 2 {
		panic(fmt.Errorf("invalid MSVC version format: %s", toolchain.GetVersion()))
	}

	// Parse major and minor
	major, err := strconv.Atoi(parts[0])
	if err != nil {
		panic(fmt.Errorf("parse major version -> %w", err))
	}

	minor, err := strconv.Atoi(parts[1])
	if err != nil {
		panic(fmt.Errorf("parse minor version -> %w", err))
	}

	if minor >= 10 {
		minor = minor / 10
	}
	return fmt.Sprintf("%d.%d", major, minor)
}

// detectPlatformTriple dynamically detects the platform triple subdirectory
// in LLVM's include and lib directories. This avoids hardcoding platform-specific
// paths like "x86_64-unknown-linux-gnu".
func (b b2) detectPlatformTriple(toolchainRoot string) (string, error) {
	// Look for platform-specific subdirectories in include/
	includePath := filepath.Join(toolchainRoot, "include")
	entries, err := os.ReadDir(includePath)
	if err != nil {
		return "", fmt.Errorf("failed to read include directory -> %w", err)
	}

	// Find the first subdirectory that contains c++/v1/__config_site
	for _, entry := range entries {
		if entry.IsDir() && entry.Name() != "c++" {
			configSite := filepath.Join(includePath, entry.Name(), "c++", "v1", "__config_site")
			if fileio.PathExists(configSite) {
				return entry.Name(), nil
			}
		}
	}

	return "", fmt.Errorf("could not find platform-specific subdirectory with __config_site")
}

func (b b2) formatUsingToolset(toolset, version, cxx string) string {
	var compilerCmd string
	if b.Ctx.CCacheEnabled() {
		compilerCmd = fmt.Sprintf(`"ccache" "%s"`, filepath.ToSlash(cxx))
	} else {
		compilerCmd = fmt.Sprintf(`"%s"`, filepath.ToSlash(cxx))
	}
	if version != "" {
		return fmt.Sprintf(`using %s : %s : %s ;`, toolset, version, compilerCmd)
	}
	return fmt.Sprintf(`using %s : : %s ;`, toolset, compilerCmd)
}
