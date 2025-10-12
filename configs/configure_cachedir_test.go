package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/encrypt"
	"errors"
	"os"
	"path/filepath"
	"testing"
)

func TestConfigure_CacheDir_Success(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	t.Cleanup(func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
	})

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))

	// Init celer.
	celer := NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))

	celer2 := NewCeler()
	check(celer2.Init())
	if celer2.CacheDir().GetDir() != dirs.TestCacheDir {
		t.Fatalf("cache dir should be `%s`", dirs.TestCacheDir)
	}

	if !encrypt.CheckToken(dirs.TestCacheDir, "token_123456") {
		t.Fatalf("cache token should be `token_123456`")
	}
}

func TestConfigure_CacheDir_DirNotExist(t *testing.T) {
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	t.Cleanup(func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
	})

	// Init celer.
	celer := NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", ""))

	if err := celer.SetCacheDir(dirs.TestCacheDir, "token_123456"); errors.Is(err, ErrCacheDirNotExist) {
		t.Fatal(ErrCacheDirNotExist)
	}
}
