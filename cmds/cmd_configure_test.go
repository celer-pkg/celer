package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/encrypt"
	"celer/pkgs/expr"
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5")
	)
	check(celer.SetPlatform(platform))
	if celer.Platform().GetName() != platform {
		t.Fatalf("platform should be `%s`", platform)
	}

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if celer2.Platform().GetName() != platform {
		t.Fatalf("platform should be `%s`", platform)
	}
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	const projectName = "project_test_01"
	check(celer.SetProject(projectName))
	if celer.Project().GetName() != projectName {
		t.Fatalf("project should be `%s`", projectName)
	}

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if celer2.Project().GetName() != projectName {
		t.Fatalf("project should be `%s`", projectName)
	}
}

func TestConfigure_Project_NotExist(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	if err := celer.SetProject("xxxx"); err == nil {
		t.Fatal("it should be failed")
	}
}

func TestConfigure_Project_Empty(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	if err := celer.SetProject(""); err == nil {
		t.Fatal("it should be failed")
	}
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	const buildType = "Release"
	check(celer.SetBuildType(buildType))
	if celer.BuildType() != "release" {
		t.Fatalf("build type should be `%s`", "release")
	}

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if celer2.BuildType() != "release" {
		t.Fatalf("build type should be `%s`", "release")
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	const buildType = "Debug"
	check(celer.SetBuildType(buildType))
	if celer.BuildType() != "debug" {
		t.Fatalf("build type should be `%s`", "debug")
	}

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if celer2.BuildType() != "debug" {
		t.Fatalf("build type should be `%s`", "debug")
	}
}

func TestConfigure_BuildType_Empty(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	if err := celer.SetBuildType(""); err != configs.ErrInvalidBuildType {
		t.Fatal(configs.ErrInvalidBuildType)
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	if err := celer.SetBuildType("xxxx"); err != configs.ErrInvalidBuildType {
		t.Fatal(configs.ErrInvalidBuildType)
	}
}

func TestConfigure_Jobs(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	const jobs = 4
	check(celer.SetJobs(jobs))
	if celer.Jobs() != jobs {
		t.Fatalf("jobs should be `%d`", jobs)
	}

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if celer2.Jobs() != jobs {
		t.Fatalf("jobs should be `%d`", jobs)
	}
}

func TestConfigure_Jobs_Invalid(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	if err := celer.SetJobs(-1); err != configs.ErrInvalidJobs {
		t.Fatal(configs.ErrInvalidJobs)
	}
}

func TestConfigure_Offline_ON(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	const offline = true
	check(celer.SetOffline(offline))
	if celer.Offline() != offline {
		t.Fatalf("offline should be `%v`", offline)
	}

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if celer2.Offline() != offline {
		t.Fatalf("offline should be `%v`", offline)
	}
}

func TestConfigure_Offline_OFF(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	const offline = false
	check(celer.SetOffline(offline))
	if celer.Offline() != offline {
		t.Fatalf("offline should be `%v`", offline)
	}

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if celer2.Offline() != offline {
		t.Fatalf("offline should be `%v`", offline)
	}
}

func TestConfigure_Verbose_ON(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	const verbose = true
	check(celer.SetVerbose(verbose))
	if celer.Verbose() != verbose {
		t.Fatalf("verbose should be `%v`", verbose)
	}

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if celer2.Verbose() != verbose {
		t.Fatalf("verbose should be `%v`", verbose)
	}
}

func TestConfigure_Verbose_OFF(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	const verbose = false
	check(celer.SetVerbose(verbose))
	if celer.Verbose() != verbose {
		t.Fatalf("verbose should be `%v`", verbose)
	}
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

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestCacheDir, os.ModePerm))
	check(celer.SetCacheDir(dirs.TestCacheDir, "token_123456"))

	celer2 := configs.NewCeler()
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	if err := celer.SetCacheDir(dirs.TestCacheDir, "token_123456"); errors.Is(err, configs.ErrCacheDirNotExist) {
		t.Fatal(configs.ErrCacheDirNotExist)
	}
}

func TestConfigure_Proxy(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	check(celer.SetProxy("127.0.0.1", 7890))
	host, port := celer.Proxy()
	if host != "127.0.0.1" {
		t.Fatalf("proxy host should be `%s`", "127.0.0.1")
	}
	if port != 7890 {
		t.Fatalf("proxy port should be `%d`", 7890)
	}
}

func TestConfigure_Proxy_Invalid_Host(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	err := celer.SetProxy("", 7890)
	if err != configs.ErrProxyInvalidHost {
		t.Fatal("it should be failed due to invalid host")
	}
}

func TestConfigure_Proxy_Invalid_Port(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.SetConfRepo("https://github.com/celer-pkg/test-conf.git", "feature/support_clang"))
	check(celer.SetBuildType("Release"))

	err := celer.SetProxy("127.0.0.1", -1)
	if err != configs.ErrProxyInvalidPort {
		t.Fatal("it should be failed due to invalid port")
	}
}
