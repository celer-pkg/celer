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
	Dir      string `toml:"dir,omitempty"`
	MaxSize  string `toml:"maxsize,omitempty"`
	Compress bool   `toml:"compress,omitempty"`

	ctx context.Context `toml:"-"`
}

func (c *CCache) Validate() error {
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
	if c.Compress {
		os.Setenv("CCACHE_COMPRESS", "true")
	} else {
		os.Setenv("CCACHE_NOCOMPRESS", "true")
	}

	return nil
}

func (c CCache) Generate(toolchain *strings.Builder) error {
	fmt.Fprintf(toolchain, "\n# CCache.\n")
	fmt.Fprintf(toolchain, "set(%-28s%q)\n", "CMAKE_C_COMPILER_LAUNCHER", "ccache")
	fmt.Fprintf(toolchain, "set(%-28s%q)\n", "CMAKE_CXX_COMPILER_LAUNCHER", "ccache")
	return nil
}
