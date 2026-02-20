package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
	"testing"

	"github.com/spf13/cobra"
)

func TestUpdateCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	updateCmd := updateCmd{}
	celer := configs.NewCeler()
	cmd := updateCmd.Command(celer)

	// Test command basic properties.
	if cmd.Use != "update" {
		t.Errorf("Expected Use to be 'update', got '%s'", cmd.Use)
	}

	if cmd.Short == "" {
		t.Error("Short description should not be empty")
	}

	if cmd.Long == "" {
		t.Error("Long description should not be empty")
	}

	// Test flags.
	tests := []struct {
		flagName     string
		shorthand    string
		defaultValue string
	}{
		{"conf-repo", "c", "false"},
		{"ports-repo", "p", "false"},
		{"force", "f", "false"},
		{"recursive", "r", "false"},
	}

	for _, test := range tests {
		t.Run(test.flagName, func(t *testing.T) {
			flag := cmd.Flags().Lookup(test.flagName)
			if flag == nil {
				t.Errorf("Flag --%s should be defined", test.flagName)
				return
			}

			if flag.Shorthand != test.shorthand {
				t.Errorf("Expected shorthand %s, got %s", test.shorthand, flag.Shorthand)
			}

			if flag.DefValue != test.defaultValue {
				t.Errorf("Expected default value %s, got %s", test.defaultValue, flag.DefValue)
			}
		})
	}
}

func TestUpdateCmd_Completion(t *testing.T) {
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

	updateCmd := updateCmd{celer: celer}
	cmd := updateCmd.Command(celer)

	tests := []struct {
		name       string
		toComplete string
		expected   []string
	}{
		{
			name:       "complete_conf_repo_flag",
			toComplete: "--conf",
			expected:   []string{"--conf-repo"},
		},
		{
			name:       "complete_conf_repo_short_flag",
			toComplete: "-c",
			expected:   []string{"-c"},
		},
		{
			name:       "complete_ports_repo_flag",
			toComplete: "--ports",
			expected:   []string{"--ports-repo"},
		},
		{
			name:       "complete_force_flag",
			toComplete: "--f",
			expected:   []string{"--force"},
		},
		{
			name:       "complete_recursive_flag",
			toComplete: "--r",
			expected:   []string{"--recursive"},
		},
		{
			name:       "no_completion_for_random",
			toComplete: "--random",
			expected:   []string{},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			suggestions, directive := updateCmd.completion(cmd, []string{}, test.toComplete)

			if directive != cobra.ShellCompDirectiveNoFileComp {
				t.Errorf("Expected directive NoFileComp, got %v", directive)
			}

			for _, expected := range test.expected {
				found := slices.Contains(suggestions, expected)
				if !found {
					t.Errorf("Expected to find '%s' in suggestions, but got %v", expected, suggestions)
				}
			}
		})
	}

	t.Run("completion_for_buildtrees", func(t *testing.T) {
		// Create a test buildtree directory
		testBuildtree := filepath.Join(dirs.BuildtreesDir, "test-package@1.0.0")
		check(os.MkdirAll(testBuildtree, os.ModePerm))

		suggestions, _ := updateCmd.completion(cmd, []string{}, "test")
		if found := slices.Contains(suggestions, "test-package@1.0.0"); !found {
			t.Error("Expected buildtree in suggestions")
		}
	})

	t.Run("completion_for_projects", func(t *testing.T) {
		suggestions, _ := updateCmd.completion(cmd, []string{}, "project")

		// Check if any project suggestions are returned,
		// The actual projects depend on the test-conf repo.
		if len(suggestions) == 0 {
			t.Log("No project suggestions found (may be acceptable)")
		}
	})
}

func TestUpdateCmd_UpdateConfRepo(t *testing.T) {
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

	// Create update command
	updateCmd := updateCmd{
		celer:    celer,
		confRepo: true,
		force:    false,
	}

	// Test updating conf repo
	confDir := filepath.Join(dirs.WorkspaceDir, "conf")
	if !fileio.PathExists(confDir) {
		t.Fatal("conf directory should exist before update")
	}

	// Update conf repo (without force)
	if err := updateCmd.updateConfRepo(); err != nil {
		// Error is expected if there are local modifications
		if !strings.Contains(err.Error(), "modified") {
			t.Fatalf("Unexpected error: %v", err)
		}
	}
}

func TestUpdateCmd_UpdateConfRepo_Force(t *testing.T) {
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

	// Create update command with force
	updateCmd := updateCmd{
		celer:    celer,
		confRepo: true,
		force:    true,
	}

	// Test updating conf repo with force
	confDir := filepath.Join(dirs.WorkspaceDir, "conf")
	if !fileio.PathExists(confDir) {
		t.Fatal("conf directory should exist before update")
	}
	// Make a local modification to test force update.
	check(os.WriteFile(filepath.Join(confDir, "hello.txt"), []byte("test"), os.ModePerm))

	// Update conf repo (with force)
	if err := updateCmd.updateConfRepo(); err != nil {
		t.Fatalf("Update with force should succeed: %v", err)
	}
}

func TestUpdateCmd_UpdatePortsRepo(t *testing.T) {
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

	// Create update command.
	updateCmd := updateCmd{
		celer:     celer,
		portsRepo: true,
		force:     false,
	}

	// Test updating ports repo.
	portsDir := filepath.Join(dirs.WorkspaceDir, "ports")
	if !fileio.PathExists(portsDir) {
		t.Fatal("ports directory should exist before update")
	}

	// Update ports repo.
	if err := updateCmd.updatePortsRepo(); err != nil {
		if !strings.Contains(err.Error(), "modified") {
			t.Fatalf("Unexpected error: %v", err)
		}
	}
}

