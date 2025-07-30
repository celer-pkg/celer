package buildsystems

import (
	"celer/pkgs/expr"
	"os"
)

// CrossTools same with `Toolchain` in config/toolchain.go
// redefine to avoid import cycle.
type CrossTools struct {
	Native          bool
	Fullpath        string
	Name            string
	Version         string
	SystemName      string
	SystemProcessor string
	Host            string
	RootFS          string
	PkgConfigPath   []string
	IncludeDirs     []string
	LibDirs         []string
	CrosstoolPrefix string
	CC              string
	CXX             string
	AS              string
	FC              string
	AR              string
	RANLIB          string
	LD              string
	NM              string
	OBJCOPY         string
	OBJDUMP         string
	STRIP           string
	READELF         string

	// Works for windows only.
	MSVC msvc
}

type msvc struct {
	IncludeDirs []string
	LibDirs     []string
	BinDirs     []string
	MT          string
	RC          string
}

func (c CrossTools) SetEnvs(buildConfig *BuildConfig) {
	os.Setenv("CROSSTOOL_PREFIX", c.CrosstoolPrefix)
	os.Setenv("HOST", c.Host)

	os.Setenv("CC", expr.If(c.RootFS != "", c.CC+" --sysroot="+c.RootFS, c.CC))
	os.Setenv("CXX", expr.If(c.RootFS != "", c.CXX+" --sysroot="+c.RootFS, c.CXX))

	if c.AS != "" {
		os.Setenv("AS", c.AS)
	}

	if c.FC != "" {
		os.Setenv("FC", c.FC)
	}

	if c.RANLIB != "" {
		os.Setenv("RANLIB", c.RANLIB)
	}

	if c.AR != "" {
		os.Setenv("AR", c.AR)
	}

	if c.LD != "" {
		os.Setenv("LD", c.LD)
	}

	if c.NM != "" {
		os.Setenv("NM", c.NM)
	}

	if c.OBJCOPY != "" {
		os.Setenv("OBJCOPY", c.OBJCOPY)
	}

	if c.OBJDUMP != "" {
		os.Setenv("OBJDUMP", c.OBJDUMP)
	}

	if c.STRIP != "" {
		os.Setenv("STRIP", c.STRIP)
	}

	if c.READELF != "" {
		os.Setenv("READELF", c.READELF)
	}
}

func (CrossTools) ClearEnvs() {
	os.Unsetenv("CROSSTOOL_PREFIX")
	os.Unsetenv("SYSROOT")
	os.Unsetenv("HOST")
	os.Unsetenv("CC")
	os.Unsetenv("CXX")
	os.Unsetenv("AS")
	os.Unsetenv("FC")
	os.Unsetenv("RANLIB")
	os.Unsetenv("AR")
	os.Unsetenv("LD")
	os.Unsetenv("NM")
	os.Unsetenv("OBJCOPY")
	os.Unsetenv("OBJDUMP")
	os.Unsetenv("STRIP")
	os.Unsetenv("READELF")

	// MSVC related envs.
	os.Unsetenv("INCLUDE")
	os.Unsetenv("LIB")
	os.Unsetenv("LIBPATH")
	os.Unsetenv("VSINSTALLDIR")
	os.Unsetenv("VCINSTALLDIR")
}
