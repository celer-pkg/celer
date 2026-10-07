package configs

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
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
	toolchainEnvs   []string // ordered "KEY=VALUE" (CC/CXX, binutils, host info)
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

			if runtime.GOOS == "windows" {
				env.siteRel = filepath.ToSlash(filepath.Join("Lib", "site-packages"))
				env.venvBinRel = "Scripts"
			} else {
				env.siteRel = filepath.ToSlash(fmt.Sprintf("lib/python%s/site-packages", minor))
				env.venvBinRel = "bin"
			}

			env.condaLibDir = buildtools.PythonTool.LdLibraryPath()
		}
	}

	// Cross-compile toolchain (mirrors SetEnvs + the toolchain's compiler flags).
	if toolchain := c.platform.Toolchain; toolchain != nil {
		env.toolchainBinRel = workspaceRel(toolchain.GetAbsDir())
		env.toolchainName = toolchain.GetName()
		env.toolchainEnvs = toolchain.buildEnvVars(c.RootFS(), "makefiles", nil)

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

	if runtime.GOOS == "windows" {
		return env.writeEnvironmentBatch()
	}

	return env.writeEnvironmentShell()
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

// writeEnvironmentShell emits a POSIX shell script.
func (env environmentScript) writeEnvironmentShell() error {
	sep := ":"
	incPrefix, libPrefix := installedIncludeLibFlags(env.toolchainName)

	var b strings.Builder
	b.WriteString("# Build environment — source this to activate the workspace cross-compile environment.\n")
	b.WriteString("# This is a mirror of the env setup in toolchain_file.cmake.\n\n")

	// Relocatable workspace root: derive it from this script's own location.
	b.WriteString("if [ -n \"${BASH_SOURCE:-}\" ]; then\n")
	b.WriteString("  _SRC=\"${BASH_SOURCE[0]}\"\n")
	b.WriteString("else\n")
	b.WriteString("  _SRC=\"$0\"\n")
	b.WriteString("fi\n")
	b.WriteString("WORKSPACE_DIR=\"$(cd \"$(dirname \"$_SRC\")\" && pwd -P)\"\n")
	b.WriteString("unset _SRC\n")
	b.WriteString("export WORKSPACE_DIR\n\n")

	// PATH: cross toolchain, host dev tools, then the project venv.
	var pathParts []string
	if env.toolchainBinRel != "" {
		pathParts = append(pathParts, fmt.Sprintf("${WORKSPACE_DIR}/%s", env.toolchainBinRel))
	}
	pathParts = append(pathParts, fmt.Sprintf("${WORKSPACE_DIR}/%s/bin", env.devRel))
	if env.haveVenv {
		pathParts = append(pathParts, fmt.Sprintf("${WORKSPACE_DIR}/%s/%s", env.venvRel, env.venvBinRel))
	}
	b.WriteString("# Compiler and host development tools.\n")
	fmt.Fprintf(&b, "export PATH=\"%s%s${PATH}\"\n\n", strings.Join(pathParts, sep), sep)

	// Compiler, binutils, and host info.
	if len(env.toolchainEnvs) > 0 {
		b.WriteString("# Cross-compile toolchain.\n")
		for _, kv := range env.toolchainEnvs {
			key, value, _ := strings.Cut(kv, "=")
			fmt.Fprintf(&b, "export %s=\"%s\"\n", key, value)
		}
		b.WriteString("\n")
	}

	// Compiler flags: expose the installed include/lib dirs first.
	fmt.Fprintf(&b, "export CFLAGS=\"%s${WORKSPACE_DIR}/%s/include %s%s${CFLAGS}\"\n",
		incPrefix, env.installedRel, env.cflags, sepSeparatorFlag(env.cflags))
	fmt.Fprintf(&b, "export CXXFLAGS=\"%s${WORKSPACE_DIR}/%s/include %s%s${CXXFLAGS}\"\n",
		incPrefix, env.installedRel, env.cxxflags, sepSeparatorFlag(env.cxxflags))
	fmt.Fprintf(&b, "export LDFLAGS=\"%s${WORKSPACE_DIR}/%s/lib %s%s${LDFLAGS}\"\n\n",
		libPrefix, env.installedRel, env.ldflags, sepSeparatorFlag(env.ldflags))

	if env.haveVenv {
		b.WriteString("# Python virtual environment.\n")
		fmt.Fprintf(&b, "export VIRTUAL_ENV=\"${WORKSPACE_DIR}/%s\"\n", env.venvRel)
		fmt.Fprintf(&b, "export PYTHONUSERBASE=\"${WORKSPACE_DIR}/.venv\"\n")
		fmt.Fprintf(&b, "export PYTHONPATH=\"${WORKSPACE_DIR}/%s/%s%s${VIRTUAL_ENV}/%s%s${PYTHONPATH}\"\n\n",
			env.installedRel, env.siteRel, sep, env.siteRel, sep)
	}

	// Native library search paths for host-side tools.
	var libPaths []string
	libPaths = append(libPaths, fmt.Sprintf("${WORKSPACE_DIR}/%s/lib", env.devRel))
	if env.condaLibDir != "" {
		libPaths = append(libPaths, env.condaLibDir)
	}
	libVar := "LD_LIBRARY_PATH"
	if runtime.GOOS == "darwin" {
		libVar = "DYLD_LIBRARY_PATH"
	}
	b.WriteString("# Native library search path (host-side tools).\n")
	fmt.Fprintf(&b, "export %s=\"%s%s${%s}\"\n\n",
		libVar, strings.Join(libPaths, sep), sep, libVar)

	// pkg-config search paths, mirroring the non-staging branch of writePkgConfig.
	b.WriteString("# pkg-config search paths.\n")
	fmt.Fprintf(&b, "export PKG_CONFIG_PATH=\"${WORKSPACE_DIR}/%s/lib/pkgconfig%s${WORKSPACE_DIR}/%s/share/pkgconfig%s${PKG_CONFIG_PATH}\"\n",
		env.installedRel, sep, env.installedRel, sep)

	return os.WriteFile(filepath.Join(dirs.WorkspaceDir, "environment.sh"), []byte(b.String()), os.ModePerm)
}

// writeEnvironmentBatch emits a Windows command script.
func (env environmentScript) writeEnvironmentBatch() error {
	sep := ";"
	incPrefix, libPrefix := installedIncludeLibFlags(env.toolchainName)
	win := func(rel string) string { return strings.ReplaceAll(rel, "/", "\\") }

	var b strings.Builder
	b.WriteString("@echo off\r\n")
	b.WriteString("rem celer environment — run this to activate the workspace cross-compile environment.\r\n")
	b.WriteString("rem Generated by `celer configure`; mirrors the env setup in toolchain_file.cmake.\r\n")
	b.WriteString("\r\n")
	b.WriteString("rem Resolve the workspace root (this script's directory) without the trailing backslash.\r\n")
	b.WriteString("set \"WORKSPACE_DIR=%~dp0\"\r\n")
	b.WriteString("if \"%WORKSPACE_DIR:~-1%\"==\"\\\" set \"WORKSPACE_DIR=%WORKSPACE_DIR:~0,-1%\"\r\n")
	b.WriteString("\r\n")

	// PATH: cross toolchain, host dev tools, then the project venv.
	var pathParts []string
	if env.toolchainBinRel != "" {
		pathParts = append(pathParts, fmt.Sprintf("%%WORKSPACE_DIR%%\\%s", win(env.toolchainBinRel)))
	}
	pathParts = append(pathParts, fmt.Sprintf("%%WORKSPACE_DIR%%\\%s\\bin", win(env.devRel)))
	if env.haveVenv {
		pathParts = append(pathParts, fmt.Sprintf("%%WORKSPACE_DIR%%\\%s\\%s", win(env.venvRel), env.venvBinRel))
	}
	b.WriteString("rem Compiler and host development tools.\r\n")
	fmt.Fprintf(&b, "set \"PATH=%s%s%%PATH%%\"\r\n\r\n", strings.Join(pathParts, sep), sep)

	if len(env.toolchainEnvs) > 0 {
		b.WriteString("rem Cross-compile toolchain.\r\n")
		for _, kv := range env.toolchainEnvs {
			key, value, _ := strings.Cut(kv, "=")
			fmt.Fprintf(&b, "set \"%s=%s\"\r\n", key, value)
		}
		b.WriteString("\r\n")
	}

	fmt.Fprintf(&b, "set \"CFLAGS=%s%%WORKSPACE_DIR%%\\%s\\include %s%s%%CFLAGS%%\"\r\n",
		incPrefix, win(env.installedRel), env.cflags, sepSeparatorFlag(env.cflags))
	fmt.Fprintf(&b, "set \"CXXFLAGS=%s%%WORKSPACE_DIR%%\\%s\\include %s%s%%CXXFLAGS%%\"\r\n",
		incPrefix, win(env.installedRel), env.cxxflags, sepSeparatorFlag(env.cxxflags))
	fmt.Fprintf(&b, "set \"LDFLAGS=%s%%WORKSPACE_DIR%%\\%s\\lib %s%s%%LDFLAGS%%\"\r\n\r\n",
		libPrefix, win(env.installedRel), env.ldflags, sepSeparatorFlag(env.ldflags))

	if env.haveVenv {
		b.WriteString("rem Python virtual environment.\r\n")
		fmt.Fprintf(&b, "set \"VIRTUAL_ENV=%%WORKSPACE_DIR%%\\%s\"\r\n", win(env.venvRel))
		fmt.Fprintf(&b, "set \"PYTHONUSERBASE=%%WORKSPACE_DIR%%\\.venv\"\r\n")
		fmt.Fprintf(&b, "set \"PYTHONPATH=%%WORKSPACE_DIR%%\\%s\\%s%s%%VIRTUAL_ENV%%\\%s%s%%PYTHONPATH%%\"\r\n\r\n",
			win(env.installedRel), win(env.siteRel), sep, win(env.siteRel), sep)
	}

	b.WriteString("rem pkg-config search paths.\r\n")
	fmt.Fprintf(&b, "set \"PKG_CONFIG_PATH=%%WORKSPACE_DIR%%\\%s\\lib\\pkgconfig%s%%WORKSPACE_DIR%%\\%s\\share\\pkgconfig%s%%PKG_CONFIG_PATH%%\"\r\n",
		win(env.installedRel), sep, win(env.installedRel), sep)

	return os.WriteFile(filepath.Join(dirs.WorkspaceDir, "environment.bat"), []byte(b.String()), os.ModePerm)
}

// sepSeparatorFlag returns a trailing space so an empty flag set doesn't leave a
// dangling separator before the inherited variable.
func sepSeparatorFlag(flags string) string {
	if strings.TrimSpace(flags) == "" {
		return ""
	}
	return " "
}

// workspaceRel returns path rel to the workspace root.
func workspaceRel(abs string) string {
	rel, err := filepath.Rel(dirs.WorkspaceDir, abs)
	if err != nil {
		return filepath.Clean(abs)
	}
	return filepath.ToSlash(rel)
}
