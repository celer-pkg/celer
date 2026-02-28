package configs

import (
	"celer/context"
	celerctx "celer/context"
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
}

func (f fakeContext) Version() string                                           { return "test" }
func (f fakeContext) Platform() celerctx.Platform                               { return fakePlatform{name: f.platform} }
func (f fakeContext) RootFS() celerctx.RootFS                                   { return nil }
func (f fakeContext) Project() celerctx.Project                                 { return fakeProject{name: f.project} }
func (f fakeContext) BuildType() string                                         { return f.build }
func (f fakeContext) Downloads() string                                         { return "" }
func (f fakeContext) Jobs() int                                                 { return 1 }
func (f fakeContext) Offline() bool                                             { return false }
func (f fakeContext) Verbose() bool                                             { return false }
func (f fakeContext) InstalledDir() string                                      { return "" }
func (f fakeContext) InstalledDevDir() string                                   { return "" }
func (f fakeContext) PackageCache() celerctx.PackageCache                       { return nil }
func (f fakeContext) ProxyHostPort() (host string, port int)                    { return "", 0 }
func (f fakeContext) Optimize(buildsystem, toolchain string) *celerctx.Optimize { return nil }
func (f fakeContext) CCacheEnabled() bool                                       { return false }
func (f fakeContext) GenerateToolchainFile() error                              { return nil }
func (f fakeContext) ExprVars() *context.ExprVars                               { return nil }

type fakePlatform struct {
	name string
}

func (f fakePlatform) Init(platformName string) error   { return nil }
func (f fakePlatform) GetName() string                  { return f.name }
func (f fakePlatform) GetHostName() string              { return f.name + "-host" }
func (f fakePlatform) GetToolchain() celerctx.Toolchain { return nil }
func (f fakePlatform) GetRootFS() celerctx.RootFS       { return nil }
func (f fakePlatform) GetArchiveChecksums() (toolchainChecksum, rootfsChecksum string, err error) {
	return "", "", nil
}
func (f fakePlatform) Setup() error { return nil }

type fakeProject struct {
	name string
}

func (f fakeProject) Init(ctx celerctx.Context, projectName string) error { return nil }
func (f fakeProject) GetName() string                                     { return f.name }
func (f fakeProject) GetPorts() []string                                  { return nil }
func (f fakeProject) GetTargetPlatform() string                           { return "" }
func (f fakeProject) Write(platformPath string) error                     { return nil }

func TestPackageCacheRead(t *testing.T) {
	oldWorkspace := dirs.WorkspaceDir
	tmpWorkspace := t.TempDir()
	dirs.Init(tmpWorkspace)
	t.Cleanup(func() { dirs.Init(oldWorkspace) })

	cacheDir := filepath.Join(tmpWorkspace, "cache")
	if err := os.MkdirAll(cacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	cache := PackageCache{
		Dir: cacheDir,
		ctx: fakeContext{
			platform: "x86_64-linux",
			project:  "proj",
			build:    "release",
		},
	}

	nameVersion := "demo@1.0.0"
	meta := "meta-data-for-test"
	metaHash := sha256.Sum256([]byte(meta))
	hash := fmt.Sprintf("%x", metaHash)

	// Create a fake package.
	packageDir := filepath.Join(tmpWorkspace, "packages", "demo@1.0.0@x86_64-linux@proj@release")
	if err := os.MkdirAll(packageDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(packageDir, "a.txt"), []byte("hello"), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := cache.Write(packageDir, meta); err != nil {
		t.Fatal(err)
	}

	t.Run("archive not exist", func(t *testing.T) {
		destDir := filepath.Join(tmpWorkspace, "out-not-exist")
		ok, err := cache.Read(nameVersion, "12334567890-not-exist-hash", destDir)
		if err != nil {
			t.Fatalf("expected no error, got: %v", err)
		}
		if ok {
			t.Fatal("expected not installed when archive not exist")
		}
	})

	t.Run("meta missing", func(t *testing.T) {
		metaPath := filepath.Join(cacheDir, "x86_64-linux", "proj", "release", nameVersion, "meta", hash+".meta")
		if err := os.Remove(metaPath); err != nil {
			t.Fatal(err)
		}

		destDir := filepath.Join(tmpWorkspace, "out-meta-missing")
		ok, err := cache.Read(nameVersion, hash, destDir)
		if err == nil {
			t.Fatal("expected error when metadata is missing")
		}
		if ok {
			t.Fatal("expected not installed when metadata is missing")
		}

		if err := os.WriteFile(metaPath, []byte(meta), os.ModePerm); err != nil {
			t.Fatal(err)
		}
	})

	t.Run("meta checksum mismatch", func(t *testing.T) {
		metaPath := filepath.Join(cacheDir, "x86_64-linux", "proj", "release", nameVersion, "meta", hash+".meta")
		if err := os.WriteFile(metaPath, []byte("tampered-meta"), os.ModePerm); err != nil {
			t.Fatal(err)
		}

		destDir := filepath.Join(tmpWorkspace, "out-meta-mismatch")
		ok, err := cache.Read(nameVersion, hash, destDir)
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
		destDir := filepath.Join(tmpWorkspace, "out-success")
		ok, err := cache.Read(nameVersion, hash, destDir)
		if err != nil {
			t.Fatalf("expected no error, got: %v", err)
		}
		if !ok {
			t.Fatal("expected install from cache success")
		}

		if !fileio.PathExists(filepath.Join(destDir, "a.txt")) {
			t.Fatal("expected extracted file a.txt in destination")
		}
	})
}
