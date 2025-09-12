package generator

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type modulesBuildType struct {
	cmakeConfig cmakeConfig
}

func (g *modulesBuildType) generate(installedDir string) error {
	if len(g.cmakeConfig.Components) == 0 {
		return fmt.Errorf("components is empty")
	}

	if g.cmakeConfig.SystemName == "" {
		return fmt.Errorf("system name is empty")
	}

	if g.cmakeConfig.BuildType == "" {
		return fmt.Errorf("build type is empty")
	}

	if g.cmakeConfig.Libname == "" {
		return fmt.Errorf("lib name is empty")
	}

	if g.cmakeConfig.Version == "" {
		return fmt.Errorf("version is empty")
	}

	if g.cmakeConfig.Libtype == "" {
		return fmt.Errorf("lib type is empty")
	}

	if g.cmakeConfig.Namespace == "" {
		g.cmakeConfig.Namespace = g.cmakeConfig.Libname
	}

	modulesTypeBytes, err := templates.ReadFile("templates/ModulesBuildType.cmake.in")
	if err != nil {
		return err
	}

	componentTemplatePath := fmt.Sprintf("templates/%s/component-%s.cmake.in",
		strings.ToLower(g.cmakeConfig.SystemName), strings.ToLower(g.cmakeConfig.Libtype))
	componentBytes, err := templates.ReadFile(componentTemplatePath)
	if err != nil {
		return err
	}

	var sections strings.Builder
	for index, component := range g.cmakeConfig.Components {
		var section = string(componentBytes)
		section = strings.ReplaceAll(section, "@NAMESPACE@", g.cmakeConfig.Namespace)
		section = strings.ReplaceAll(section, "@BUILD_TYPE@", g.cmakeConfig.BuildType)
		section = strings.ReplaceAll(section, "@BUILD_TYPE_UPPER@", strings.ToUpper(g.cmakeConfig.BuildType))
		section = strings.ReplaceAll(section, "@COMPONENT@", component.Component)
		section = strings.ReplaceAll(section, "@FILENAME@", component.Filename)
		section = strings.ReplaceAll(section, "@SONAME@", component.Soname)
		section = strings.ReplaceAll(section, "@IMPNAME@", component.Impname)

		if index == 0 {
			sections.WriteString(section + "\n")
		} else if index == len(g.cmakeConfig.Components)-1 {
			sections.WriteString("\n" + section)
		} else {
			sections.WriteString("\n" + section + "\n")
		}
	}

	content := string(modulesTypeBytes)
	content = strings.ReplaceAll(content, "@BUILD_TYPE@", g.cmakeConfig.BuildType)
	content = strings.ReplaceAll(content, "@COMPONENT_SECTIONS@", sections.String())

	// Make dirs for writing file.
	fileName := fmt.Sprintf("%sModules-%s.cmake", g.cmakeConfig.Namespace, strings.ToLower(g.cmakeConfig.BuildType))
	filePath := filepath.Join(installedDir, "lib", "cmake", g.cmakeConfig.Namespace, fileName)
	if err := os.MkdirAll(filepath.Dir(filePath), os.ModePerm); err != nil {
		return err
	}

	// Do write file.
	if err := os.WriteFile(filePath, []byte(content), os.ModePerm); err != nil {
		return err
	}

	return nil
}
