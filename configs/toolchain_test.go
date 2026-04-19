package configs

import (
	"reflect"
	"runtime"
	"strings"
	"testing"
)

func TestToolchainEffectiveFlags(t *testing.T) {
	toolchain := Toolchain{
		CFlags:         []string{"-O2"},
		CXXFlags:       []string{"-O2"},
		LinkFlags:      []string{"-Wl,--as-needed"},
		CFlagsDebug:    []string{"-O0", "-g"},
		CXXFlagsDebug:  []string{"-O0", "-g"},
		LinkFlagsDebug: []string{"-Wl,--export-dynamic"},
	}

	cflags, cxxflags, linkflags := toolchain.effectiveFlags("debug")
	if !reflect.DeepEqual(cflags, toolchain.CFlagsDebug) {
		t.Fatalf("debug cflags = %v, want %v", cflags, toolchain.CFlagsDebug)
	}
	if !reflect.DeepEqual(cxxflags, toolchain.CXXFlagsDebug) {
		t.Fatalf("debug cxxflags = %v, want %v", cxxflags, toolchain.CXXFlagsDebug)
	}
	if !reflect.DeepEqual(linkflags, toolchain.LinkFlagsDebug) {
		t.Fatalf("debug linkflags = %v, want %v", linkflags, toolchain.LinkFlagsDebug)
	}

	cflags, cxxflags, linkflags = toolchain.effectiveFlags("release")
	if !reflect.DeepEqual(cflags, toolchain.CFlags) {
		t.Fatalf("release cflags = %v, want %v", cflags, toolchain.CFlags)
	}
	if !reflect.DeepEqual(cxxflags, toolchain.CXXFlags) {
		t.Fatalf("release cxxflags = %v, want %v", cxxflags, toolchain.CXXFlags)
	}
	if !reflect.DeepEqual(linkflags, toolchain.LinkFlags) {
		t.Fatalf("release linkflags = %v, want %v", linkflags, toolchain.LinkFlags)
	}
}

func TestToolchainGenerate_UsesDebugFlags(t *testing.T) {
	var buffer strings.Builder

	toolchain := Toolchain{
		Name:            "gcc",
		SystemName:      "linux",
		SystemProcessor: "x86_64",
		Path:            "/usr/bin",
		CC:              "gcc",
		CXX:             "g++",
		CFlags:          []string{"-O2"},
		CXXFlags:        []string{"-O2"},
		LinkFlags:       []string{"-Wl,--as-needed"},
		CFlagsDebug:     []string{"-O0", "-g3"},
		CXXFlagsDebug:   []string{"-O0", "-g3"},
		LinkFlagsDebug:  []string{"-Wl,--export-dynamic"},
		ctx:             fakeContext{build: "debug"},
	}

	if err := toolchain.generate(&buffer); err != nil {
		t.Fatalf("generate() error = %v", err)
	}

	output := buffer.String()
	expected := []string{
		`string(APPEND CMAKE_C_FLAGS_INIT " -O0")`,
		`string(APPEND CMAKE_C_FLAGS_INIT " -g3")`,
		`string(APPEND CMAKE_CXX_FLAGS_INIT " -O0")`,
		`string(APPEND CMAKE_CXX_FLAGS_INIT " -g3")`,
		`string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " -Wl,--export-dynamic")`,
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

func TestWritePkgConfig_PrefersTmpDeps(t *testing.T) {
	var toolchain strings.Builder

	celer := NewCeler()
	celer.Global.Platform = "x86_64-linux"
	celer.Global.Project = "project_test"
	celer.Global.BuildType = "release"

	celer.writePkgConfig(&toolchain)
	output := toolchain.String()

	expected := []string{
		"set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH FALSE)",
		"if(DEFINED TMP_DEP_DIR)",
		"  set(PKG_CONFIG_PATH",
		`    "${TMP_DEP_DIR}/lib/pkgconfig"`,
		`    "${TMP_DEP_DIR}/share/pkgconfig"`,
		`"${TMP_DEP_DIR}/lib/pkgconfig"`,
		`"${TMP_DEP_DIR}/share/pkgconfig"`,
	}

	if runtime.GOOS == "linux" {
		expected = append(expected, `set(ENV{PKG_CONFIG_SYSROOT_DIR} "${WORKSPACE_ROOT}")`)
	}

	for _, item := range expected {
		if !strings.Contains(output, item) {
			t.Fatalf("generated pkg-config section missing %q\noutput:\n%s", item, output)
		}
	}

	if strings.Contains(output, "set(PKG_CONFIG_EXECUTABLE         ") {
		t.Fatalf("generated pkg-config section should not use padded alignment\noutput:\n%s", output)
	}
}
