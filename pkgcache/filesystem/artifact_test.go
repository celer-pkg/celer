package filesystem

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"
)

// writeCacheFile writes a cache object under remoteDir and pins its modification time.
func writeCacheFile(t *testing.T, remoteDir, name, content string, modified time.Time) {
	t.Helper()

	filePath := filepath.Join(remoteDir, name)
	if err := os.MkdirAll(filepath.Dir(filePath), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filePath, []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.Chtimes(filePath, modified, modified); err != nil {
		t.Fatal(err)
	}
}

func TestLatestMetaHash(t *testing.T) {
	remoteDir := t.TempDir()
	base := time.Now().Add(-time.Hour)

	var (
		older   = strings.Repeat("a", 64)
		newer   = strings.Repeat("b", 64)
		orphan  = strings.Repeat("c", 64)
		notHash = "notes"
	)

	writeCacheFile(t, remoteDir, "metas/"+older+".meta", "old", base)
	writeCacheFile(t, remoteDir, older+".tar.gz", "old", base)
	writeCacheFile(t, remoteDir, "metas/"+newer+".meta", "new", base.Add(30*time.Minute))
	writeCacheFile(t, remoteDir, newer+".tar.gz", "new", base.Add(30*time.Minute))

	// Newest meta without its archive: a half-finished Store, it must be ignored.
	writeCacheFile(t, remoteDir, "metas/"+orphan+".meta", "orphan", base.Add(time.Hour))

	// Newest of all, but not a build hash name: ignored as well.
	writeCacheFile(t, remoteDir, "metas/"+notHash+".meta", "notes", base.Add(2*time.Hour))
	writeCacheFile(t, remoteDir, notHash+".tar.gz", "notes", base.Add(2*time.Hour))

	hash, found, err := latestMetaHash(remoteDir)
	if err != nil {
		t.Fatal(err)
	}
	if !found {
		t.Fatal("expected a cached meta to be found")
	}
	if hash != newer {
		t.Fatalf("expected the newest meta with an archive, got '%s'", hash)
	}
}

func TestLatestMetaHash_NeverCached(t *testing.T) {
	// Neither the metas dir nor the port dir exists.
	if _, found, err := latestMetaHash(t.TempDir()); err != nil || found {
		t.Fatalf("expected no meta and no error, got found=%v err=%v", found, err)
	}

	// Metas exist but none of them has an archive.
	remoteDir := t.TempDir()
	writeCacheFile(t, remoteDir, "metas/"+strings.Repeat("a", 64)+".meta", "orphan", time.Now())
	if _, found, err := latestMetaHash(remoteDir); err != nil || found {
		t.Fatalf("expected orphan metas to be ignored, got found=%v err=%v", found, err)
	}
}
