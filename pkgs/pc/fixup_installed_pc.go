package pc

import (
	"bytes"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/pkgs/fileio"
)

// FixupPkgConfigFile rewrites .pc files under packageDir to use a
// self-locating ${pcfiledir} prefix, so the installed tree stays relocatable.
func FixupPkgConfigFile(packageDir string) error {
	// No installed tree (e.g. a nobuild port) means nothing to fix up.
	if !fileio.PathExists(packageDir) {
		return nil
	}

	return filepath.WalkDir(packageDir, func(path string, d os.DirEntry, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}
		if d.IsDir() {
			return nil
		}
		if !strings.HasSuffix(d.Name(), ".pc") {
			return nil
		}

		// Only handle configs inside a pkgconfig/ dir.
		if filepath.Base(filepath.Dir(path)) != "pkgconfig" {
			return nil
		}
		return doFixupPkgConfigFile(path)
	})
}

func doFixupPkgConfigFile(pkgPath string) error {
	// Ensure the file is writable before opening it for RDWR.
	if err := os.Chmod(pkgPath, os.ModePerm); err != nil {
		return err
	}

	data, err := os.ReadFile(pkgPath)
	if err != nil {
		return err
	}
	lines := strings.Split(string(data), "\n")

	// Drop the trailing empty element a final newline produces, so the output
	// keeps the same line count as the original (bufio.Scanner semantics).
	if n := len(lines); n > 0 && lines[n-1] == "" {
		lines = lines[:n-1]
	}

	// Find the original prefix value. For vendored packages built via
	// ExternalProject, CMAKE_INSTALL_PREFIX points at the build
	// directory, so the .pc bakes that build-dir absolute path — not the
	// relocated lib path or share path. Using this original value as the rewrite
	// anchor makes both regular ports (old prefix = package dir) and vendored
	// ones relocatable.
	oldPrefix := ""
	for _, line := range lines {
		line = strings.ReplaceAll(line, "prefix =", "prefix=")
		if after, ok := strings.CutPrefix(line, "prefix="); ok {
			oldPrefix = strings.TrimSpace(after)
			break
		}
	}
	if oldPrefix != "" {
		oldPrefix = filepath.ToSlash(oldPrefix)
		oldPrefix = strings.TrimSuffix(oldPrefix, "/")
	}

	var buffer bytes.Buffer
	for _, line := range lines {
		// Remove space before `=`.
		line = strings.ReplaceAll(line, "prefix =", "prefix=")

		// Rewrite prefix to self-locating prefix using pkgconf's built-in ${pcfiledir} variable.
		if strings.HasPrefix(line, "prefix=") {
			fmt.Fprintf(&buffer, "prefix=${pcfiledir}/../..\n")
			continue
		}

		// Strip pkgconf sysroot variables (cross-compilation artifacts).
		line = strings.ReplaceAll(line, "${pc_sysrootdir}", "")
		line = strings.ReplaceAll(line, "${pc_sys_root_dir}", "")

		// Replace any absolute old-prefix path with ${prefix}.
		if oldPrefix != "" {
			line = strings.ReplaceAll(line, oldPrefix+"/", "${prefix}/")
			line = strings.ReplaceAll(line, oldPrefix, "${prefix}")
		}

		fmt.Fprintf(&buffer, "%s\n", line)
	}

	if buffer.Len() > 0 {
		os.WriteFile(pkgPath, buffer.Bytes(), os.ModePerm)
	}

	return nil
}
