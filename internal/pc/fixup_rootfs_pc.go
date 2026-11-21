package pc

import (
	"bufio"
	"bytes"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

// FixupRootFSPC rewrites .pc files in the rootfs to use self-locating
// ${pcfiledir} prefix so pkgconf returns sysroot paths without SYSROOT_DIR.
func FixupRootFSPC(rootfsDir string) error {
	var pcDirs []string
	filepath.Walk(filepath.Join(rootfsDir, "usr"), func(path string, info os.FileInfo, err error) error {
		if err == nil && info.IsDir() && info.Name() == "pkgconfig" {
			pcDirs = append(pcDirs, path)
		}
		return nil
	})
	for _, pcDir := range pcDirs {
		entries, err := os.ReadDir(pcDir)
		if err != nil {
			return err
		}
		for _, e := range entries {
			if strings.HasSuffix(e.Name(), ".pc") {
				if err := doFixupRootFSPC(filepath.Join(pcDir, e.Name()), rootfsDir); err != nil {
					return err
				}
			}
		}
	}
	return nil
}

// doFixupRootFSPC rewrites prefix= to self-locating ${pcfiledir}/... form.
// Handles both literal (prefix=/usr) and one-level chained (prefix=${original_prefix}).
func doFixupRootFSPC(pkgPath, rootfsDir string) error {
	input, err := os.ReadFile(pkgPath)
	if err != nil {
		return err
	}
	content := string(input)

	// Resolve actual prefix — handle prefix=/usr and prefix=${var}.
	prefix, err := readPCValue(content, "prefix")
	if err != nil {
		return err
	}
	if prefix == "" || !strings.HasPrefix(prefix, "/") || strings.Contains(prefix, "${") {
		return nil // already fixed or unhandled pattern, skip
	}
	prefix = strings.TrimRight(prefix, "/")

	// Relative path from pcfiledir to <rootfs>/<prefix>.
	rel, err := filepath.Rel(filepath.Dir(pkgPath), filepath.Join(rootfsDir, strings.TrimLeft(prefix, "/")))
	if err != nil || rel == "." {
		return nil
	}

	// Rewrite prefix, libdir, includedir lines that use absolute paths.
	var buf bytes.Buffer
	donePrefix := false
	for line := range strings.SplitSeq(content, "\n") {
		line = strings.ReplaceAll(line, "prefix =", "prefix=")
		switch {
		case !donePrefix && strings.HasPrefix(line, "prefix="):
			fmt.Fprintf(&buf, "prefix=${pcfiledir}/%s\n", filepath.ToSlash(rel))
			donePrefix = true

		case strings.HasPrefix(line, "libdir="),
			strings.HasPrefix(line, "includedir="):
			val := strings.TrimSpace(line[strings.IndexByte(line, '=')+1:])
			if strings.HasPrefix(val, prefix+"/") {
				suffix := strings.TrimPrefix(val, prefix)
				fmt.Fprintf(&buf, "%s=${prefix}%s\n", line[:strings.IndexByte(line, '=')], suffix)
				continue
			}
			fmt.Fprintf(&buf, "%s\n", line)

		default:
			fmt.Fprintf(&buf, "%s\n", line)
		}
	}
	return os.WriteFile(pkgPath, buf.Bytes(), os.ModePerm)
}

// readPCValue extracts the value of a variable from .pc content,
// following one level of indirection (e.g. prefix=${original_prefix}).
func readPCValue(content, name string) (string, error) {
	vars := map[string]string{}
	scanner := bufio.NewScanner(strings.NewReader(content))
	for scanner.Scan() {
		key, value, ok := strings.Cut(strings.TrimSpace(scanner.Text()), "=")
		if !ok || strings.HasPrefix(key, "#") {
			continue
		}
		vars[strings.TrimSpace(key)] = strings.TrimSpace(value)

		if err := scanner.Err(); err != nil {
			return "", err
		}
	}

	val := vars[name]
	if ref, ok := strings.CutPrefix(val, "${"); ok {
		return vars[strings.TrimSuffix(ref, "}")], nil
	}
	return val, nil
}
