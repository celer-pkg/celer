package generator

import (
	"strings"
)

type generator struct {
	// It's the name of the binary file.
	// in linux, it would be libyaml-cpp.a or libyaml-cpp.so.0.8.0
	// in windows, it would be yaml-cpp.lib or yaml-cpp.dll
	Filename string `toml:"filename"`

	Soname  string `toml:"soname"`  // linux, for example: libyaml-cpp.so.0.8
	Impname string `toml:"impname"` // windows, for example: yaml-cpp.lib

	Components []component `toml:"components"` // case for cmake component
	Libraries  []string    `toml:"libraries"`  // case for interface type.

	// Internal fields.
	Namespace  string `toml:"-"` // if empty, use libName instead
	SystemName string `toml:"-"` // for example: Linux, Windows or Darwin
	Libname    string `toml:"-"`
	Version    string `toml:"-"`
	BuildType  string `toml:"-"`
	Libtype    string `toml:"-"` // it would be static, shared or imported
}

func (c *generator) Generate(packagesDir string) error {
	c.Libtype = strings.ToLower(c.Libtype)

	return nil
}
