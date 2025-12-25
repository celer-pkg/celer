package generator

import (
	"celer/pkgs/expr"
	"embed"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strings"

	"github.com/BurntSushi/toml"
)

//go:embed templates
var templates embed.FS

type libInfo interface {
	GetNamespace() string
}

type cmakeConfig struct {
	Namespace string       `toml:"namespace"`
	Linux     targetConfig `toml:"linux"`
	Windows   targetConfig `toml:"windows"`
}

func (c *cmakeConfig) GetNamespace() string {
	return c.Namespace
}

type targetConfig struct {
	Filename   string      `toml:"filename"`
	Filenames  []string    `toml:"filenames"`
	Components []component `toml:"components"`

	libInfo libInfo `toml:"-"`
}

type component struct {
	Component    string   `toml:"component"`
	Filename     string   `toml:"filename"`
	Dependencies []string `toml:"dependencies"`
}

func (t *targetConfig) GenerateCMakeLists(repoDir, libName, libVersion string) error {
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
	tempalte := expr.If(len(t.Components) == 0, "templates/single/CMakeLists.txt.in", "templates/components/CMakeLists.txt.in")
	cmakeListBytes, err := templates.ReadFile(tempalte)
	if err != nil {
		return err
	}

	// Set namespace to libName if it is empty.
	namespace := t.libInfo.GetNamespace()
	if namespace == "" {
		namespace = libName
	}

	// If version is empty or invalid, set to 0.0.1
	if libVersion == "" || !t.isValidVersionFormat(libVersion) {
		libVersion = "0.0.1"
	}

	// Replace the placeholders with the actual values.
	content := string(cmakeListBytes)
	content = strings.ReplaceAll(content, "@NAMESPACE@", namespace)
	content = strings.ReplaceAll(content, "@LIBNAME@", libName)
	content = strings.ReplaceAll(content, "@VERSION@", libVersion)

	// Replace filenames.
	if len(t.Filenames) == 0 {
		content = strings.ReplaceAll(content, "@FILENAMES@", t.Filename)
	} else {
		content = strings.ReplaceAll(content, "@FILENAMES@", strings.Join(t.Filenames, " "))
	}

	// Generate components if any.
	if strings.Contains(content, "@COMPONENTS@") {
		components, err := t.generateComponents()
		if err != nil {
			return err
		}
		content = strings.ReplaceAll(content, "@COMPONENTS@", components)
	}

	return os.WriteFile(filepath.Join(repoDir, "CMakeLists.txt"), []byte(content), os.ModePerm)
}

func (t *targetConfig) generateComponents() (string, error) {
	var components strings.Builder
	for _, component := range t.Components {
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

func (t *targetConfig) isValidVersionFormat(version string) bool {
	// Match patterns:
	// 1. ^\d+(\.\d+)*$ -- number version (1, 1.2, 1.2.3, 1.2.3.4)
	// 2. ^\d+(\.\d+)*[-+][a-zA-Z0-9._]+$ -- number version with suffix (1.0.0-alpha1, 2.0+beta)
	pattern := `^(\d+(\.\d+)*)([-+][a-zA-Z0-9._]+)?$`
	matched, _ := regexp.MatchString(pattern, version)
	return matched
}

// ReadCMakeConfig Find matched cmake config.
func ReadCMakeConfig(configPath, systemName string) (*targetConfig, error) {
	// Read the cmake_config.toml file.
	bytes, err := os.ReadFile(configPath)
	if err != nil {
		return nil, err
	}
	var cmakeConfig cmakeConfig
	if err := toml.Unmarshal(bytes, &cmakeConfig); err != nil {
		return nil, err
	}

	// Find the matched config.
	var targetConfig *targetConfig

	switch strings.ToLower(systemName) {
	case "linux":
		targetConfig = &cmakeConfig.Linux

	case "windows":
		targetConfig = &cmakeConfig.Windows

	default:
		return nil, fmt.Errorf("unknown config refer: %s", systemName)
	}

	// Set common fields.
	targetConfig.libInfo = &cmakeConfig
	return targetConfig, nil
}
