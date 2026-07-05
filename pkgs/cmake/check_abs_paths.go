package cmake

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

// CheckCMakeAbsPaths scans cmake config files under packageDir for absolute
// paths baked into target properties that make the installed package
// non-relocatable.
func CheckCMakeAbsPaths(packageDir, workspaceDir string) error {
	cmakeDirs := []string{
		filepath.Join(packageDir, "lib", "cmake"),
		filepath.Join(packageDir, "share", "cmake"),
	}

	var violations []string

	for _, cmakeDir := range cmakeDirs {
		if !pathExists(cmakeDir) {
			continue
		}

		err := filepath.WalkDir(cmakeDir, func(path string, d os.DirEntry, walkErr error) error {
			if walkErr != nil {
				return walkErr
			}
			if d.IsDir() {
				return nil
			}
			if filepath.Ext(path) != ".cmake" {
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
				violations = append(violations, fmt.Sprintf("  %s: line %d: %s", relPath, lineNum+1, trimmed))
			}
			return nil
		})
		if err != nil {
			return err
		}
	}

	if len(violations) > 0 {
		var sb strings.Builder
		for _, v := range violations {
			sb.WriteString(" ->")
			sb.WriteString(v)
		}
		sb.WriteString("\n\n  This usually means a CMakeLists.txt didn't use find_package and target_link_libraries the imported targets. " +
			"This will cause cached target package cannot be relocatable.\n")
		return fmt.Errorf("%s", sb.String())
	}

	return nil
}

// isLinkedAbsolutePath reports whether a cmake config line bakes an absolute
// workspace path into a target's link interface or imported location — the
// two properties install(EXPORT) writes that actually affect relocatability.
func isLinkedAbsolutePath(line, workspaceDir string) bool {
	if !strings.Contains(line, workspaceDir+"/") {
		return false
	}
	return strings.Contains(line, "INTERFACE_LINK_LIBRARIES") ||
		strings.Contains(line, "IMPORTED_LOCATION")
}

func pathExists(path string) bool {
	_, err := os.Stat(path)
	if err == nil {
		return true
	}
	return !os.IsNotExist(err)
}
