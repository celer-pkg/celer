package configs

import (
	"celer/context"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

const ccacheDefaultMaxSize = "10G"

type CCache struct {
	Enabled       bool   `toml:"enabled"`                  // ENV: CCACHE_ENABLED
	MaxSize       string `toml:"maxsize"`                  // ENV: CCACHE_MAXSIZE
	Dir           string `toml:"dir"`                      // ENV: CCACHE_DIR
	RemoteStorage string `toml:"remote_storage,omitempty"` // ENV: CCACHE_REMOTE_STORAGE
	RemoteOnly    bool   `toml:"remote_only,omitempty"`    // ENV: CCACHE_REMOTE_ONLY

	ctx context.Context `toml:"-"`
}

func (c *CCache) Validate() error {
	if c.Enabled {
		os.Unsetenv("CCACHE_DISABLE")
	} else {
		os.Setenv("CCACHE_DISABLE", "1")
	}

	if c.Dir == "" {
		c.Dir = filepath.Join(os.Getenv("HOME"), ".ccache")
	} else if !fileio.PathExists(c.Dir) {
		return fmt.Errorf("ccache dir does not exist: %s", c.Dir)
	}

	if c.MaxSize == "" {
		c.MaxSize = "5G"
	} else if !strings.HasSuffix(c.MaxSize, "M") && !strings.HasSuffix(c.MaxSize, "G") {
		return fmt.Errorf("ccache maxsize must end with `M` or `G`: %s", c.MaxSize)
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

func (c CCache) Generate(toolchain *strings.Builder) error {
	fmt.Fprintf(toolchain, "\n# CCache.\n")
	fmt.Fprintf(toolchain, "set(%-28s%q)\n", "CMAKE_C_COMPILER_LAUNCHER", "ccache")
	fmt.Fprintf(toolchain, "set(%-28s%q)\n", "CMAKE_CXX_COMPILER_LAUNCHER", "ccache")
	return nil
}
