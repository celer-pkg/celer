package configs

import (
	"os"
	"path/filepath"
	"testing"

	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
)

func TestCeler_Init_NewConfig(t *testing.T) {
	tmpDir := t.TempDir()
	dirs.Init(tmpDir)
	celerPath := filepath.Join(tmpDir, "celer.toml")

	if fileio.PathExists(celerPath) {
		t.Fatalf("celer.toml should not exist initially")
	}

	// Test init.
	celer := NewCeler()
	if err := celer.Init(); err != nil {
		t.Errorf("Init() error = %v", err)
	}

	// Test cases.
	if !fileio.PathExists(celerPath) {
		t.Error("celer.toml should be created")
	}
	if buildType := celer.BuildType(); buildType != "Release" {
		t.Errorf("BuildType() = %v, want Release", buildType)
	}
	if jobs := celer.Jobs(); jobs <= 0 {
		t.Errorf("Jobs() = %v, want positive number", jobs)
	}
}

func TestCeler_Init_ExistingConfig(t *testing.T) {
	// Prepare test environment.
	tmpDir := t.TempDir()
	dirs.Init(tmpDir)

	cacheDir := filepath.Join(tmpDir, "cache_dir")
	if err := os.MkdirAll(cacheDir, os.ModePerm); err != nil {
		t.Fatalf("MkdirAll() error = %v", err)
	}

	// Create a test config.
	existingConfig := `[global]
	build_type = "debug"
	jobs = 4
	platform = ""
	project = ""`

	celerPath := filepath.Join(tmpDir, "celer.toml")
	if err := os.MkdirAll(tmpDir, os.ModePerm); err != nil {
		t.Fatalf("MkdirAll() error = %v", err)
	}
	if err := os.WriteFile(celerPath, []byte(existingConfig), os.ModePerm); err != nil {
		t.Fatalf("WriteFile() error = %v", err)
	}

	celer := NewCeler()
	if err := celer.Init(); err != nil {
		t.Fatalf("Init() error = %v", err)
	}

	if buildType := celer.BuildType(); buildType != "debug" {
		t.Errorf("BuildType() = %v, want debug", buildType)
	}
	if jobs := celer.Jobs(); jobs != 4 {
		t.Errorf("Jobs() = %v, want 4", jobs)
	}
}

func TestCeler_Init_InvalidTOML(t *testing.T) {
	// Prepare test environment.
	tmpDir := t.TempDir()
	dirs.Init(tmpDir)
	celerPath := filepath.Join(tmpDir, "celer.toml")

	// Create invalid TOML content.
	invalidConfig := `invalid toml content`
	if err := os.MkdirAll(tmpDir, os.ModePerm); err != nil {
		t.Fatalf("MkdirAll() error = %v", err)
	}
	if err := os.WriteFile(celerPath, []byte(invalidConfig), os.ModePerm); err != nil {
		t.Fatalf("WriteFile() error = %v", err)
	}

	// Test.
	celer := NewCeler()
	if err := celer.Init(); err == nil {
		t.Error("Init() should return error for invalid TOML")
	}
}

func TestCeler_Init_InvalidCacheDir(t *testing.T) {
	// Prepare test environment.
	tmpDir := t.TempDir()
	dirs.Init(tmpDir)
	celerPath := filepath.Join(tmpDir, "celer.toml")

	// Create config with invalid cache dir.
	configWithInvalidCache := `[cache_dir]
dir = ""
`
	if err := os.MkdirAll(tmpDir, os.ModePerm); err != nil {
		t.Fatalf("MkdirAll() error = %v", err)
	}
	if err := os.WriteFile(celerPath, []byte(configWithInvalidCache), os.ModePerm); err != nil {
		t.Fatalf("WriteFile() error = %v", err)
	}

	// Test cases.
	celer := NewCeler()
	if err := celer.Init(); err == nil {
		t.Error("Init() should return error for invalid cache dir")
	}
}
