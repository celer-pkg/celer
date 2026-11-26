package integration

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/dag"
	"github.com/celer-pkg/celer/internal/dirs"
	"github.com/celer-pkg/celer/internal/fileio"
	"github.com/celer-pkg/celer/internal/git"
	"github.com/celer-pkg/celer/pkgcache"
)

func TestInstall_PkgCache_Artifact_Success(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "eigen@3.4.0"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetProject(project))
	// The cache dir has to exist before it is configured.
	check(t, os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))
	check(t, celer.SetPkgCacheFSDir(dirs.TestPkgCacheDir))
	check(t, celer.SetPkgCacheReadOnly(false))
	check(t, celer.SetPlatform(platform))

	var port configs.Port
	var options = configs.InstallOptions{Prefer: configs.PreferSource}
	check(t, port.Init(celer, nameVersion))
	if _, err := port.Install(options); err != nil {
		t.Fatal(err)
	}

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
	check(t, port.Remove(removeOptions))
	check(t, port.MatchedConfig.Clean())

	// Install from package should fail.
	options.Prefer = configs.PreferPackage
	if fromWhere, err := port.Install(options); err != nil {
		t.Fatal(err)
	} else if fromWhere != "" {
		t.Fatal("should install failed from package")
	}

	// Install from cache should success.
	options.Prefer = configs.PreferPkgCache
	if fromWhere, err := port.Install(options); err != nil {
		t.Fatal(err)
	} else if fromWhere == "" {
		t.Fatal("should install successfully from cache")
	}

	// Clean up.
	check(t, port.Remove(removeOptions))
}

func TestInstall_PkgCache_Artifact_With_Deps_Success(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "glog@0.6.0"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetProject(project))
	// The cache dir has to exist before it is configured.
	check(t, os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))
	check(t, celer.SetPkgCacheFSDir(dirs.TestPkgCacheDir))
	check(t, celer.SetPkgCacheReadOnly(false))
	check(t, celer.SetPlatform(platform))

	var glogPort configs.Port
	var options = configs.InstallOptions{Prefer: configs.PreferSource}
	check(t, glogPort.Init(celer, nameVersion))
	if _, err := glogPort.Install(options); err != nil {
		t.Fatal(err)
	}

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
	check(t, glogPort.Remove(removeOptions))
	check(t, glogPort.MatchedConfig.Clean())

	// Install from package should fail.
	options.Prefer = configs.PreferPackage
	if fromWhere, err := glogPort.Install(options); err != nil {
		t.Fatal(err)
	} else if fromWhere != "" {
		t.Fatal("should install failed from package")
	}

	// Install from cache should success.
	options.Prefer = configs.PreferPkgCache
	if fromWhere, err := glogPort.Install(options); err != nil {
		t.Fatal(err)
	} else if fromWhere == "" {
		t.Fatal("should install successfully from cache")
	}

	// gflags should also be installed from cache.
	var gflagsPort configs.Port
	check(t, gflagsPort.Init(celer, "gflags@2.2.2"))
	if fromWhere, err := gflagsPort.Installed(); err != nil {
		t.Fatal(err)
	} else if !fromWhere {
		t.Fatal("gflags not installed")
	}

	// Clean up.
	check(t, glogPort.Remove(removeOptions))
	check(t, gflagsPort.Remove(removeOptions))
}

func TestInstall_PkgCache_Prebuilt_Success(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "cuda_cccl@12.9.27"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetProject(project))
	// The cache dir has to exist before it is configured.
	check(t, os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))
	check(t, celer.SetPkgCacheFSDir(dirs.TestPkgCacheDir))
	check(t, celer.SetPkgCacheReadOnly(false))
	check(t, celer.SetPlatform(platform))

	var port configs.Port
	var options = configs.InstallOptions{Prefer: configs.PreferSource}
	check(t, port.Init(celer, nameVersion))
	if _, err := port.Install(options); err != nil {
		t.Fatal(err)
	}

	// Check package.
	packageDir := filepath.Join(dirs.PackagesDir, platform, project, celer.BuildType(), nameVersion)
	if !fileio.PathExists(packageDir) {
		t.Fatal("package cannot found: " + packageDir)
	}

	// Prebuilt binaries are architecture/toolchain-specific, so they are cached
	// as a repo archive under pkgcache/repos with a toolchain-prefixed key (not
	// the bare nameVersion), allowing different toolchains to coexist.
	toolchain := port.MatchedConfig.Ctx.Platform().GetToolchain()
	expectedKey := strings.Join([]string{
		toolchain.GetSystemProcessor(),
		strings.ToLower(toolchain.GetSystemName()),
		toolchain.GetName(),
		toolchain.GetVersion(),
		strings.ToLower(port.MatchedConfig.Ctx.BuildType()),
		nameVersion,
	}, "-")

	repoName, _, _ := strings.Cut(nameVersion, "@")
	repoCacheDir := filepath.Join(dirs.TestPkgCacheDir, "repos", repoName)
	entries, err := os.ReadDir(repoCacheDir)
	check(t, err)

	var found bool
	for _, entry := range entries {
		if strings.HasPrefix(entry.Name(), expectedKey) {
			found = true
			break
		}
	}
	if !found {
		t.Fatalf("prebuilt repo should be cached with toolchain-prefixed key %q, got entries: %v", expectedKey, entries)
	}

	// Totally remove port (including src), then reinstall: the prebuilt repo
	// should be restored from the repo cache.
	var removeOptions = configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(t, port.Remove(removeOptions))
	check(t, port.MatchedConfig.Clean())

	if _, err := port.Install(options); err != nil {
		t.Fatal(err)
	}

	// Clean up.
	check(t, port.Remove(removeOptions))
}

