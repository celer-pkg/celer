package generator

import (
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strings"

	"github.com/BurntSushi/toml"
)

func (c *cmakeConfig) GenerateCMakeLists(repoDir, libName string) error {
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
	cmakeListBytes, err := templates.ReadFile("templates/single/CMakeLists.txt.in")
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
	cmakeListContent := string(cmakeListBytes)
	cmakeListContent = strings.ReplaceAll(cmakeListContent, "@NAMESPACE@", c.Namespace)
	cmakeListContent = strings.ReplaceAll(cmakeListContent, "@LIBNAME@", libName)
	cmakeListContent = strings.ReplaceAll(cmakeListContent, "@VERSION@", c.Version)
	cmakeListContent = strings.ReplaceAll(cmakeListContent, "@FILENAME@", c.Filename)

	return os.WriteFile(filepath.Join(repoDir, "CMakeLists.txt"), []byte(cmakeListContent), os.ModePerm)
}

// ReadCMakeConfig Find matched cmake config.
func ReadCMakeConfig(cmakeConfigPath, systemName, libraryType string) (*cmakeConfig, error) {
	// Read the cmake_config.toml file.
	bytes, err := os.ReadFile(cmakeConfigPath)
	if err != nil {
		return nil, err
	}
	var cmakeConfigs cmakeConfigs
	if err := toml.Unmarshal(bytes, &cmakeConfigs); err != nil {
		return nil, err
	}

	var cmakeConfig *cmakeConfig
	configRefer := strings.ToLower(fmt.Sprintf("%s_%s", systemName, libraryType))

	switch configRefer {
	case "linux_static":
		cmakeConfig = &cmakeConfigs.LinuxStatic

	case "linux_shared":
		cmakeConfig = &cmakeConfigs.LinuxShared

	case "windows_static":
		cmakeConfig = &cmakeConfigs.WindowsStatic

	case "windows_shared":
		cmakeConfig = &cmakeConfigs.WindowsShared

	case "linux_interface":
		cmakeConfig = &cmakeConfigs.LinuxInterface

	case "windows_interface":
		cmakeConfig = &cmakeConfigs.WindowsInterface

	default:
		return nil, fmt.Errorf("unknown config refer: %s", configRefer)
	}

	cmakeConfig.Namespace = cmakeConfigs.Namespace
	cmakeConfig.Libtype = libraryType
	return cmakeConfig, nil
}

func (c *cmakeConfig) isValidVersionFormat(version string) bool {
	// Match pattern:
	// 1. ^\d+(\.\d+)*$ -- number version (1, 1.2, 1.2.3, 1.2.3.4)
	// 2. ^\d+(\.\d+)*[-+][a-zA-Z0-9._]+$ -- number version with suffix (1.0.0-alpha1, 2.0+beta)

	pattern := `^(\d+(\.\d+)*)([-+][a-zA-Z0-9._]+)?$`
	matched, _ := regexp.MatchString(pattern, version)
	return matched
}
