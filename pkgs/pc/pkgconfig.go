package pc

import (
	"bufio"
	"bytes"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/pkgs/fileio"
)

type PkgConfig struct{}

// Apply changes standard pkg-config directories as specified.
func (p PkgConfig) Apply(packageDir, prefix string) error {
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
					if err := p.apply(pkgPath, prefix); err != nil {
						return err
					}
				}
			}
		}
	}

	return nil
}

func (p PkgConfig) apply(pkgPath, prefix string) error {
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
		// Remove space before `=`
		line := scanner.Text()
		line = strings.ReplaceAll(line, "prefix =", "prefix=")
		line = strings.ReplaceAll(line, "exec_prefix =", "exec_prefix=")
		line = strings.ReplaceAll(line, "libdir =", "libdir=")
		line = strings.ReplaceAll(line, "sharedlibdir =", "sharedlibdir=")
		line = strings.ReplaceAll(line, "includedir =", "includedir=")

		switch {
		case strings.HasPrefix(line, "prefix="):
			if line != "prefix=" {
				fmt.Fprintf(&buffer, "prefix=%s\n", prefix)
			} else {
				fmt.Fprintf(&buffer, "%s\n", line)
			}

		case strings.HasPrefix(line, "exec_prefix="):
			if line != "exec_prefix=${prefix}" {
				fmt.Fprintf(&buffer, "exec_prefix=${prefix}\n")
			} else {
				fmt.Fprintf(&buffer, "%s\n", line)
			}

		case strings.HasPrefix(line, "libdir="):
			if line != "libdir=${prefix}/lib" {
				fmt.Fprintf(&buffer, "libdir=${prefix}/lib\n")
			} else {
				fmt.Fprintf(&buffer, "%s\n", line)
			}

		case strings.HasPrefix(line, "sharedlibdir="):
			if line != "sharedlibdir=${prefix}/lib" {
				fmt.Fprintf(&buffer, "sharedlibdir=${prefix}/lib\n")
			} else {
				fmt.Fprintf(&buffer, "%s\n", line)
			}

		case strings.HasPrefix(line, "includedir="):
			if line != "includedir=${prefix}/include" {
				fmt.Fprintf(&buffer, "includedir=${prefix}/include\n")
			} else {
				fmt.Fprintf(&buffer, "%s\n", line)
			}

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