func TestInstall_PkgCache_DirNotDefined_ShouldSkipStoreCache(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "eigen@3.4.0"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetProject(project))
	check(t, celer.SetPlatform(platform))

	var port configs.Port
	var options = configs.InstallOptions{Prefer: configs.PreferSource}
	check(t, port.Init(celer, nameVersion))
	if _, err := port.Install(options); err != nil {
		t.Fatal(err)
	}
}

func TestInstall_PkgCache_With_Commit_Success(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "eigen@3.4.0"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetProject(project))
	// The cache dir has to exist before it is configured.
	check(t, os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))
	check(t, celer.SetPkgCacheFSDir(dirs.TestPkgCacheDir))
	check(t, celer.SetPkgCacheReadOnly(false))
	check(t, celer.SetPlatform(platform))

	var port configs.Port
	var options = configs.InstallOptions{Prefer: configs.PreferSource}
	check(t, port.Init(celer, nameVersion))
	if _, err := port.Install(options); err != nil {
		t.Fatal(err)
	}

	// Read commit hash.
	commit, err := git.GetCommitHash(port.MatchedConfig.PortConfig.RepoDir)
	check(t, err)

	// Remove installed and src dir.
	removeOptions := configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(t, port.Remove(removeOptions))
	check(t, port.MatchedConfig.Clean())

	// Install from cache with commit.
	port.Package.Checksum = commit
	options.Prefer = configs.PreferPkgCache
	if fromWhere, err := port.Install(options); err != nil {
		t.Fatal(err)
	} else if fromWhere == "" {
		t.Fatal("should be installed from cache")
	}

	// Clean up.
	check(t, port.Remove(removeOptions))
}

func TestInstall_PkgCache_With_Commit_Missing_FallsBackToSource(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "eigen@3.4.0"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetProject(project))
	// The cache dir has to exist before it is configured.
	check(t, os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))
	check(t, celer.SetPkgCacheFSDir(dirs.TestPkgCacheDir))
	check(t, celer.SetPkgCacheReadOnly(false))
	check(t, celer.SetPlatform(platform))

	var port configs.Port
	var options = configs.InstallOptions{Prefer: configs.PreferSource}
	check(t, port.Init(celer, nameVersion))
	if _, err := port.Install(options); err != nil {
		t.Fatal(err)
	}

	commit, err := git.GetCommitHash(port.MatchedConfig.PortConfig.RepoDir)
	check(t, err)

	// Remove installed package and artifact cache, but keep source available for fallback.
	removeOptions := configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(t, port.Remove(removeOptions))

	// Drop the cached artifact to simulate a cache miss; the fs pkgcache layout
	// is {cache_dir}/artifacts-{major.minor}.x/{platform}/{project}/{buildType}/{nameVersion}.
	fs := pkgcache.FS{Dir: dirs.TestPkgCacheDir}
	artifactsDir := fs.GetDir(pkgcache.DirArtifacts, celer.Version())
	check(t, os.RemoveAll(artifactsDir))

	port.Package.Checksum = commit
	port.MatchedConfig.PortConfig.Checksum = commit

	options.Prefer = configs.PreferPkgCache
	if fromWhere, err := port.Install(options); err != nil {
		t.Fatal(err)
	} else if fromWhere != "" {
		t.Fatal("should not be installed from missing artifact cache")
	}

	options.Prefer = "" // Reset 'prefer', so that it can try install from default prefer chain.
	installedFrom, err := port.Install(options)
	check(t, err)
	if installedFrom != "source" {
		t.Fatalf("expected install from source, got %q", installedFrom)
	}

	actualCommit, err := git.GetCommitHash(port.MatchedConfig.PortConfig.RepoDir)
	check(t, err)
	if actualCommit != commit {
		t.Fatalf("expected checkout %s, got %s", commit, actualCommit)
	}

	// Clean up.
	check(t, port.Remove(removeOptions))
}

func TestInstall_Command_ReportContainsPkgCacheSource(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	var (
		nameVersion = "eigen@3.4.0"
		platform    = buildTestPlatform()
		project     = testProjectName
	)

	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetProject(project))
	// The cache dir has to exist before it is configured.
	check(t, os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))
	check(t, celer.SetPkgCacheFSDir(dirs.TestPkgCacheDir))
	check(t, celer.SetPkgCacheReadOnly(false))
	check(t, celer.SetPlatform(platform))

	// Prepare cache by installing from source once.
	var port configs.Port
	var options = configs.InstallOptions{Prefer: configs.PreferSource}
	check(t, port.Init(celer, nameVersion))
	if _, err := port.Install(options); err != nil {
		t.Fatal(err)
	}

	// Remove installed and source to force package-cache path.
	removeOptions := configs.RemoveOptions{
		Purge:      true,
		Recursive:  true,
		BuildCache: true,
	}
	check(t, port.Remove(removeOptions))
	check(t, port.MatchedConfig.Clean())

	// Run the same install flow the install command uses, with default flags.
	_, err := dag.Install(celer, nameVersion, false, configs.InstallOptions{}, dag.Expectation{Banner: true})
	check(t, err)

	// Report should contain package cache source.
	statisticPath := filepath.Join(dirs.InstalledDir, "infos", "statistics", platform, project, celer.BuildType(),
		"eigen_3.4.0.md")
	if !fileio.PathExists(statisticPath) {
		t.Fatalf("install report not found: %s", statisticPath)
	}

	report, err := os.ReadFile(statisticPath)
	check(t, err)
	if !strings.Contains(string(report), "pkgcache") {
		t.Fatalf("expected report to contain package cache source, report: %s", statisticPath)
	}
}
