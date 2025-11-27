package configs

import (
	"celer/context"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"strings"
)

type CCache struct {
	Enabled  bool   `toml:"enabled"`
	Dir      string `toml:"dir,omitempty"`
	MaxSize  string `toml:"maxsize,omitempty"`
	Compress bool   `toml:"compress,omitempty"`

	ctx context.Context `toml:"-"`
}

func (c *CCache) Validate() error {
	if c.Dir == "" {
		c.Dir = filepath.Join(os.Getenv("HOME"), ".ccache")
	} else if !fileio.PathExists(c.Dir) {
		return fmt.Errorf("ccache dir exists: %s", c.Dir)
	}

	if c.MaxSize == "" {
		c.MaxSize = "5G"
	} else if !strings.HasSuffix(c.MaxSize, "M") && !strings.HasSuffix(c.MaxSize, "G") {
		return fmt.Errorf("ccache maxsize must end with `M` or `G`: %s", c.MaxSize)
	}

	os.Setenv("CCACHE_DIR", c.Dir)
	os.Setenv("CCACHE_MAXSIZE", c.MaxSize)
	if c.Compress {
		os.Setenv("CCACHE_COMPRESS", "true")
	} else {
		os.Setenv("CCACHE_NOCOMPRESS", "true")
	}

	return nil
}

func (c CCache) Generate(toolchain *strings.Builder) error {
	fmt.Fprintf(toolchain, "\n# CCache.\n")
	fmt.Fprintf(toolchain, "set(%-28s%q)\n", "ENV{CCACHE_DIR}", c.Dir)
	fmt.Fprintf(toolchain, "set(%-28s%q)\n", "ENV{CCACHE_MAXSIZE}", c.MaxSize)
	if c.Compress {
		fmt.Fprintf(toolchain, "set(%-28s%q)\n", "ENV{CCACHE_COMPRESS}", "true")
	} else {
		fmt.Fprintf(toolchain, "set(%-28s%q)\n", "ENV{CCACHE_NOCOMPRESS}", "true")
	}
	fmt.Fprintf(toolchain, "set(%-28s%q)\n", "CMAKE_C_COMPILER_LAUNCHER", "ccache")
	fmt.Fprintf(toolchain, "set(%-28s%q)\n", "CMAKE_CXX_COMPILER_LAUNCHER", "ccache")
	return nil
}
