//go:build !windows

package configs

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/celer-pkg/celer/pkgs/dirs"
)

// venvLayout returns the site-packages dir (relative to the venv root) and the
// venv bin dir for the current platform.
func venvLayout(minor string) (siteRel, binRel string) {
	return filepath.ToSlash(fmt.Sprintf("lib/python%s/site-packages", minor)), "bin"
}

// writeScript emits a POSIX shell script (environment.sh).
func (env environmentScript) writeScript() error {
	sep := ":"
	incPrefix, libPrefix := installedIncludeLibFlags(env.toolchainName)

	var buffer strings.Builder
	buffer.WriteString("# Build environment — source this to activate the workspace cross-compile environment.\n")
	buffer.WriteString("# This is a mirror of the env setup in toolchain_file.cmake.\n\n")

	// Relocatable workspace root: derive it from this script's own location.
	buffer.WriteString("if [ -n \"${BASH_SOURCE:-}\" ]; then\n")
	buffer.WriteString("  _SRC=\"${BASH_SOURCE[0]}\"\n")
	buffer.WriteString("else\n")
	buffer.WriteString("  _SRC=\"$0\"\n")
	buffer.WriteString("fi\n")
	buffer.WriteString("WORKSPACE_DIR=\"$(cd \"$(dirname \"$_SRC\")\" && pwd -P)\"\n")
	buffer.WriteString("unset _SRC\n")
	buffer.WriteString("export WORKSPACE_DIR\n\n")

	// PATH: cross toolchain, host dev tools, then the project venv.
	var pathParts []string
	if env.toolchainBinRel != "" {
		pathParts = append(pathParts, fmt.Sprintf("${WORKSPACE_DIR}/%s", env.toolchainBinRel))
	}
	pathParts = append(pathParts, fmt.Sprintf("${WORKSPACE_DIR}/%s/bin", env.devRel))
	if env.haveVenv {
		pathParts = append(pathParts, fmt.Sprintf("${WORKSPACE_DIR}/%s/%s", env.venvRel, env.venvBinRel))
	}
	buffer.WriteString("# Compiler and host development tools.\n")
	fmt.Fprintf(&buffer, "export PATH=\"%s%s${PATH}\"\n\n", strings.Join(pathParts, sep), sep)

	// Cross-compile sysroot, relocatable via WORKSPACE_DIR.
	if env.sysrootRel != "" {
		buffer.WriteString("# Cross-compile sysroot.\n")
		fmt.Fprintf(&buffer, "export SYSROOT=\"${WORKSPACE_DIR}/%s\"\n\n", env.sysrootRel)
	}

	// Compiler, binutils, and host info.
	if len(env.toolchainEnvs) > 0 {
		buffer.WriteString("# Cross-compile toolchain.\n")
		for _, kv := range env.toolchainEnvs {
			key, value, _ := strings.Cut(kv, "=")
			value = relocateSysroot(value, env.sysrootAbs, "${SYSROOT}")
			fmt.Fprintf(&buffer, "export %s=\"%s\"\n", key, value)
		}

		// Lowercase `host` lets autotools configure scripts (e.g. x264) detect
		// the cross-compile target without an explicit --host flag.
		if env.host != "" {
			fmt.Fprintf(&buffer, "export host=\"%s\"\n", env.host)
		}
		buffer.WriteString("\n")
	}

	// Compiler flags: expose the installed include/lib dirs.
	fmt.Fprintf(&buffer, "export CFLAGS=\"%s${WORKSPACE_DIR}/%s/include %s%s${CFLAGS}\"\n",
		incPrefix, env.installedRel, env.cflags, sepSeparatorFlag(env.cflags))
	fmt.Fprintf(&buffer, "export CXXFLAGS=\"%s${WORKSPACE_DIR}/%s/include %s%s${CXXFLAGS}\"\n",
		incPrefix, env.installedRel, env.cxxflags, sepSeparatorFlag(env.cxxflags))
	fmt.Fprintf(&buffer, "export LDFLAGS=\"%s${WORKSPACE_DIR}/%s/lib %s%s${LDFLAGS}\"\n\n",
		libPrefix, env.installedRel, env.ldflags, sepSeparatorFlag(env.ldflags))

	if env.haveVenv {
		buffer.WriteString("# Python virtual environment.\n")
		fmt.Fprintf(&buffer, "export VIRTUAL_ENV=\"${WORKSPACE_DIR}/%s\"\n", env.venvRel)
		fmt.Fprintf(&buffer, "export PYTHONUSERBASE=\"${WORKSPACE_DIR}/.venv\"\n")
		fmt.Fprintf(&buffer, "export PYTHONPATH=\"${WORKSPACE_DIR}/%s/%s%s${VIRTUAL_ENV}/%s%s${PYTHONPATH}\"\n\n",
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
	buffer.WriteString("# Native library search path (host-side tools).\n")
	fmt.Fprintf(&buffer, "export %s=\"%s%s${%s}\"\n\n",
		libVar, strings.Join(libPaths, sep), sep, libVar)

	// pkg-config search paths, mirroring the non-staging branch of writePkgConfig.
	buffer.WriteString("# pkg-config search paths.\n")
	fmt.Fprintf(&buffer, "export PKG_CONFIG_PATH=\"${WORKSPACE_DIR}/%s/lib/pkgconfig%s${WORKSPACE_DIR}/%s/share/pkgconfig%s${PKG_CONFIG_PATH}\"\n",
		env.installedRel, sep, env.installedRel, sep)

	return os.WriteFile(filepath.Join(dirs.WorkspaceDir, "environment.sh"), []byte(buffer.String()), os.ModePerm)
}
