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

func TestInstall_BinaryCache_Success(t *testing.T) {
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
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetBinaryCacheDir(dirs.TestCacheDir))
	check(celer.SetBinaryCacheToken("token_123456"))
	check(celer.SetPlatform(platform))
	check(celer.Setup())

	var port configs.Port
	var installOptions = configs.InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
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
	check(os.RemoveAll(port.MatchedConfig.PortConfig.RepoDir))

	// Install from package should fail.
	installed, err := port.InstallFromPackage(installOptions)
	check(err)
	if installed {
		t.Fatal("should install failed from package")
	}

	// Install from cache should success.
	installed, err = port.InstallFromBinaryCache(installOptions)
	check(err)
	if !installed {
		t.Fatal("should install successfully from cache")
	}

	// Clean up.
	check(port.Remove(removeOptions))
}

func TestInstall_BinaryCache_With_Deps_Success(t *testing.T) {
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
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetBinaryCacheDir(dirs.TestCacheDir))
	check(celer.SetBinaryCacheToken("token_123456"))
	check(celer.SetPlatform(platform))
	check(celer.Setup())

	var glogPort configs.Port
	var options = configs.InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
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
	check(os.RemoveAll(glogPort.MatchedConfig.PortConfig.RepoDir))

	// Install from package should fail.
	installed, err := glogPort.InstallFromPackage(options)
	check(err)
	if installed {
		t.Fatal("should install failed from package")
	}

	// Install from cache should success.
	installed, err = glogPort.InstallFromBinaryCache(options)
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

func TestInstall_BinaryCache_Prebuilt_Success(t *testing.T) {
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
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetBinaryCacheDir(dirs.TestCacheDir))
	check(celer.SetBinaryCacheToken("token_123456"))
	check(celer.SetPlatform(platform))
	check(celer.Setup())

	var port configs.Port
	var options = configs.InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(options))

	// Check package & repo.
	packageDir := fmt.Sprintf("%s/%s@%s@%s@%s",
		dirs.PackagesDir, nameVersion,
		platform, project,
		celer.BuildType(),
	)

	if !fileio.PathExists(packageDir) {
		t.Fatal("package cannot found")
	}
	if fileio.PathExists(port.MatchedConfig.PortConfig.RepoDir) {
		t.Fatal("repo should not exist")
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
	installed, err = port.InstallFromBinaryCache(options)
	check(err)
	if !installed {
		t.Fatal("should install successfully from cache")
	}

	// Clean up.
	check(port.Remove(removeOptions))
}

func TestInstall_BinaryCache_DirNotDefined_Failed(t *testing.T) {
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
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	// check(celer.SetBinaryCacheDir(dirs.TestCacheDir))
	check(celer.SetPlatform(platform))
	check(celer.Setup())

	var port configs.Port
	var options = configs.InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
	check(port.Init(celer, nameVersion))
	if err := port.InstallFromSource(options); err != errors.ErrBinaryCacheDirNotConfigured {
		t.Fatal("should return ErrCacheDirNotConfigured")
	}
}

func TestInstall_BinaryCache_TokenNotDefined_Failed(t *testing.T) {
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
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetBinaryCacheDir(dirs.TestCacheDir))
	// check(celer.SetBinaryCacheToken(""))
	check(celer.SetPlatform(platform))
	check(celer.Setup())

	var port configs.Port
	var options = configs.InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
	check(port.Init(celer, nameVersion))
	if err := port.InstallFromSource(options); err != errors.ErrBinaryCacheTokenNotConfigured {
		t.Fatal("should return ErrCacheTokenNotConfigured")
	}
}

func TestInstall_BinaryCache_TokenNotSpecified_Failed(t *testing.T) {
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
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetBinaryCacheDir(dirs.TestCacheDir))
	check(celer.SetBinaryCacheToken("token_123456"))
	check(celer.SetPlatform(platform))
	check(celer.Setup())

	var port configs.Port
	var options = configs.InstallOptions{
		StoreCache: true,
		CacheToken: "", // Token not specified
	}
	check(port.Init(celer, nameVersion))
	if err := port.InstallFromSource(options); err != errors.ErrBinaryCacheTokenNotSpecified {
		t.Fatal("should return ErrCacheTokenNotSpecified")
	}
}

func TestInstall_BinaryCache_TokenNotMatch_Failed(t *testing.T) {
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
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetBinaryCacheDir(dirs.TestCacheDir))
	check(celer.SetBinaryCacheToken("token_123456"))
	check(celer.SetPlatform(platform))
	check(celer.Setup())

	var port configs.Port
	var options = configs.InstallOptions{
		StoreCache: true,
		CacheToken: "token_654321", // Token not match.
	}
	check(port.Init(celer, nameVersion))
	if err := port.InstallFromSource(options); err != errors.ErrBinaryCacheTokenNotMatch {
		t.Fatal("should return ErrCacheTokenNotMatch")
	}
}

func TestInstall_BinaryCache_With_Commit_Success(t *testing.T) {
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
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetBinaryCacheDir(dirs.TestCacheDir))
	check(celer.SetBinaryCacheToken("token_123456"))
	check(celer.SetPlatform(platform))
	check(celer.Setup())

	var port configs.Port
	var options = configs.InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
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
	check(os.RemoveAll(port.MatchedConfig.PortConfig.RepoDir))

	// Install from cache with commit.
	port.Package.Commit = commit
	installed, err := port.InstallFromBinaryCache(options)
	check(err)
	if !installed {
		t.Fatal("should be installed from cache")
	}

	// Clean up.
	check(port.Remove(removeOptions))
}

func TestInstall_BinaryCache_With_Commit_Failed(t *testing.T) {
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
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetBinaryCacheDir(dirs.TestCacheDir))
	check(celer.SetBinaryCacheToken("token_123456"))
	check(celer.SetPlatform(platform))
	check(celer.Setup())

	var port configs.Port
	var options = configs.InstallOptions{
		StoreCache: true,
		CacheToken: "token_123456",
	}
	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(options))

	// Remove installed and src dir.
	removeOptions := configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))
	check(os.RemoveAll(port.MatchedConfig.PortConfig.RepoDir))

	// Install from cache with not matched commit.
	port.Package.Commit = "not_matched_commit_xxxxxx"
	installed, err := port.InstallFromBinaryCache(options)
	if err == nil || !errors.Is(err, errors.ErrCacheNotFoundWithCommit) {
		t.Fatal("should return ErrCacheNotFoundWithCommit")
	}
	if installed {
		t.Fatal("should not be installed from cache")
	}
}
