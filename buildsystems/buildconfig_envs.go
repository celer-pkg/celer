package buildsystems

import (
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

		index := strings.Index(env, "=")
		if index == -1 {
			color.Printf(color.Yellow, "invalid environment variable `%s` and is ignored.", env)
			continue
		}

		key := strings.TrimSpace(env[:index])
		currentValue := strings.TrimSpace(env[index+1:])
		currentValue = b.replaceHolders(currentValue)

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

	// This allows the bin to locate the libraries in the relative lib dir.
	if strings.ToLower(b.PortConfig.Toolchain.SystemName) == "linux" &&
		b.buildSystem.Name() == "makefiles" {
		b.envBackup.setenv("LDFLAGS", env.JoinSpace("-Wl,-rpath=\\$$ORIGIN/../lib", os.Getenv("LDFLAGS")))
	}

	// C/C++ standard.
	b.setLanguageStandards()

	// Set CFLGAGS/CXXFLAGS/LDFLAGS.
	b.setEnvFlags()

	// Setup pkg-config.
	b.setupPkgConfig()

	// Set ACLOCAL_PATH for ports that rely on macros.
	macrosRequired := slices.ContainsFunc(b.DevDependencies, func(element string) bool {
		return strings.HasPrefix(element, "macros@")
	})
	if macrosRequired {
		b.envBackup.setenv("ACLOCAL_PATH", fileio.ToCygpath(
			filepath.Join(dirs.TmpDepsDir, b.PortConfig.HostName+"-dev", "share", "aclocal")))
	}

	// Expose dev/bin to PATH.
	devBinDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.HostName+"-dev", "bin")
	b.envBackup.setenv("PATH", env.JoinPaths("PATH", devBinDir))
}

func (b BuildConfig) setupPkgConfig() {
	var (
		configPaths   []string
		configLibDirs []string
		pathDivider   string
		sysrootDir    string
	)

	tmpDepsDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryFolder)

	switch runtime.GOOS {
	case "windows":
		if b.buildSystem.Name() == "meson" {
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
		if b.DevDep {
			configPaths = []string{
				filepath.Join(tmpDepsDir, "lib", "pkgconfig"),
				filepath.Join(tmpDepsDir, "share", "pkgconfig"),
			}

			// In this case, there is no rootfs and the pc prefix would be `/`,
			// to make sure pkgconf can work, we need to create a virtual rootfs for pkgconf.
			sysrootDir = dirs.WorkspaceDir
			pathDivider = ":"
		} else if b.PortConfig.Toolchain.RootFS != "" {
			// PKG_CONFIG related.
			for _, configPath := range b.PortConfig.Toolchain.PkgConfigPath {
				configLibDirs = append(configLibDirs, filepath.Join(
					b.PortConfig.Toolchain.RootFS, configPath,
				))
			}

			sysrootDir = b.PortConfig.Toolchain.RootFS
			pathDivider = ":"

			// Tmpdeps dir is a symlink in rootfs.
			rootfs := b.PortConfig.Toolchain.RootFS
			tmpDepsDir := filepath.Join(rootfs, "tmp", "deps", b.PortConfig.LibraryFolder)

			// Append pkgconfig with tmp/deps directory.
			configPaths = append([]string{
				filepath.Join(tmpDepsDir, "lib", "pkgconfig"),
				filepath.Join(tmpDepsDir, "share", "pkgconfig"),
			}, configPaths...)
		}
	}

	// Set merged pkgconfig envs.
	b.envBackup.setenv("PKG_CONFIG_PATH", strings.Join(configPaths, pathDivider))
	b.envBackup.setenv("PKG_CONFIG_LIBDIR", strings.Join(configLibDirs, pathDivider))
	b.envBackup.setenv("PKG_CONFIG_SYSROOT_DIR", sysrootDir)
}