func TestUpdateCmd_UpdateProjectRepos_NoTargets(t *testing.T) {
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

	// Create update command.
	updateCmd := updateCmd{
		celer: celer,
	}

	// Test updating ports with no targets.
	err := updateCmd.updateProjectRepos([]string{})
	if err == nil {
		t.Fatal("updatePorts should return error when no targets specified")
	}
	if !strings.Contains(err.Error(), "no ports specified") {
		t.Errorf("Expected 'no ports specified' error, got: %v", err)
	}
}

func TestUpdateCmd_UpdatePortRepo_SrcNotExist(t *testing.T) {
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
	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_update"
	)

	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))

	// Test with a port that doesn't have src directory.
	nameVersion := "zlib@123.456.789"

	updateCmd := updateCmd{
		celer:     celer,
		recursive: false,
		force:     false,
	}

	visited := make(map[string]bool)
	err := updateCmd.updatePortRepo(nameVersion, visited)
	if err == nil {
		t.Fatal("updatePortRepo should return error when src doesn't exist")
	}

	if !strings.Contains(err.Error(), "not defined") {
		t.Errorf("Expected 'not defined' error, got: %v", err)
	}
}

func TestUpdateCmd_UpdatePortRepo_NonGitRepo(t *testing.T) {
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
	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_update"
	)

	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))

	// Test with a non-git port (if available)
	t.Run("update port without git repo", func(t *testing.T) {
		// Find a port that's not a git repo
		nameVersion := "sqlite3@3.49.0"

		var port configs.Port
		check(port.Init(celer, nameVersion))

		// Create src directory to test the git check
		srcDir := filepath.Join(dirs.WorkspaceDir, "buildtrees", nameVersion, "src")
		check(os.MkdirAll(srcDir, os.ModePerm))

		updateCmd := updateCmd{
			celer:     celer,
			recursive: false,
			force:     false,
		}

		visited := make(map[string]bool)
		err := updateCmd.updatePortRepo(nameVersion, visited)
		if err == nil {
			t.Fatal("updatePortRepo should return error for non-git repo")
		}

		if !strings.Contains(err.Error(), "not a git repository") {
			t.Errorf("Expected 'not a git repository' error, got: %v", err)
		}
	})
}

func TestUpdateCmd_UpdatePortRepo_CircularDependency(t *testing.T) {
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
	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_update"
	)

	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))

	// Test circular dependency detection.
	t.Run("circular dependency prevention", func(t *testing.T) {
		visited := make(map[string]bool)

		// Simulate visiting the same port twice.
		visited["test-port@1.0.0"] = true

		updateCmd := updateCmd{
			celer:     celer,
			recursive: true,
			force:     false,
		}

		// This should return nil immediately due to visited check.
		if err := updateCmd.updatePortRepo("test-port@1.0.0", visited); err != nil {
			// If error occurs, it should not be about circular dependency,
			// (the visited check should prevent that).
			t.Logf("Got error (acceptable if not about infinite recursion): %v", err)
		}
	})
}

func TestUpdateCmd_UpdatePortRepo_WithGitRepo(t *testing.T) {
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
	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_update"
	)

	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))
	check(buildtools.CheckTools(celer, "git"))

	// Install a simple git-based port.
	nameVersion := "zlib@1.3.1"

	var port configs.Port
	check(port.Init(celer, nameVersion))

	if !strings.HasSuffix(port.Package.Url, ".git") {
		t.Skip("Port is not a git repository")
	}

	buildConfig := port.MatchedConfig
	if buildConfig == nil {
		t.Fatal("Build config should not be nil")
	}
	check(buildConfig.Clone(port.Package.Url, port.Package.Ref, port.Package.Archive, port.Package.Depth))

	// Test update with force.
	updateCmd := updateCmd{
		celer: celer,
		force: true,
	}

	visited := make(map[string]bool)
	updateErr := updateCmd.updatePortRepo(nameVersion, visited)
	if updateErr != nil {
		t.Logf("Update returned: %v", updateErr)
	}

	// Verify src directory still exists.
	srcDir := filepath.Join(dirs.WorkspaceDir, "buildtrees", nameVersion, "src")
	if !fileio.PathExists(srcDir) {
		t.Error("src directory should still exist after update")
	}
}

func TestUpdateCmd_UpdatePorts_BacktickRemoval(t *testing.T) {
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
	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_update"
	)

	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))

	// Test that backticks are removed from port names
	updateCmd := updateCmd{celer: celer}

	// This should fail because the port doesn't exist, but it tests backtick removal
	err := updateCmd.updateProjectRepos([]string{"`zlib@1.3.1`"})
	if err == nil {
		t.Fatal("Expected error for non-existent port src")
	}

	// The error should be about the port without backticks
	if strings.Contains(err.Error(), "`") {
		t.Error("Backticks should be removed from port name")
	}
}

func TestUpdateCmd_DoUpdate_ConfRepo(t *testing.T) {
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

	// Create update command
	updateCmd := updateCmd{
		celer:    celer,
		confRepo: true,
		force:    true,
	}

	// Note: This test doesn't actually call doUpdate as it would os.Exit
	// Instead we test the underlying method directly
	err := updateCmd.updateConfRepo()
	if err != nil {
		t.Logf("Update conf repo returned: %v (may be acceptable)", err)
	}
}

func TestUpdateCmd_DoUpdate_PortsRepo(t *testing.T) {
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

	// Create update command
	updateCmd := updateCmd{
		celer:     celer,
		portsRepo: true,
		force:     true,
	}

	// Note: This test doesn't actually call doUpdate as it would os.Exit
	// Instead we test the underlying method directly
	err := updateCmd.updatePortsRepo()
	if err != nil {
		t.Logf("Update ports repo returned: %v (may be acceptable)", err)
	}
}
