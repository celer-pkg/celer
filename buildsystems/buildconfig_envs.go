package buildsystems

import (
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/env"
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
	if strings.ToLower(b.PortConfig.CrossTools.SystemName) == "linux" &&
		b.buildSystem.Name() == "makefiles" {
		b.envBackup.setenv("LDFLAGS", env.JoinSpace("-Wl,-rpath=\\$$ORIGIN/../lib", os.Getenv("LDFLAGS")))
	}

	// C/C++ standard.
	b.setLanguageStandards()

	// Set CFLGAGS/CXXFLAGS/LDFLAGS.
	b.setEnvFlags()

	// Setup pkg-config.
	b.setupPkgConfig()

	// Set build environment for msvc.
	if b.PortConfig.CrossTools.Name == "msvc" {
		b.setupMSVC()
	}

	// Set ACLOCAL_PATH for ports that rely on macros.
	macrosRequired := slices.ContainsFunc(b.DevDependencies, func(element string) bool {
		return strings.HasPrefix(element, "macros@")
	})
	if macrosRequired {
		b.envBackup.setenv("ACLOCAL_PATH", fileio.ToCygpath(
			filepath.Join(dirs.TmpDepsDir, b.PortConfig.HostName+"-dev", "share", "aclocal")))
	}

	// Make sure so in dev/lib can be load by dev/bin.
	if runtime.GOOS == "linux" {
		devBinDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.HostName+"-dev", "bin")
		b.envBackup.setenv("PATH", env.JoinPaths("PATH", devBinDir))
	}
}

func (b BuildConfig) setupMSVC() {
	tmpDepsDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryFolder)

	includeDirs := append(b.PortConfig.CrossTools.MSVC.IncludeDirs, filepath.Join(tmpDepsDir, "include"))
	b.envBackup.setenv("INCLUDE", strings.Join(includeDirs, ";"))

	libDirs := append(b.PortConfig.CrossTools.MSVC.LibDirs, filepath.Join(tmpDepsDir, "lib"))
	b.envBackup.setenv("LIB", strings.Join(libDirs, ";"))

	binDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.HostName+"-dev", "bin")
	b.envBackup.setenv("PATH", env.JoinPaths("PATH", binDir))
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
		} else if b.PortConfig.CrossTools.RootFS != "" {
			// PKG_CONFIG related.
			for _, configPath := range b.PortConfig.CrossTools.PkgConfigPath {
				configLibDirs = append(configLibDirs, filepath.Join(
					b.PortConfig.CrossTools.RootFS, configPath,
				))
			}

			sysrootDir = b.PortConfig.CrossTools.RootFS
			pathDivider = ":"

			// Tmpdeps dir is a symlink in rootfs.
			rootfs := b.PortConfig.CrossTools.RootFS
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
	if b.CStandard != "" {
		var cflag string
		switch b.PortConfig.CrossTools.Name {
		case "msvc":
			cflag = "/std:" + b.CStandard
		case "gcc":
			cflag = "-std=" + b.CStandard
		}
		b.envBackup.setenv("CFLAGS", env.JoinSpace(cflag, os.Getenv("CFLAGS")))
	}

	// Set C++ standard.
	if b.CXXStandard != "" {
		var cxxflag string
		switch b.PortConfig.CrossTools.Name {
		case "msvc":
			cxxflag = "/std:" + b.CXXStandard
		case "gcc":
			cxxflag = "-std=" + b.CXXStandard
		}
		b.envBackup.setenv("CXXFLAGS", env.JoinSpace(cxxflag, os.Getenv("CXXFLAGS")))
	}
}

func (b *BuildConfig) setEnvFlags() {
	tmpDepsDir := filepath.Join(dirs.TmpDepsDir, b.PortConfig.LibraryFolder)

	switch runtime.GOOS {
	case "windows":
		// TODO adapter later...

	case "linux":
		// Pkg config paths and sysroot dir.
		if b.DevDep {
			// Update CFLAGS/CXXFLAGS
			includeFlag := "-isystem " + filepath.Join(tmpDepsDir, "include")
			b.envBackup.setenv("CFLAGS", env.JoinSpace(includeFlag, os.Getenv("CFLAGS")))
			b.envBackup.setenv("CXXFLAGS", env.JoinSpace(includeFlag, os.Getenv("CXXFLAGS")))

			// Update LDFLAGS
			b.appendLibPath(filepath.Join(tmpDepsDir, "lib"))
		} else if b.PortConfig.CrossTools.RootFS != "" {
			// Set sysroot.
			rootfs := b.PortConfig.CrossTools.RootFS
			b.envBackup.setenv("SYSROOT", rootfs)

			// Update CFLAGS/CXXFLAGS
			var includeDirs []string
			includeDirs = append(includeDirs, "-isystem "+filepath.Join(tmpDepsDir, "include"))
			for _, item := range b.PortConfig.CrossTools.IncludeDirs {
				includeDir := filepath.Join(b.PortConfig.CrossTools.RootFS, item)
				includeDirs = append(includeDirs, "-isystem "+includeDir)
			}
			includeFlags := strings.Join(includeDirs, " ")
			b.envBackup.setenv("CFLAGS", env.JoinSpace(includeFlags, os.Getenv("CFLAGS")))
			b.envBackup.setenv("CXXFLAGS", env.JoinSpace(includeFlags, os.Getenv("CXXFLAGS")))

			// Update LDFLAGS
			b.appendLibPath(filepath.Join(tmpDepsDir, "lib"))
			for _, item := range b.PortConfig.CrossTools.LibDirs {
				libDir := filepath.Join(b.PortConfig.CrossTools.RootFS, item)
				b.appendLibPath(libDir)
			}
		}
	}
}

func (b BuildConfig) rollbackEnvs() {
	b.envBackup.rollback()
}

func (b *BuildConfig) appendLibPath(libDir string) {
	ldflags := os.Getenv("LDFLAGS")
	parts := strings.Fields(ldflags)

	lFlag := "-L" + libDir
	rpathFlag := "-Wl,-rpath-link=" + libDir
	rpathFlagv2 := "-Wl,-rpath-link," + libDir

	// Add -L/rpath-link flag.
	if !slices.Contains(parts, lFlag) {
		parts = append(parts, lFlag)
	}
	if !slices.Contains(parts, rpathFlag) && !slices.Contains(parts, rpathFlagv2) {
		parts = append(parts, rpathFlag)
	}

	// Update environment variable with modified flags.
	b.envBackup.setenv("LDFLAGS", strings.Join(parts, " "))
}