func (b *BuildConfig) setLanguageStandards() {
	// Set C standard.
	cstandard := expr.If(b.CStandard != "", b.CStandard, b.PortConfig.Toolchain.CStandard)
	if cstandard != "" {
		var cflag string
		switch b.PortConfig.Toolchain.Name {
		case "msvc":
			cflag = "/std:" + b.CStandard
		case "gcc":
			cflag = "-std=" + b.CStandard
		default:
			panic("unsupported toolchain: " + b.PortConfig.Toolchain.Name)
		}
		b.envBackup.setenv("CFLAGS", env.JoinSpace(cflag, os.Getenv("CFLAGS")))
	}

	// Set C++ standard.
	cxxstandard := expr.If(b.CXXStandard != "", b.CXXStandard, b.PortConfig.Toolchain.CXXStandard)
	if cxxstandard != "" {
		var cxxflag string
		switch b.PortConfig.Toolchain.Name {
		case "msvc":
			cxxflag = "/std:" + b.CXXStandard
		case "gcc":
			cxxflag = "-std=" + b.CXXStandard
		default:
			panic("unsupported toolchain: " + b.PortConfig.Toolchain.Name)
		}
		b.envBackup.setenv("CXXFLAGS", env.JoinSpace(cxxflag, os.Getenv("CXXFLAGS")))
	}
}

func (b *BuildConfig) setEnvFlags() {
	tmpDepsDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryFolder)

	// sysroot and tmp dir.
	if b.DevDep {
		// Update CFLAGS/CXXFLAGS/LDFLAGS
		b.appendIncludeDir(filepath.Join(tmpDepsDir, "include"))
		b.appendLibDir(filepath.Join(tmpDepsDir, "lib"))
	} else if b.PortConfig.Toolchain.RootFS != "" {
		// Set sysroot.
		rootfs := b.PortConfig.Toolchain.RootFS
		b.envBackup.setenv("SYSROOT", rootfs)

		// Update CFLAGS/CXXFLAGS
		b.appendIncludeDir(filepath.Join(tmpDepsDir, "include"))
		for _, item := range b.PortConfig.Toolchain.IncludeDirs {
			includeDir := filepath.Join(b.PortConfig.Toolchain.RootFS, item)
			b.appendIncludeDir(includeDir)
		}

		// Update LDFLAGS
		b.appendLibDir(filepath.Join(tmpDepsDir, "lib"))
		for _, item := range b.PortConfig.Toolchain.LibDirs {
			libDir := filepath.Join(b.PortConfig.Toolchain.RootFS, item)
			b.appendLibDir(libDir)
		}
	}
}

func (b BuildConfig) rollbackEnvs() {
	b.envBackup.rollback()
}

func (b *BuildConfig) appendIncludeDir(includeDir string) {
	var newAppended = false
	includeDir = "-isystem " + includeDir

	cflags := strings.Fields(os.Getenv("CFLAGS"))
	cxxflags := strings.Fields(os.Getenv("CXXFLAGS"))

	// Append include dir if not exists.
	if !slices.Contains(cflags, includeDir) {
		cflags = append(cflags, includeDir)
		newAppended = true
	}
	if !slices.Contains(cxxflags, includeDir) {
		cxxflags = append(cxxflags, includeDir)
		newAppended = true
	}

	// Update environment variable with modified flags.
	if newAppended {
		b.envBackup.setenv("CFLAGS", strings.Join(cflags, " "))
		b.envBackup.setenv("CXXFLAGS", strings.Join(cxxflags, " "))
	}
}

func (b *BuildConfig) appendLibDir(libDir string) {
	var newAppended = false

	ldflags := os.Getenv("LDFLAGS")
	parts := strings.Fields(ldflags)

	lFlag := "-L" + libDir
	rFlag := "-Wl,-rpath-link," + libDir

	// Add -L/rpath-link flag.
	if !slices.Contains(parts, lFlag) {
		parts = append(parts, lFlag)
		newAppended = true
	}
	if !slices.Contains(parts, rFlag) {
		parts = append(parts, rFlag)
		newAppended = true
	}

	// Update environment variable with modified flags.
	if newAppended {
		b.envBackup.setenv("LDFLAGS", strings.Join(parts, " "))
	}
}
