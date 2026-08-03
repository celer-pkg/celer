package pkgcache

import (
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgcache/nfs"
)

func BuildAritifactCache(ctx context.Context, writable bool) context.AritifactCache {
	return nfs.NewArtifactConfig(ctx, writable)
}

func BuildRepoCache(ctx context.Context, writable bool) context.RepoCache {
	return nfs.NewRepoConfig(ctx, writable)
}
