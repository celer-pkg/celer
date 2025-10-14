package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"errors"
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

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build envs.
	check(celer.Platform().Setup())

	var port Port
	var options = InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
	check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
	check(port.InstallFromSource(options))

	// Check package.
	var packageDir string
	if runtime.GOOS == "windows" {
		packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-windows-msvc-14.44@test_project_01@release")
	} else {
		packageDir = filepath.Join(dirs.PackagesDir, "eigen@3.4.0@x86_64-linux-ubuntu-22.04@test_project_01@release")
	}
	if !fileio.PathExists(packageDir) {
		t.Fatal("package cannot found")
	}

	// Totally remove port and src.
	check(port.Remove(true, true, true))
	check(os.RemoveAll(port.MatchedConfig.PortConfig.RepoDir))

	// Install from package should fail.
	installed, err := port.InstallFromPackage(options)
	check(err)
	if installed {
		t.Fatal("should install failed from package")
	}

	// Install from cache should success.
	installed, err = port.InstallFromCache(options)
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

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build envs.
	check(celer.Platform().Setup())

	var glogPort Port
	var options = InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
	check(glogPort.Init(celer, "glog@0.6.0", celer.BuildType()))
	check(glogPort.InstallFromSource(options))

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

	// Totally remove port and src.
	check(glogPort.Remove(true, true, true))
	check(os.RemoveAll(glogPort.MatchedConfig.PortConfig.RepoDir))

	// Install from package should fail.
	installed, err := glogPort.InstallFromPackage(options)
	check(err)
	if installed {
		t.Fatal("should install failed from package")
	}

	// Install from cache should success.
	installed, err = glogPort.InstallFromCache(options)
	check(err)
	if !installed {
		t.Fatal("should install successfully from cache")
	}

	// gflags should also be installed from cache.
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

func TestInstall_CacheDir_Prebuilt_Success(t *testing.T) {
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

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("test_project_02"))
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build envs.
	check(celer.Platform().Setup())

	var port Port
	var options = InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
	check(port.Init(celer, "prebuilt-x264@stable", celer.BuildType()))
	check(port.InstallFromSource(options))

	// Check package & repo.
	packageDir := filepath.Join(dirs.PackagesDir, "prebuilt-x264@stable@x86_64-linux-ubuntu-22.04@test_project_02@release")
	if !fileio.PathExists(packageDir) {
		t.Fatal("package cannot found")
	}
	if fileio.PathExists(port.MatchedConfig.PortConfig.RepoDir) {
		t.Fatal("repo should not exist")
	}

	// Totally remove port.
	check(port.Remove(true, true, true))

	// Install from package should fail.
	installed, err := port.InstallFromPackage(options)
	check(err)
	if installed {
		t.Fatal("should install failed from package")
	}

	// Install from cache should success.
	installed, err = port.InstallFromCache(options)
	check(err)
	if !installed {
		t.Fatal("should install successfully from cache")
	}

	// Clean up.
	check(port.Remove(true, true, true))
}

func TestInstall_CacheDir_DirNotDefined_Failed(t *testing.T) {
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

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetCacheDir("", "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build envs.
	check(celer.Platform().Setup())

	var port Port
	var options = InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
	check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
	if err := port.InstallFromSource(options); err != ErrCacheDirNotConfigured {
		t.Fatal("should return ErrCacheDirNotConfigured")
	}
}

func TestInstall_CacheDir_TokenNotDefined_Failed(t *testing.T) {
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

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetCacheDir(dirs.TestCacheDir, ""))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build envs.
	check(celer.Platform().Setup())

	var port Port
	var options = InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
	check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
	if err := port.InstallFromSource(options); err != ErrCacheTokenNotConfigured {
		t.Fatal("should return ErrCacheTokenNotConfigured")
	}
}

func TestInstall_CacheDir_TokenNotSpecified_Failed(t *testing.T) {
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

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build envs.
	check(celer.Platform().Setup())

	var port Port
	var options = InstallOptions{
		StoreCache: true,
		CacheToken: "", // Token not specified
	}
	check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
	if err := port.InstallFromSource(options); err != ErrCacheTokenNotSpecified {
		t.Fatal("should return ErrCacheTokenNotSpecified")
	}
}

func TestInstall_CacheDir_TokenNotMatch_Failed(t *testing.T) {
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

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build envs.
	check(celer.Platform().Setup())

	var port Port
	var options = InstallOptions{
		StoreCache: true,
		CacheToken: "token_654321", // Token not match.
	}
	check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
	if err := port.InstallFromSource(options); err != ErrCacheTokenNotMatch {
		t.Fatal("should return ErrCacheTokenNotMatch")
	}
}

func TestInstall_CacheDir_With_Commit_Success(t *testing.T) {
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

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build envs.
	check(celer.Platform().Setup())

	var port Port
	var options = InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
	check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
	check(port.InstallFromSource(options))

	// Read commit.
	commit, err := git.ReadLocalCommit(port.MatchedConfig.PortConfig.RepoDir)
	check(err)

	// Remove installed and src dir.
	check(port.Remove(true, true, true))
	check(os.RemoveAll(port.MatchedConfig.PortConfig.RepoDir))

	// Install from cache with commit.
	port.Package.Commit = commit
	installed, err := port.InstallFromCache(options)
	check(err)
	if !installed {
		t.Fatal("should be installed from cache")
	}
}

func TestInstall_CacheDir_With_Commit_Failed(t *testing.T) {
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

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := NewCeler()
	check(celer.Init())

	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("test_project_01"))
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))

	// Setup build envs.
	check(celer.Platform().Setup())

	var port Port
	var options = InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
	check(port.Init(celer, "eigen@3.4.0", celer.BuildType()))
	check(port.InstallFromSource(options))

	// Remove installed and src dir.
	check(port.Remove(true, true, true))
	check(os.RemoveAll(port.MatchedConfig.PortConfig.RepoDir))

	// Install from cache with not matched commit.
	port.Package.Commit = "not_matched_commit_xxxxxx"
	installed, err := port.InstallFromCache(options)
	if err == nil || !errors.Is(err, ErrCacheNotFoundWithCommit) {
		t.Fatal("should return ErrCacheNotFoundWithCommit")
	}
	if installed {
		t.Fatal("should not be installed from cache")
	}
}
