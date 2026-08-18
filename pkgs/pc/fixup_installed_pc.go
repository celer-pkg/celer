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
					if err := doFixupPkgConfigFile(pkgPath, packageDir); err != nil {
						return err
					}
				}
			}
		}
	}

	return nil
}

func doFixupPkgConfigFile(pkgPath string, packageDir string) error {
	// Normalize packageDir to forward slashes for matching against .pc content.
	packageDir = filepath.ToSlash(packageDir)
	packageDir = strings.TrimSuffix(packageDir, "/")

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

		// Rewrite prefix to self-locating prefix using pkgconf's built-in ${pcfiledir} variable.
		if strings.HasPrefix(line, "prefix=") {
			fmt.Fprintf(&buffer, "prefix=${pcfiledir}/../..\n")
			continue
		}

		// Strip pkgconf sysroot variables (cross-compilation artifacts).
		line = strings.ReplaceAll(line, "${pc_sysrootdir}", "")
		line = strings.ReplaceAll(line, "${pc_sys_root_dir}", "")

		// Replace any absolute packageDir path with ${prefix}.
		line = strings.ReplaceAll(line, packageDir+"/", "${prefix}/")
		line = strings.ReplaceAll(line, packageDir, "${prefix}")

		fmt.Fprintf(&buffer, "%s\n", line)
	}

	if err := scanner.Err(); err != nil {
		return err
	}

	if buffer.Len() > 0 {
		os.WriteFile(pkgPath, buffer.Bytes(), os.ModePerm)
	}

	return nil
}
