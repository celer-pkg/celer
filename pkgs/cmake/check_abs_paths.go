package cmake

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/pkgs/fileio"
)

// CheckCMakeAbsPaths scans cmake config files under packageDir for absolute
// workspace paths baked into target properties that make the installed package
// non-relocatable. It covers regular lib/cmake and share/cmake as well as
// vendored opt/<vendor>/**/cmake.
func CheckCMakeAbsPaths(packageDir, workspaceDir string) error {
	// No installed tree (e.g. a nobuild port) means nothing to check.
	if !fileio.PathExists(packageDir) {
		return nil
	}

	var violations []string

	err := filepath.WalkDir(packageDir, func(path string, d os.DirEntry, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}
		if d.IsDir() {
			return nil
		}
		if filepath.Ext(path) != ".cmake" {
			return nil
		}
		// Only .cmake config files living under a cmake/ directory.
		if !strings.Contains(filepath.ToSlash(path), "/cmake/") {
			return nil
		}

		data, err := os.ReadFile(path)
		if err != nil {
			return err
		}

		for lineNum, line := range strings.Split(string(data), "\n") {
			trimmed := strings.TrimSpace(line)
			if strings.HasPrefix(trimmed, "#") {
				continue
			}
			if !isLinkedAbsolutePath(trimmed, workspaceDir) {
				continue
			}
			relPath, _ := filepath.Rel(packageDir, path)
			violations = append(violations, fmt.Sprintf("%s: line %d: %s", relPath, lineNum+1, trimmed))
		}
		return nil
	})
	if err != nil {
		return err
	}

	if len(violations) > 0 {
		var builder strings.Builder
		for index, v := range violations {
			if index == 0 {
				fmt.Fprintf(&builder, "%s", v)
			} else {
				fmt.Fprintf(&builder, " -> %s", v)
			}
		}
		builder.WriteString("\n\n  This usually means a CMakeLists.txt didn't use find_package and target_link_libraries the imported targets. " +
			"This will cause cached target package cannot be relocatable.\n")
		return fmt.Errorf("%s", builder.String())
	}

	return nil
}

// isLinkedAbsolutePath reports whether a cmake config line bakes an absolute
// workspace path into a target's link/include interface or imported location。
func isLinkedAbsolutePath(line, workspaceDir string) bool {
	if !strings.Contains(line, workspaceDir+"/") {
		return false
	}

	return strings.Contains(line, "INTERFACE_LINK_LIBRARIES") ||
		strings.Contains(line, "INTERFACE_INCLUDE_DIRECTORIES") ||
		strings.Contains(line, "IMPORTED_LOCATION") ||
		strings.Contains(line, "IMPORTED_LINK_INTERFACE_LIBRARIES")
}
