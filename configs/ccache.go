package configs

import (
	"celer/pkgs/dirs"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"
)

const ccacheDefaultMaxSize = "10G"

type CCache struct {
	Enabled       bool   `toml:"enabled"`                  // ENV: CCACHE_ENABLED
	MaxSize       string `toml:"maxsize"`                  // ENV: CCACHE_MAXSIZE
	Dir           string `toml:"dir"`                      // ENV: CCACHE_DIR
	RemoteStorage string `toml:"remote_storage,omitempty"` // ENV: CCACHE_REMOTE_STORAGE
	RemoteOnly    bool   `toml:"remote_only,omitempty"`    // ENV: CCACHE_REMOTE_ONLY
}

// Setup setup ccache.
func (c CCache) Setup() error {
	if c.Enabled {
		os.Unsetenv("CCACHE_DISABLE")
	} else {
		os.Setenv("CCACHE_DISABLE", "1")
	}

	// Create ccache dir if not exist.
	if err := os.MkdirAll(c.Dir, os.ModePerm); err != nil {
		return fmt.Errorf("failed to mkdir ccache dir.\n %w", err)
	}

	os.Setenv("CCACHE_DIR", c.Dir)
	os.Setenv("CCACHE_MAXSIZE", c.MaxSize)

	if c.RemoteStorage != "" {
		os.Setenv("CCACHE_REMOTE_STORAGE", c.RemoteStorage)

		if c.RemoteOnly {
			os.Setenv("CCACHE_REMOTE_ONLY", "1")
		} else {
			os.Unsetenv("CCACHE_REMOTE_ONLY")
		}
	}

	// Default to workspace dir as base dir.
	os.Setenv("CCACHE_BASEDIR", dirs.WorkspaceDir)
	return nil
}

func (c *CCache) init() {
	c.Enabled = true
	c.MaxSize = ccacheDefaultMaxSize
	c.MaxSize = "5G"

	switch runtime.GOOS {
	case "windows":
		c.Dir = filepath.Join(os.Getenv("USERPROFILE"), "ccache")
	default:
		c.Dir = filepath.Join(os.Getenv("HOME"), ".cache", "ccache")
	}
}

func (c CCache) Generate(toolchain *strings.Builder) error {
	fmt.Fprintf(toolchain, "\n# CCache.\n")
	fmt.Fprintf(toolchain, "set(%-28s%q)\n", "CMAKE_C_COMPILER_LAUNCHER", "ccache")
	fmt.Fprintf(toolchain, "set(%-28s%q)\n", "CMAKE_CXX_COMPILER_LAUNCHER", "ccache")
	return nil
}
