package cmds

import (
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/expr"

	"github.com/spf13/cobra"
)

func TestConfigureCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	configCmd := configureCmd{}
	celer := configs.NewCeler()
	cmd := configCmd.Command(celer)

	// Test command basic properties
	if cmd.Use != "configure" {
		t.Errorf("Expected Use to be 'configure', got '%s'", cmd.Use)
	}

	if cmd.Short == "" {
		t.Error("Short description should not be empty")
	}

	// Test flags existence
	expectedFlags := []struct {
		name      string
		shorthand string
	}{
		{"platform", ""},
		{"project", ""},
		{"build-type", ""},
		{"jobs", ""},
		{"offline", ""},
		{"verbose", ""},
		{"downloads", ""},
		{"pkgcache-dir", ""},
		{"pkgcache-writable", ""},
		{"pkgcache-cache-artifacts", ""},
		{"pkgcache-cache-downloads", ""},
		{"proxy-host", ""},
		{"proxy-port", ""},
		{"ccache-enabled", ""},
		{"ccache-dir", ""},
		{"ccache-maxsize", ""},
		{"ccache-remote-storage", ""},
		{"ccache-remote-only", ""},
		{"port", ""},
		{"port-url", ""},
		{"port-ref", ""},
	}

	for _, ef := range expectedFlags {
		flag := cmd.Flags().Lookup(ef.name)
		if flag == nil {
			t.Errorf("--%s flag should be defined", ef.name)
		}
	}
}

func TestConfigureCmd_Completion(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	configCmd := configureCmd{}
	celer := configs.NewCeler()
	cmd := configCmd.Command(celer)

	tests := []struct {
		name       string
		toComplete string
		expected   []string
	}{
		{
			name:       "complete_platform_flag",
			toComplete: "--plat",
			expected:   []string{"--platform"},
		},
		{
			name:       "complete_project_flag",
			toComplete: "--proj",
			expected:   []string{"--project"},
		},
		{
			name:       "complete_build_type_flag",
			toComplete: "--build",
			expected:   []string{"--build-type"},
		},
		{
			name:       "complete_jobs_flag",
			toComplete: "--job",
			expected:   []string{"--jobs"},
		},
		{
			name:       "complete_offline_flag",
			toComplete: "--off",
			expected:   []string{"--offline"},
		},
		{
			name:       "complete_verbose_flag",
			toComplete: "--verb",
			expected:   []string{"--verbose"},
		},
		{
			name:       "complete_downloads_flag",
			toComplete: "--down",
			expected:   []string{"--downloads"},
		},
		{
			name:       "complete_pkgcache_dir_flag",
			toComplete: "--pkgcache-d",
			expected:   []string{"--pkgcache-dir"},
		},
		{
			name:       "complete_pkgcache_writable_flag",
			toComplete: "--pkgcache-w",
			expected:   []string{"--pkgcache-writable"},
		},
		{
			name:       "complete_pkgcache_cache_artifacts_flag",
			toComplete: "--pkgcache-cache-a",
			expected:   []string{"--pkgcache-cache-artifacts"},
		},
		{
			name:       "complete_pkgcache_cache_downloads_flag",
			toComplete: "--pkgcache-cache-d",
			expected:   []string{"--pkgcache-cache-downloads"},
		},
		{
			name:       "complete_proxy_host_flag",
			toComplete: "--proxy-h",
			expected:   []string{"--proxy-host"},
		},
		{
			name:       "complete_proxy_port_flag",
			toComplete: "--proxy-p",
			expected:   []string{"--proxy-port"},
		},
		{
			name:       "complete_ccache_enable_flag",
			toComplete: "--ccache-e",
			expected:   []string{"--ccache-enabled"},
		},
		{
			name:       "complete_ccache_dir_flag",
			toComplete: "--ccache-d",
			expected:   []string{"--ccache-dir"},
		},
		{
			name:       "complete_ccache_maxsize_flag",
			toComplete: "--ccache-m",
			expected:   []string{"--ccache-maxsize"},
		},
		{
			name:       "complete_ccache_remote_storage_flag",
			toComplete: "--ccache-remote-s",
			expected:   []string{"--ccache-remote-storage"},
		},
		{
			name:       "complete_ccache_remote_only_flag",
			toComplete: "--ccache-remote-o",
			expected:   []string{"--ccache-remote-only"},
		},
		{
			name:       "complete_port_flag",
			toComplete: "--port",
			expected:   []string{"--port", "--port-url", "--port-ref"},
		},
		{
			name:       "complete_port_url_flag",
			toComplete: "--port-u",
			expected:   []string{"--port-url"},
		},
		{
			name:       "complete_port_ref_flag",
			toComplete: "--port-r",
			expected:   []string{"--port-ref"},
		},
		{
			name:       "no_completion_for_random",
			toComplete: "--random",
			expected:   []string{},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			suggestions, directive := configCmd.completion(cmd, []string{}, test.toComplete)

			if directive != cobra.ShellCompDirectiveNoFileComp {
				t.Errorf("Expected directive %v, got %v", cobra.ShellCompDirectiveNoFileComp, directive)
			}

			if len(suggestions) != len(test.expected) {
				t.Errorf("Expected %d suggestions, got %d: %v", len(test.expected), len(suggestions), suggestions)
				return
			}

			for i, expected := range test.expected {
				if i < len(suggestions) && suggestions[i] != expected {
					t.Errorf("Expected suggestion[%d] to be %s, got %s", i, expected, suggestions[i])
				}
			}
		})
	}
}

