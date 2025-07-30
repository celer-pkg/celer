package generator

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type configVersion struct {
	cmakeConfig cmakeConfig
}

func (g *configVersion) generate(installedDir string) error {
	if g.cmakeConfig.Libname == "" {
		return fmt.Errorf("lib name is empty")
	}

	if g.cmakeConfig.Version == "" {
		g.cmakeConfig.Version = "0.0.0"
	}

	bytes, err := templates.ReadFile("templates/ConfigVersion.cmake.in")
	if err != nil {
		return err
	}

	// Replace the placeholders with the actual values.
	content := string(bytes)
	content = strings.ReplaceAll(content, "@VERSION@", g.cmakeConfig.Version)

	// Make dirs for writing file.
	filePath := filepath.Join(installedDir, "lib", "cmake", g.cmakeConfig.Namespace, g.cmakeConfig.Namespace+"ConfigVersion.cmake")
	if err := os.MkdirAll(filepath.Dir(filePath), os.ModePerm); err != nil {
		return err
	}

	// Do write file.
	if err := os.WriteFile(filePath, []byte(content), os.ModePerm); err != nil {
		return err
	}

	return nil
}
