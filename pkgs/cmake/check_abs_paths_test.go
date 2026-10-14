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

	if err := CheckCMakeAbsPaths(dir, "/workspace"); err != nil {
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
  INTERFACE_LINK_LIBRARIES "/workspace/tmp/staging/lib/liblz4.so"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "mylibTargets.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckCMakeAbsPaths(dir, "/workspace"); err == nil {
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
  INTERFACE_LINK_LIBRARIES "/workspace/tmp/staging/aarch64/lib/liblz4.so"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "mylibTargets.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckCMakeAbsPaths(dir, "/workspace"); err == nil {
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

	if err := CheckCMakeAbsPaths(dir, "/workspace"); err == nil {
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

	if err := CheckCMakeAbsPaths(dir, "/workspace"); err != nil {
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

	if err := CheckCMakeAbsPaths(dir, "/workspace"); err != nil {
		t.Fatalf("CheckAbsPaths() should skip comment lines, got: %v", err)
	}
}

func TestCheckAbsPaths_NoCmakeDir(t *testing.T) {
	dir := t.TempDir()
	// No lib/cmake or share/cmake directories — should pass cleanly.
	if err := CheckCMakeAbsPaths(dir, "/workspace"); err != nil {
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
  INTERFACE_LINK_LIBRARIES "/workspace/tmp/staging/lib/libfoo.so"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "mylibConfig.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckCMakeAbsPaths(dir, "/workspace"); err == nil {
		t.Fatal("CheckAbsPaths() should find violation in share/cmake/")
	}
}

func TestCheckAbsPaths_CatchesSysrootSystemLib(t *testing.T) {
	// OGRE-style: a vendored target under opt/ bakes sysroot system library
	// paths (X11/GL) into INTERFACE_LINK_LIBRARIES. The sysroot lives under
	// workspace/downloads/tools, so these are still workspace absolute paths
	// and must be flagged.
	dir := t.TempDir()
	cmakeDir := filepath.Join(dir, "opt", "rviz_ogre_vendor", "lib", "cmake", "OGRE")
	if err := os.MkdirAll(cmakeDir, 0755); err != nil {
		t.Fatal(err)
	}

	content := `
set_target_properties(OgreMain PROPERTIES
  INTERFACE_LINK_LIBRARIES "/workspace/downloads/tools/sysroot/usr/lib/aarch64-linux-gnu/libX11.so;/workspace/downloads/tools/sysroot/usr/lib/libGL.so"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "OgreTargets.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckCMakeAbsPaths(dir, "/workspace"); err == nil {
		t.Fatal("CheckAbsPaths() should flag sysroot system library absolute paths")
	}
}

func TestCheckAbsPaths_CatchesSourceIncludeDir(t *testing.T) {
	// rviz-style: a source include directory (gtest_vendor) baked into
	// INTERFACE_INCLUDE_DIRECTORIES makes the install non-relocatable.
	dir := t.TempDir()
	cmakeDir := filepath.Join(dir, "share", "rviz_visual_testing_framework", "cmake")
	if err := os.MkdirAll(cmakeDir, 0755); err != nil {
		t.Fatal(err)
	}

	content := `
set_target_properties(rviz_visual_testing_framework PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "/workspace/src/ros2/rviz/rviz_visual_testing_framework/../gtest_vendor/include"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "rviz_visual_testing_frameworkTargets.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckCMakeAbsPaths(dir, "/workspace"); err == nil {
		t.Fatal("CheckAbsPaths() should flag source include dir baked into INTERFACE_INCLUDE_DIRECTORIES")
	}
}

func TestCheckAbsPaths_OptVendorRelocatablePasses(t *testing.T) {
	// A well-formed vendored config under opt/ that only derives paths from
	// _IMPORT_PREFIX must pass, confirming the opt/**/cmake scan does not
	// produce false positives.
	dir := t.TempDir()
	cmakeDir := filepath.Join(dir, "opt", "rviz_ogre_vendor", "lib", "cmake", "OGRE")
	if err := os.MkdirAll(cmakeDir, 0755); err != nil {
		t.Fatal(err)
	}

	content := `
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
set_target_properties(OgreMain PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include/OGRE"
  INTERFACE_LINK_LIBRARIES "OGRE::OgreMain"
)
`
	if err := os.WriteFile(filepath.Join(cmakeDir, "OgreTargets.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckCMakeAbsPaths(dir, "/workspace"); err != nil {
		t.Fatalf("CheckAbsPaths() should pass for a relocatable opt/ config, got: %v", err)
	}
}

func TestCheckAbsPaths_TargetsFileOutsideCmakeDir(t *testing.T) {
	// libccd-style: install(EXPORT) results in import-target files installed
	// directly under lib/<pkg>/ (not lib/cmake/<pkg>/), e.g.
	// lib/ccd/ccd-targets-release.cmake. A sysroot system library baked into a
	// per-config imported link-interface property must still be caught.
	dir := t.TempDir()
	ccdDir := filepath.Join(dir, "lib", "ccd")
	if err := os.MkdirAll(ccdDir, 0755); err != nil {
		t.Fatal(err)
	}

	content := `
set_target_properties(ccd PROPERTIES
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "/workspace/downloads/tools/sysroot/usr/lib/aarch64-linux-gnu/libm.so"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libccd.so.2.0"
)
`
	if err := os.WriteFile(filepath.Join(ccdDir, "ccd-targets-release.cmake"), []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	if err := CheckCMakeAbsPaths(dir, "/workspace"); err == nil {
		t.Fatal("CheckAbsPaths() should catch absolute path in lib/<pkg>/*-targets*.cmake")
	}
}

func TestCheckAbsPaths_NonexistentDir(t *testing.T) {
	// A nobuild port has no installed tree; checking must be a no-op, not an error.
	if err := CheckCMakeAbsPaths(filepath.Join(t.TempDir(), "does-not-exist"), "/workspace"); err != nil {
		t.Fatalf("CheckAbsPaths() should skip a nonexistent dir, got: %v", err)
	}
}
