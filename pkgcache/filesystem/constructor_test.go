package filesystem

import (
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache"
)

// testContext overrides only the context methods the constructors use; the
// embedded nil interface panics if anything else is called.
type testContext struct {
	context.Context
	pkgCache pkgcache.PkgCache
	version  string
}

func (c testContext) PkgCache() pkgcache.PkgCache { return c.pkgCache }
func (c testContext) Version() string             { return c.version }

type testPkgCache struct {
	pkgcache.PkgCache
	fs *pkgcache.FS
}

func (p testPkgCache) GetFS() *pkgcache.FS { return p.fs }
func (p testPkgCache) IsReadOnly() bool    { return false }

func TestNewArtifactConfig(t *testing.T) {
	cacheDir := t.TempDir()
	ctx := testContext{
		pkgCache: testPkgCache{fs: &pkgcache.FS{Dir: cacheDir}},
		version:  "v1.2.3",
	}

	config := NewArtifactConfig(ctx)
	if config == nil {
		t.Fatal("NewArtifactConfig() = nil")
	}
	if config.ctx == nil {
		t.Fatal("NewArtifactConfig() left ctx unset")
	}
	if want := filepath.Join(cacheDir, "artifacts-v1.2.x"); config.cacheDirNode != want {
		t.Errorf("cacheDirNode = %q, want %q", config.cacheDirNode, want)
	}
	if config.cacheDirRoot != cacheDir {
		t.Errorf("cacheDirRoot = %q, want %q", config.cacheDirRoot, cacheDir)
	}
}
