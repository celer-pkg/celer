package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/errors"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"fmt"
	"os"
	"runtime"
	"testing"
)

func TestInstall_PackageCache_Success(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "eigen@3.4.0"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPackageCacheDir(dirs.TestCacheDir))
	check(celer.SetPackageCacheWritable(true))
	check(celer.SetPlatform(platform))

	var port configs.Port
	var installOptions = configs.InstallOptions{}
	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(installOptions))

	// Check package.
	packageDir := fmt.Sprintf("%s/%s@%s@%s@%s",
		dirs.PackagesDir, nameVersion,
		platform, project,
		celer.BuildType(),
	)
	if !fileio.PathExists(packageDir) {
		t.Fatal("package cannot found")
	}

	// Totally remove port and src.
	var removeOptions = configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))
	check(port.MatchedConfig.Clean())

	// Install from package should fail.
	installed, err := port.InstallFromPackage(installOptions)
	check(err)
	if installed {
		t.Fatal("should install failed from package")
	}

	// Install from cache should success.
	installed, err = port.InstallFromPackageCache(installOptions)
	check(err)
	if !installed {
		t.Fatal("should install successfully from cache")
	}

	// Clean up.
	check(port.Remove(removeOptions))
}

func TestInstall_PackageCache_With_Deps_Success(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "glog@0.6.0"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPackageCacheDir(dirs.TestCacheDir))
	check(celer.SetPackageCacheWritable(true))
	check(celer.SetPlatform(platform))

	var glogPort configs.Port
	var options = configs.InstallOptions{}
	check(glogPort.Init(celer, nameVersion))
	check(glogPort.InstallFromSource(options))

	packageDir := func(nameVersion string) string {
		return fmt.Sprintf("%s/%s@%s@%s@%s", dirs.PackagesDir, nameVersion,
			platform, project, celer.BuildType())
	}
	glogPackageDir := packageDir("glog@0.6.0")
	gflagsPackageDir := packageDir("gflags@2.2.2")

	if !fileio.PathExists(glogPackageDir) || !fileio.PathExists(gflagsPackageDir) {
		t.Fatal("gflags or glog package cannot found")
	}

	// Totally remove port and src.
	var removeOptions = configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(glogPort.Remove(removeOptions))
	check(glogPort.MatchedConfig.Clean())

	// Install from package should fail.
	installed, err := glogPort.InstallFromPackage(options)
	check(err)
	if installed {
		t.Fatal("should install failed from package")
	}

	// Install from cache should success.
	installed, err = glogPort.InstallFromPackageCache(options)
	check(err)
	if !installed {
		t.Fatal("should install successfully from cache")
	}

	// gflags should also be installed from cache.
	var gflagsPort configs.Port
	check(gflagsPort.Init(celer, "gflags@2.2.2"))
	installed, err = gflagsPort.Installed()
	check(err)
	if !installed {
		t.Fatal("gflags not installed")
	}

	// Clean up.
	check(glogPort.Remove(removeOptions))
	check(gflagsPort.Remove(removeOptions))
}

func TestInstall_PackageCache_Prebuilt_Success(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "prebuilt-x264@stable"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPackageCacheDir(dirs.TestCacheDir))
	check(celer.SetPackageCacheWritable(true))
	check(celer.SetPlatform(platform))

	var port configs.Port
	var options = configs.InstallOptions{}
	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(options))

	// Check package & repo.
	packageDir := fmt.Sprintf("%s/%s@%s@%s@%s",
		dirs.PackagesDir, nameVersion,
		platform, project,
		celer.BuildType(),
	)

	if !fileio.PathExists(packageDir) {
		t.Fatal("package cannot found: " + packageDir)
	}
	if !fileio.PathExists(port.MatchedConfig.PortConfig.RepoDir) {
		t.Fatal("repo should be exist: " + port.MatchedConfig.PortConfig.RepoDir)
	}

	// Totally remove port.
	var removeOptions = configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))

	// Install from package should fail.
	installed, err := port.InstallFromPackage(options)
	check(err)
	if installed {
		t.Fatal("should install failed from package")
	}

	// Install from cache should success.
	installed, err = port.InstallFromPackageCache(options)
	check(err)
	if !installed {
		t.Fatal("should install successfully from cache")
	}

	// Clean up.
	check(port.Remove(removeOptions))
}

func TestInstall_PackageCache_DirNotDefined_ShouldSkipStoreCache(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "eigen@3.4.0"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPlatform(platform))

	var port configs.Port
	var options = configs.InstallOptions{}
	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(options))
}

func TestInstall_PackageCache_With_Commit_Success(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "eigen@3.4.0"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPackageCacheDir(dirs.TestCacheDir))
	check(celer.SetPackageCacheWritable(true))
	check(celer.SetPlatform(platform))

	var port configs.Port
	var options = configs.InstallOptions{}
	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(options))

	// Read commit.
	commit, err := git.ReadLocalCommit(port.MatchedConfig.PortConfig.RepoDir)
	check(err)

	// Remove installed and src dir.
	removeOptions := configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))
	check(port.MatchedConfig.Clean())

	// Install from cache with commit.
	port.Package.Commit = commit
	installed, err := port.InstallFromPackageCache(options)
	check(err)
	if !installed {
		t.Fatal("should be installed from cache")
	}

	// Clean up.
	check(port.Remove(removeOptions))
}

func TestInstall_PackageCache_With_Commit_Failed(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "eigen@3.4.0"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPackageCacheDir(dirs.TestCacheDir))
	check(celer.SetPackageCacheWritable(true))
	check(celer.SetPlatform(platform))

	var port configs.Port
	var options = configs.InstallOptions{}
	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(options))

	// Remove installed and src dir.
	removeOptions := configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))
	check(port.MatchedConfig.Clean())

	// Install from cache with not matched commit.
	port.Package.Commit = "not_matched_commit_xxxxxx"
	installed, err := port.InstallFromPackageCache(options)
	if err == nil || !errors.Is(err, errors.ErrCacheNotFoundWithCommit) {
		t.Fatal("should return ErrCacheNotFoundWithCommit")
	}
	if installed {
		t.Fatal("should not be installed from cache")
	}
}
