package script

import (
	"io"
	"os"
	"path/filepath"
	"regexp"
	"slices"
	"strings"

	"github.com/celer-pkg/celer/internal/fileio"
)

// FixupInstalledScripts scrubs transient build-time paths from installed
// shell scripts, and makes config scripts self-locating.
func FixupInstalledScripts(packageDir, transientDir string) error {
	// No installed tree (e.g. a nobuild port) means nothing to fix up.
	if !fileio.PathExists(packageDir) {
		return nil
	}

	packageDir = filepath.Clean(packageDir)
	binDirs := []string{
		filepath.Join(packageDir, "bin"),
		filepath.Join(packageDir, "sbin"),
	}
	transientRegexp := newTransientPattern(transientDir)

	return filepath.WalkDir(packageDir, func(path string, d os.DirEntry, walkErr error) error {
		if walkErr != nil {
			return walkErr
		}
		if d.IsDir() {
			return nil
		}
		// Never follow symlinks; their targets may live outside the package.
		if !d.Type().IsRegular() {
			return nil
		}

		// Skip pc file and non-script file.
		isPkgConfig := isPkgConfigFile(path)
		isScript := slices.Contains(binDirs, filepath.Dir(path)) && isShellScriptFile(path)
		if !isPkgConfig && !isScript {
			return nil
		}

		// Only self-locate scripts that live in a prefix bin dir.
		selfLocate := isScript
		data, err := os.ReadFile(path)
		if err != nil {
			return err
		}

		content := string(data)
		if transientRegexp != nil {
			content = transientRegexp.ReplaceAllString(content, "")
		}
		if selfLocate {
			content = selfLocatePrefix(content, packageDir)
		}
		if content == string(data) {
			return nil
		}

		info, err := d.Info()
		if err != nil {
			return err
		}
		if err := os.Chmod(path, info.Mode().Perm()); err != nil {
			return err
		}
		return os.WriteFile(path, []byte(content), info.Mode().Perm())
	})
}

func isPkgConfigFile(path string) bool {
	return strings.HasSuffix(path, ".pc") && filepath.Base(filepath.Dir(path)) == "pkgconfig"
}

// isShellScriptFile reports whether path starts with a POSIX shell shebang.
func isShellScriptFile(path string) bool {
	file, err := os.Open(path)
	if err != nil {
		return false
	}
	defer file.Close()

	header := make([]byte, 256)
	n, err := io.ReadFull(file, header)
	if err != nil && err != io.ErrUnexpectedEOF && err != io.EOF {
		return false
	}

	firstLine, _, _ := strings.Cut(string(header[:n]), "\n")
	if !strings.HasPrefix(firstLine, "#!") {
		return false
	}
	for _, field := range strings.Fields(strings.TrimPrefix(firstLine, "#!")) {
		switch filepath.Base(field) {
		case "sh", "bash", "dash", "zsh", "ksh", "ash":
			return true
		}
	}
	return false
}

// newTransientPattern matches any whitespace-delimited token containing the
// transient tmp dir (which covers every staging dir). Matching the whole token
// keeps flags such as `-L<staging>/lib` from turning into bogus `-L/lib` entries.
func newTransientPattern(transientDir string) *regexp.Regexp {
	transientDir = filepath.Clean(transientDir)
	if transientDir == "" || transientDir == "." {
		return nil
	}

	transientDir = filepath.ToSlash(transientDir)
	alternatives := []string{regexp.QuoteMeta(transientDir)}
	backslash := strings.ReplaceAll(transientDir, "/", `\`)
	if backslash != transientDir {
		alternatives = append(alternatives, regexp.QuoteMeta(backslash))
	}

	// Pattern: \S*(?:<path1>|<path2>)\S*
	//
	//	\S*          zero or more non-whitespace chars (the token's prefix)
	//	(?:...|...)  non-capturing group: match either the slash or backslash form
	//	\S*          zero or more non-whitespace chars (the token's suffix)
	return regexp.MustCompile(`\S*(?:` + strings.Join(alternatives, "|") + `)\S*`)
}

// selfLocatePrefix rewrites an absolute `prefix=` assignment that points into
// packageDir to a location derived from the script itself, so the script keeps
// working after the package is moved or restored from pkgcache.
func selfLocatePrefix(content, packageDir string) string {
	lines := strings.Split(content, "\n")
	changed := false
	for index, line := range lines {
		value, ok := prefixValue(line)
		if !ok || !filepath.IsAbs(value) {
			continue
		}

		rel, err := filepath.Rel(packageDir, filepath.Clean(value))
		if err != nil || rel == ".." || strings.HasPrefix(rel, ".."+string(filepath.Separator)) {
			continue
		}

		expr := `$(dirname $(dirname "$0"))`
		if rel != "." {
			expr += "/" + filepath.ToSlash(rel)
		}
		lines[index] = "prefix=" + expr
		changed = true
	}
	if !changed {
		return content
	}
	return strings.Join(lines, "\n")
}

// prefixValue extracts the value of a `prefix=` line, stripping matching surrounding quotes.
func prefixValue(line string) (string, bool) {
	trimmed := strings.TrimSpace(line)
	if !strings.HasPrefix(trimmed, "prefix=") {
		return "", false
	}

	value := strings.TrimSpace(strings.TrimPrefix(trimmed, "prefix="))
	if len(value) >= 2 && (value[0] == '"' || value[0] == '\'') && value[len(value)-1] == value[0] {
		value = value[1 : len(value)-1]
	}
	return value, true
}
