package generator

import (
	"embed"
)

//go:embed templates
var templates embed.FS

type cmakeConfigs struct {
	Namespace string `toml:"namespace"`

	Linux   cmakeConfig `toml:"linux"`
	Windows cmakeConfig `toml:"windows"`
}

// cmakeConfig is the information of the library.
type cmakeConfig struct {
	Filename   string      `toml:"filename"`
	Filenames  []string    `toml:"filenames"`
	Version    string      `toml:"version"`
	Components []component `toml:"components"`

	// Internal fields.
	Namespace string `toml:"-"`
}

type component struct {
	Component    string   `toml:"component"`
	Soname       string   `toml:"soname"`
	Impname      string   `toml:"impname"`
	Filename     string   `toml:"filename"`
	Dependencies []string `toml:"dependencies"`
}
