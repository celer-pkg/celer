package configs

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"
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
	if buildType := celer.BuildType(); buildType != "release" {
		t.Errorf("BuildType() = %v, want release", buildType)
	}
	if jobs := celer.Jobs(); jobs <= 0 {
		t.Errorf("Jobs() = %v, want positive number", jobs)
	}
}

func TestCeler_Init_ExistingConfig(t *testing.T) {
	// Prepare test environment.
	tmpDir := t.TempDir()
	dirs.Init(tmpDir)

	packageCacheDir := filepath.Join(tmpDir, "pkgcache_dir")
	if err := os.MkdirAll(packageCacheDir, os.ModePerm); err != nil {
		t.Fatalf("MkdirAll() error = %v", err)
	}

	// Create a test config.
	existingConfig := `[main]
	build_type = "debug"
	jobs = 4
	platform = "native"
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

func TestCeler_NativePlatform(t *testing.T) {
	tmpDir := t.TempDir()
	dirs.Init(tmpDir)

	// A ports dir with content skips the clone this test does not want to make.
	if err := os.MkdirAll(filepath.Join(tmpDir, "ports", ".git"), os.ModePerm); err != nil {
		t.Fatalf("MkdirAll() error = %v", err)
	}

	config := `[main]
build_type = "release"
jobs = 2
platform = "native"
project = ""`
	celerPath := filepath.Join(tmpDir, "celer.toml")
	if err := os.WriteFile(celerPath, []byte(config), os.ModePerm); err != nil {
		t.Fatalf("WriteFile() error = %v", err)
	}

	celer := NewCeler()
	if err := celer.Init(); err != nil {
		t.Fatalf("Init() error = %v", err)
	}

	// The reserved name is kept as-is, so the workspace stays portable.
	if !celer.IsNativePlatform() {
		t.Error("UsesNativePlatform() = false, want true")
	}
	if celer.Main.Platform != NativePlatform {
		t.Errorf("Main.Platform = %q, want %q", celer.Main.Platform, NativePlatform)
	}

	// It resolves to the host platform, which also names the install dir.
	hostName := celer.Platform().GetHostName()
	if name := celer.Platform().GetName(); name != hostName {
		t.Errorf("Platform().GetName() = %q, want the host name %q", name, hostName)
	}

	// No project configured: the built-in default one keeps the paths well formed.
	if name := celer.Project().GetName(); name != DefaultProjectName {
		t.Errorf("Project().GetName() = %q, want %q", name, DefaultProjectName)
	}
	want := "/installed/" + hostName + "/" + DefaultProjectName + "/release"
	if dir := filepath.ToSlash(celer.InstalledDir()); !strings.HasSuffix(dir, want) {
		t.Errorf("InstalledDir() = %q, want it to end with %q", dir, want)
	}

	// "native" needs no platform file, so creating one is refused.
	if err := celer.CreatePlatform(NativePlatform); err == nil {
		t.Error("CreatePlatform(native) = nil, want an error")
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
	configWithInvalidCache := `[pkgcache.fs]
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
