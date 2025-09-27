package generator

import (
	"celer/pkgs/expr"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type configVersion struct {
	cmakeConfig cmakeConfig
}

func (c *configVersion) generate(installedDir string) error {
	if c.cmakeConfig.Libname == "" {
		return fmt.Errorf("lib name is empty")
	}

	if c.cmakeConfig.Version == "" {
		c.cmakeConfig.Version = "0.0.0"
	}

	template := expr.If(c.cmakeConfig.Libtype == "interface",
		"templates/interface/ConfigVersion.cmake.in",
		"templates/ConfigVersion.cmake.in",
	)
	bytes, err := templates.ReadFile(template)
	if err != nil {
		return err
	}

	// Replace the placeholders with the actual values.
	content := string(bytes)
	content = strings.ReplaceAll(content, "@VERSION@", c.cmakeConfig.Version)
	content = strings.ReplaceAll(content, "@LIB_NAME@", c.cmakeConfig.Libname)

	// Make dirs for writing file.
	filePath := filepath.Join(installedDir, "lib", "cmake", c.cmakeConfig.Namespace, c.cmakeConfig.Namespace+"ConfigVersion.cmake")
	if err := os.MkdirAll(filepath.Dir(filePath), os.ModePerm); err != nil {
		return err
	}

	// Do write file.
	if err := os.WriteFile(filePath, []byte(content), os.ModePerm); err != nil {
		return err
	}

	return nil
}
