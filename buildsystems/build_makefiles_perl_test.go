package buildsystems

import (
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"testing"
)

func TestShouldConfigureWithPerl_FromConfigureScript(t *testing.T) {
	src := t.TempDir()
	if err := os.WriteFile(filepath.Join(src, "Configure"), []byte("perl"), 0o644); err != nil {
		t.Fatal(err)
	}

	m := makefiles{BuildConfig: &BuildConfig{
		PortConfig: PortConfig{SrcDir: src},
	}}
	if !m.shouldConfigureWithPerl() {
		t.Fatal("capital Configure script should select perl")
	}
}

func TestCheckTools_WindowsPerlUsesStrawberryPerl(t *testing.T) {
	if runtime.GOOS != "windows" {
		t.Skip("strawberry-perl is a Windows-only makefile tool")
	}

	src := t.TempDir()
	if err := os.WriteFile(filepath.Join(src, "Configure"), []byte("perl"), 0o644); err != nil {
		t.Fatal(err)
	}

	m := NewMakefiles(&BuildConfig{
		PortConfig: PortConfig{SrcDir: src},
	})
	tools := m.CheckTools()
	if !slices.Contains(tools, "strawberry-perl") {
		t.Fatalf("expected strawberry-perl, got %v", tools)
	}
	if slices.Contains(tools, "msys2") {
		t.Fatalf("openssl perl Configure must not use msys2, got %v", tools)
	}
}
