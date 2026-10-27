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
	hasPrefixLine := false
	for _, line := range lines {
		line = strings.ReplaceAll(line, "prefix =", "prefix=")
		if after, ok := strings.CutPrefix(line, "prefix="); ok {
			hasPrefixLine = true
			oldPrefix = strings.TrimSpace(after)
			break
		}
	}

	// Some install templates omit the prefix= line entirely.
	// Fill the libdir/includedir with absolute path. The .pc lives in
	// <prefix>/lib/pkgconfig, so the parent of libdir is the prefix.
	if !hasPrefixLine {
		oldPrefix = prefixFromAbsoluteDirs(lines)
	}
	if oldPrefix != "" {
		oldPrefix = filepath.ToSlash(oldPrefix)
		oldPrefix = strings.TrimSuffix(oldPrefix, "/")
	}

	var buffer bytes.Buffer
	if !hasPrefixLine && oldPrefix != "" {
		fmt.Fprintf(&buffer, "prefix=${pcfiledir}/../..\n")
	}
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

		// Some build systems emit `-L/path/to/libfoo.so` when a find_package
		// hands them full library paths.
		line = standardizeLibPathFlags(line)

		fmt.Fprintf(&buffer, "%s\n", line)
	}

	if buffer.Len() > 0 {
		os.WriteFile(pkgPath, buffer.Bytes(), os.ModePerm)
	}

	return nil
}

// prefixFromAbsoluteDirs derives the install prefix for a .pc that has no
// prefix= line. The only absolute paths in such templates are the install
// dirs, so the parent of libdir= (or includedir=) is the prefix the file was
// generated with.
func prefixFromAbsoluteDirs(lines []string) string {
	for _, dirName := range []string{"libdir", "includedir"} {
		for _, line := range lines {
			line = strings.ReplaceAll(line, dirName+" =", dirName+"=")
			pathValue, ok := strings.CutPrefix(line, dirName+"=")
			if !ok {
				continue
			}

			pathValue = strings.TrimSpace(pathValue)
			if !filepath.IsAbs(pathValue) {
				break // Already self-locating (or var-indirected); try the next var.
			}
			return filepath.Dir(filepath.ToSlash(pathValue))
		}
	}
	return ""
}

// standardizeLibPathFlags rewrites `-L/abs/path/libfoo.so` (or libfoo.a) into `-lfoo`.
func standardizeLibPathFlags(line string) string {
	if !strings.Contains(line, "-L/") {
		return line
	}

	changed := false
	fields := strings.Fields(line)
	for index, field := range fields {
		if libName := readLibNameFromPath(field); libName != "" {
			fields[index] = "-l" + libName
			changed = true
		}
	}
	if !changed {
		return line
	}

	return strings.Join(fields, " ")
}

// readLibNameFromPath returns the library name for a `-L` flag that points at
// a library file rather than a directory, or "" when the flag is not of that
// shape. Only absolute paths are rewritten, so `-L${libdir}` stays untouched.
func readLibNameFromPath(field string) string {
	path, ok := strings.CutPrefix(field, "-L")
	if !ok || !filepath.IsAbs(path) {
		return ""
	}

	// The 'base' would be like 'libssl.so'
	base := filepath.Base(filepath.ToSlash(path))
	if !strings.HasPrefix(base, "lib") {
		return ""
	}

	// Strip the lib prefix, then the extension (.so, .so.1.1, .a).
	name := strings.TrimPrefix(base, "lib")
	for _, ext := range []string{".so", ".a"} {
		if stripped, _, found := strings.Cut(name, ext); found {
			return stripped
		}
	}

	return ""
}
