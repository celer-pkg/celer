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

func TestInstall_CacheDir_Success(t *testing.T) {
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
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	port.StoreCache = true
	port.CacheToken = "token_123456"
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

	// Totally remove port
	check(port.Remove(true, true, true))

	// Install from package should fail.
	installed, err := port.installFromPackage()
	check(err)
	if installed {
		t.Fatal("should install failed from package")
	}

	// Install from cache should success.
	installed, err = port.installFromCache()
	check(err)
	if !installed {
		t.Fatal("should install successfully from cache")
	}

	// Clean up.
	check(port.Remove(true, true, true))
}

func TestInstall_CacheDir_WithDependencies_Success(t *testing.T) {
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
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var glogPort Port
	glogPort.StoreCache = true
	glogPort.CacheToken = "token_123456"
	check(glogPort.Init(celer, "glog@0.6.0", celer.BuildType()))
	check(glogPort.installFromSource())

	var glogPackageDir, gflagsPackageDir string
	if runtime.GOOS == "windows" {
		glogPackageDir = filepath.Join(dirs.PackagesDir, "glog@0.6.0@x86_64-windows-msvc-14.44@test_project_01@release")
		gflagsPackageDir = filepath.Join(dirs.PackagesDir, "gflags@2.2.2@x86_64-windows-msvc-14.44@test_project_01@release")
	} else {
		glogPackageDir = filepath.Join(dirs.PackagesDir, "glog@0.6.0@x86_64-linux-ubuntu-22.04@test_project_01@release")
		gflagsPackageDir = filepath.Join(dirs.PackagesDir, "gflags@2.2.2@x86_64-linux-ubuntu-22.04@test_project_01@release")
	}
	if !fileio.PathExists(glogPackageDir) || !fileio.PathExists(gflagsPackageDir) {
		t.Fatal("gflags or glog package cannot found")
	}

	// Totally remove port
	check(glogPort.Remove(true, true, true))

	// Install from package should fail.
	installed, err := glogPort.installFromPackage()
	check(err)
	if installed {
		t.Fatal("should install failed from package")
	}

	// Install from cache should success.
	installed, err = glogPort.installFromCache()
	check(err)
	if !installed {
		t.Fatal("should install successfully from cache")
	}

	var gflagsPort Port
	check(gflagsPort.Init(celer, "gflags@2.2.2", celer.BuildType()))
	installed, err = gflagsPort.Installed()
	check(err)
	if !installed {
		t.Fatal("gflags not installed")
	}

	// Clean up.
	check(glogPort.Remove(true, true, true))
	check(gflagsPort.Remove(true, true, true))
}

func TestInstall_CacheDir_DirNotDefined(t *testing.T) {
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
	check(celer.SetCacheDir("", "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	port.StoreCache = true
	port.CacheToken = "token_123456"
	check(port.Init(celer, "sqlite3@3.49.0", celer.BuildType()))
	if err := port.installFromSource(); err != ErrCacheDirNotConfigured {
		t.Fatal("should return ErrCacheDirNotConfigured")
	}
}

func TestInstall_CacheDir_TokenNotDefined(t *testing.T) {
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
	check(celer.SetCacheDir(dirs.TestCacheDir, ""))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	port.StoreCache = true
	port.CacheToken = "token_123456"
	check(port.Init(celer, "sqlite3@3.49.0", celer.BuildType()))
	if err := port.installFromSource(); err != ErrCacheTokenNotConfigured {
		t.Fatal("should return ErrCacheTokenNotConfigured")
	}
}

func TestInstall_CacheDir_TokenNotSpecified(t *testing.T) {
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
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	port.StoreCache = true
	port.CacheToken = "" // Token not specified
	check(port.Init(celer, "sqlite3@3.49.0", celer.BuildType()))
	if err := port.installFromSource(); err != ErrCacheTokenNotSpecified {
		t.Fatal("should return ErrCacheTokenNotSpecified")
	}
}

func TestInstall_CacheDir_TokenNotMatch(t *testing.T) {
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
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build environment.
	check(celer.Platform().Setup())

	var port Port
	port.StoreCache = true
	port.CacheToken = "token_654321" // Token not match.
	check(port.Init(celer, "sqlite3@3.49.0", celer.BuildType()))
	if err := port.installFromSource(); err != ErrCacheTokenNotMatch {
		t.Fatal("should return ErrCacheTokenNotMatch")
	}
}
