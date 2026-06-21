package cmds

import (
	"os"
	"path/filepath"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/spf13/cobra"
)

func TestInitCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	initCmd := initCmd{}
	celer := configs.NewCeler()
	cmd := initCmd.Command(celer)

	// Test command basic properties.
	if cmd.Use != "init" {
		t.Errorf("Expected Use to be 'init', got '%s'", cmd.Use)
	}

	if cmd.Short == "" {
		t.Error("Short description should not be empty")
	}

	// Test flags.
	urlFlag := cmd.Flags().Lookup("url")
	if urlFlag == nil {
		t.Error("--url flag should be defined")
	} else {
		if urlFlag.Shorthand != "u" {
			t.Errorf("Expected url flag shorthand to be 'u', got '%s'", urlFlag.Shorthand)
		}
	}

	branchFlag := cmd.Flags().Lookup("branch")
	if branchFlag == nil {
		t.Error("--branch flag should be defined")
	} else {
		if branchFlag.Shorthand != "b" {
			t.Errorf("Expected branch flag shorthand to be 'b', got '%s'", branchFlag.Shorthand)
		}
	}
}

func TestInitCmd_Completion(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	initCmd := initCmd{}
	celer := configs.NewCeler()
	cmd := initCmd.Command(celer)

	tests := []struct {
		name       string
		toComplete string
		expected   []string
	}{
		{
			name:       "complete_url_flag",
			toComplete: "--u",
			expected:   []string{"--url"},
		},
		{
			name:       "complete_url_short_flag",
			toComplete: "-u",
			expected:   []string{"-u"},
		},
		{
			name:       "complete_branch_flag",
			toComplete: "--b",
			expected:   []string{"--branch"},
		},
		{
			name:       "complete_branch_short_flag",
			toComplete: "-b",
			expected:   []string{"-b"},
		},
		{
			name:       "no_completion_for_random",
			toComplete: "--random",
			expected:   []string{},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			suggestions, directive := initCmd.completion(cmd, []string{}, test.toComplete)

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

func runInit(t *testing.T, args ...string) (string, error) {
	t.Helper()
	dirs.RemoveAllForTest()
	_ = os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "conf"))

	cmd := &initCmd{}
	return runCommand(t, cmd.Command(configs.NewCeler()), args...)
}

func TestInitCmd_Command(t *testing.T) {
	tests := []struct {
		name        string
		args        []string
		expectError bool
		description string
	}{
		{
			name:        "valid_git_repo",
			args:        []string{"--url=" + test_conf_repo_url},
			expectError: false,
			description: "Should succeed with valid git repository",
		},
		{
			name:        "valid_git_repo_with_branch",
			args:        []string{"--url=" + test_conf_repo_url, "--branch=master"},
			expectError: false,
			description: "Should succeed with valid git repository and specific branch",
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			stderr, err := runInit(t, test.args...)

			if test.expectError && err == nil {
				t.Errorf("Expected error but got none")
			} else if !test.expectError && err != nil {
				t.Errorf("Expected no error but got: %v\nstderr:\n%s", err, stderr)
			}

			// Verify that celer.toml was created.
			celerPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")
			if !fileio.PathExists(celerPath) {
				t.Error("celer.toml should be created after init")
			}

			// Verify conf repo was cloned.
			confDir := filepath.Join(dirs.WorkspaceDir, "conf")
			if !fileio.PathExists(confDir) {
				t.Error("conf directory should be created when URL is provided")
			}
		})
	}
}

func TestInitCmd_Initialize(t *testing.T) {
	// `celer init` without --url must fail at cobra's required-flag check,
	// long before RunE.
	t.Run("init_without_url", func(t *testing.T) {
		stderr, err := runInit(t)
		if err == nil {
			t.Fatalf("Init without --url should fail, but got no error")
		}
		// cobra's required-flag check writes to stderr/err, depending on
		// version. Either signal is fine — we just require an error.
		_ = stderr
	})

	t.Run("init_with_url", func(t *testing.T) {
		stderr, err := runInit(t, "--url="+test_conf_repo_url)
		if err != nil {
			t.Fatalf("Init with --url should succeed: %v\nstderr:\n%s", err, stderr)
		}

		// Verify both celer.toml and conf directory exist.
		celerPath := filepath.Join(dirs.WorkspaceDir, "celer.toml")
		confDir := filepath.Join(dirs.WorkspaceDir, "conf")

		if !fileio.PathExists(celerPath) {
			t.Error("celer.toml should be created")
		}
		if !fileio.PathExists(confDir) {
			t.Error("conf directory should be created")
		}

		// Re-init from disk should round-trip without error.
		celer2 := configs.NewCeler()
		if err := celer2.Init(); err != nil {
			t.Fatalf("Failed to re-init celer: %v", err)
		}
	})
}

func TestInitCmd_EdgeCases(t *testing.T) {
	tests := []struct {
		name        string
		args        []string
		expectError bool
		description string
	}{
		{
			name:        "branch_with_master",
			args:        []string{"--url=" + test_conf_repo_url, "--branch=master"},
			expectError: false,
			description: "Should succeed with existing branch",
		},
		{
			name:        "branch_nonexistent",
			args:        []string{"--url=" + test_conf_repo_url, "--branch=does-not-exist-9999"},
			expectError: true,
			description: "Should fail when branch does not exist on remote",
		},
		{
			name:        "url_not_a_git_repo",
			args:        []string{"--url=https://example.com/not-a-repo.git"},
			expectError: true,
			description: "Should fail when URL is not a real git repository",
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			stderr, err := runInit(t, test.args...)
			if test.expectError && err == nil {
				t.Errorf("%s: expected error, got none\nstderr:\n%s", test.description, stderr)
			}
			if !test.expectError && err != nil {
				t.Errorf("%s: unexpected error: %v\nstderr:\n%s", test.description, err, stderr)
			}
		})
	}
}

// Verify trim behavior: leading/trailing spaces in --url should still be
// handled by RunE's strings.TrimSpace before reaching CloneConf.
func TestInitCmd_TrimsUrlAndBranch(t *testing.T) {
	stderr, err := runInit(t,
		"--url=  "+test_conf_repo_url+"  ",
		"--branch=  master  ",
	)
	if err != nil {
		t.Fatalf("init should trim whitespace and succeed: %v\nstderr:\n%s", err, stderr)
	}

	// Verify both celer.toml and conf directory exist after trimmed values
	// land in the same code path.
	if !fileio.PathExists(filepath.Join(dirs.WorkspaceDir, "celer.toml")) {
		t.Error("celer.toml should be created after init")
	}
	if !fileio.PathExists(filepath.Join(dirs.WorkspaceDir, "conf")) {
		t.Error("conf directory should be created after init")
	}
}