func TestConfigureCmd_NoFlagShouldFail(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Init celer so that checkIfInitialized passes — otherwise the command
	// fails for the wrong reason (uninitialized) instead of no-flag.
	celer := configs.NewCeler()
	if err := celer.Init(); err != nil {
		t.Fatal(err)
	}
	if err := celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true); err != nil {
		t.Fatal(err)
	}

	configCmd := configureCmd{}
	cmd := configCmd.Command(celer)
	cmd.SetArgs([]string{})

	if err := cmd.Execute(); err == nil {
		t.Fatal("expected error when no configuration flag is provided")
	}
}

func TestConfigureCmd_PkgCacheGroupShouldSucceed(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	if err := os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	// Init celer so that checkIfInitialized passes.
	celer := configs.NewCeler()
	if err := celer.Init(); err != nil {
		t.Fatal(err)
	}
	if err := celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true); err != nil {
		t.Fatal(err)
	}

	configCmd := configureCmd{}
	cmd := configCmd.Command(celer)
	cmd.SetArgs([]string{"--pkgcache-dir", dirs.TestPkgCacheDir, "--pkgcache-writable=true"})

	if err := cmd.Execute(); err != nil {
		t.Fatalf("expected success when package-cache group flags are provided, got: %v", err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}

	if celer2.PkgCache().GetDir(context.PkgCacheDirRoot) != dirs.TestPkgCacheDir {
		t.Fatalf("cache dir should be `%s`", dirs.TestPkgCacheDir)
	}
	if !celer2.PkgCache().IsWritable() {
		t.Fatal("cache writable should be `true`")
	}
}

func TestConfigureCmd_CrossGroupFlagsShouldFail(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Init celer so that checkIfInitialized passes — otherwise the command
	// fails for the wrong reason (uninitialized) instead of cross-group flags.
	celer := configs.NewCeler()
	if err := celer.Init(); err != nil {
		t.Fatal(err)
	}
	if err := celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true); err != nil {
		t.Fatal(err)
	}

	configCmd := configureCmd{}
	cmd := configCmd.Command(celer)
	cmd.SetArgs([]string{"--proxy-host=127.0.0.1", "--ccache-enabled=true"})

	if err := cmd.Execute(); err == nil {
		t.Fatal("expected error when flags from different groups are provided")
	}
}

func TestConfigure_Platform(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
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
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
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
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	if err := celer.SetProject("xxxx"); err == nil {
		t.Fatal("it should be failed")
	}
}

func TestConfigure_Project_Empty(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	if err := celer.SetProject(""); err == nil {
		t.Fatal("it should be failed")
	}
}

func TestConfigure_BuildType_Release(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
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
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
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
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	if err := celer.SetBuildType(""); err != errors.ErrInvalidBuildType {
		t.Fatal(errors.ErrInvalidBuildType)
	}
}

func TestConfigure_BuildType_Invalid(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	if err := celer.SetBuildType("xxxx"); err != errors.ErrInvalidBuildType {
		t.Fatal(errors.ErrInvalidBuildType)
	}
}

func TestConfigure_Jobs(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
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
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Cleanup.

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	if err := celer.SetJobs(-1); err == nil {
		t.Fatal("expected error for invalid jobs")
	}
}

func TestConfigure_Offline_ON(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
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
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
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
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
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
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	const verbose = false
	check(celer.SetVerbose(verbose))
	if celer.Verbose() != verbose {
		t.Fatalf("verbose should be `%v`", verbose)
	}
}

