package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"errors"
	"os"
	"path/filepath"
	"runtime"
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

func TestConfigure_BuildType_Release(t *testing.T) {
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
	check(celer.SetBuildType("Release"))

	if celer.BuildType() != "release" {
		t.Fatalf("build type should be `release`")
	}
}

func TestConfigure_BuildType_Debug(t *testing.T) {
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
	check(celer.SetBuildType("Debug"))

	if celer.BuildType() != "debug" {
		t.Fatalf("build type should be `debug`")
	}
}

func TestConfigure_BuildType_Invalid(t *testing.T) {
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

	if err := celer.SetBuildType("xxxx"); err != ErrInvalidBuildType {
		t.Fatal(ErrInvalidBuildType)
	}
}

func TestConfigure_JobNum(t *testing.T) {
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
	check(celer.SetJobNum(4))

	if celer.JobNum() != 4 {
		t.Fatalf("job num should be `4`")
	}
}

func TestConfigure_JobNum_Invalid(t *testing.T) {
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

	if err := celer.SetJobNum(-1); err != ErrInvalidJobNum {
		t.Fatal(ErrInvalidJobNum)
	}
}

func TestConfigure_Offline(t *testing.T) {
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
	check(celer.SetProject("test_project_01"))
	check(celer.SetPlatform(expr.If(runtime.GOOS == "windows", "x86_64-windows-msvc-14.44", "x86_64-linux-ubuntu-22.04")))
	check(celer.SetOffline(true))

	if celer.Global.Offline != true {
		t.Fatalf("offline should be `true`")
	}

	if fileio.PathExists(dirs.DownloadedDir) {
		check(os.Rename(dirs.DownloadedDir, dirs.DownloadedDir+".bak"))
	}

	t.Cleanup(func() {
		check(os.RemoveAll(dirs.DownloadedDir))
		check(os.Rename(dirs.DownloadedDir+".bak", dirs.DownloadedDir))
	})

	if err := celer.Platform().Setup(); err == nil || !errors.Is(err, fileio.ErrOffline) {
		t.Fatal("setup should fail due to offline")
	}
}

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
	if celer2.CacheDir().Dir != dirs.TestCacheDir {
		t.Fatalf("cache dir should be `%s`", dirs.TestCacheDir)
	}

	if celer2.CacheDir().Token != "token_123456" {
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

	if err := celer.SetCacheDir(dirs.TestCacheDir, "token_123456"); err != ErrCacheDirNotExist {
		t.Fatal(ErrCacheDirNotExist)
	}
}
