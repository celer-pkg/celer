package buildsystems

import (
	"celer/buildtools"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/env"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
)

type envsBackup struct {
	originalEnvs map[string]string
	modifiedEnvs map[string]bool
}

func (e *envsBackup) backup() {
	e.originalEnvs = make(map[string]string)
	for _, env := range os.Environ() {
		parts := strings.SplitN(env, "=", 2)
		e.originalEnvs[parts[0]] = parts[1]
	}
	e.modifiedEnvs = make(map[string]bool)
}

func (e *envsBackup) rollback() {
	for key := range e.modifiedEnvs {
		if originalValue, exists := e.originalEnvs[key]; exists {
			os.Setenv(key, originalValue)
		} else {
			os.Unsetenv(key)
		}
	}
}

func (e *envsBackup) setenv(key, value string) {
	os.Setenv(key, value)
	e.modifiedEnvs[key] = true
}

func (b *BuildConfig) setupEnvs() {
	b.envBackup.backup()

	for _, env := range b.Envs {
		env = strings.TrimSpace(env)

		before, after, ok := strings.Cut(env, "=")
		if !ok {
			color.Printf(color.Warning, "invalid environment variable `%s` and is ignored.", env)
			continue
		}

		key := strings.TrimSpace(before)
		currentValue := strings.TrimSpace(after)
		currentValue = b.expandVariables(currentValue)

		switch key {
		case "CPATH", "LIBRARY_PATH", "PATH":
			lastValue := filepath.ToSlash(os.Getenv(key))
			if strings.TrimSpace(lastValue) == "" {
				b.envBackup.setenv(key, currentValue)
			} else {
				b.envBackup.setenv(key, fmt.Sprintf("%s%s%s", currentValue, string(os.PathListSeparator), lastValue))
			}

		case "CFLAGS", "CXXFLAGS", "LDFLAGS", "CPPFLAGS":
			// celer can wrap CFLAGS, CXXFLAGS and CPPFLAGS automatically, so we need to remove them.
			currentValue = strings.ReplaceAll(currentValue, "${CFLAGS}", "")
			currentValue = strings.ReplaceAll(currentValue, "${CXXFLAGS}", "")
			currentValue = strings.ReplaceAll(currentValue, "${CPPFLAGS}", "")
			currentValue = strings.ReplaceAll(currentValue, "${LDFLAGS}", "")

			lastValue := filepath.ToSlash(os.Getenv(key))
			if strings.TrimSpace(lastValue) == "" {
				b.envBackup.setenv(key, strings.TrimSpace(currentValue))
			} else {
				b.envBackup.setenv(key, fmt.Sprintf("%s %s", lastValue, currentValue))
			}

		default:
			b.envBackup.setenv(key, currentValue)
		}
	}

	if b.buildSystem.Name() != "cmake" {
		// This allows the bin to locate the libraries in the relative lib dir.
		toolchain := b.Ctx.Platform().GetToolchain()
		if strings.ToLower(toolchain.GetSystemName()) == "linux" &&
			b.buildSystem.Name() == "makefiles" {
			b.envBackup.setenv("LDFLAGS", env.JoinSpace("-Wl,-rpath=\\$$ORIGIN/../lib", os.Getenv("LDFLAGS")))
		}

		// C/C++ standard.
		b.setLanguageStandard()

		// Set CFLGAGS/CXXFLAGS/LDFLAGS.
		b.setEnvFlags()

		// Setup pkg-config.
		b.setupPkgConfig()
	}

	// Set CFLGAGS/CXXFLAGS/LDFLAGS.
	b.setEnvFlags()

	// Setup pkg-config.
	b.setupPkgConfig()

	tmpDevDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.HostName+"-dev")

	// Set ACLOCAL_PATH for ports that rely on macros.
	if slices.ContainsFunc(b.DevDependencies, func(element string) bool {
		return strings.HasPrefix(element, "macros@")
	}) {
		joined := env.JoinPaths("ACLOCAL_PATH", filepath.Join(tmpDevDir, "share", "aclocal"))
		b.envBackup.setenv("ACLOCAL_PATH", joined)
	}

	if slices.ContainsFunc(b.DevDependencies, func(element string) bool {
		return strings.HasPrefix(element, "libtool@")
	}) {
		joined := env.JoinPaths("ACLOCAL_PATH", filepath.Join(tmpDevDir, "share", "libtool"))
		b.envBackup.setenv("ACLOCAL_PATH", joined)
	}

	// Expose dev/bin and python venv bin to PATH.
	venvBin := filepath.Join(dirs.PythonUserBase, "bin")
	if fileio.PathExists(venvBin) {
		b.envBackup.setenv("PATH", env.JoinPaths("PATH", venvBin))
	}

	// Expose dev/bin to PATH.
	b.envBackup.setenv("PATH", env.JoinPaths("PATH", filepath.Join(tmpDevDir, "bin")))

	// Ensure PYTHONPATH for python.
	b.envBackup.setenv("PYTHONUSERBASE", dirs.PythonUserBase)

	// Expose LLVM_CONFIG for ports that rely on llvm-config.
	if buildtools.LLVMPath != "" {
		llvmConfig := expr.If(runtime.GOOS == "windows", "llvm-config.exe", "llvm-config")
		b.envBackup.setenv("LLVM_CONFIG", filepath.Join(buildtools.LLVMPath, "bin", llvmConfig))
	}

	// Find the actual site-packages directory and set PYTHONPATH's value with it.
	// (Python installs to python3/lib/python3.X/site-packages).
	libDir := filepath.Join(dirs.PythonUserBase, "lib")
	if fileio.PathExists(libDir) {
		entries, err := os.ReadDir(libDir)
		if err == nil {
			for _, entry := range entries {
				if entry.IsDir() && strings.HasPrefix(entry.Name(), "python") {
					sitePackages := filepath.Join(libDir, entry.Name(), "site-packages")
					if fileio.PathExists(sitePackages) {
						b.envBackup.setenv("PYTHONPATH", sitePackages)
						break
					}
				}
			}
		}
	}

	// Provider environment variables for QNX.
	toolchain := b.Ctx.Platform().GetToolchain()
	if toolchain.GetName() == "qcc" {
		b.envBackup.setenv("CFLAGS", env.JoinSpace("-D_QNX_SOURCE", os.Getenv("CFLAGS")))
		b.envBackup.setenv("CXXFLAGS", env.JoinSpace("-D_QNX_SOURCE", os.Getenv("CXXFLAGS")))

		// QNX_HOST and QNX_TARGET is mandatory.
		switch runtime.GOOS {
		case "linux":
			b.envBackup.setenv("QNX_HOST", filepath.Join(toolchain.GetAbsPath(), "host/linux/x86_64"))
		case "windows":
			b.envBackup.setenv("QNX_HOST", filepath.Join(toolchain.GetAbsPath(), "host/win64/x86_64/usr/bin"))
		case "darwin":
			b.envBackup.setenv("QNX_HOST", filepath.Join(toolchain.GetAbsPath(), "host/darwin/x86_64/usr/bin"))
		}
		b.envBackup.setenv("QNX_TARGET", filepath.Join(toolchain.GetAbsPath(), "target/qnx"))
	}
}