func TestConfigure_PkgCacheDir(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))
	check(celer.SetPkgCacheDir(dirs.TestPkgCacheDir))

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if celer2.PkgCache().GetDir(context.PkgCacheDirRoot) != dirs.TestPkgCacheDir {
		t.Fatalf("cache dir should be `%s`", dirs.TestPkgCacheDir)
	}
}

func TestConfigure_PkgCacheWritable(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	// Must create cache dir before setting cache dir.
	check(os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm))
	check(celer.SetPkgCacheDir(dirs.TestPkgCacheDir))
	check(celer.SetPkgCacheWritable(true))

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if !celer2.PkgCache().IsWritable() {
		t.Fatal("cache writable should be `true`")
	}
}

func TestConfigure_PkgCacheDir_DirNotExist(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	if err := celer.SetPkgCacheDir(dirs.TestPkgCacheDir); !errors.Is(err, errors.ErrPkgCacheDirNotExist) {
		t.Fatal("expected error for package cache dir not exist")
	}
}

func TestConfigure_Proxy(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	check(celer.SetProxyHost("127.0.0.1"))
	check(celer.SetProxyPort(7890))
	host, port := celer.ProxyHostPort()
	if host != "127.0.0.1" {
		t.Fatalf("proxy host should be `%s`", "127.0.0.1")
	}
	if port != 7890 {
		t.Fatalf("proxy port should be `%d`", 7890)
	}
}

func TestConfigure_Proxy_Invalid_Host(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	if celer.SetProxyHost("") == nil {
		t.Fatal("it should be failed due to invalid host")
	}
}

func TestConfigure_Proxy_Invalid_Port(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	if celer.SetProxyPort(-1) == nil {
		t.Fatal("it should be failed due to invalid port")
	}
}

func TestConfigure_CCacheEnabled_ON(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	ccacheDir := filepath.Join(dirs.TmpDir, "ccache")
	check(os.MkdirAll(ccacheDir, os.ModePerm))
	check(celer.SetCCacheDir(ccacheDir))
	check(celer.SetCCacheEnabled(true))

	// Verify by reloading config.
	celer2 := configs.NewCeler()
	check(celer2.Init())

	if celer2.CCache.Enabled != true {
		t.Fatalf("ccache enabled should be `%v`", true)
	}
}

func TestConfigure_CCacheEnabled_OFF(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	ccacheDir := filepath.Join(dirs.TmpDir, "ccache")
	check(os.MkdirAll(ccacheDir, os.ModePerm))
	check(celer.SetCCacheDir(ccacheDir))
	check(celer.SetCCacheEnabled(false))

	// Verify by reloading config.
	celer2 := configs.NewCeler()
	check(celer2.Init())

	if celer2.CCache.Enabled != false {
		t.Fatalf("ccache enabled should be `%v`", false)
	}
}

func TestConfigure_CCacheDir(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	ccacheDir := filepath.Join(dirs.TmpDir, "ccache")
	check(os.MkdirAll(ccacheDir, os.ModePerm))
	check(celer.SetCCacheDir(ccacheDir))

	// Verify by reloading config.
	celer2 := configs.NewCeler()
	check(celer2.Init())

	// The value should be persisted in celer.toml,
	// We can verify by setting it again and checking no error.
	check(celer2.SetCCacheDir(ccacheDir))
}

func TestConfigure_CCacheMaxSize(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	const maxSize = "10G"
	check(celer.SetCCacheMaxSize(maxSize))

	// Verify by reloading config.
	celer2 := configs.NewCeler()
	check(celer2.Init())

	// The value should be persisted in celer.toml,
	// We can verify by setting it again and checking no error.
	check(celer2.SetCCacheMaxSize(maxSize))
}

func TestConfigure_BuildType_RelWithDebInfo(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	const buildType = "RelWithDebInfo"
	check(celer.SetBuildType(buildType))
	if celer.BuildType() != "relwithdebinfo" {
		t.Fatalf("build type should be `%s`", "relwithdebinfo")
	}

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if celer2.BuildType() != "relwithdebinfo" {
		t.Fatalf("build type should be `%s`", "relwithdebinfo")
	}
}

func TestConfigure_BuildType_MinSizeRel(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	const buildType = "MinSizeRel"
	check(celer.SetBuildType(buildType))
	if celer.BuildType() != "minsizerel" {
		t.Fatalf("build type should be `%s`", "minsizerel")
	}

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if celer2.BuildType() != "minsizerel" {
		t.Fatalf("build type should be `%s`", "minsizerel")
	}
}

func TestConfigure_Jobs_Zero(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))

	if err := celer.SetJobs(0); err == nil {
		t.Fatal("jobs cannot be 0")
	}
}

