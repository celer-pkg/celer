package buildsystems

import (
	"bufio"
	"bytes"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/pkgs/fileio"
)

// FixupPkgConfigFile fix pkgconfig file to use self-locating ${pcfiledir} prefix
func FixupPkgConfigFile(packageDir string) error {
	pkgConfigs := []string{
		filepath.Join(packageDir, "share", "pkgconfig"),
		filepath.Join(packageDir, "lib", "pkgconfig"),
		filepath.Join(packageDir, "lib64", "pkgconfig"),
	}

	for _, pkgConfig := range pkgConfigs {
		if fileio.PathExists(pkgConfig) {
			entities, err := os.ReadDir(pkgConfig)
			if err != nil {
				return err
			}

			for _, entity := range entities {
				if strings.HasSuffix(entity.Name(), ".pc") {
					pkgPath := filepath.Join(pkgConfig, entity.Name())
					if err := doFixupPkgConfigFile(pkgPath); err != nil {
						return err
					}
				}
			}
		}
	}

	return nil
}

func doFixupPkgConfigFile(pkgPath string) error {
	// Ensure the file is writable before opening it for RDWR.
	if err := os.Chmod(pkgPath, os.ModePerm); err != nil {
		return err
	}

	pkgFile, err := os.OpenFile(pkgPath, os.O_RDWR, os.ModePerm)
	if err != nil {
		return err
	}
	defer pkgFile.Close()

	var buffer bytes.Buffer
	scanner := bufio.NewScanner(pkgFile)
	for scanner.Scan() {
		line := scanner.Text()

		// Remove space before `=`.
		line = strings.ReplaceAll(line, "prefix =", "prefix=")

		switch {
		case strings.HasPrefix(line, "prefix="):
			// Rewrite to self-locating prefix using pkgconf's built-in ${pcfiledir} variable.
			fmt.Fprintf(&buffer, "prefix=${pcfiledir}/../..\n")

		case strings.HasPrefix(line, "pkgdatadir="),
			strings.HasPrefix(line, "xcbincludedir="),
			strings.HasPrefix(line, "pythondir="):
			line = strings.ReplaceAll(line, "${pc_sysrootdir}", "")
			line = strings.ReplaceAll(line, "${pc_sys_root_dir}", "")
			fmt.Fprintf(&buffer, "%s\n", line)

		case strings.HasPrefix(line, "Libs:"):
			lineOrigin := strings.ReplaceAll(line, "  ", " ")
			line = strings.TrimPrefix(line, "Libs:")
			line = strings.TrimSpace(line)

			parts := strings.Split(line, " ")
			for _, part := range parts {
				part = strings.TrimSpace(part)
				if strings.HasPrefix(part, "-L") && part != "-L${libdir}" {
					lineOrigin = strings.ReplaceAll(lineOrigin, part, "-L${libdir}")
				}
			}
			fmt.Fprintf(&buffer, "%s\n", lineOrigin)

		case strings.HasPrefix(line, "Libs.private:"):
			lineOrigin := strings.ReplaceAll(line, "  ", " ")

			line = strings.TrimPrefix(line, "Libs.private:")
			line = strings.TrimSpace(line)

			parts := strings.Split(line, " ")
			for _, part := range parts {
				part = strings.TrimSpace(part)
				if strings.HasPrefix(part, "-L") && part != "-L${libdir}" {
					lineOrigin = strings.ReplaceAll(lineOrigin, part, "-L${libdir}")
				}
			}
			fmt.Fprintf(&buffer, "%s\n", lineOrigin)

		default:
			fmt.Fprintf(&buffer, "%s\n", line)
		}
	}

	if err := scanner.Err(); err != nil {
		return err
	}

	if buffer.Len() > 0 {
		os.WriteFile(pkgPath, buffer.Bytes(), os.ModePerm)
	}

	return nil
}
