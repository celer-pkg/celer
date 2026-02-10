package configs

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestCCache_Setup_Enabled(t *testing.T) {
	disable, _ := os.LookupEnv("CCACHE_DISABLE")
	defer func() {
		if disable != "" {
			os.Setenv("CCACHE_DISABLE", disable)
		} else {
			os.Unsetenv("CCACHE_DISABLE")
		}
	}()

	dir := t.TempDir()
	ccache := &CCache{Enabled: true, Dir: dir}

	if err := ccache.Setup(); err != nil {
		t.Fatalf("Setup() error = %v", err)
	}

	if v := os.Getenv("CCACHE_DISABLE"); v == "1" {
		t.Errorf("CCACHE_DISABLE = %q, want 0", v)
	}
}

func TestCCache_Setup_Disabled(t *testing.T) {
	disable, _ := os.LookupEnv("CCACHE_DISABLE")
	defer func() {
		if disable != "" {
			os.Setenv("CCACHE_DISABLE", disable)
		} else {
			os.Unsetenv("CCACHE_DISABLE")
		}
	}()

	dir := t.TempDir()
	ccache := &CCache{Enabled: false, Dir: dir}

	if err := ccache.Setup(); err != nil {
		t.Fatalf("Setup() error = %v", err)
	}

	if v := os.Getenv("CCACHE_DISABLE"); v != "1" {
		t.Errorf("CCACHE_DISABLE = %q, want 1", v)
	}
}

func TestCCache_Setup_RemoteStorage(t *testing.T) {
	keys := []string{"CCACHE_REMOTE_STORAGE", "CCACHE_REMOTE_ONLY"}
	saved := make(map[string]string)
	for _, key := range keys {
		v, _ := os.LookupEnv(key)
		saved[key] = v
	}
	defer func() {
		for k, v := range saved {
			if v != "" {
				os.Setenv(k, v)
			} else {
				os.Unsetenv(k)
			}
		}
	}()

	dir := t.TempDir()
	ccahce := &CCache{
		Enabled:       true,
		Dir:           dir,
		MaxSize:       "1G",
		RemoteStorage: "file:///tmp/ccache-remote",
		RemoteOnly:    true,
	}

	if err := ccahce.Setup(); err != nil {
		t.Fatalf("Setup() error = %v", err)
	}

	if v := os.Getenv("CCACHE_REMOTE_STORAGE"); v != ccahce.RemoteStorage {
		t.Errorf("CCACHE_REMOTE_STORAGE = %q, want %q", v, ccahce.RemoteStorage)
	}
	if v := os.Getenv("CCACHE_REMOTE_ONLY"); v != "1" {
		t.Errorf("CCACHE_REMOTE_ONLY = %q, want 1", v)
	}
}

func TestCCache_Setup_MkdirFails(t *testing.T) {
	// Use a path that cannot be created: parent is a file, not a directory.
	dir := t.TempDir()
	badDir := filepath.Join(dir, "file")
	if err := os.WriteFile(badDir, []byte("x"), 0644); err != nil {
		t.Fatalf("setup: %v", err)
	}
	impossibleDir := filepath.Join(badDir, "subdir")

	ccache := &CCache{Enabled: true, Dir: impossibleDir, MaxSize: "1G"}
	if err := ccache.Setup(); err == nil {
		t.Error("Setup() should fail when mkdir fails")
	}
}

func TestCCache_Generate(t *testing.T) {
	var builder strings.Builder
	ccache := CCache{}

	if err := ccache.Generate(&builder); err != nil {
		t.Fatalf("Generate() error = %v", err)
	}

	out := builder.String()
	if !strings.Contains(out, "# CCache.") {
		t.Errorf("output should contain # CCache., got:\n%s", out)
	}
	if !strings.Contains(out, "CMAKE_C_COMPILER_LAUNCHER") {
		t.Errorf("output should set CMAKE_C_COMPILER_LAUNCHER, got:\n%s", out)
	}
	if !strings.Contains(out, "CMAKE_CXX_COMPILER_LAUNCHER") {
		t.Errorf("output should set CMAKE_CXX_COMPILER_LAUNCHER, got:\n%s", out)
	}
	if !strings.Contains(out, "ccache") {
		t.Errorf("output should contain ccache, got:\n%s", out)
	}
}
