package generator

import (
	"celer/pkgs/expr"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strings"

	"github.com/BurntSushi/toml"
)

// ReadCMakeConfig Find matched cmake config.
func ReadCMakeConfig(cmakeConfigPath, systemName string) (*cmakeConfig, error) {
	// Read the cmake_config.toml file.
	bytes, err := os.ReadFile(cmakeConfigPath)
	if err != nil {
		return nil, err
	}
	var cmakeConfigs cmakeConfigs
	if err := toml.Unmarshal(bytes, &cmakeConfigs); err != nil {
		return nil, err
	}

	// Find the matched config.
	var cmakeConfig *cmakeConfig

	switch strings.ToLower(systemName) {
	case "linux":
		cmakeConfig = &cmakeConfigs.Linux

	case "windows":
		cmakeConfig = &cmakeConfigs.Windows

	default:
		return nil, fmt.Errorf("unknown config refer: %s", systemName)
	}

	// Set common fields.
	cmakeConfig.Namespace = cmakeConfigs.Namespace
	return cmakeConfig, nil
}

func (c *cmakeConfig) GenerateCMakeLists(repoDir, libName string) error {
	// Create the cmake directory.
	if err := os.MkdirAll(filepath.Join(repoDir, "cmake"), os.ModePerm); err != nil {
		return err
	}

	// Write the Config.cmake.in file.
	configBytes, err := templates.ReadFile("templates/Config.cmake.in")
	if err != nil {
		return err
	}
	if err := os.WriteFile(filepath.Join(repoDir, "cmake", "Config.cmake.in"), configBytes, os.ModePerm); err != nil {
		return err
	}

	// Generate CMakeLists.txt with the template.
	tempalte := expr.If(len(c.Components) == 0, "templates/single/CMakeLists.txt.in", "templates/components/CMakeLists.txt.in")
	cmakeListBytes, err := templates.ReadFile(tempalte)
	if err != nil {
		return err
	}

	// Set namespace to libName if it is empty.
	if c.Namespace == "" {
		c.Namespace = libName
	}

	// If version is empty or invalid, set to 0.0.1
	if c.Version == "" || !c.isValidVersionFormat(c.Version) {
		c.Version = "0.0.1"
	}

	// Replace the placeholders with the actual values.
	content := string(cmakeListBytes)
	content = strings.ReplaceAll(content, "@NAMESPACE@", c.Namespace)
	content = strings.ReplaceAll(content, "@LIBNAME@", libName)
	content = strings.ReplaceAll(content, "@VERSION@", c.Version)

	if len(c.Filenames) == 0 {
		content = strings.ReplaceAll(content, "@FILENAMES@", c.Filename)
	} else {
		content = strings.ReplaceAll(content, "@FILENAMES@", strings.Join(c.Filenames, " "))
	}

	if strings.Contains(content, "@COMPONENTS@") {
		components, err := c.generateComponents()
		if err != nil {
			return err
		}
		content = strings.ReplaceAll(content, "@COMPONENTS@", components)
	}

	return os.WriteFile(filepath.Join(repoDir, "CMakeLists.txt"), []byte(content), os.ModePerm)
}

func (c *cmakeConfig) generateComponents() (string, error) {
	var components strings.Builder
	for _, component := range c.Components {
		bytes, err := templates.ReadFile("templates/components/Component.cmake.in")
		if err != nil {
			return "", err
		}

		content := string(bytes)
		content = strings.ReplaceAll(content, "@COMPONENT@", component.Component)
		content = strings.ReplaceAll(content, "@FILENAME@", component.Filename)

		if len(component.Dependencies) == 0 {
			content = strings.ReplaceAll(content, "@DEPENDENCIES@", "\n")
		} else {
			content = strings.ReplaceAll(content, "@DEPENDENCIES@", strings.Join(component.Dependencies, " ")+"\n")
		}

		components.WriteString(content)
		components.WriteString("\n")
	}
	return components.String(), nil
}

func (c *cmakeConfig) isValidVersionFormat(version string) bool {
	// Match patterns:
	// 1. ^\d+(\.\d+)*$ -- number version (1, 1.2, 1.2.3, 1.2.3.4)
	// 2. ^\d+(\.\d+)*[-+][a-zA-Z0-9._]+$ -- number version with suffix (1.0.0-alpha1, 2.0+beta)
	pattern := `^(\d+(\.\d+)*)([-+][a-zA-Z0-9._]+)?$`
	matched, _ := regexp.MatchString(pattern, version)
	return matched
}
