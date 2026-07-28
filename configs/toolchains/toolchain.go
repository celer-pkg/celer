package toolchains

import (
	"fmt"
	"strings"

	"github.com/celer-pkg/celer/context"
)

type Toolchain interface {
	Dir(abspath string) string
	AssembleBuildTools(toolchain *strings.Builder)
	ReadBuiltinEnvs() (map[string]string, error)
	CFlags() []string
	CXXFlags() []string
	LDFlags() []string
	RuntimeFlags() []string
}

type Infos struct {
	Url             string `toml:"url"`                       // Download url or local file url.
	SHA256          string `toml:"sha256"`                    // SHA256 of the toolchain archive, used for verification and caching.
	Name            string `toml:"name"`                      // It should be "gcc", "msvc", "clang-cl", "clang" and "msys2".
	Version         string `toml:"version"`                   // It should be version of gcc/msvc/clang.
	Archive         string `toml:"archive,omitempty"`         // Archive can be changed to avoid conflict.
	Path            string `toml:"path"`                      // Runtime path of tool, it's relative path and would be converted to absolute path later.
	SystemName      string `toml:"system_name"`               // It would be "Windows", "Linux", "Android" and so on.
	SystemProcessor string `toml:"system_processor"`          // It would be "x86_64", "aarch64" and so on.
	SystemVersion   string `toml:"system_version,omitempty"`  // It would be a version for Android API level, etc.
	Host            string `toml:"host"`                      // It would be "x86_64-linux-gnu", "aarch64-linux-gnu" and so on.
	EmbeddedSystem  bool   `toml:"embedded_system,omitempty"` // Whether it's for embedded system, like mcu or bare-metal.
}

type BuildTools struct {
	// Mandatory fields.
	CC  string `toml:"cc"`  // C language compiler.
	CXX string `toml:"cxx"` // C++ language compiler.

	// Core compiler tools (Essential).
	CPP string `toml:"cpp,omitempty"` // C preprocessor.
	AR  string `toml:"ar,omitempty"`  // Archive static library.
	LD  string `toml:"ld,omitempty"`  // Link executable.
	AS  string `toml:"as,omitempty"`  // Assemble assembly code.

	// Object file manipulation tools.
	OBJCOPY string `toml:"objcopy,omitempty"` // Copy object file.
	OBJDUMP string `toml:"objdump,omitempty"` // Dump object file.
	STRIP   string `toml:"strip,omitempty"`   // Strip executable and library.
	READELF string `toml:"readelf,omitempty"` // Read ELF file.
	SIZE    string `toml:"size,omitempty"`    // Display file size.
	STRINGS string `toml:"strings,omitempty"` // Display strings in file.

	// Symbol and archive tools.
	NM     string `toml:"nm,omitempty"`     // List symbols in object file.
	RANLIB string `toml:"ranlib,omitempty"` // Index static library.

	// Code coverage tools.
	GCOV string `toml:"gcov,omitempty"` // Gcov code coverage.

	// Debug and analysis tools.
	ADDR2LINE string `toml:"addr2line,omitempty"` // Convert address to line number.
	CXXFILT   string `toml:"cxxfilt,omitempty"`   // C++ symbol demangler.

	// Additional compiler tools.
	FC string `toml:"fc,omitempty"` // Compile Fortran code.

	// Compiler target triplets for multi-target compiler drivers (e.g. qcc).
	CCompilerTarget   string `toml:"c_compiler_target,omitempty"`
	CXXCompilerTarget string `toml:"cxx_compiler_target,omitempty"`

	// It would be like "x86_64-linux-gnu-"
	CrosstoolPrefix string `toml:"crosstool_prefix"`

	MSVC context.MSVC `toml:"-"`
}

func (b BuildTools) writeIfNotEmpty(toolchain *strings.Builder, key, value string) {
	if value != "" {
		fmt.Fprintf(toolchain, "set(%s %q)\n", key, "${TOOLCHAIN}/"+value)
	}
}

type BuildFlags struct {
	CFlags        []string `toml:"cflags"`
	CXXFlags      []string `toml:"cxxflags"`
	LDFlags       []string `toml:"ldflags"`
	CFlagsDebug   []string `toml:"cflags_debug"`
	CXXFlagsDebug []string `toml:"cxxflags_debug"`
	LDFlagsDebug  []string `toml:"ldflags_debug"`
}

// NewToolchain create different toolchain implementation with its name.
func NewToolchain(ctx context.Context, toolchainName string, infos Infos, buildTools BuildTools, buildFlags BuildFlags) Toolchain {
	switch toolchainName {
	case "clang":
		return &Clang{
			Context:    ctx,
			Infos:      infos,
			BuildTools: buildTools,
			BuildFlags: buildFlags,
		}

	case "gcc":
		return &GCC{
			Context:    ctx,
			Infos:      infos,
			BuildTools: buildTools,
			BuildFlags: buildFlags,
		}

	case "msvc":
		return &MSVC{
			Context:    ctx,
			Infos:      infos,
			BuildTools: buildTools,
			BuildFlags: buildFlags,
		}

	case "clang-cl":
		return &ClangCL{
			Context:    ctx,
			Infos:      infos,
			BuildTools: buildTools,
			BuildFlags: buildFlags,
		}

	case "qcc":
		return &QCC{
			Context:    ctx,
			Infos:      infos,
			BuildTools: buildTools,
			BuildFlags: buildFlags,
		}

	default:
		panic("unsupported toolchnain " + toolchainName)
	}
}
