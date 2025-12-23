package generator

import (
	"celer/pkgs/fileio"
	"embed"
	"fmt"
	"os"
	"path/filepath"
	"strings"

	"github.com/BurntSushi/toml"
)

//go:embed templates
var templates embed.FS

type cmakeConfigs struct {
	Namespace string `toml:"namespace"`

	LinuxStatic      cmakeConfig `toml:"linux_static"`
	LinuxShared      cmakeConfig `toml:"linux_shared"`
	WindowsStatic    cmakeConfig `toml:"windows_static"`
	WindowsShared    cmakeConfig `toml:"windows_shared"`
	LinuxInterface   cmakeConfig `toml:"linux_interface"`
	WindowsInterface cmakeConfig `toml:"windows_interface"`

	// INTERFACE IMPORTED
	Name     string   `toml:"name"`
	Version  string   `toml:"version"`
	Sonames  []string `toml:"sonames"`
	Impnames []string `toml:"impnames"`
}

// FindMatchedConfig Find matched cmake config.
func FindMatchedConfig(portDir, preferedPortDir, systemName, libraryType string) (*cmakeConfig, error) {
	defaultConfigPath := filepath.Join(portDir, "cmake_config.toml")
	preferedConfigPath := filepath.Join(preferedPortDir, "cmake_config.toml")

	var configPath string
	if fileio.PathExists(preferedConfigPath) {
		configPath = preferedConfigPath
	} else if fileio.PathExists(defaultConfigPath) {
		configPath = defaultConfigPath
	} else {
		return nil, nil
	}

	// Read the cmake_config.toml file.
	bytes, err := os.ReadFile(configPath)
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

// cmakeConfig is the information of the library.
type cmakeConfig struct {
	// It's the name of the binary file.
	// in linux, it would be libyaml-cpp.a or libyaml-cpp.so.0.8.0
	// in windows, it would be yaml-cpp.lib or yaml-cpp.dll
	Filename string `toml:"filename"`
	Version  string `toml:"version"`

	Soname  string `toml:"soname"`  // linux, for example: libyaml-cpp.so.0.8
	Impname string `toml:"impname"` // windows, for example: yaml-cpp.lib

	Components []component `toml:"components"` // case for cmake component
	Libraries  []string    `toml:"libraries"`  // case for interface type.

	// Internal fields.
	Namespace  string `toml:"-"` // if empty, use libName instead
	SystemName string `toml:"-"` // for example: Linux, Windows or Darwin
	BuildType  string `toml:"-"`
	Libtype    string `toml:"-"` // it would be static, shared or imported
}

type component struct {
	Component    string   `toml:"component"`
	Soname       string   `toml:"soname"`
	Impname      string   `toml:"impname"`
	Filename     string   `toml:"filename"`
	Dependencies []string `toml:"dependencies"`
}

type generate interface {
	generate(packagesDir string) error
}

func (c cmakeConfig) Generate(packagesDir string) error {
	c.Libtype = strings.ToLower(c.Libtype)
	// var generators []generate

	// if c.Libtype == "interface" {
	// 	generators = []generate{
	// 		&config{c},
	// 		&configVersion{c},
	// 	}
	// } else {
	// 	if len(c.Components) == 0 {
	// 		generators = []generate{
	// 			&config{c},
	// 			&targets{c},
	// 			&configVersion{c},
	// 			&targetsBuildType{c},
	// 		}
	// 	} else {
	// 		generators = []generate{
	// 			&config{c},
	// 			&configVersion{c},
	// 			&modules{c},
	// 			&modulesBuildType{c},
	// 		}
	// 	}
	// }

	// for _, gen := range generators {
	// 	if err := gen.generate(packagesDir); err != nil {
	// 		return fmt.Errorf("generate cmake config: %w", err)
	// 	}
	// }

	return nil
}