func (b BuildConfig) setupPkgConfig() {
	var (
		configPaths   []string
		configLibDirs []string
		pathDivider   string
		sysrootDir    string
	)

	rootfs := b.Ctx.Platform().GetRootFS()
	tmpDepsDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryFolder)

	switch runtime.GOOS {
	case "windows":
		if b.buildSystem.Name() == "meson" || b.buildSystem.Name() == "b2" {
			// For meson in windows, we use windows version pkgconf,
			// so we need to provider with windows format path.
			configPaths = []string{
				filepath.Join(tmpDepsDir, "lib", "pkgconfig"),
				filepath.Join(tmpDepsDir, "share", "pkgconfig"),
			}

			sysrootDir = tmpDepsDir
			pathDivider = ";"
		} else {
			configPaths = []string{
				fileio.ToCygpath(filepath.Join(tmpDepsDir, "lib", "pkgconfig")),
				fileio.ToCygpath(filepath.Join(tmpDepsDir, "share", "pkgconfig")),
			}

			sysrootDir = fileio.ToCygpath(tmpDepsDir)
			pathDivider = ":"
		}

	case "linux":
		// Pkg config paths and sysroot dir.
		if rootfs != nil {
			sysrootDir = rootfs.GetAbsPath()

			// PKG_CONFIG related.
			for _, configPath := range rootfs.GetPkgConfigPath() {
				configLibDirs = append(configLibDirs, filepath.Join(sysrootDir, configPath))
			}

			pathDivider = ":"

			// Use actual tmpDepsDir path (not sysroot symlink) for pkgconfig to ensure correct library paths
			tmpDepsDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryFolder)

			// Prepend pkgconfig with actual tmp/deps directory (not sysroot symlink) to prioritize it.
			configPaths = append([]string{
				filepath.Join(tmpDepsDir, "lib", "pkgconfig"),
				filepath.Join(tmpDepsDir, "share", "pkgconfig"),
			}, configPaths...)
		} else {
			configPaths = []string{
				filepath.Join(tmpDepsDir, "lib", "pkgconfig"),
				filepath.Join(tmpDepsDir, "share", "pkgconfig"),
			}

			// In this case, there is no rootfs and the pc prefix would be `/`,
			// to make sure pkgconf can work, we need to create a virtual rootfs for pkgconf.
			sysrootDir = dirs.WorkspaceDir
			pathDivider = ":"
		}
	}

	// Set merged pkgconfig envs.
	if len(configLibDirs) > 0 {
		b.envBackup.setenv("PKG_CONFIG_LIBDIR", strings.Join(configLibDirs, pathDivider))
	}
	b.envBackup.setenv("PKG_CONFIG_PATH", strings.Join(configPaths, pathDivider))

	// For dev dependencies, .pc files use absolute paths, so we should not set PKG_CONFIG_SYSROOT_DIR.
	// PKG_CONFIG_SYSROOT_DIR is only needed for cross-compilation when .pc files use relative paths.
	b.envBackup.setenv("PKG_CONFIG_SYSROOT_DIR", expr.If(b.DevDep, "", sysrootDir))
}

