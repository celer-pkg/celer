package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"path/filepath"
	"runtime"
	"testing"
)

func TestInstall_From_Package_Success(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "sqlite3@3.49.0", celer.BuildType()))
	check(port.installFromSource())

	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "sqlite3@3.49.0@x86_64-windows-msvc-14.44@test_project_01@release")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "sqlite3@3.49.0@x86_64-linux-ubuntu-22.04@test_project_01@release")
	}
	if !fileio.PathExists(packageDir) {
		t.Fatal("package cannot found")
	}
	check(port.Remove(true, false, true))

	installed, err := port.installFromPackage()
	check(err)
	if !installed {
		t.Fatal("cannot install from package")
	}

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_From_Package_Failed(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "sqlite3@3.49.0", celer.BuildType()))
	check(port.installFromSource())

	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "sqlite3@3.49.0@x86_64-windows-msvc-14.44@test_project_01@release")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "sqlite3@3.49.0@x86_64-linux-ubuntu-22.04@test_project_01@release")
	}
	if !fileio.PathExists(packageDir) {
		t.Fatal("package cannot found")
	}
	check(port.Remove(true, true, true))

	installed, err := port.installFromPackage()
	check(err)
	if installed {
		t.Fatal("it should be failed to install from package.")
	}
}
