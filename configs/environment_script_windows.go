//go:build windows

package configs

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/pkgs/dirs"
)

// venvLayout returns the site-packages dir (relative to the venv root) and the
// venv bin dir for the current platform.
func venvLayout(_ string) (siteRel, binRel string) {
	return filepath.ToSlash(filepath.Join("Lib", "site-packages")), "Scripts"
}

// writeScript emits a Windows command script (environment.bat).
func (env environmentScript) writeScript() error {
	sep := ";"
	incPrefix, libPrefix := installedIncludeLibFlags(env.toolchainName)
	win := func(rel string) string { return strings.ReplaceAll(rel, "/", "\\") }

	var buffer strings.Builder
	buffer.WriteString("@echo off\r\n")
	buffer.WriteString("rem build environment — run this to activate the workspace cross-compile environment.\r\n")
	buffer.WriteString("rem This is a mirror of the env setup in toolchain_file.cmake.\r\n")
	buffer.WriteString("\r\n")
	buffer.WriteString("rem Resolve the workspace root (this script's directory) without the trailing backslash.\r\n")
	buffer.WriteString("set \"WORKSPACE_DIR=%~dp0\"\r\n")
	buffer.WriteString("if \"%WORKSPACE_DIR:~-1%\"==\"\\\" set \"WORKSPACE_DIR=%WORKSPACE_DIR:~0,-1%\"\r\n")
	buffer.WriteString("\r\n")

	// MSVC needs vcvarsall.bat to set up INCLUDE/LIB (system headers and libs).
	if env.vcvars != "" {
		buffer.WriteString("rem MSVC environment (INCLUDE/LIB).\r\n")
		fmt.Fprintf(&buffer, "call \"%s\" x64 > nul\r\n\r\n", env.vcvars)
	}

	// PATH: cross toolchain, host dev tools, then the project venv.
	var pathParts []string
	if env.toolchainBinRel != "" {
		pathParts = append(pathParts, workspacePath(env.toolchainBinRel))
	}
	pathParts = append(pathParts, fmt.Sprintf("%%WORKSPACE_DIR%%\\%s\\bin", win(env.devRel)))
	if env.haveVenv {
		pathParts = append(pathParts, fmt.Sprintf("%%WORKSPACE_DIR%%\\%s\\%s", win(env.venvRel), env.venvBinRel))
	}
	buffer.WriteString("rem Compiler and host development tools.\r\n")
	fmt.Fprintf(&buffer, "set \"PATH=%s%s%%PATH%%\"\r\n\r\n", strings.Join(pathParts, sep), sep)

	if env.sysrootRel != "" {
		buffer.WriteString("rem Cross-compile sysroot.\r\n")
		fmt.Fprintf(&buffer, "set \"SYSROOT=%s\"\r\n\r\n", workspacePath(env.sysrootRel))
	}

	if len(env.toolchainEnvs) > 0 {
		buffer.WriteString("rem Cross-compile toolchain.\r\n")
		for _, kv := range env.toolchainEnvs {
			key, value, _ := strings.Cut(kv, "=")
			value = relocateSysroot(value, env.sysrootAbs, "%SYSROOT%")
			fmt.Fprintf(&buffer, "set \"%s=%s\"\r\n", key, value)
		}
		if env.host != "" {
			fmt.Fprintf(&buffer, "set \"host=%s\"\r\n", env.host)
		}
		buffer.WriteString("\r\n")
	}

	fmt.Fprintf(&buffer, "set \"CFLAGS=%s%%WORKSPACE_DIR%%\\%s\\include %s%s%%CFLAGS%%\"\r\n",
		incPrefix, win(env.installedRel), env.cflags, sepSeparatorFlag(env.cflags))
	fmt.Fprintf(&buffer, "set \"CXXFLAGS=%s%%WORKSPACE_DIR%%\\%s\\include %s%s%%CXXFLAGS%%\"\r\n",
		incPrefix, win(env.installedRel), env.cxxflags, sepSeparatorFlag(env.cxxflags))
	fmt.Fprintf(&buffer, "set \"LDFLAGS=%s%%WORKSPACE_DIR%%\\%s\\lib %s%s%%LDFLAGS%%\"\r\n\r\n",
		libPrefix, win(env.installedRel), env.ldflags, sepSeparatorFlag(env.ldflags))

	if env.haveVenv {
		buffer.WriteString("rem Python virtual environment.\r\n")
		fmt.Fprintf(&buffer, "set \"VIRTUAL_ENV=%%WORKSPACE_DIR%%\\%s\"\r\n", win(env.venvRel))
		fmt.Fprintf(&buffer, "set \"PYTHONUSERBASE=%%WORKSPACE_DIR%%\\.venv\"\r\n")
		fmt.Fprintf(&buffer, "set \"PYTHONPATH=%%VIRTUAL_ENV%%\\%s%s%%WORKSPACE_DIR%%\\%s\\%s%s%%PYTHONPATH%%\"\r\n\r\n",
			win(env.siteRel), sep, win(env.installedRel), win(env.siteRel), sep)
	}

	buffer.WriteString("rem pkg-config search paths.\r\n")
	fmt.Fprintf(&buffer, "set \"PKG_CONFIG_PATH=%%WORKSPACE_DIR%%\\%s\\lib\\pkgconfig%s%%WORKSPACE_DIR%%\\%s\\share\\pkgconfig%s%%PKG_CONFIG_PATH%%\"\r\n",
		win(env.installedRel), sep, win(env.installedRel), sep)

	return os.WriteFile(filepath.Join(dirs.WorkspaceDir, "environment.bat"), []byte(buffer.String()), os.ModePerm)
}

// workspacePath renders a workspace-relative path for the .bat script. Paths
// that live outside the workspace (e.g. a cross-drive toolchain dir such as the
// MSVC install on C:) are emitted as absolute paths instead of being prefixed
// with %WORKSPACE_DIR%, which would otherwise produce a broken path.
func workspacePath(rel string) string {
	p := strings.ReplaceAll(rel, "/", "\\")
	if filepath.IsAbs(p) {
		return p
	}
	return "%WORKSPACE_DIR%\\" + p
}
