package configs

import (
	"fmt"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/buildtools"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// environmentScript describes everything the generated environment script
// exports, independent of the shell dialect it is rendered into.
type environmentScript struct {
	installedRel string
	devRel       string

	haveVenv    bool
	venvRel     string
	siteRel     string
	venvBinRel  string
	condaLibDir string

	toolchainBinRel string
	toolchainName   string
	vcvars          string   // vcvarsall.bat path (MSVC only), used to set up INCLUDE/LIB
	toolchainEnvs   []string // ordered "KEY=VALUE" (CC/CXX, binutils, host info)
	host            string   // target triplet (e.g. aarch64-linux-gnu)
	sysrootRel      string   // rootfs path relative to the workspace ("" when no rootfs)
	sysrootAbs      string   // rootfs absolute path, used to relocate --sysroot in CC/CXX
	cflags          string
	cxxflags        string
	ldflags         string
}

// GenerateEnvironmentScript writes a shell script (environment.sh on POSIX,
// environment.bat on Windows) next to toolchain_file.cmake. Sourcing it exports
// the same cross-compile environment that builds rely on.
func (c *Celer) GenerateEnvironmentScript() error {
	env := environmentScript{
		installedRel: workspaceRel(c.InstalledDir()),
		devRel:       workspaceRel(c.InstalledDevDir()),
	}

	// Python virtual environment (mirrors writePythonVenvConfig guards).
	if buildtools.PythonTool != nil && buildtools.PythonTool.Path != "" {
		pythonVersion := buildtools.GetDefaultPythonVersion()
		if pc := c.PythonConfig(); pc != nil && pc.GetVersion() != "" {
			pythonVersion = pc.GetVersion()
		}

		minor := pythonVersion
		if strings.Count(pythonVersion, ".") > 1 {
			parts := strings.Split(pythonVersion, ".")
			minor = parts[0] + "." + parts[1]
		}

		venvFolder := fmt.Sprintf("venv-%s@%s", minor, c.Project().GetName())
		venvAbs := filepath.Join(dirs.WorkspaceDir, "installed", venvFolder)
		if fileio.PathExists(venvAbs) {
			env.haveVenv = true
			env.venvRel = filepath.ToSlash(filepath.Join("installed", venvFolder))
			env.siteRel, env.venvBinRel = venvLayout(minor)
			env.condaLibDir = buildtools.PythonTool.LdLibraryPath()
		}
	}

	// Cross-compile toolchain (mirrors SetEnvs + the toolchain's compiler flags).
	if toolchain := c.platform.Toolchain; toolchain != nil {
		env.toolchainBinRel = workspaceRel(toolchain.GetAbsDir())
		env.toolchainName = toolchain.GetName()
		if msvc := toolchain.GetMSVC(); msvc != nil && msvc.VCVars != "" {
			env.vcvars = msvc.VCVars
		}
		env.toolchainEnvs = toolchain.buildEnvVars(c.RootFS(), "makefiles", nil)
		env.host = toolchain.GetHost()

		if rootfs := c.RootFS(); rootfs != nil {
			env.sysrootRel = workspaceRel(rootfs.GetAbsDir())
			env.sysrootAbs = rootfs.GetAbsDir()
		}

		cflags, cxxflags, ldflags := toolchain.effectiveFlags("release")
		if std := toolchain.GetCStandard(); std != "" {
			cflags = append(cflags, standardFlag(toolchain.GetName(), std))
		}
		if std := toolchain.GetCXXStandard(); std != "" {
			cxxflags = append(cxxflags, standardFlag(toolchain.GetName(), std))
		}
		env.cflags = strings.Join(cflags, " ")
		env.cxxflags = strings.Join(cxxflags, " ")
		env.ldflags = strings.Join(ldflags, " ")
	}

	return env.writeScript()
}

// standardFlag returns the -std=<x> (/std:<x> for MSVC) flag matching
// setLanguageStandard in buildconfig_envs.go.
func standardFlag(toolchainName, std string) string {
	switch toolchainName {
	case "gcc", "clang":
		return "-std=" + std
	case "msvc", "clang-cl":
		return "/std:" + std
	default:
		return ""
	}
}

// installedIncludeLibFlags returns the flag prefixes used to expose the
// installed include/lib dirs on CFLAGS/CXXFLAGS and LDFLAGS.
func installedIncludeLibFlags(toolchainName string) (incPrefix, libPrefix string) {
	switch toolchainName {
	case "msvc", "clang-cl":
		return "/I", "/LIBPATH:"
	default: // gcc, clang, qcc
		return "-I", "-L"
	}
}

// sepSeparatorFlag returns a trailing space so an empty flag set doesn't leave a
// dangling separator before the inherited variable.
func sepSeparatorFlag(flags string) string {
	if strings.TrimSpace(flags) == "" {
		return ""
	}
	return " "
}

// relocateSysroot rewrites the absolute "--sysroot=<abs>" token in a CC/CXX
// value to reference the relocatable SYSROOT variable for the current shell
// dialect (${SYSROOT} on POSIX, %SYSROOT% on Windows).
func relocateSysroot(value, sysrootAbs, varRef string) string {
	if sysrootAbs == "" {
		return value
	}
	return strings.ReplaceAll(value, "--sysroot="+sysrootAbs, "--sysroot="+varRef)
}

// workspaceRel returns path rel to the workspace root.
func workspaceRel(abs string) string {
	rel, err := filepath.Rel(dirs.WorkspaceDir, abs)
	if err != nil {
		return filepath.Clean(abs)
	}
	return filepath.ToSlash(rel)
}
