package minio

import (
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
	minioCfg *pkgcache.Minio
}

func (p testPkgCache) GetMinio() *pkgcache.Minio { return p.minioCfg }
func (p testPkgCache) IsReadOnly() bool          { return false }

func TestNewArtifactConfigPath(t *testing.T) {
	ctx := testContext{
		pkgCache: testPkgCache{minioCfg: &pkgcache.Minio{}},
		version:  "v1.2.3",
	}

	config := NewArtifactConfig(ctx, nil)
	if config == nil {
		t.Fatal("NewArtifactConfig() = nil")
	}
	if config.ctx == nil {
		t.Fatal("NewArtifactConfig() left ctx unset")
	}
	if want := "artifacts-v1.2.x"; config.cacheDirNode != want {
		t.Errorf("cacheDirNode = %q, want %q", config.cacheDirNode, want)
	}
}
