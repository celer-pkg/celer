package configs

import (
	"reflect"
	"strings"
	"testing"

	"github.com/celer-pkg/celer/configs/toolchains"
	"github.com/celer-pkg/celer/context"
)

// fakeContext implements the subset of context.Context used by toolchain
// tests; any other method call panics via the nil embedded interface.
type fakeContext struct {
	context.Context
	build string
}

func (f fakeContext) BuildType() string           { return f.build }
func (f fakeContext) ExprVars() *context.ExprVars { return nil }

func TestToolchainEffectiveFlags(t *testing.T) {
	toolchain := Toolchain{}
	toolchain.Name = "gcc"
	toolchain.CFlags = []string{"-O2"}
	toolchain.CXXFlags = []string{"-O2"}
	toolchain.LDFlags = []string{"-Wl,--as-needed"}
	toolchain.CFlagsDebug = []string{"-O0", "-g"}
	toolchain.CXXFlagsDebug = []string{"-O0", "-g"}
	toolchain.CXXFlagsDebug = []string{"-O0", "-g"}
	toolchain.LDFlagsDebug = []string{"-Wl,--export-dynamic"}

	// Initialize the toolchain implementation.
	toolchain.toolchain = toolchains.NewToolchain(
		toolchain.ctx,
		toolchain.Name,
		toolchain.Infos,
		toolchain.BuildTools,
		toolchain.BuildFlags,
	)

	cflags, cxxflags, ldflags := toolchain.effectiveFlags("debug")
	if !reflect.DeepEqual(cflags, toolchain.CFlagsDebug) {
		t.Fatalf("debug cflags = %v, want %v", cflags, toolchain.CFlagsDebug)
	}
	if !reflect.DeepEqual(cxxflags, toolchain.CXXFlagsDebug) {
		t.Fatalf("debug cxxflags = %v, want %v", cxxflags, toolchain.CXXFlagsDebug)
	}
	if !reflect.DeepEqual(ldflags, toolchain.LDFlagsDebug) {
		t.Fatalf("debug ldflags = %v, want %v", ldflags, toolchain.LDFlagsDebug)
	}

	cflags, cxxflags, ldflags = toolchain.effectiveFlags("release")
	if !reflect.DeepEqual(cflags, toolchain.CFlags) {
		t.Fatalf("release cflags = %v, want %v", cflags, toolchain.CFlags)
	}
	if !reflect.DeepEqual(cxxflags, toolchain.CXXFlags) {
		t.Fatalf("release cxxflags = %v, want %v", cxxflags, toolchain.CXXFlags)
	}
	if !reflect.DeepEqual(ldflags, toolchain.LDFlags) {
		t.Fatalf("release ldflags = %v, want %v", ldflags, toolchain.LDFlags)
	}
}

func TestToolchainGenerate_UsesDebugFlags(t *testing.T) {
	var buffer strings.Builder

	toolchain := Toolchain{}
	toolchain.Name = "gcc"
	toolchain.SystemName = "linux"
	toolchain.SystemProcessor = "x86_64"
	toolchain.Path = "/usr/bin"
	toolchain.CC = "gcc"
	toolchain.CXX = "g++"
	toolchain.CFlags = []string{"-O2"}
	toolchain.CXXFlags = []string{"-O2"}
	toolchain.LDFlags = []string{"-Wl,--as-needed"}
	toolchain.CFlagsDebug = []string{"-O0", "-g3"}
	toolchain.CXXFlagsDebug = []string{"-O0", "-g3"}
	toolchain.CXXFlagsDebug = []string{"-O2"}
	toolchain.LDFlagsDebug = []string{"-Wl,--export-dynamic"}
	toolchain.ctx = fakeContext{build: "debug"}

	// Initialize the toolchain implementation.
	toolchain.toolchain = toolchains.NewToolchain(
		toolchain.ctx,
		toolchain.Name,
		toolchain.Infos,
		toolchain.BuildTools,
		toolchain.BuildFlags,
	)

	if err := toolchain.generate(&buffer); err != nil {
		t.Fatalf("generate() error = %v", err)
	}

	output := buffer.String()
	// When both cflags and cxxflags exist, they use foreach loop
	expected := []string{
		`foreach(flag_var CMAKE_C_FLAGS_INIT CMAKE_CXX_FLAGS_INIT)`,
		`  string(APPEND ${flag_var} " -O0")`,
		`  string(APPEND ${flag_var} " -g3")`,
		`endforeach()`,
		`foreach(flag_var CMAKE_EXE_LINKER_FLAGS_INIT CMAKE_SHARED_LINKER_FLAGS_INIT CMAKE_MODULE_LINKER_FLAGS_INIT)`,
		`  string(APPEND ${flag_var} " -Wl,--export-dynamic")`,
	}

	for _, item := range expected {
		if !strings.Contains(output, item) {
			t.Fatalf("generated toolchain file missing %q\noutput:\n%s", item, output)
		}
	}

	unexpected := []string{
		`string(APPEND CMAKE_C_FLAGS_INIT " -O2")`,
		`string(APPEND CMAKE_CXX_FLAGS_INIT " -O2")`,
		`string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " -Wl,--as-needed")`,
	}

	for _, item := range unexpected {
		if strings.Contains(output, item) {
			t.Fatalf("generated toolchain file should not contain %q\noutput:\n%s", item, output)
		}
	}
}
