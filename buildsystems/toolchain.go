package buildsystems

import (
	"celer/context"
	"celer/pkgs/expr"
	"os"
)

// Toolchain same with `Toolchain` in config/toolchain.go
// redefine to avoid import cycle.
type Toolchain struct {
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
	CStandard       string
	CXXStandard     string
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

	MSVC context.MSVC
}

func (t Toolchain) SetEnvs(buildConfig *BuildConfig) {
	os.Setenv("CROSSTOOL_PREFIX", t.CrosstoolPrefix)
	os.Setenv("HOST", t.Host)

	os.Setenv("CC", expr.If(t.RootFS != "", t.CC+" --sysroot="+t.RootFS, t.CC))
	os.Setenv("CXX", expr.If(t.RootFS != "", t.CXX+" --sysroot="+t.RootFS, t.CXX))

	if t.AS != "" {
		os.Setenv("AS", t.AS)
	}

	if t.FC != "" {
		os.Setenv("FC", t.FC)
	}

	if t.RANLIB != "" {
		os.Setenv("RANLIB", t.RANLIB)
	}

	if t.AR != "" {
		os.Setenv("AR", t.AR)
	}

	if t.LD != "" {
		os.Setenv("LD", t.LD)
	}

	if t.NM != "" {
		os.Setenv("NM", t.NM)
	}

	if t.OBJCOPY != "" {
		os.Setenv("OBJCOPY", t.OBJCOPY)
	}

	if t.OBJDUMP != "" {
		os.Setenv("OBJDUMP", t.OBJDUMP)
	}

	if t.STRIP != "" {
		os.Setenv("STRIP", t.STRIP)
	}

	if t.READELF != "" {
		os.Setenv("READELF", t.READELF)
	}
}

func (t Toolchain) ClearEnvs() {
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
