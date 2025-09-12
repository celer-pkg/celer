package generator

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type config struct {
	cmakeConfig cmakeConfig
}

func (g *config) generate(installedDir string) error {
	if g.cmakeConfig.Libname == "" {
		return fmt.Errorf("lib name is empty")
	}

	// Set namespace to libName if it is empty.
	if g.cmakeConfig.Namespace == "" {
		g.cmakeConfig.Namespace = g.cmakeConfig.Libname
	}

	bytes, err := templates.ReadFile("templates/Config.cmake.in")
	if err != nil {
		return err
	}

	// Replace the placeholders with the actual values.
	libNameUpper := strings.ReplaceAll(g.cmakeConfig.Libname, "-", "_")
	libNameUpper = strings.ToUpper(libNameUpper)

	content := string(bytes)
	content = strings.ReplaceAll(content, "@LIBNAME@", g.cmakeConfig.Libname)
	content = strings.ReplaceAll(content, "@LIBNAME_UPPER@", libNameUpper)
	content = strings.ReplaceAll(content, "@NAMESPACE@", g.cmakeConfig.Namespace)

	if len(g.cmakeConfig.Components) > 0 {
		content = strings.ReplaceAll(content, "@CONFIG_OR_MODULE_FILE@", g.cmakeConfig.Namespace+"Modules.cmake")
	} else {
		content = strings.ReplaceAll(content, "@CONFIG_OR_MODULE_FILE@", g.cmakeConfig.Namespace+"Targets.cmake")
	}

	// Mkdirs for writing file.
	filePath := filepath.Join(installedDir, "lib", "cmake", g.cmakeConfig.Namespace, g.cmakeConfig.Namespace+"config.cmake")
	if err := os.MkdirAll(filepath.Dir(filePath), os.ModePerm); err != nil {
		return err
	}

	// Do write file.
	if err := os.WriteFile(filePath, []byte(content), os.ModePerm); err != nil {
		return err
	}

	return nil
}
