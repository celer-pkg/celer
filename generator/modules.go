package generator

import (
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type modules struct {
	cmakeConfig cmakeConfig
}

func (g *modules) generate(installedDir string) error {
	if len(g.cmakeConfig.Components) == 0 {
		return fmt.Errorf("components is empty")
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
	g.cmakeConfig.Libtype = strings.ToLower(g.cmakeConfig.Libtype)

	if g.cmakeConfig.Namespace == "" {
		g.cmakeConfig.Namespace = g.cmakeConfig.Libname
	}

	modules, err := templates.ReadFile("templates/Modules.cmake.in")
	if err != nil {
		return err
	}

	addLibraryDepedencies, err := templates.ReadFile("templates/Modules-addLibrary-depedencies.cmake.in")
	if err != nil {
		return err
	}

	addLibraryIndependent, err := templates.ReadFile("templates/Modules-addLibrary-independent.cmake.in")
	if err != nil {
		return err
	}

	var libNames []string
	var addLibrarySections strings.Builder

	for index, component := range g.cmakeConfig.Components {
		if strings.ToLower(g.cmakeConfig.SystemName) == "linux" {
			if g.cmakeConfig.Libtype == "shared" {
				// Check soname and filename are not empty.
				var emptyFields []string
				if component.Soname == "" {
					emptyFields = append(emptyFields, "soname")
				}
				if component.Filename == "" {
					emptyFields = append(emptyFields, "filename")
				}
				if len(emptyFields) > 0 {
					return fmt.Errorf("%s of %s is empty to generate cmake config for linux shared library",
						strings.Join(emptyFields, " and "), component.Component)
				}

				// Check soname and filename are exist.
				var notExistFields []string
				if !fileio.PathExists(filepath.Join(installedDir, "lib", component.Filename)) {
					notExistFields = append(notExistFields, "filename")
				}
				if !fileio.PathExists(filepath.Join(installedDir, "lib", component.Soname)) {
					notExistFields = append(notExistFields, "soname")
				}
				if len(notExistFields) > 0 {
					return fmt.Errorf("%s of %s is not exist to generate cmake config for linux shared library",
						strings.Join(notExistFields, " and "), component.Component)
				}
			}
			if g.cmakeConfig.Libtype == "static" && component.Filename == "" {
				return fmt.Errorf("filename of %s is empty to generate cmake config for linux static library", component.Component)
			}
		}

		if strings.ToLower(g.cmakeConfig.SystemName) == "windows" {
			if g.cmakeConfig.Libtype == "shared" {
				// Check impname and filename are not empty.
				var emptyFields []string
				if component.Impname == "" {
					emptyFields = append(emptyFields, "impname")
				}
				if component.Filename == "" {
					emptyFields = append(emptyFields, "filename")
				}
				if len(emptyFields) > 0 {
					return fmt.Errorf("%s of %s is empty to generate cmake config for windows shared library",
						strings.Join(emptyFields, " and "), component.Component)
				}

				// Check impname and filename are exist.
				var notExistFields []string
				if !fileio.PathExists(filepath.Join(installedDir, "bin", component.Filename)) {
					notExistFields = append(notExistFields, "filename "+component.Filename)
				}
				if !fileio.PathExists(filepath.Join(installedDir, "lib", component.Impname)) {
					notExistFields = append(notExistFields, "impname "+component.Impname)
				}
				if len(notExistFields) > 0 {
					return fmt.Errorf("%s of %s is not exist to generate cmake config for windows shared library",
						strings.Join(notExistFields, " and "), component.Component)
				}
			}
			if g.cmakeConfig.Libtype == "static" && component.Filename == "" {
				return fmt.Errorf("filename of %s is empty to generate cmake config for windows static library", component.Component)
			}
		}

		libNames = append(libNames, g.cmakeConfig.Libname+"::"+component.Component)

		var section string
		if len(component.Dependencies) > 0 {
			section = string(addLibraryDepedencies)
		} else {
			section = string(addLibraryIndependent)
		}

		var dependencies []string
		for _, dependency := range component.Dependencies {
			dependencies = append(dependencies, g.cmakeConfig.Namespace+"::"+dependency)
		}

		section = strings.ReplaceAll(section, "@NAMESPACE@", g.cmakeConfig.Namespace)
		section = strings.ReplaceAll(section, "@LIBNAME@", g.cmakeConfig.Libname)
		section = strings.ReplaceAll(section, "@COMPONENT@", component.Component)
		section = strings.ReplaceAll(section, "@LIBTYPE_UPPER@", strings.ToUpper(g.cmakeConfig.Libtype))
		section = strings.ReplaceAll(section, "@DEPEDENCIES@", strings.Join(dependencies, ";"))

		if index == 0 {
			addLibrarySections.WriteString(section + "\n")
		} else if index == len(g.cmakeConfig.Components)-1 {
			addLibrarySections.WriteString("\n" + section)
		} else {
			addLibrarySections.WriteString("\n" + section + "\n")
		}
	}

	content := string(modules)
	content = strings.ReplaceAll(content, "@NAMESPACE@", g.cmakeConfig.Namespace)
	content = strings.ReplaceAll(content, "@LIB_NAMES@", strings.Join(libNames, " "))
	content = strings.ReplaceAll(content, "@LIBNAME@", g.cmakeConfig.Libname)
	content = strings.ReplaceAll(content, "@LIBNAME_UPPER@", strings.ToUpper(g.cmakeConfig.Libname))
	content = strings.ReplaceAll(content, "@ADD_LIBRARY_SECTIONS@", addLibrarySections.String())

	// Make dirs for writing file.
	filePath := filepath.Join(installedDir, "lib", "cmake", g.cmakeConfig.Namespace, g.cmakeConfig.Namespace+"Modules.cmake")
	if err := os.MkdirAll(filepath.Dir(filePath), os.ModePerm); err != nil {
		return err
	}

	// Do write file.
	if err := os.WriteFile(filePath, []byte(content), os.ModePerm); err != nil {
		return err
	}

	return nil
}
