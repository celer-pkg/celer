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
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	stderr, err := runCommand(t, cmd.Command(celer))
	if err == nil {
		t.Fatal("expected error when no configuration flag is provided")
	}

	if !errors.Is(err, errors.ErrNoConfigFlagProvided) {
		t.Fatalf("stderr should report missing flag, got:\n%s", stderr)
	}
}

func TestConfigureCmd_PkgCacheGroupShouldSucceed(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	if err := os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	if _, err := runCommand(t, cmd.Command(celer),
		"--pkgcache-dir="+dirs.TestPkgCacheDir,
		"--pkgcache-writable=true",
	); err != nil {
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
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	stderr, err := runCommand(t, cmd.Command(celer),
		"--proxy-host=127.0.0.1",
		"--ccache-enabled=true",
	)
	if err == nil {
		t.Fatal("expected error when flags from different groups are provided")
	}
	if !strings.Contains(stderr, "different groups") {
		t.Fatalf("stderr should report cross-group violation, got:\n%s", stderr)
	}
}

func TestConfigure_Platform(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
	)

	if _, err := runCommand(t, cmd.Command(celer), "--platform="+platform); err != nil {
		t.Fatal(err)
	}
	if celer.Platform().GetName() != platform {
		t.Fatalf("platform should be `%s`", platform)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if celer2.Platform().GetName() != platform {
		t.Fatalf("platform should be `%s`", platform)
	}
}

func TestConfigure_Project(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	const projectName = "project_test_01"
	if _, err := runCommand(t, cmd.Command(celer), "--project="+projectName); err != nil {
		t.Fatal(err)
	}
	if celer.Project().GetName() != projectName {
		t.Fatalf("project should be `%s`", projectName)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if celer2.Project().GetName() != projectName {
		t.Fatalf("project should be `%s`", projectName)
	}
}

func TestConfigure_Project_NotExist(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	stderr, err := runCommand(t, cmd.Command(celer), "--project=xxxx")
	if err == nil {
		t.Fatal("it should be failed")
	}
	if !strings.Contains(stderr, "project not exist") {
		t.Fatalf("stderr should report missing project, got:\n%s", stderr)
	}
}

func TestConfigure_Project_Empty(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	// `--project=` (empty value) reaches RunE with the flag marked changed,
	// so the project setter is invoked with "" and must reject it.
	if _, err := runCommand(t, cmd.Command(celer), "--project="); err == nil {
		t.Fatal("it should be failed")
	}
}

func TestConfigure_BuildType_Release(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	if _, err := runCommand(t, cmd.Command(celer), "--build-type=Release"); err != nil {
		t.Fatal(err)
	}
	if celer.BuildType() != "release" {
		t.Fatalf("build type should be `%s`", "release")
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if celer2.BuildType() != "release" {
		t.Fatalf("build type should be `%s`", "release")
	}
}

func TestConfigure_BuildType_Debug(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	if _, err := runCommand(t, cmd.Command(celer), "--build-type=Debug"); err != nil {
		t.Fatal(err)
	}
	if celer.BuildType() != "debug" {
		t.Fatalf("build type should be `%s`", "debug")
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if celer2.BuildType() != "debug" {
		t.Fatalf("build type should be `%s`", "debug")
	}
}

func TestConfigure_BuildType_Empty(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	stderr, err := runCommand(t, cmd.Command(celer), "--build-type=")
	if err == nil {
		t.Fatal("expected error for empty build type")
	}
	if !strings.Contains(stderr, "invalid build type") {
		t.Fatalf("stderr should report invalid build type, got:\n%s", stderr)
	}
}

func TestConfigure_BuildType_Invalid(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	stderr, err := runCommand(t, cmd.Command(celer), "--build-type=xxxx")
	if err == nil {
		t.Fatal("expected error for invalid build type")
	}
	if !strings.Contains(stderr, "invalid build type") {
		t.Fatalf("stderr should report invalid build type, got:\n%s", stderr)
	}
}

func TestConfigure_Jobs(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	if _, err := runCommand(t, cmd.Command(celer), "--jobs=4"); err != nil {
		t.Fatal(err)
	}
	if celer.Jobs() != 4 {
		t.Fatalf("jobs should be `%d`", 4)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if celer2.Jobs() != 4 {
		t.Fatalf("jobs should be `%d`", 4)
	}
}

func TestConfigure_Jobs_Invalid(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	stderr, err := runCommand(t, cmd.Command(celer), "--jobs=-1")
	if err == nil {
		t.Fatal("expected error for invalid jobs")
	}
	if !strings.Contains(stderr, "invalid jobs") {
		t.Fatalf("stderr should report invalid jobs, got:\n%s", stderr)
	}
}

func TestConfigure_Offline_ON(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	if _, err := runCommand(t, cmd.Command(celer), "--offline=true"); err != nil {
		t.Fatal(err)
	}
	if !celer.Offline() {
		t.Fatal("offline should be `true`")
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if !celer2.Offline() {
		t.Fatal("offline should be `true`")
	}
}

func TestConfigure_Offline_OFF(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	if _, err := runCommand(t, cmd.Command(celer), "--offline=false"); err != nil {
		t.Fatal(err)
	}
	if celer.Offline() {
		t.Fatal("offline should be `false`")
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if celer2.Offline() {
		t.Fatal("offline should be `false`")
	}
}

func TestConfigure_Verbose_ON(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	if _, err := runCommand(t, cmd.Command(celer), "--verbose=true"); err != nil {
		t.Fatal(err)
	}
	if !celer.Verbose() {
		t.Fatal("verbose should be `true`")
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if !celer2.Verbose() {
		t.Fatal("verbose should be `true`")
	}
}

func TestConfigure_Verbose_OFF(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	if _, err := runCommand(t, cmd.Command(celer), "--verbose=false"); err != nil {
		t.Fatal(err)
	}
	if celer.Verbose() {
		t.Fatal("verbose should be `false`")
	}
}

func TestConfigure_PkgCacheDir(t *testing.T) {
	celer := newInitializedCeler(t)

	// Must create cache dir before setting cache dir.
	if err := os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	cmd := &configureCmd{}
	if _, err := runCommand(t, cmd.Command(celer), "--pkgcache-dir="+dirs.TestPkgCacheDir); err != nil {
		t.Fatal(err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if celer2.PkgCache().GetDir(context.PkgCacheDirRoot) != dirs.TestPkgCacheDir {
		t.Fatalf("cache dir should be `%s`", dirs.TestPkgCacheDir)
	}
}

func TestConfigure_PkgCacheWritable(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	// Must create cache dir before setting cache dir.
	if err := os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	// pkgcache-dir and pkgcache-writable are in the same group, so we can set
	// them together in a single command just like a user would.
	if _, err := runCommand(t, cmd.Command(celer),
		"--pkgcache-dir="+dirs.TestPkgCacheDir,
		"--pkgcache-writable=true",
	); err != nil {
		t.Fatal(err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if !celer2.PkgCache().IsWritable() {
		t.Fatal("cache writable should be `true`")
	}
}

func TestConfigure_PkgCacheDir_DirNotExist(t *testing.T) {
	celer := newInitializedCeler(t)

	cmd := &configureCmd{}
	stderr, err := runCommand(t, cmd.Command(celer), "--pkgcache-dir="+dirs.TestPkgCacheDir)
	if err == nil {
		t.Fatal("expected error for package cache dir not exist")
	}
	if !strings.Contains(stderr, "pkgcache dir not exist") {
		t.Fatalf("stderr should report missing pkgcache dir, got:\n%s", stderr)
	}
}

func TestConfigure_Proxy(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	if _, err := runCommand(t, cmd.Command(celer),
		"--proxy-host=127.0.0.1",
		"--proxy-port=7890",
	); err != nil {
		t.Fatal(err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	host, port := celer2.ProxyHostPort()
	if host != "127.0.0.1" {
		t.Fatalf("proxy host should be `%s`", "127.0.0.1")
	}
	if port != 7890 {
		t.Fatalf("proxy port should be `%d`", 7890)
	}
}

func TestConfigure_Proxy_Invalid_Host(t *testing.T) {
	celer := newInitializedCeler(t)

	cmd := &configureCmd{}
	stderr, err := runCommand(t, cmd.Command(celer), "--proxy-host=")
	if err == nil {
		t.Fatal("it should be failed due to invalid host")
	}
	if !strings.Contains(stderr, "proxy host is invalid") {
		t.Fatalf("stderr should report invalid host, got:\n%s", stderr)
	}
}

func TestConfigure_Proxy_Invalid_Port(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	stderr, err := runCommand(t, cmd.Command(celer), "--proxy-port=-1")
	if err == nil {
		t.Fatal("it should be failed due to invalid port")
	}
	if !strings.Contains(stderr, "proxy port is invalid") {
		t.Fatalf("stderr should report invalid port, got:\n%s", stderr)
	}
}

func TestConfigure_CCacheEnabled_ON(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	ccacheDir := filepath.Join(dirs.TmpDir, "ccache")
	if err := os.MkdirAll(ccacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	// Two configure runs: ccache-dir then ccache-enabled (same group, but
	// SetCCacheEnabled writes via a separate setter — the flag-group rule
	// only forbids mixing *groups*, so two single-flag runs are fine).
	if _, err := runCommand(t, cmd.Command(celer), "--ccache-dir="+ccacheDir); err != nil {
		t.Fatal(err)
	}
	if _, err := runCommand(t, cmd.Command(celer), "--ccache-enabled=true"); err != nil {
		t.Fatal(err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if !celer2.CCache.Enabled {
		t.Fatalf("ccache enabled should be `%v`", true)
	}
}

func TestConfigure_CCacheEnabled_OFF(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	ccacheDir := filepath.Join(dirs.TmpDir, "ccache")
	if err := os.MkdirAll(ccacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	if _, err := runCommand(t, cmd.Command(celer), "--ccache-dir="+ccacheDir); err != nil {
		t.Fatal(err)
	}
	if _, err := runCommand(t, cmd.Command(celer), "--ccache-enabled=false"); err != nil {
		t.Fatal(err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if celer2.CCache.Enabled {
		t.Fatalf("ccache enabled should be `%v`", false)
	}
}

func TestConfigure_CCacheDir(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	ccacheDir := filepath.Join(dirs.TmpDir, "ccache")
	if err := os.MkdirAll(ccacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if _, err := runCommand(t, cmd.Command(celer), "--ccache-dir="+ccacheDir); err != nil {
		t.Fatal(err)
	}

	// Verify by reloading config and setting the same dir again — should be
	// idempotent and persisted in celer.toml.
	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if _, err := runCommand(t, cmd.Command(celer), "--ccache-dir="+ccacheDir); err != nil {
		t.Fatal(err)
	}
}

func TestConfigure_CCacheMaxSize(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	const maxSize = "10G"
	if _, err := runCommand(t, cmd.Command(celer), "--ccache-maxsize="+maxSize); err != nil {
		t.Fatal(err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if _, err := runCommand(t, cmd.Command(celer), "--ccache-maxsize="+maxSize); err != nil {
		t.Fatal(err)
	}
}

func TestConfigure_BuildType_RelWithDebInfo(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	if _, err := runCommand(t, cmd.Command(celer), "--build-type=RelWithDebInfo"); err != nil {
		t.Fatal(err)
	}
	if celer.BuildType() != "relwithdebinfo" {
		t.Fatalf("build type should be `%s`", "relwithdebinfo")
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if celer2.BuildType() != "relwithdebinfo" {
		t.Fatalf("build type should be `%s`", "relwithdebinfo")
	}
}

func TestConfigure_BuildType_MinSizeRel(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	if _, err := runCommand(t, cmd.Command(celer), "--build-type=MinSizeRel"); err != nil {
		t.Fatal(err)
	}
	if celer.BuildType() != "minsizerel" {
		t.Fatalf("build type should be `%s`", "minsizerel")
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if celer2.BuildType() != "minsizerel" {
		t.Fatalf("build type should be `%s`", "minsizerel")
	}
}

func TestConfigure_Jobs_Zero(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	stderr, err := runCommand(t, cmd.Command(celer), "--jobs=0")
	if err == nil {
		t.Fatal("jobs cannot be 0")
	}
	if !strings.Contains(stderr, "invalid jobs") {
		t.Fatalf("stderr should report invalid jobs, got:\n%s", stderr)
	}
}

func TestConfigure_CCacheRemoteStorage_Valid(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	const remoteStorage = "http://localhost:8080/ccache"
	if _, err := runCommand(t, cmd.Command(celer), "--ccache-remote-storage="+remoteStorage); err != nil {
		t.Fatal(err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if _, err := runCommand(t, cmd.Command(celer), "--ccache-remote-storage="+remoteStorage); err != nil {
		t.Fatal(err)
	}
}

func TestConfigure_CCacheRemoteStorage_InvalidURL(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	stderr, err := runCommand(t, cmd.Command(celer), "--ccache-remote-storage=localhost:8080/ccache")
	if err == nil {
		t.Fatal("should fail for URL without scheme")
	}
	if !strings.Contains(stderr, "remote storage URL") {
		t.Fatalf("stderr should report invalid remote storage URL, got:\n%s", stderr)
	}
}

func TestConfigure_CCacheRemoteStorage_Empty(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	// Empty string should be allowed (to clear the setting).
	if _, err := runCommand(t, cmd.Command(celer), "--ccache-remote-storage="); err != nil {
		t.Fatal(err)
	}
}

func TestConfigure_Downloads(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	downloads := filepath.Join(dirs.TmpDir, "downloads")
	if err := os.MkdirAll(downloads, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if _, err := runCommand(t, cmd.Command(celer), "--downloads="+downloads); err != nil {
		t.Fatal(err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if celer2.Downloads() != downloads {
		t.Fatalf("downloads dir should be `%s`, got `%s`", downloads, celer2.Downloads())
	}
}

func TestConfigure_Downloads_NotExist(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	stderr, err := runCommand(t, cmd.Command(celer),
		"--downloads="+filepath.Join(dirs.TmpDir, "downloads-does-not-exist"),
	)
	if err == nil {
		t.Fatal("expected error when downloads dir does not exist")
	}
	if !strings.Contains(stderr, "downloads dir") {
		t.Fatalf("stderr should report missing downloads dir, got:\n%s", stderr)
	}
}

func TestConfigure_CCacheRemoteOnly_ON(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	if _, err := runCommand(t, cmd.Command(celer), "--ccache-remote-only=true"); err != nil {
		t.Fatal(err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if !celer2.CCache.RemoteOnly {
		t.Fatal("ccache remote-only should be `true`")
	}
}

func TestConfigure_CCacheRemoteOnly_OFF(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	// Flip it on then off so the assertion is meaningful.
	if _, err := runCommand(t, cmd.Command(celer), "--ccache-remote-only=true"); err != nil {
		t.Fatal(err)
	}
	if _, err := runCommand(t, cmd.Command(celer), "--ccache-remote-only=false"); err != nil {
		t.Fatal(err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if celer2.CCache.RemoteOnly {
		t.Fatal("ccache remote-only should be `false`")
	}
}

func TestConfigure_PkgCacheCacheArtifacts(t *testing.T) {
	celer := newInitializedCeler(t)

	// Must create cache dir before setting cache dir; --pkgcache-cache-artifacts
	// requires that the pkgcache dir is already configured (same group, runs in
	// one command).
	if err := os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	cmd := &configureCmd{}
	if _, err := runCommand(t, cmd.Command(celer),
		"--pkgcache-dir="+dirs.TestPkgCacheDir,
		"--pkgcache-cache-artifacts=true",
	); err != nil {
		t.Fatal(err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if !celer2.PkgCache().GetCacheArtifacts() {
		t.Fatal("pkgcache cache-artifacts should be `true`")
	}
}

func TestConfigure_PkgCacheCacheDownloads(t *testing.T) {
	celer := newInitializedCeler(t)

	if err := os.MkdirAll(dirs.TestPkgCacheDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	cmd := &configureCmd{}
	if _, err := runCommand(t, cmd.Command(celer),
		"--pkgcache-dir="+dirs.TestPkgCacheDir,
		"--pkgcache-cache-downloads=true",
	); err != nil {
		t.Fatal(err)
	}

	celer2 := configs.NewCeler()
	if err := celer2.Init(); err != nil {
		t.Fatal(err)
	}
	if !celer2.PkgCache().GetCacheDownloads() {
		t.Fatal("pkgcache cache-downloads should be `true`")
	}
}

// Regression: calling CacheArtifacts/CacheDownloads when PkgCache is nil
// used to panic by dereferencing c.configData.PkgCache.Dir before checking
// for nil. The setters must now return ErrPkgCacheDirEmpty instead.
func TestConfigure_PkgCacheCacheArtifacts_BeforeDirFails(t *testing.T) {
	celer := newInitializedCeler(t)

	cmd := &configureCmd{}
	stderr, err := runCommand(t, cmd.Command(celer),
		"--pkgcache-cache-artifacts=true",
	)
	if err == nil {
		t.Fatal("expected error when pkgcache dir has not been configured")
	}
	if !strings.Contains(stderr, "pkgcache dir") {
		t.Fatalf("stderr should report missing pkgcache dir, got:\n%s", stderr)
	}
}

func TestConfigure_PkgCacheCacheDownloads_BeforeDirFails(t *testing.T) {
	celer := newInitializedCeler(t)

	cmd := &configureCmd{}
	stderr, err := runCommand(t, cmd.Command(celer),
		"--pkgcache-cache-downloads=true",
	)
	if err == nil {
		t.Fatal("expected error when pkgcache dir has not been configured")
	}
	if !strings.Contains(stderr, "pkgcache dir") {
		t.Fatalf("stderr should report missing pkgcache dir, got:\n%s", stderr)
	}
}

func TestConfigureCmd_Port_UpdatesUrlAndRef(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	const (
		nameVersion = "eigen@3.4.0"
		newURL      = "https://example.com/eigen.git"
		newRef      = "test-branch"
	)
	portFile := dirs.GetPortPath("eigen", "3.4.0")
	original, err := os.ReadFile(portFile)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() {
		_ = os.WriteFile(portFile, original, os.ModePerm)
	})

	if _, err := runCommand(t, cmd.Command(celer),
		"--port="+nameVersion,
		"--port-url="+newURL,
		"--port-ref="+newRef,
	); err != nil {
		t.Fatalf("expected success when configuring port url/ref, got: %v", err)
	}

	updated, err := os.ReadFile(portFile)
	if err != nil {
		t.Fatal(err)
	}
	got := string(updated)
	if !strings.Contains(got, newURL) {
		t.Fatalf("port file should contain url %q, got:\n%s", newURL, got)
	}
	if !strings.Contains(got, newRef) {
		t.Fatalf("port file should contain ref %q, got:\n%s", newRef, got)
	}
}

func TestConfigureCmd_Port_MissingUrlAndRefShouldFail(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	_, err := runCommand(t, cmd.Command(celer), "--port=eigen@3.4.0")
	if err == nil {
		t.Fatal("expected error when --port is provided without --port-url or --port-ref")
	}
}

func TestConfigureCmd_Port_UnknownPortShouldFail(t *testing.T) {
	celer := newInitializedCeler(t)
	cmd := &configureCmd{}

	_, err := runCommand(t, cmd.Command(celer),
		"--port=this-port-does-not-exist@1.0.0",
		"--port-url=https://example.com/foo.git",
	)
	if err == nil {
		t.Fatal("expected error when --port refers to a non-existent port")
	}
}
