package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"path/filepath"
	"runtime"
	"testing"
)

func TestInstall_MakeFiles_Global_BuildType_Release(t *testing.T) {
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
	check(port.Init(celer, "x264@stable", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "x264@stable@x86_64-windows-msvc-14.44@test_project_01@release")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "x264@stable@x86_64-linux-ubuntu-22.04@test_project_01@release")
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

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_MakeFiles_Global_BuildType_Debug(t *testing.T) {
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
	check(celer.SetBuildType("Debug"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "x264@stable", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "x264@stable@x86_64-windows-msvc-14.44@test_project_01@debug")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "x264@stable@x86_64-linux-ubuntu-22.04@test_project_01@debug")
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

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_CMake_Global_BuildType_Release(t *testing.T) {
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
	check(port.Init(celer, "glog@0.6.0", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "x264@stable@x86_64-windows-msvc-14.44@test_project_01@release")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "x264@stable@x86_64-linux-ubuntu-22.04@test_project_01@release")
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

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_CMake_Global_BuildType_Debug(t *testing.T) {
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
	check(celer.SetBuildType("Debug"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "glog@0.6.0", celer.BuildType()))
	check(port.installFromSource())

	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "glog@0.6.0@x86_64-windows-msvc-14.44@test_project_01@debug")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "glog@0.6.0@x86_64-linux-ubuntu-22.04@test_project_01@debug")
	}

	// Check if package dir exists.
	if !fileio.PathExists(packageDir) {
		t.Fatal("package dir cannot found")
	}

	// Check if installed.
	installed, err := port.Installed()
	check(err)
	if !installed {
		t.Fatal("package is not installed")
	}

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_B2_Global_BuildType_Release(t *testing.T) {
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
	check(port.Init(celer, "boost@1.87.0", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "boost@1.87.0@x86_64-windows-msvc-14.44@test_project_01@release")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "boost@1.87.0@x86_64-linux-ubuntu-22.04@test_project_01@release")
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

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_B2_Global_BuildType_Debug(t *testing.T) {
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
	check(celer.SetBuildType("Debug"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "boost@1.87.0", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "boost@1.87.0@x86_64-windows-msvc-14.44@test_project_01@debug")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "boost@1.87.0@x86_64-linux-ubuntu-22.04@test_project_01@debug")
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

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_GYP_Global_BuildType_Release(t *testing.T) {
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
	check(port.Init(celer, "nss@3.55", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "nss@3.55@x86_64-windows-msvc-14.44@test_project_01@release")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "nss@3.55@x86_64-linux-ubuntu-22.04@test_project_01@release")
	}
	if !fileio.PathExists(packageDir) {
		t.Fatal("package cannot found")
	}

	// Check if installed.
	installed, err := port.Installed()
	check(err)
	if !installed {
		t.Fatal("package is not installed")
	}

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_GYP_Global_BuildType_Debub(t *testing.T) {
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
	check(celer.SetBuildType("Debug"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "nss@3.55", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "nss@3.55@x86_64-windows-msvc-14.44@test_project_01@debug")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "nss@3.55@x86_64-linux-ubuntu-22.04@test_project_01@debug")
	}
	if !fileio.PathExists(packageDir) {
		t.Fatal("package dir cannot found")
	}

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_Meson_Global_BuildType_Release(t *testing.T) {
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
	check(port.Init(celer, "pixman@0.44.2", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "pixman@0.44.2@x86_64-windows-msvc-14.44@test_project_01@release")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "pixman@0.44.2@x86_64-linux-ubuntu-22.04@test_project_01@release")
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

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_Meson_Global_BuildType_Debug(t *testing.T) {
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
	check(celer.SetBuildType("Debug"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "pixman@0.44.2", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "pixman@0.44.2@x86_64-windows-msvc-14.44@test_project_01@debug")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "pixman@0.44.2@x86_64-linux-ubuntu-22.04@test_project_01@debug")
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

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_Prebuilt_Global_BuildType_Release(t *testing.T) {
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
	check(celer.SetPlatform("x86_64-linux-ubuntu-22.04"))
	check(celer.SetProject("test_project_02"))
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "prebuilt-x264@stable", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	packageDir := filepath.Join(dirs.PackagesDir, "prebuilt-x264@stable@x86_64-linux-ubuntu-22.04@test_project_02@release")
	if !fileio.PathExists(packageDir) {
		t.Fatal("package cannot found")
	}

	// Check if installed.
	installed, err := port.Installed()
	check(err)
	if !installed {
		t.Fatal("package is not installed")
	}

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_Prebuilt_Global_BuildType_Debug(t *testing.T) {
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
	check(celer.SetBuildType("Debug"))
	check(celer.SetPlatform("x86_64-linux-ubuntu-22.04"))
	check(celer.SetProject("test_project_02"))
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "prebuilt-x264@stable", celer.BuildType()))
	check(port.installFromSource())

	// Check if package dir exists.
	packageDir := filepath.Join(dirs.PackagesDir, "prebuilt-x264@stable@x86_64-linux-ubuntu-22.04@test_project_02@debug")
	if !fileio.PathExists(packageDir) {
		t.Fatal("package dir cannot found")
	}

	// Check if installed.
	installed, err := port.Installed()
	check(err)
	if !installed {
		t.Fatal("package is not installed")
	}

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_Nobuild_Global_BuildType_Release(t *testing.T) {
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
	check(celer.SetProject("test_project_02"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "gnulib@master", celer.BuildType()))
	check(port.installFromSource())

	if !fileio.PathExists(port.MatchedConfig.PortConfig.RepoDir) {
		t.Fatal("src dir cannot found")
	}

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_MakeFiles_BuildType_Debug(t *testing.T) {
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
	check(port.Init(celer, "x264@stable", "Debug"))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "x264@stable@x86_64-windows-msvc-14.44@test_project_01@debug")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "x264@stable@x86_64-linux-ubuntu-22.04@test_project_01@debug")
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

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}

func TestInstall_MakeFiles_BuildType_Release(t *testing.T) {
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
	check(celer.SetBuildType("Debug"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	check(port.Init(celer, "x264@stable", "Debug"))
	check(port.installFromSource())

	// Check if package dir exists.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "x264@stable@x86_64-windows-msvc-14.44@test_project_01@release")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "x264@stable@x86_64-linux-ubuntu-22.04@test_project_01@release")
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

	t.Cleanup(func() {
		port.Remove(true, true, true)
	})
}
