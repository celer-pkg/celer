package configs

import (
	"celer/context"
	"celer/pkgcache"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"testing"
)

type fakeContext struct {
	platform string
	project  string
	build    string
	offline  bool
	pkgCache context.PkgCache
}

func (f fakeContext) Version() string                                          { return "test" }
func (f fakeContext) Platform() context.Platform                               { return fakePlatform{name: f.platform} }
func (f fakeContext) RootFS() context.RootFS                                   { return nil }
func (f fakeContext) Project() context.Project                                 { return fakeProject{name: f.project} }
func (f fakeContext) BuildType() string                                        { return f.build }
func (f fakeContext) Downloads() string                                        { return "" }
func (f fakeContext) Jobs() int                                                { return 1 }
func (f fakeContext) Offline() bool                                            { return f.offline }
func (f fakeContext) Verbose() bool                                            { return false }
func (f fakeContext) InstalledDir() string                                     { return "" }
func (f fakeContext) InstalledDevDir() string                                  { return "" }
func (f fakeContext) PkgCache() context.PkgCache                               { return f.pkgCache }
func (f fakeContext) ProxyHostPort() (host string, port int)                   { return "", 0 }
func (f fakeContext) Optimize(buildsystem, toolchain string) *context.Optimize { return nil }
func (f fakeContext) CCacheEnabled() bool                                      { return false }
func (f fakeContext) GenerateToolchainFile() error                             { return nil }
func (f fakeContext) ExprVars() *context.ExprVars                              { return nil }

type fakePlatform struct {
	name string
}

func (f fakePlatform) Init(platformName string) error  { return nil }
func (f fakePlatform) GetName() string                 { return f.name }
func (f fakePlatform) GetHostName() string             { return f.name + "-host" }
func (f fakePlatform) GetToolchain() context.Toolchain { return nil }
func (f fakePlatform) GetRootFS() context.RootFS       { return nil }
func (f fakePlatform) GetArchiveChecksums() (toolchainChecksum, rootfsChecksum string, err error) {
	return "", "", nil
}
func (f fakePlatform) Setup() error { return nil }

type fakeProject struct {
	name string
}

func (f fakeProject) Init(ctx context.Context, projectName string) error { return nil }
func (f fakeProject) GetName() string                                    { return f.name }
func (f fakeProject) GetPorts() []string                                 { return nil }
func (f fakeProject) GetTargetPlatform() string                          { return "" }
func (f fakeProject) Write(platformPath string) error                    { return nil }

func TestArtifactCacheStoreAndFetch(t *testing.T) {
	oldWorkspace := dirs.WorkspaceDir
	tmpWorkspace := t.TempDir()
	dirs.Init(tmpWorkspace)
	t.Cleanup(func() { dirs.Init(oldWorkspace) })

	cacheDir := filepath.Join(tmpWorkspace, "cache")
	if err := os.MkdirAll(cacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	// creates a fresh cache entry per subtest so test cases
	// stay isolated and do not depend on execution order.
	setupArtifactFixture := func(t *testing.T) (artifactCache context.AritifactCache, nameVersion, meta, hash, packageDir string) {
		t.Helper()

		pkgCache := NewPkgCache(fakeContext{
			platform: "x86_64-linux",
			project:  "proj",
			build:    "release",
		}, cacheDir, true)
		if err := pkgCache.Validate(); err != nil {
			t.Fatal(err)
		}

		nameVersion = "demo@1.0.0"
		meta = "meta-data-for-test"
		metaHash := sha256.Sum256([]byte(meta))
		hash = fmt.Sprintf("%x", metaHash)

		packageDir = filepath.Join(tmpWorkspace, "packages", "demo@1.0.0@x86_64-linux@proj@release")
		if err := os.MkdirAll(packageDir, os.ModePerm); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(filepath.Join(packageDir, "a.txt"), []byte("hello"), os.ModePerm); err != nil {
			t.Fatal(err)
		}

		artifactCache = pkgCache.GetArtifactCache()
		if artifactCache == nil {
			t.Fatal("artifact cache should not be nil")
		}
		if err := artifactCache.Store(packageDir, meta); err != nil {
			t.Fatal(err)
		}

		return artifactCache, nameVersion, meta, hash, packageDir
	}

	t.Run("artifact not exist", func(t *testing.T) {
		artifactCache, nameVersion, _, _, _ := setupArtifactFixture(t)
		destDir := filepath.Join(tmpWorkspace, "out-not-exist")
		ok, err := artifactCache.Fetch(nameVersion, "not-exist-hash", destDir)
		if err != nil {
			t.Fatalf("expected no error, got: %v", err)
		}
		if ok {
			t.Fatal("expected not installed when archive not exist")
		}
	})

	t.Run("meta missing", func(t *testing.T) {
		artifactCache, nameVersion, _, hash, _ := setupArtifactFixture(t)

		// Remove meta file.
		metaPath := filepath.Join(cacheDir, pkgcache.ArtifactCacheDir, "x86_64-linux", "proj", "release", nameVersion, "meta", hash+".meta")
		if err := os.Remove(metaPath); err != nil {
			t.Fatal(err)
		}

		// Fetch cache to test_package.
		packageDir := filepath.Join(tmpWorkspace, "test_package")
		ok, err := artifactCache.Fetch(nameVersion, hash, packageDir)
		if err == nil {
			t.Fatal("expected error when metadata is missing")
		}
		if ok {
			t.Fatal("expected not installed when metadata is missing")
		}
	})

	t.Run("meta checksum mismatch", func(t *testing.T) {
		artifactCache, nameVersion, meta, hash, _ := setupArtifactFixture(t)

		// Remove meta file.
		metaPath := filepath.Join(cacheDir, pkgcache.ArtifactCacheDir, "x86_64-linux", "proj", "release", nameVersion, "meta", hash+".meta")
		if err := os.WriteFile(metaPath, []byte("tampered-meta"), os.ModePerm); err != nil {
			t.Fatal(err)
		}

		destDir := filepath.Join(tmpWorkspace, "out-meta-mismatch")
		ok, err := artifactCache.Fetch(nameVersion, hash, destDir)
		if err == nil {
			t.Fatal("expected error when metadata checksum mismatches")
		}
		if ok {
			t.Fatal("expected not installed when metadata checksum mismatches")
		}

		if err := os.WriteFile(metaPath, []byte(meta), os.ModePerm); err != nil {
			t.Fatal(err)
		}
	})

	t.Run("read success", func(t *testing.T) {
		artifactCache, nameVersion, _, hash, _ := setupArtifactFixture(t)
		destDir := filepath.Join(tmpWorkspace, "out-success")
		ok, err := artifactCache.Fetch(nameVersion, hash, destDir)
		if err != nil {
			t.Fatalf("expected no error, got: %v", err)
		}
		if !ok {
			t.Fatal("expected install from cache success")
		}

		if !fileio.PathExists(filepath.Join(destDir, "a.txt")) {
			t.Fatal("expected extracted file a.txt in destination")
		}

		// Verify extracted content, not just path existence.
		content, err := os.ReadFile(filepath.Join(destDir, "a.txt"))
		if err != nil {
			t.Fatalf("read extracted file failed: %v", err)
		}
		if string(content) != "hello" {
			t.Fatalf("unexpected extracted content: %q", string(content))
		}
	})
}
