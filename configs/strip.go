package configs

import (
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/logger"
)

// Development / build-only suffixes that are never needed at runtime.
var stripSkipSuffixes = []string{
	".a", ".lib", ".la",
	".h", ".hpp", ".hh", ".hxx", ".h++", ".inl", ".ipp", ".tcc", ".inc",
	".cmake", ".pc", ".pdb",
	".o", ".obj",
	".exp", ".ilk", ".idb",
}

// Path segments that mark whole trees as non-runtime (matched case-insensitively).
var stripSkipDirNames = map[string]struct{}{
	"include":   {},
	"cmake":     {},
	"pkgconfig": {},
	"man":       {},
	"doc":       {},
	"info":      {},
	"aclocal":   {},
}

// Strip builds a runtime-oriented tree under workspace/stripped/<libraryFolder>/.
//
// Kept:
//   - ELF / PE binaries and shared libraries (written via strip -o)
//   - Runtime companions: shell scripts, config/data files, and version symlinks
//
// Dropped:
//   - Static libraries, headers, CMake/pkg-config metadata, PDBs, and related build trees
//
// Requires toolchain.strip. On Windows, use a PE-capable tool such as llvm-strip.
func (c *Celer) Strip() error {
	stripTool, err := c.resolveStripTool()
	if err != nil {
		return err
	}

	installedDir := c.InstalledDir()
	if !fileio.PathExists(installedDir) {
		return fmt.Errorf("installed directory does not exist: %s", installedDir)
	}

	strippedDir := filepath.Join(dirs.WorkspaceDir, "stripped", c.LibraryFolder())
	logger.Printf(logger.Title, "\n[strip: %s]\n\n", c.LibraryFolder())

	return filepath.WalkDir(installedDir, func(path string, d fs.DirEntry, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}

		relPath, err := filepath.Rel(installedDir, path)
		if err != nil {
			return err
		}
		if relPath == "." {
			return nil
		}
		relPath = filepath.ToSlash(relPath)

		if d.IsDir() {
			if shouldSkipStripDir(relPath) {
				return filepath.SkipDir
			}
			return nil
		}

		if shouldSkipStripFile(path, relPath) {
			return nil
		}

		destPath := filepath.Join(strippedDir, relPath)
		if err := os.MkdirAll(filepath.Dir(destPath), os.ModePerm); err != nil {
			return err
		}

		// Preserve .so soname symlinks and other links as-is.
		info, err := d.Info()
		if err != nil {
			return err
		}
		if info.Mode()&os.ModeSymlink != 0 {
			if err := fileio.CopyFile(path, destPath); err != nil {
				return fmt.Errorf("failed to copy symlink %s -> %w", path, err)
			}
			printStripLog("copy", relPath)
			return nil
		}

		if fileio.IsStrippableBinary(path) {
			if _, err := cmd.NewExecutor("", stripTool, "-o", destPath, path).ExecuteOutput(); err != nil {
				return fmt.Errorf("failed to strip %s -> %w", path, err)
			}
			printStripLog("strip", relPath)
			return nil
		}

		// Configs, shell scripts, and other runtime data.
		if err := fileio.CopyFile(path, destPath); err != nil {
			return fmt.Errorf("failed to copy %s -> %w", path, err)
		}
		printStripLog("copy", relPath)
		return nil
	})
}

// resolveStripTool returns the absolute path of the configured strip tool.
func (c *Celer) resolveStripTool() (string, error) {
	toolchain := c.Platform().GetToolchain()
	stripBin := strings.TrimSpace(toolchain.GetSTRIP())
	if stripBin == "" {
		return "", fmt.Errorf("strip is not configured in %s", c.Platform().GetName())
	}

	candidates := []string{stripBin}
	if runtime.GOOS == "windows" && !strings.HasSuffix(strings.ToLower(stripBin), ".exe") {
		candidates = append(candidates, stripBin+".exe")
	}

	absDir := toolchain.GetAbsDir()
	for _, name := range candidates {
		if filepath.IsAbs(name) {
			if fileio.PathExists(name) {
				return name, nil
			}
			continue
		}
		if absDir != "" {
			joined := filepath.Join(absDir, name)
			if fileio.PathExists(joined) {
				return joined, nil
			}
		}
	}

	return "", fmt.Errorf("strip tool not found: %s (toolchain dir: %s)", stripBin, absDir)
}

func printStripLog(op, relPath string) {
	// Align action tags: [copy ] / [strip]
	logger.Printf(logger.Hint, "[✔] [%-5s] %s\n", op, relPath)
}

func shouldSkipStripDir(relPath string) bool {
	for part := range strings.SplitSeq(filepath.ToSlash(relPath), "/") {
		if _, ok := stripSkipDirNames[strings.ToLower(part)]; ok {
			return true
		}
	}
	return false
}

func shouldSkipStripFile(path, relPath string) bool {
	if shouldSkipStripDir(filepath.Dir(relPath)) {
		return true
	}

	lower := strings.ToLower(path)
	for _, suffix := range stripSkipSuffixes {
		if strings.HasSuffix(lower, suffix) {
			return true
		}
	}
	return false
}