func (b *BuildConfig) setLanguageStandard() {
	toolchain := b.Ctx.Platform().GetToolchain()

	// Set C standard.
	cstandard := expr.If(b.CStandard != "", b.CStandard, toolchain.GetCStandard())
	if cstandard != "" {
		var cflag string
		switch toolchain.GetName() {
		case "gcc", "clang":
			cflag = "-std=" + cstandard

		case "msvc", "clang-cl":
			cflag = "/std:" + cstandard

		default:
			panic("unsupported toolchain: " + toolchain.GetName())
		}

		b.envBackup.setenv("CFLAGS", env.JoinSpace(cflag, os.Getenv("CFLAGS")))
	}

	// Set C++ standard.
	cxxstandard := expr.If(b.CXXStandard != "", b.CXXStandard, toolchain.GetCXXStandard())
	if cxxstandard != "" {
		var cxxflag string
		switch toolchain.GetName() {
		case "gcc", "clang":
			cxxflag = "-std=" + cxxstandard

		case "msvc", "clang-cl":
			cxxflag = "/std:" + cxxstandard

		default:
			panic("unsupported toolchain: " + toolchain.GetName())
		}

		b.envBackup.setenv("CXXFLAGS", env.JoinSpace(cxxflag, os.Getenv("CXXFLAGS")))
	}
}

func (b *BuildConfig) setEnvFlags() {
	rootfs := b.Ctx.Platform().GetRootFS()
	tmpDepsDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryFolder)

	// sysroot and tmp dir.
	if rootfs != nil {
		// Set sysroot.
		sysrootDir := rootfs.GetAbsPath()
		b.envBackup.setenv("SYSROOT", sysrootDir)

		// Update CFLAGS/CXXFLAGS
		b.appendIncludeDir(filepath.Join(tmpDepsDir, "include"))
		for _, item := range rootfs.GetIncludeDirs() {
			includeDir := filepath.Join(sysrootDir, item)
			b.appendIncludeDir(includeDir)
		}

		// Update LDFLAGS
		// Add dependency lib dir first (so it takes higher priority than sysroot lib dirs).
		b.appendLibDir(filepath.Join(tmpDepsDir, "lib"))

		// Add sysroot lib dirs.
		for _, item := range rootfs.GetLibDirs() {
			libDir := filepath.Join(sysrootDir, item)
			if fileio.PathExists(libDir) {
				b.appendLibDir(libDir)
			}
		}
	} else {
		// Update CFLAGS/CXXFLAGS/LDFLAGS
		b.appendIncludeDir(filepath.Join(tmpDepsDir, "include"))
		b.appendLibDir(filepath.Join(tmpDepsDir, "lib"))
	}
}

func (b BuildConfig) rollbackEnvs() {
	b.envBackup.rollback()
}

