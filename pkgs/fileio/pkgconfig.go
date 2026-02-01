package fileio

import (
	"bufio"
	"bytes"
	"os"
	"path/filepath"
	"strings"
)

// FixupPkgConfig change prefix as specified.
func FixupPkgConfig(packageDir, prefix string) error {
	pkgConfigs := []string{
		filepath.Join(packageDir, "share", "pkgconfig"),
		filepath.Join(packageDir, "lib", "pkgconfig"),
		filepath.Join(packageDir, "lib64", "pkgconfig"),
	}

	for _, pkgConfig := range pkgConfigs {
		if PathExists(pkgConfig) {
			entities, err := os.ReadDir(pkgConfig)
			if err != nil {
				return err
			}

			for _, entity := range entities {
				if strings.HasSuffix(entity.Name(), ".pc") {
					pkgPath := filepath.Join(pkgConfig, entity.Name())
					if err := doFixupPkgConfig(pkgPath, prefix); err != nil {
						return err
					}
				}
			}
		}
	}

	return nil
}

func doFixupPkgConfig(pkgPath, prefix string) error {
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

		// Remove space before `=`
		line = strings.ReplaceAll(line, "prefix =", "prefix=")
		line = strings.ReplaceAll(line, "exec_prefix =", "exec_prefix=")
		line = strings.ReplaceAll(line, "libdir =", "libdir=")
		line = strings.ReplaceAll(line, "sharedlibdir =", "sharedlibdir=")
		line = strings.ReplaceAll(line, "includedir =", "includedir=")

		switch {
		case strings.HasPrefix(line, "prefix="):
			if line != "prefix=" {
				buffer.WriteString("prefix=" + prefix + "\n")
			} else {
				buffer.WriteString(line + "\n")
			}

		case strings.HasPrefix(line, "exec_prefix="):
			if line != "exec_prefix=${prefix}" {
				buffer.WriteString("exec_prefix=${prefix}" + "\n")
			} else {
				buffer.WriteString(line + "\n")
			}

		case strings.HasPrefix(line, "libdir="):
			if line != "libdir=${prefix}/lib" {
				buffer.WriteString("libdir=${prefix}/lib" + "\n")
			} else {
				buffer.WriteString(line + "\n")
			}

		case strings.HasPrefix(line, "sharedlibdir="):
			if line != "sharedlibdir=${prefix}/lib" {
				buffer.WriteString("sharedlibdir=${prefix}/lib" + "\n")
			} else {
				buffer.WriteString(line + "\n")
			}

		case strings.HasPrefix(line, "includedir="):
			if line != "includedir=${prefix}/include" {
				buffer.WriteString("includedir=${prefix}/include" + "\n")
			} else {
				buffer.WriteString(line + "\n")
			}

		case strings.HasPrefix(line, "pkgdatadir="),
			strings.HasPrefix(line, "xcbincludedir="),
			strings.HasPrefix(line, "pythondir="):
			line = strings.ReplaceAll(line, "${pc_sysrootdir}", "")
			line = strings.ReplaceAll(line, "${pc_sys_root_dir}", "")
			buffer.WriteString(line + "\n")

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
			buffer.WriteString(lineOrigin + "\n")

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
			buffer.WriteString(lineOrigin + "\n")

		default:
			buffer.WriteString(line + "\n")
		}
	}

	if buffer.Len() > 0 {
		os.WriteFile(pkgPath, buffer.Bytes(), os.ModePerm)
	}

	return nil
}
