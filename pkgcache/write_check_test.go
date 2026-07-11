package pkgcache

import (
	"os"
	"os/user"
	"path/filepath"
	"strconv"
	"strings"
	"testing"
)

func TestCheckWriteAccess_SkipWhenNotWritable(t *testing.T) {
	cacheDir := t.TempDir()
	ctx := fakeContext{
		pkgCache: fakePkgCache{dir: cacheDir, writable: false},
	}
	if err := CheckWriteAccess(ctx); err != nil {
		t.Fatalf("expected no error when not writable, got: %v", err)
	}
}

func TestCheckWriteAccess_SkipWhenOffline(t *testing.T) {
	cacheDir := t.TempDir()
	ctx := fakeContext{
		offline:  true,
		pkgCache: fakePkgCache{dir: cacheDir, writable: true},
	}
	if err := CheckWriteAccess(ctx); err != nil {
		t.Fatalf("expected no error when offline, got: %v", err)
	}
}

func TestCheckWriteAccess_SkipWhenNilPkgCache(t *testing.T) {
	ctx := fakeContext{}
	if err := CheckWriteAccess(ctx); err != nil {
		t.Fatalf("expected no error when pkgcache nil, got: %v", err)
	}
}

func TestCheckWriteAccess_NotInCelerGroup(t *testing.T) {
	cacheDir := setupWritableCacheDir(t)

	oldLookup := lookupGroup
	oldGetGroups := getProcessGroups
	t.Cleanup(func() {
		lookupGroup = oldLookup
		getProcessGroups = oldGetGroups
	})

	lookupGroup = func(name string) (*user.Group, error) {
		return &user.Group{Name: nfsUser, Gid: "424242"}, nil
	}
	getProcessGroups = func() ([]int, error) {
		return []int{os.Getgid()}, nil
	}

	ctx := fakeContext{
		pkgCache: fakePkgCache{dir: cacheDir, writable: true},
	}
	err := CheckWriteAccess(ctx)
	if err == nil {
		t.Fatal("expected error when process is not in celer group")
	}
	if !strings.Contains(err.Error(), "newgrp") {
		t.Fatalf("expected newgrp hint, got: %v", err)
	}
}

func TestCheckWriteAccess_WriteProbeFails(t *testing.T) {
	cacheDir := t.TempDir()
	if err := os.Chmod(cacheDir, 0555); err != nil {
		t.Fatal(err)
	}

	withMockCelerGroup(t, 5151)

	ctx := fakeContext{
		pkgCache: fakePkgCache{dir: cacheDir, writable: true},
	}
	err := CheckWriteAccess(ctx)
	if err == nil {
		t.Fatal("expected error when cache dir is not writable")
	}
	if !strings.Contains(err.Error(), "repos") {
		t.Fatalf("expected repos probe failure, got: %v", err)
	}
	if !strings.Contains(err.Error(), "parent") {
		t.Fatalf("expected parent details in error, got: %v", err)
	}
}

func TestCheckWriteAccess_Success(t *testing.T) {
	cacheDir := setupWritableCacheDir(t)
	withMockCelerGroup(t, 6161)

	ctx := fakeContext{
		pkgCache: fakePkgCache{dir: cacheDir, writable: true},
	}
	if err := CheckWriteAccess(ctx); err != nil {
		t.Fatalf("expected success, got: %v", err)
	}

	for _, sub := range []string{"repos", "downloads", "artifacts"} {
		probe := filepath.Join(cacheDir, sub, writeProbeRel)
		if !fileExists(probe) {
			t.Fatalf("expected probe file at %s", probe)
		}
	}
}

func setupWritableCacheDir(t *testing.T) string {
	t.Helper()
	cacheDir := t.TempDir()
	for _, sub := range []string{"repos", "downloads", "artifacts"} {
		path := filepath.Join(cacheDir, sub)
		if err := os.MkdirAll(path, 0o2775); err != nil {
			t.Fatal(err)
		}
	}
	return cacheDir
}

func withMockCelerGroup(t *testing.T, gid int) {
	t.Helper()
	oldLookup := lookupGroup
	oldGetGroups := getProcessGroups
	t.Cleanup(func() {
		lookupGroup = oldLookup
		getProcessGroups = oldGetGroups
	})

	gidStr := strconv.Itoa(gid)
	lookupGroup = func(name string) (*user.Group, error) {
		return &user.Group{Name: nfsUser, Gid: gidStr}, nil
	}
	getProcessGroups = func() ([]int, error) {
		return []int{gid}, nil
	}
}

func fileExists(path string) bool {
	_, err := os.Stat(path)
	return err == nil
}
