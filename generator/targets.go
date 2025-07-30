package generator

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type targets struct {
	cmakeConfig cmakeConfig
}

func (g *targets) generate(installedDir string) error {
	if g.cmakeConfig.Libname == "" {
		return fmt.Errorf("lib name is empty")
	}

	if g.cmakeConfig.Libtype == "" {
		return fmt.Errorf("lib type is empty")
	}

	// Set namespace to libName if it is empty.
	if g.cmakeConfig.Namespace == "" {
		g.cmakeConfig.Namespace = g.cmakeConfig.Libname
	}

	bytes, err := templates.ReadFile("templates/Targets.cmake.in")
	if err != nil {
		return err
	}

	// Replace the placeholders with the actual values.
	libNameUpper := strings.ReplaceAll(g.cmakeConfig.Libname, "-", "_")
	libNameUpper = strings.ToUpper(libNameUpper)

	content := string(bytes)
	content = strings.ReplaceAll(content, "@NAMESPACE@", g.cmakeConfig.Namespace)
	content = strings.ReplaceAll(content, "@LIBNAME@", g.cmakeConfig.Libname)
	content = strings.ReplaceAll(content, "@LIBNAME_UPPER@", libNameUpper)
	content = strings.ReplaceAll(content, "@LIBTYPE@", g.cmakeConfig.Libtype)
	content = strings.ReplaceAll(content, "@LIBTYPE_UPPER@", strings.ToUpper(g.cmakeConfig.Libtype))

	// Make dirs for writing file.
	filePath := filepath.Join(installedDir, "lib", "cmake", g.cmakeConfig.Namespace, g.cmakeConfig.Namespace+"Targets.cmake")
	if err := os.MkdirAll(filepath.Dir(filePath), os.ModePerm); err != nil {
		return err
	}

	// Do write file.
	if err := os.WriteFile(filePath, []byte(content), os.ModePerm); err != nil {
		return err
	}

	return nil
}
