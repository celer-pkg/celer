package minio

import (
	"testing"
	"time"
)

func TestPickLatestBuild(t *testing.T) {
	base := time.Now()
	builds := map[string]cachedBuild{
		"old":    {hasMeta: true, hasArchive: true, metaAt: base},
		"new":    {hasMeta: true, hasArchive: true, metaAt: base.Add(time.Hour)},
		"orphan": {hasMeta: true, metaAt: base.Add(2 * time.Hour)},
	}

	hash, found := pickLatestBuild(builds)
	if !found || hash != "new" {
		t.Fatalf("expected the newest complete build, got '%s' (found=%v)", hash, found)
	}
}

func TestPickLatestBuild_NothingToCompare(t *testing.T) {
	if hash, found := pickLatestBuild(nil); found || hash != "" {
		t.Fatalf("expected no build, got '%s' (found=%v)", hash, found)
	}

	// A meta without its archive is a half-finished Store.
	builds := map[string]cachedBuild{"orphan": {hasMeta: true, metaAt: time.Now()}}
	if hash, found := pickLatestBuild(builds); found || hash != "" {
		t.Fatalf("expected incomplete builds to be ignored, got '%s' (found=%v)", hash, found)
	}
}
