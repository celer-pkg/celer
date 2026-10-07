package ros

import (
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/pkgs/fileio"
)

// FixupInstalledROS rewrites the ROS/ament/colcon artifacts that bake the
// transient staging path (tmp/staging-.../release) into the installed tree,
// so the package stays relocatable and reusable from pkgcache.
func FixupInstalledROS(packageDir, workspaceDir string) error {
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

		if isParentPrefixPathMarker(path) {
			return fixupParentPrefixPath(path, workspaceDir)
		}
		if isPrefixChainScript(d.Name()) {
			return fixupPrefixChainScript(path, workspaceDir)
		}
		return nil
	})
}

// isParentPrefixPathMarker reports whether path is an ament parent_prefix_path
// resource file (share/ament_index/resource_index/parent_prefix_path/<pkg>).
func isParentPrefixPathMarker(path string) bool {
	return filepath.Base(filepath.Dir(path)) == "parent_prefix_path"
}

// isPrefixChainScript reports whether name is one of colcon's root prefix_chain
// setup scripts (generated from prefix_chain.<ext>.em).
func isPrefixChainScript(name string) bool {
	switch name {
	case "setup.sh", "setup.bash", "setup.zsh", "setup.ps1":
		return true
	}
	return false
}

func fixupParentPrefixPath(path, workspaceDir string) error {
	return rewriteIfChanged(path, func(data []byte) []byte {
		// ament stores a colon-separated list of parent prefix paths.
		original := strings.TrimSpace(string(data))
		var parts []string
		for entry := range strings.SplitSeq(original, ":") {
			entry = strings.TrimSpace(entry)
			if entry == "" {
				continue
			}

			// Any prefix under the workspace (the staging tree) relocates to
			// {prefix}; external underlays (e.g. /opt/ros/humble) are preserved.
			if isUnderWorkspace(entry, workspaceDir) {
				entry = "{prefix}"
			}
			parts = append(parts, entry)
		}
		return []byte(strings.Join(parts, ":"))
	})
}

func fixupPrefixChainScript(path, workspaceDir string) error {
	return rewriteIfChanged(path, func(data []byte) []byte {
		// Only touch colcon prefix_chain scripts (they carry this marker header);
		// an unrelated setup.sh must be left alone.
		if !strings.Contains(string(data), "prefix_chain.") {
			return data
		}

		lines := strings.Split(string(data), "\n")

		// Drop the trailing empty element a final newline produces.
		if n := len(lines); n > 0 && lines[n-1] == "" {
			lines = lines[:n-1]
		}

		// The chained-prefix block is two lines for sh/bash/zsh:
		//   COLCON_CURRENT_PREFIX="<workspace>..."
		//   _colcon_prefix_chain_*_source_script "$COLCON_CURRENT_PREFIX/local_setup.*"
		// and a single inline line for powershell:
		//   _colcon_prefix_chain_powershell_source_script "<workspace>.../local_setup.ps1"
		// Removing the assignment collapses the following source line for sh/bash/zsh.
		var out []string
		var dropNextSource = false
		for _, line := range lines {
			if strings.Contains(line, workspaceDir) {
				if strings.HasPrefix(strings.TrimSpace(line), "COLCON_CURRENT_PREFIX=") {
					dropNextSource = true
				}
				continue
			}
			if dropNextSource && strings.Contains(line, `_source_script "$COLCON_CURRENT_PREFIX/local_setup`) {
				dropNextSource = false
				continue
			}
			dropNextSource = false
			out = append(out, line)
		}
		return []byte(strings.Join(out, "\n") + "\n")
	})
}

// rewriteIfChanged applies transform to the file's contents and writes the
// result back only when it differs from what is already on disk.
func rewriteIfChanged(path string, transform func([]byte) []byte) error {
	if err := os.Chmod(path, os.ModePerm); err != nil {
		return err
	}
	data, err := os.ReadFile(path)
	if err != nil {
		return err
	}
	rewritten := transform(data)
	if string(rewritten) == string(data) {
		return nil
	}
	return os.WriteFile(path, rewritten, os.ModePerm)
}

// isUnderWorkspace reports whether p is the workspace dir or a path below it.
func isUnderWorkspace(path, workspaceDir string) bool {
	if path == workspaceDir {
		return true
	}

	// Compare slash-normalized to tolerate backslashes on Windows hosts.
	path = filepath.ToSlash(path)
	workspaceDir = strings.TrimSuffix(filepath.ToSlash(workspaceDir), "/")
	return strings.HasPrefix(path, workspaceDir+"/")
}
