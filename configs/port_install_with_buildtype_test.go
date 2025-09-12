package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"os"
	"path/filepath"
	"runtime"
	"testing"
)

func TestInstall_Global_BuildType_Release_Success(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	t.Cleanup(func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
	})

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
	check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-windows-msvc-14.44@test_project_01@release")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-linux-ubuntu-22.04@test_project_01@release")
	}
	if !fileio.PathExists(packageDir) {
		t.Fatal("package dir cannot found")
	}

	// Check if installed.
	installed, err := port.Installed()
	check(err)

	if !installed {
		t.Fatal("package is not installed")
	}

	// Clean up.
	check(port.Remove(true, true, true))
}

func TestInstall_Global_BuildType_Debug_Success(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	t.Cleanup(func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
	})

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Debug"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-windows-msvc-14.44@test_project_01@debug")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-linux-ubuntu-22.04@test_project_01@debug")
	}
	if !fileio.PathExists(packageDir) {
		t.Fatal("package dir cannot found")
	}

	// Check if installed.
	installed, err := port.Installed()
	check(err)

	if !installed {
		t.Fatal("package is not installed")
	}

	// Clean up.
	check(port.Remove(true, true, true))
}

func TestInstall_Private_BuildType_Debug_Success(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	t.Cleanup(func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
	})

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
	check(port.Init(celer, "eigen@3.4.0", "Debug"))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-windows-msvc-14.44@test_project_01@debug")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-linux-ubuntu-22.04@test_project_01@debug")
	}
	if !fileio.PathExists(packageDir) {
		t.Fatal("package dir cannot found")
	}

	// Check if installed.
	installed, err := port.Installed()
	check(err)
	if !installed {
		t.Fatal("package is not installed")
	}

	// Clean up.
	check(port.Remove(true, true, true))
}

func TestInstall_Private_BuildType_Release_Success(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	t.Cleanup(func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
	})

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Debug"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "eigen@3.4.0", "Release"))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-windows-msvc-14.44@test_project_01@release")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-linux-ubuntu-22.04@test_project_01@release")
	}
	if !fileio.PathExists(packageDir) {
		t.Fatal("package dir cannot found")
	}

	// Check if installed.
	installed, err := port.Installed()
	check(err)
	if !installed {
		t.Fatal("package is not installed")
	}

	// Clean up.
	check(port.Remove(true, true, true))
}
