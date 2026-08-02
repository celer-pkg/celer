package generator

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func tempDir(t *testing.T) string {
	t.Helper()
	dir, err := os.MkdirTemp("", "celer-gen-test-*")
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { os.RemoveAll(dir) })
	return dir
}

func writeFile(t *testing.T, path, content string) {
	t.Helper()
	if err := os.MkdirAll(filepath.Dir(path), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(path, []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}
}

// ── AutoDetectConfig ────────────────────────────────────────────────

func TestAutoDetectConfig(t *testing.T) {
	dir := tempDir(t)

	// Create lib/ with some library files.
	libDir := filepath.Join(dir, "lib")
	writeFile(t, filepath.Join(libDir, "libfoo.a"), "")
	writeFile(t, filepath.Join(libDir, "libfoo.so"), "")
	writeFile(t, filepath.Join(libDir, "libfoo.so.1"), "")
	writeFile(t, filepath.Join(libDir, "libfoo.so.1.0.0"), "")

	cfg := AutoDetectConfig(dir)
	if cfg == nil {
		t.Fatal("expected non-nil config")
	}

	if len(cfg.Filenames) != 4 {
		t.Fatalf("expected 4 filenames, got %d: %v", len(cfg.Filenames), cfg.Filenames)
	}

	// Check that only lib/ files are picked up (not bin/).
	for _, f := range cfg.Filenames {
		if strings.HasSuffix(f, ".dll") {
			t.Fatalf("should not include .dll files: %s", f)
		}
	}
}

func TestAutoDetectConfig_NoLibs(t *testing.T) {
	dir := tempDir(t)
	cfg := AutoDetectConfig(dir)
	if cfg != nil {
		t.Fatal("expected nil config when no lib directory")
	}
}

func TestAutoDetectConfig_EmptyLib(t *testing.T) {
	dir := tempDir(t)
	writeFile(t, filepath.Join(dir, "lib", "readme.txt"), "")
	cfg := AutoDetectConfig(dir)
	if cfg != nil {
		t.Fatal("expected nil config when lib dir has no library files")
	}
}

func TestAutoDetectConfig_OnlyBinIgnored(t *testing.T) {
	dir := tempDir(t)
	writeFile(t, filepath.Join(dir, "bin", "foo.dll"), "")
	writeFile(t, filepath.Join(dir, "bin", "foo.exe"), "")
	cfg := AutoDetectConfig(dir)
	if cfg != nil {
		t.Fatal("expected nil config when only bin/ exists")
	}
}

// ── GenerateCMakeLists: single target, auto-detect ──────────────────

func TestGenerateCMakeLists_Single_AutoDetect(t *testing.T) {
	dir := tempDir(t)

	// Simulate an installed package: lib/ with some library files.
	writeFile(t, filepath.Join(dir, "lib", "libfoo.a"), "")
	writeFile(t, filepath.Join(dir, "lib", "libfoo.so.1.0"), "")

	cfg := AutoDetectConfig(dir)
	if cfg == nil {
		t.Fatal("expected non-nil config from auto-detect")
	}

	if err := cfg.GenerateCMakeLists(dir, "foo", "1.0.0"); err != nil {
		t.Fatal(err)
	}

	// Verify generated files exist.
	assertFileExists(t, filepath.Join(dir, "CMakeLists.txt"))
	assertFileExists(t, filepath.Join(dir, "cmake", "Config.cmake.in"))

	// CMakeLists.txt should contain the auto-detected filenames.
	content := readFile(t, filepath.Join(dir, "CMakeLists.txt"))
	if !strings.Contains(content, "libfoo.a") || !strings.Contains(content, "libfoo.so.1.0") {
		t.Fatalf("CMakeLists.txt should contain auto-detected filenames, got:\n%s", content)
	}
	if !strings.Contains(content, "foo::${PROJECT_NAME}") {
		t.Fatalf("CMakeLists.txt should contain namespace alias, got:\n%s", content)
	}
}

// ── GenerateCMakeLists: single target, explicit filenames ──────────

func TestGenerateCMakeLists_Single_ExplicitFilenames(t *testing.T) {
	dir := tempDir(t)

	cfg := &TargetConfig{Filenames: []string{"libbar.a", "libbar.so"}}
	if err := cfg.GenerateCMakeLists(dir, "bar", "2.1.0"); err != nil {
		t.Fatal(err)
	}

	content := readFile(t, filepath.Join(dir, "CMakeLists.txt"))
	if !strings.Contains(content, "libbar.a") || !strings.Contains(content, "libbar.so") {
		t.Fatalf("CMakeLists.txt should contain explicit filenames, got:\n%s", content)
	}
	if !strings.Contains(content, "bar::${PROJECT_NAME}") {
		t.Fatalf("CMakeLists.txt should contain namespace alias, got:\n%s", content)
	}
}

func TestGenerateCMakeLists_Single_Filename(t *testing.T) {
	dir := tempDir(t)

	cfg := &TargetConfig{Filename: "libsingle.a"} // legacy single field
	if err := cfg.GenerateCMakeLists(dir, "single", "0.5"); err != nil {
		t.Fatal(err)
	}

	content := readFile(t, filepath.Join(dir, "CMakeLists.txt"))
	if !strings.Contains(content, "libsingle.a") {
		t.Fatalf("CMakeLists.txt should contain filename, got:\n%s", content)
	}
}

// ── GenerateCMakeLists: components ──────────────────────────────────

func TestGenerateCMakeLists_Components(t *testing.T) {
	dir := tempDir(t)

	cfg := &TargetConfig{
		Components: []component{
			{Component: "core", Filename: "libcore.a", Dependencies: nil},
			{Component: "extra", Filename: "libextra.so", Dependencies: []string{"core"}},
		},
	}

	if err := cfg.GenerateCMakeLists(dir, "mylib", "3.0.0"); err != nil {
		t.Fatal(err)
	}

	content := readFile(t, filepath.Join(dir, "CMakeLists.txt"))
	if !strings.Contains(content, `${PROJECT_NAME}::core`) {
		t.Fatalf("should contain ${PROJECT_NAME}::core alias, got:\n%s", content)
	}
	if !strings.Contains(content, "libcore.a") {
		t.Fatalf("should reference libcore.a, got:\n%s", content)
	}
}

// ── ReadCMakeConfig ─────────────────────────────────────────────────

func TestReadCMakeConfig_Linux(t *testing.T) {
	dir := tempDir(t)

	writeFile(t, filepath.Join(dir, "cmake_config.toml"), `
namespace = "testns"

[linux]
filename = "libtest.a"

[windows]
filename = "test.lib"
`)

	cfg, err := ReadCMakeConfig(filepath.Join(dir, "cmake_config.toml"), "linux")
	if err != nil {
		t.Fatal(err)
	}

	if cfg.Filename != "libtest.a" {
		t.Fatalf("expected libtest.a, got %s", cfg.Filename)
	}
	if cfg.libInfo.GetNamespace() != "testns" {
		t.Fatalf("expected namespace testns, got %s", cfg.libInfo.GetNamespace())
	}
}

func TestReadCMakeConfig_Windows(t *testing.T) {
	dir := tempDir(t)

	writeFile(t, filepath.Join(dir, "cmake_config.toml"), `
[linux]
filename = "libtest.a"

[windows]
filename = "test.lib"
`)

	cfg, err := ReadCMakeConfig(filepath.Join(dir, "cmake_config.toml"), "windows")
	if err != nil {
		t.Fatal(err)
	}

	if cfg.Filename != "test.lib" {
		t.Fatalf("expected test.lib, got %s", cfg.Filename)
	}
}

// ── Helpers ─────────────────────────────────────────────────────────

func assertFileExists(t *testing.T, path string) {
	t.Helper()
	if info, err := os.Stat(path); err != nil {
		t.Fatalf("file %s should exist: %v", path, err)
	} else if info.IsDir() {
		t.Fatalf("%s is a directory, expected a file", path)
	}
}

func readFile(t *testing.T, path string) string {
	t.Helper()
	data, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	return string(data)
}
