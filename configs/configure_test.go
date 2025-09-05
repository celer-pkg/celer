package configs

import (
	"celer/pkgs/dirs"
	"os"
	"path/filepath"
	"testing"
)

func TestConfigure_Platform(t *testing.T) {
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

	t.Run("configure platform success", func(t *testing.T) {
		const newName = "x86_64-linux-ubuntu-22.04"
		check(celer.SetPlatform(newName))
		if celer.platform.Name != newName {
			t.Fatalf("platform should be `%s`", newName)
		}
	})

	t.Run("configure platform error: none exist platform", func(t *testing.T) {
		if err := celer.SetPlatform("xxxx"); err == nil {
			t.Fatal("it should be failed")
		}
	})

	t.Run("configure platform error: empty platform", func(t *testing.T) {
		if err := celer.SetPlatform(""); err != nil {
			if err.Error() != "platform name is empty" {
				t.Fatal("error should be 'platform name is empty'")
			}
		} else {
			t.Fatal("it should be failed")
		}
	})
}

func TestConfigure_Project(t *testing.T) {
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

	t.Run("configure project success", func(t *testing.T) {
		const projectName = "test_project_01"
		if err := celer.SetProject(projectName); err != nil {
			t.Fatal(err)
		}
		if celer.project.Name != projectName {
			t.Fatalf("project should be `%s`", projectName)
		}
	})

	t.Run("configure project error: none exist project", func(t *testing.T) {
		if err := celer.SetProject("xxxx"); err == nil {
			t.Fatal("it should be failed")
		}
	})

	t.Run("configure project error: empty project", func(t *testing.T) {
		if err := celer.SetProject(""); err == nil {
			t.Fatal("it should be failed")
		}
	})
}

func TestConfigure_CacheDir(t *testing.T) {
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
	if celer2.CacheDir().Dir != dirs.TestCacheDir {
		t.Fatalf("cache dir should be `%s`", dirs.TestCacheDir)
	}

	if celer2.CacheDir().Token != "token_123456" {
		t.Fatalf("cache token should be `token_123456`")
	}
}
