package cmake

import (
	"os"
	"path/filepath"
	"testing"
)

func TestCheckAbsPaths_NoViolations(t *testing.T) {
	dir := t.TempDir()
	cmakeDir := filepath.Join(dir, "lib", "cmake", "mylib")
	if err := os.MkdirAll(cmakeDir, 0755); err != nil {
		t.Fatal(err)
	}

	// A clean cmake config with only relative paths derived from _IMPORT_PREFIX.
	content := `
set(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_DIR}/../../..")
set_target_properties(mylib PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libmylib.so"
  INTERFACE_LINK_LIBRARIES "LZ4::lz4_shared"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "mylibTargets.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckAbsPaths(dir, "/workspace"); err != nil {
		t.Fatalf("CheckAbsPaths() should pass for relative paths, got: %v", err)
	}
}

func TestCheckAbsPaths_WithViolation(t *testing.T) {
	dir := t.TempDir()
	cmakeDir := filepath.Join(dir, "lib", "cmake", "mylib")
	if err := os.MkdirAll(cmakeDir, 0755); err != nil {
		t.Fatal(err)
	}

	// Bakes an absolute workspace path into a target property AND the file
	// does not derive paths from its own location → not relocatable.
	content := `
set_target_properties(mylib PROPERTIES
  INTERFACE_LINK_LIBRARIES "/workspace/tmp/deps/lib/liblz4.so"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "mylibTargets.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckAbsPaths(dir, "/workspace"); err == nil {
		t.Fatal("CheckAbsPaths() should fail for absolute paths")
	}
}

// Regression: flann-style targets.cmake mixes _IMPORT_PREFIX boilerplate with
// a real INTERFACE_LINK_LIBRARIES violation. The earlier "skip whole file if
// it mentions _IMPORT_PREFIX" rule missed this. Now we judge line-by-line.
func TestCheckAbsPaths_MixedImportPrefixAndViolation(t *testing.T) {
	dir := t.TempDir()
	cmakeDir := filepath.Join(dir, "lib", "cmake", "mylib")
	if err := os.MkdirAll(cmakeDir, 0755); err != nil {
		t.Fatal(err)
	}

	content := `
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)

set_target_properties(mylib PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include"
  INTERFACE_LINK_LIBRARIES "/workspace/tmp/deps/aarch64/lib/liblz4.so"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "mylibTargets.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckAbsPaths(dir, "/workspace"); err == nil {
		t.Fatal("CheckAbsPaths() should catch INTERFACE_LINK_LIBRARIES violation alongside _IMPORT_PREFIX boilerplate")
	}
}

func TestCheckAbsPaths_CatchesImportedLocation(t *testing.T) {
	dir := t.TempDir()
	cmakeDir := filepath.Join(dir, "lib", "cmake", "mylib")
	if err := os.MkdirAll(cmakeDir, 0755); err != nil {
		t.Fatal(err)
	}

	content := `
set_target_properties(mylib PROPERTIES
  IMPORTED_LOCATION_RELEASE "/workspace/packages/mylib/lib/libmylib.so"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "mylibTargets.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckAbsPaths(dir, "/workspace"); err == nil {
		t.Fatal("CheckAbsPaths() should catch IMPORTED_LOCATION_* with absolute path")
	}
}

func TestCheckAbsPaths_SkipsWhenCmakeCurrentListDirPresent(t *testing.T) {
	dir := t.TempDir()
	cmakeDir := filepath.Join(dir, "lib", "cmake", "mylib")
	if err := os.MkdirAll(cmakeDir, 0755); err != nil {
		t.Fatal(err)
	}

	// Boost-style: the config derives the real path from CMAKE_CURRENT_LIST_DIR
	// and only uses the absolute path as a symlink-equivalence optimization
	// inside an if(EXISTS). Fully relocatable; whole file should be skipped.
	content := `
get_filename_component(_BOOST_CMAKEDIR "${CMAKE_CURRENT_LIST_DIR}/../" REALPATH)

if(EXISTS "/workspace/packages/mylib/lib/cmake")
  set(_BOOST_CMAKEDIR "/workspace/packages/mylib/lib/cmake")
endif()

set_target_properties(mylib PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_BOOST_CMAKEDIR}/../libmylib.so"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "mylibConfig.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckAbsPaths(dir, "/workspace"); err != nil {
		t.Fatalf("CheckAbsPaths() should skip files that use CMAKE_CURRENT_LIST_DIR, got: %v", err)
	}
}

func TestCheckAbsPaths_SkipsComments(t *testing.T) {
	dir := t.TempDir()
	cmakeDir := filepath.Join(dir, "lib", "cmake", "mylib")
	if err := os.MkdirAll(cmakeDir, 0755); err != nil {
		t.Fatal(err)
	}

	content := `
# This file was generated from /workspace/buildtrees/mylib
# see /workspace/packages for details
set_target_properties(mylib PROPERTIES
  IMPORTED_LOCATION_RELEASE "lib/libmylib.so"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "mylibTargets.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckAbsPaths(dir, "/workspace"); err != nil {
		t.Fatalf("CheckAbsPaths() should skip comment lines, got: %v", err)
	}
}

func TestCheckAbsPaths_NoCmakeDir(t *testing.T) {
	dir := t.TempDir()
	// No lib/cmake or share/cmake directories — should pass cleanly.
	if err := CheckAbsPaths(dir, "/workspace"); err != nil {
		t.Fatalf("CheckAbsPaths() should pass when no cmake dirs exist, got: %v", err)
	}
}

func TestCheckAbsPaths_ShareCmake(t *testing.T) {
	dir := t.TempDir()
	cmakeDir := filepath.Join(dir, "share", "cmake", "mylib")
	if err := os.MkdirAll(cmakeDir, 0755); err != nil {
		t.Fatal(err)
	}

	content := `
set_target_properties(mylib PROPERTIES
  INTERFACE_LINK_LIBRARIES "/workspace/tmp/deps/lib/libfoo.so"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "mylibConfig.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckAbsPaths(dir, "/workspace"); err == nil {
		t.Fatal("CheckAbsPaths() should find violation in share/cmake/")
	}
}