func (b *BuildConfig) appendIncludeDir(includeDir string) {
	// Windows: MSVC/Clang-cl ------------------------------ Linux: GCC/Clang
	// INCLUDE=xxx\include;%INCLUDE%  ---------------------- -I "xxx\include"
	// CL=/external:anglebrackets /external:W0 %CL% -------- -I "xxx\include"

	// Toolchain may throw error if include dir not exists.
	if !fileio.PathExists(includeDir) {
		return
	}

	toolchain := b.Ctx.Platform().GetToolchain()
	switch toolchain.GetName() {
	case "gcc", "clang", "qcc":
		cflags := strings.Fields(os.Getenv("CFLAGS"))
		cxxflags := strings.Fields(os.Getenv("CXXFLAGS"))

		// Check if this is a dependency include dir (tmpDeps/include) - if so, prepend it.
		tmpDepsPrefix := filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryFolder)
		isDepsIncludeDir := strings.Contains(includeDir, tmpDepsPrefix)

		// Append include dir if not exists.
		// Prepend dependency include dir flags to prioritize them.
		includeFlag := "-I" + includeDir
		var newAppended = false
		if !slices.Contains(cflags, includeFlag) {
			if isDepsIncludeDir {
				cflags = append([]string{includeFlag}, cflags...)
			} else {
				cflags = append(cflags, includeFlag)
			}
			newAppended = true
		}
		if !slices.Contains(cxxflags, includeFlag) {
			if isDepsIncludeDir {
				cxxflags = append([]string{includeFlag}, cxxflags...)
			} else {
				cxxflags = append(cxxflags, includeFlag)
			}
			newAppended = true
		}

		// Update environment variable with modified flags.
		if newAppended {
			b.envBackup.setenv("CFLAGS", strings.Join(cflags, " "))
			b.envBackup.setenv("CXXFLAGS", strings.Join(cxxflags, " "))
		}

	case "msvc", "clang-cl":
		// Append include dir if not exists.
		includes := strings.Fields(os.Getenv("INCLUDE"))
		if !slices.Contains(includes, includeDir) {
			includes = append(includes, includeDir)
			b.envBackup.setenv("INCLUDE", strings.Join(includes, ";"))
		}

		// Avoid warning by setting "CL=/external:anglebrackets /external:W0 %CL%"
		cl := strings.Fields(os.Getenv("CL"))
		if !slices.Contains(cl, "/external:anglebrackets") {
			cl = append(cl, "/external:anglebrackets")
			b.envBackup.setenv("CL", strings.Join(cl, " "))
		}

		// Below setting seems cannot work, it seems that MSVC use "/external:W3" by default,
		// and we cannot change it.
		if !slices.Contains(cl, "/external:W0") {
			cl = append(cl, "/external:W0")
			b.envBackup.setenv("CL", strings.Join(cl, " "))
		}

	default:
		panic("unsupported toolchain: " + toolchain.GetName())
	}
}

func (b *BuildConfig) appendLibDir(libDir string) {
	// Windows: MSVC/Clang-cl ----------- Linux: GCC/Clang
	// LIB=xxx\lib;%LIB% ---------------- -L "xxx/lib"
	// LINK=mylib.lib %LINK% ------------ -l "mylib"

	// Toolchain may throw error if lib dir not exists.
	if !fileio.PathExists(libDir) {
		return
	}

	toolchain := b.Ctx.Platform().GetToolchain()
	switch toolchain.GetName() {
	case "gcc", "clang", "qcc":
		ldflags := os.Getenv("LDFLAGS")
		parts := strings.Fields(ldflags)

		// -L flag: used to specify the directory that libraries looking for directly.
		linkFlag := "-L" + libDir

		// -Wl,-rpath-link, used to specify the directory that libraries looking for indirectly.
		rpathlinkFlag := "-Wl,-rpath-link," + libDir

		// Check if this is a dependency lib dir (tmpDeps/lib) - if so, prepend it.
		tmpDepsPrefix := filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryFolder)
		isDepsLibDir := strings.Contains(libDir, tmpDepsPrefix)

		// Prepend dependency lib dir flags to prioritize them.
		var newAppended = false
		if !slices.Contains(parts, linkFlag) {
			if isDepsLibDir {
				parts = append([]string{linkFlag}, parts...)
			} else {
				parts = append(parts, linkFlag)
			}
			newAppended = true
		}
		if !slices.Contains(parts, rpathlinkFlag) {
			if isDepsLibDir {
				parts = append([]string{rpathlinkFlag}, parts...)
			} else {
				parts = append(parts, rpathlinkFlag)
			}
			newAppended = true
		}

		// Update environment variable with modified flags.
		if newAppended {
			b.envBackup.setenv("LDFLAGS", strings.Join(parts, " "))
		}

	case "msvc", "clang-cl":
		libs := os.Getenv("LIB")
		parts := strings.Fields(libs)

		if !slices.Contains(parts, libDir) {
			parts = append(parts, libDir)
			b.envBackup.setenv("LIB", strings.Join(parts, ";"))
		}

	default:
		panic("unsupported toolchain: " + toolchain.GetName())
	}
}
