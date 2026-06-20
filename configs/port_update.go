package configs

import (
	"fmt"
	"os"

	"github.com/BurntSushi/toml"
)

// Update updates the port.toml's [package] section with the given
// url and/or ref overrides.
func (p *Port) Update(url, ref string) (string, error) {
	// Read and parse TOML.
	bytes, err := os.ReadFile(p.portFile)
	if err != nil {
		return "", fmt.Errorf("failed to read %s -> %w", p.portFile, err)
	}

	var port map[string]any
	if err := toml.Unmarshal(bytes, &port); err != nil {
		return "", fmt.Errorf("failed to parse %s -> %w", p.portFile, err)
	}

	// Get or create [package] section.
	pkgNode, ok := port["package"].(map[string]any)
	if !ok {
		pkgNode = make(map[string]any)
		port["package"] = pkgNode
	}

	// Apply overrides.
	if url != "" {
		pkgNode["url"] = url
	}
	if ref != "" {
		pkgNode["ref"] = ref
	}

	// Marshal back and write.
	out, err := toml.Marshal(port)
	if err != nil {
		return "", fmt.Errorf("failed to marshal port config -> %w", err)
	}
	if err := os.WriteFile(p.portFile, out, os.ModePerm); err != nil {
		return "", fmt.Errorf("failed to write %s -> %w", p.portFile, err)
	}

	return p.portFile, nil
}
