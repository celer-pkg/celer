package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/encrypt"
	"errors"
	"os"
	"path/filepath"
	"testing"
)

func TestConfigure(t *testing.T) {
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

	// ============= Configure platform ============= //
	t.Run("Configure platform success", func(t *testing.T) {
		const newName = "x86_64-linux-ubuntu-22.04-gcc-11.5"
		check(celer.SetPlatform(newName))
		if celer.Platform().GetName() != newName {
			t.Fatalf("platform should be `%s`", newName)
		}

		celer2 := configs.NewCeler()
		check(celer2.Init())
		if celer2.Platform().GetName() != newName {
			t.Fatalf("platform should be `%s`", newName)
		}
	})

	t.Run("Configure platform failed: None Exist Platform", func(t *testing.T) {
		if err := celer.SetPlatform("xxxx"); err == nil {
			t.Fatal("it should be failed")
		}
	})

	t.Run("Configure platform failed: Empty Platform", func(t *testing.T) {
		if err := celer.SetPlatform(""); err != nil {
			if err.Error() != "platform name is empty" {
				t.Fatal("error should be 'platform name is empty'")
			}
		} else {
			t.Fatal("it should be failed")
		}
	})

	// ============= Configure project ============= //
	t.Run("configure project success", func(t *testing.T) {
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
	})

	t.Run("configure project failed: none exist project", func(t *testing.T) {
		if err := celer.SetProject("xxxx"); err == nil {
			t.Fatal("it should be failed")
		}
	})

	t.Run("configure project failed: empty project", func(t *testing.T) {
		if err := celer.SetProject(""); err == nil {
			t.Fatal("it should be failed")
		}
	})

	// ============= Configure build type ============= //
	t.Run("configure build type as release", func(t *testing.T) {
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
	})

	t.Run("configure build type as debug", func(t *testing.T) {
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
	})

	t.Run("configure build type failed: empty build type", func(t *testing.T) {
		if err := celer.SetBuildType(""); err != configs.ErrInvalidBuildType {
			t.Fatal(configs.ErrInvalidBuildType)
		}
	})

	t.Run("configure build type failed: invalid build type", func(t *testing.T) {
		if err := celer.SetBuildType("xxxx"); err != configs.ErrInvalidBuildType {
			t.Fatal(configs.ErrInvalidBuildType)
		}
	})

	// ============= Configure jobs ============= //
	t.Run("configure jobs success", func(t *testing.T) {
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
	})

	t.Run("configure jobs failed: invalid jobs", func(t *testing.T) {
		if err := celer.SetJobs(-1); err != configs.ErrInvalidJobs {
			t.Fatal(configs.ErrInvalidJobs)
		}
	})

	// ============= Configure offline ============= //
	t.Run("configure offline on", func(t *testing.T) {
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
	})

	t.Run("configure offline off", func(t *testing.T) {
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
	})

	// ============= Configure verbose ============= //
	t.Run("configure verbose on", func(t *testing.T) {
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
	})

	t.Run("configure verbose off", func(t *testing.T) {
		const verbose = false
		check(celer.SetVerbose(verbose))
		if celer.Verbose() != verbose {
			t.Fatalf("verbose should be `%v`", verbose)
		}
	})

	// ============= Configure cache dir ============= //
	t.Run("configure cache dir success", func(t *testing.T) {
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
	})

	t.Run("configure cache dir failed: cache dir not exist", func(t *testing.T) {
		if err := celer.SetCacheDir(dirs.TestCacheDir, "token_123456"); errors.Is(err, configs.ErrCacheDirNotExist) {
			t.Fatal(configs.ErrCacheDirNotExist)
		}
	})

	// ============= Configure proxy ============= //
	t.Run("configure proxy success", func(t *testing.T) {
		check(celer.SetProxy("127.0.0.1", 7890))
		host, port := celer.Proxy()
		if host != "127.0.0.1" {
			t.Fatalf("proxy host should be `%s`", "127.0.0.1")
		}
		if port != 7890 {
			t.Fatalf("proxy port should be `%d`", 7890)
		}

		celer2 := configs.NewCeler()
		check(celer2.Init())
		host, port = celer2.Proxy()
		if host != "127.0.0.1" {
			t.Fatalf("proxy host should be `%s`", "127.0.0.1")
		}
		if port != 7890 {
			t.Fatalf("proxy port should be `%d`", 7890)
		}
	})

	t.Run("configure proxy failed: invalid host", func(t *testing.T) {
		err := celer.SetProxy("", 7890)
		if err != configs.ErrProxyInvalidHost {
			t.Fatal("it should be failed due to invalid host")
		}
	})

	t.Run("configure proxy failed: invalid port", func(t *testing.T) {
		err := celer.SetProxy("127.0.0.1", -1)
		if err != configs.ErrProxyInvalidPort {
			t.Fatal("it should be failed due to invalid port")
		}
	})
}
