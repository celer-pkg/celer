package cmds

import (
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/git"
)

func TestInstall_PkgCache_Artifact_Success(t *testing.T) {
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
	check(os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "eigen@3.4.0"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	if err := celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true); err != nil {
		t.Fatal(err)
	}
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPkgCacheDir(dirs.TestPkgCacheDir))
	check(celer.SetPkgCacheWritable(true))
	check(celer.SetPlatform(platform))

	var port configs.Port
	var installOptions = configs.InstallOptions{}
	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(installOptions))

	// Check package.
	packageDir := filepath.Join(dirs.PackagesDir, platform, project, celer.BuildType(), nameVersion)
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
	installed, err = port.InstallFromPkgCache(installOptions)
	check(err)
	if !installed {
		t.Fatal("should install successfully from cache")
	}

	// Clean up.
	check(port.Remove(removeOptions))
}

func TestInstall_PkgCache_Artifact_With_Deps_Success(t *testing.T) {
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
	check(os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "glog@0.6.0"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	if err := celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true); err != nil {
		t.Fatal(err)
	}
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPkgCacheDir(dirs.TestPkgCacheDir))
	check(celer.SetPkgCacheWritable(true))
	check(celer.SetPlatform(platform))

	var glogPort configs.Port
	var options = configs.InstallOptions{}
	check(glogPort.Init(celer, nameVersion))
	check(glogPort.InstallFromSource(options))

	packageDir := func(nameVersion string) string {
		return filepath.Join(dirs.PackagesDir, platform, project, celer.BuildType(), nameVersion)
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
	installed, err = glogPort.InstallFromPkgCache(options)
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

func TestInstall_PkgCache_Prebuilt_Success(t *testing.T) {
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
	check(os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))

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
	check(celer.SetPkgCacheDir(dirs.TestPkgCacheDir))
	check(celer.SetPkgCacheWritable(true))
	check(celer.SetPlatform(platform))

	var port configs.Port
	var options = configs.InstallOptions{}
	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(options))

	// Check package & repo.
	packageDir := filepath.Join(dirs.PackagesDir, platform, project, celer.BuildType(), nameVersion)

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
	installed, err = port.InstallFromPkgCache(options)
	check(err)
	if !installed {
		t.Fatal("should install successfully from cache")
	}

	// Clean up.
	check(port.Remove(removeOptions))
}

func TestInstall_PkgCache_DirNotDefined_ShouldSkipStoreCache(t *testing.T) {
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
	check(os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))

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

func TestInstall_PkgCache_With_Commit_Success(t *testing.T) {
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
	check(os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))

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
	check(celer.SetPkgCacheDir(dirs.TestPkgCacheDir))
	check(celer.SetPkgCacheWritable(true))
	check(celer.SetPlatform(platform))

	var port configs.Port
	var options = configs.InstallOptions{}
	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(options))

	// Read commit hash.
	commit, err := git.GetCommitHash(port.MatchedConfig.PortConfig.RepoDir)
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
	port.Package.Checksum = commit
	installed, err := port.InstallFromPkgCache(options)
	check(err)
	if !installed {
		t.Fatal("should be installed from cache")
	}

	// Clean up.
	check(port.Remove(removeOptions))
}

func TestInstall_PkgCache_With_Commit_Missing_FallsBackToSource(t *testing.T) {
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
	check(os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))

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
	check(celer.SetPkgCacheDir(dirs.TestPkgCacheDir))
	check(celer.SetPkgCacheWritable(true))
	check(celer.SetPlatform(platform))

	var port configs.Port
	var options = configs.InstallOptions{}
	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(options))

	commit, err := git.GetCommitHash(port.MatchedConfig.PortConfig.RepoDir)
	check(err)

	// Remove installed package and artifact cache, but keep source available for fallback.
	removeOptions := configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))
	check(celer.PkgCacheConfig().GetArtifactCache().(interface{ Remove(string) error }).Remove(nameVersion))

	port.Package.Checksum = commit
	port.MatchedConfig.PortConfig.Checksum = commit

	installed, err := port.InstallFromPkgCache(options)
	check(err)
	if installed {
		t.Fatal("should not be installed from missing artifact cache")
	}

	installedFrom, err := port.Install(options)
	check(err)
	if installedFrom != "source" {
		t.Fatalf("expected install from source, got %q", installedFrom)
	}

	actualCommit, err := git.GetCommitHash(port.MatchedConfig.PortConfig.RepoDir)
	check(err)
	if actualCommit != commit {
		t.Fatalf("expected checkout %s, got %s", commit, actualCommit)
	}

	// Clean up.
	check(port.Remove(removeOptions))
}

func TestInstall_Command_ReportContainsPkgCacheSource(t *testing.T) {
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
	check(os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())

	var (
		nameVersion     = "eigen@3.4.0"
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_install"
	)

	if err := celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true); err != nil {
		msg := err.Error()
		if strings.Contains(msg, "Could not resolve host") ||
			strings.Contains(msg, "not accessible") {
			t.Skipf("skip due to unavailable network: %v", err)
		}
		t.Fatal(err)
	}
	check(celer.SetBuildType("Release"))
	check(celer.SetProject(project))
	check(celer.SetPkgCacheDir(dirs.TestPkgCacheDir))
	check(celer.SetPkgCacheWritable(true))
	check(celer.SetPlatform(platform))

	// Prepare cache by installing from source once.
	var port configs.Port
	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(configs.InstallOptions{}))

	// Remove installed and source to force package-cache path.
	removeOptions := configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(port.Remove(removeOptions))
	check(port.MatchedConfig.Clean())

	// Run install command flow.
	install := installCmd{celer: celer}
	check(install.runInstall([]string{nameVersion}))

	// Report should contain package cache source.
	statisticPath := filepath.Join(dirs.InstalledDir, "celer", "statistics", platform, project, celer.BuildType(),
		"eigen_3.4.0.md")
	if !fileio.PathExists(statisticPath) {
		t.Fatalf("install report not found: %s", statisticPath)
	}

	report, err := os.ReadFile(statisticPath)
	check(err)
	if !strings.Contains(string(report), "pkgcache") {
		t.Fatalf("expected report to contain package cache source, report: %s", statisticPath)
	}
}