func TestConfigure_CCacheRemoteStorage_Valid(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	const remoteStorage = "http://localhost:8080/ccache"
	check(celer.SetCCacheRemoteStorage(remoteStorage))

	// Verify by reloading config.
	celer2 := configs.NewCeler()
	check(celer2.Init())

	// The value should be persisted in celer.toml,
	// We can verify by setting it again and checking no error.
	check(celer2.SetCCacheRemoteStorage(remoteStorage))
}

func TestConfigure_CCacheRemoteStorage_InvalidURL(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	// Test invalid URL (missing scheme)
	if err := celer.SetCCacheRemoteStorage("localhost:8080/ccache"); err == nil {
		t.Fatal("should fail for URL without scheme")
	}
}

func TestConfigure_CCacheRemoteStorage_Empty(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	// Empty string should be allowed (to clear the setting)
	check(celer.SetCCacheRemoteStorage(""))
}

func TestConfigure_Downloads(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	downloads := filepath.Join(dirs.TmpDir, "downloads")
	check(os.MkdirAll(downloads, os.ModePerm))
	check(celer.SetDownloads(downloads))

	celer2 := configs.NewCeler()
	check(celer2.Init())
	if celer2.Downloads() != downloads {
		t.Fatalf("downloads dir should be `%s`, got `%s`", downloads, celer2.Downloads())
	}
}

func TestConfigure_Downloads_NotExist(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	if err := celer.SetDownloads(filepath.Join(dirs.TmpDir, "downloads-does-not-exist")); err == nil {
		t.Fatal("expected error when downloads dir does not exist")
	}
}

func TestConfigure_CCacheRemoteOnly_ON(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	check(celer.SetCCacheRemoteOnly(true))

	// Verify by reloading config.
	celer2 := configs.NewCeler()
	check(celer2.Init())

	if !celer2.CCache.RemoteOnly {
		t.Fatal("ccache remote-only should be `true`")
	}
}

func TestConfigure_CCacheRemoteOnly_OFF(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))

	// Flip it on then off so the assertion is meaningful.
	check(celer.SetCCacheRemoteOnly(true))
	check(celer.SetCCacheRemoteOnly(false))

	// Verify by reloading config.
	celer2 := configs.NewCeler()
	check(celer2.Init())

	if celer2.CCache.RemoteOnly {
		t.Fatal("ccache remote-only should be `false`")
	}
}

func TestConfigureCmd_Port_UpdatesUrlAndRef(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer so that the configure command's checkIfInitialized passes
	// and ports/ is populated.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))

	const (
		nameVersion = "eigen@3.4.0"
		newURL      = "https://example.com/eigen.git"
		newRef      = "test-branch"
	)
	portFile := dirs.GetPortPath("eigen", "3.4.0")
	original, err := os.ReadFile(portFile)
	check(err)
	t.Cleanup(func() {
		_ = os.WriteFile(portFile, original, os.ModePerm)
	})

	configCmd := configureCmd{}
	cmd := configCmd.Command(celer)
	cmd.SetArgs([]string{
		"--port=" + nameVersion,
		"--port-url=" + newURL,
		"--port-ref=" + newRef,
	})

	if err := cmd.Execute(); err != nil {
		t.Fatalf("expected success when configuring port url/ref, got: %v", err)
	}

	updated, err := os.ReadFile(portFile)
	check(err)
	got := string(updated)
	if !strings.Contains(got, newURL) {
		t.Fatalf("port file should contain url %q, got:\n%s", newURL, got)
	}
	if !strings.Contains(got, newRef) {
		t.Fatalf("port file should contain ref %q, got:\n%s", newRef, got)
	}
}

func TestConfigureCmd_Port_MissingUrlAndRefShouldFail(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer so that checkIfInitialized passes.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))

	configCmd := configureCmd{}
	cmd := configCmd.Command(celer)
	cmd.SetArgs([]string{"--port=eigen@3.4.0"})

	if err := cmd.Execute(); err == nil {
		t.Fatal("expected error when --port is provided without --port-url or --port-ref")
	}
}

func TestConfigureCmd_Port_UnknownPortShouldFail(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer so that checkIfInitialized passes.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))

	configCmd := configureCmd{}
	cmd := configCmd.Command(celer)
	cmd.SetArgs([]string{
		"--port=this-port-does-not-exist@1.0.0",
		"--port-url=https://example.com/foo.git",
	})

	if err := cmd.Execute(); err == nil {
		t.Fatal("expected error when --port refers to a non-existent port")
	}
}
