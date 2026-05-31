package cmds

import (
	"os"
	"os/exec"
	"path/filepath"
	"slices"
	"testing"

	"github.com/celer-pkg/celer/buildsystems"
	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/git"
	"github.com/celer-pkg/celer/pkgs/refs"

	"github.com/spf13/cobra"
)

func TestDeployCmd_ArgsValidation(t *testing.T) {
	dirs.RemoveAllForTest()

	tests := []struct {
		name                 string
		args                 []string
		setSnapshot          bool
		snapshotValue        string
		expectError          bool
		expectedSnapshotPath string
	}{
		{
			name:        "default_no_snapshot_should_succeed",
			args:        []string{},
			expectError: false,
		},
		{
			name:        "positional_args_should_fail",
			args:        []string{"unexpected"},
			expectError: true,
		},
		{
			name:          "empty_snapshot_should_fail",
			setSnapshot:   true,
			snapshotValue: "",
			expectError:   true,
		},
		{
			name:                 "dot_snapshot_should_succeed",
			setSnapshot:          true,
			snapshotValue:        ".",
			expectError:          false,
			expectedSnapshotPath: filepath.Clean("."),
		},
		{
			name:                 "parent_snapshot_should_succeed",
			setSnapshot:          true,
			snapshotValue:        "..",
			expectError:          false,
			expectedSnapshotPath: filepath.Clean(".."),
		},
		{
			name:                 "escape_workspace_should_succeed",
			setSnapshot:          true,
			snapshotValue:        "../snapshots/2026-02-21",
			expectError:          false,
			expectedSnapshotPath: filepath.Clean("../snapshots/2026-02-21"),
		},
		{
			name:                 "absolute_snapshot_should_succeed",
			setSnapshot:          true,
			snapshotValue:        filepath.Join(dirs.WorkspaceDir, "snapshots", "2026-02-21"),
			expectError:          false,
			expectedSnapshotPath: filepath.Clean(filepath.Join(dirs.WorkspaceDir, "snapshots", "2026-02-21")),
		},
		{
			name:                 "valid_relative_snapshot_should_succeed",
			setSnapshot:          true,
			snapshotValue:        "snapshots/../snapshots/2026-02-21",
			expectError:          false,
			expectedSnapshotPath: filepath.Clean("snapshots/../snapshots/2026-02-21"),
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			deploy := &deployCmd{}
			cmd := deploy.Command(configs.NewCeler())

			if test.setSnapshot {
				if err := cmd.Flags().Set("snapshot", test.snapshotValue); err != nil {
					t.Fatalf("failed to set --snapshot: %v", err)
				}
			}

			err := cmd.Args(cmd, test.args)
			if test.expectError && err == nil {
				t.Fatal("expected args validation error")
			}
			if !test.expectError && err != nil {
				t.Fatalf("expected args validation success, got: %v", err)
			}

			if test.expectedSnapshotPath != "" && deploy.snapshotPath != test.expectedSnapshotPath {
				t.Fatalf("expected cleaned snapshot path %s, got %s", test.expectedSnapshotPath, deploy.snapshotPath)
			}
		})
	}
}

func TestDeployCmd_Completion(t *testing.T) {
	dirs.RemoveAllForTest()

	deploy := deployCmd{}
	cmd := deploy.Command(configs.NewCeler())

	tests := []struct {
		name       string
		toComplete string
		expected   []string
	}{
		{
			name:       "complete_long_force_flag",
			toComplete: "--fo",
			expected:   []string{"--force"},
		},
		{
			name:       "complete_short_force_flag",
			toComplete: "-f",
			expected:   []string{"-f"},
		},
		{
			name:       "complete_snapshot_flag",
			toComplete: "--sna",
			expected:   []string{"--snapshot"},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			suggestions, directive := deploy.completion(cmd, []string{}, test.toComplete)

			if directive != cobra.ShellCompDirectiveNoFileComp {
				t.Fatalf("expected directive %v, got %v", cobra.ShellCompDirectiveNoFileComp, directive)
			}

			for _, expected := range test.expected {
				if !slices.Contains(suggestions, expected) {
					t.Fatalf("expected completion %s, got %v", expected, suggestions)
				}
			}
		})
	}
}

// deployFakeContext is a minimal context.Context implementation for testing
// BuildConfig.Clone() without pkgcache or remote network access.
type deployFakeContext struct{}

func (f deployFakeContext) Version() string                        { return "test" }
func (f deployFakeContext) Platform() context.Platform             { return nil }
func (f deployFakeContext) RootFS() context.RootFS                 { return nil }
func (f deployFakeContext) Project() context.Project               { return nil }
func (f deployFakeContext) BuildType() string                      { return "Release" }
func (f deployFakeContext) Downloads() string                      { return "" }
func (f deployFakeContext) Jobs() int                              { return 1 }
func (f deployFakeContext) Offline() bool                          { return true }
func (f deployFakeContext) Verbose() bool                          { return false }
func (f deployFakeContext) InstalledDir() string                   { return "" }
func (f deployFakeContext) InstalledDevDir() string                { return "" }
func (f deployFakeContext) PkgCache() context.PkgCache             { return nil }
func (f deployFakeContext) ProxyHostPort() (host string, port int) { return "", 0 }
func (f deployFakeContext) CCacheEnabled() bool                    { return false }
func (f deployFakeContext) GenerateToolchainFile() error           { return nil }
func (f deployFakeContext) ExprVars() *context.ExprVars            { return nil }
func (f deployFakeContext) PythonConfig() context.PythonConfig     { return nil }

// setupTestRepo creates a bare git repo as clone source in a temp directory
// and returns its path along with known commit hashes for testing.
func setupTestRepo(t *testing.T) (repoUrl string, headCommit, olderCommit string) {
	t.Helper()

	// Use the existing testdata repo as source.
	testdataDir := filepath.Join("..", "pkgs", "git", "testdata")
	if _, err := os.Stat(testdataDir); err != nil {
		t.Skip("git testdata repo not available")
	}

	// Create a bare clone as the "remote" source for test clones.
	bareDir := filepath.Join(t.TempDir(), "source.git")
	cmd := exec.Command("git", "clone", "--bare", testdataDir, bareDir)
	if output, err := cmd.CombinedOutput(); err != nil {
		t.Fatalf("failed to create bare test repo: %v\n%s", err, output)
	}

	// Resolve two commits: HEAD and HEAD~3.
	resolveCommit := func(ref string) string {
		cmd := exec.Command("git", "--git-dir", bareDir, "rev-parse", ref)
		output, err := cmd.Output()
		if err != nil {
			t.Fatalf("failed to resolve %s: %v", ref, err)
		}
		return string(output[:40])
	}

	headCommit = resolveCommit("HEAD")
	olderCommit = resolveCommit("HEAD~3")
	return bareDir, headCommit, olderCommit
}

func TestDeploy_Clone_ExistingRepo_ResetsToResolvedCommit(t *testing.T) {
	dirs.RemoveAllForTest()
	defer refs.StoreResolvedCommits(nil)

	repoUrl, headCommit, olderCommit := setupTestRepo(t)

	// Clone the repo first so it already exists in RepoDir.
	repoDir := filepath.Join(t.TempDir(), "src")
	if err := git.CloneRepo("[test]", "test@1.0", repoUrl, "master", 0, repoDir); err != nil {
		t.Fatalf("initial clone failed: %v", err)
	}

	// Verify we're at HEAD.
	currentCommit, err := git.GetCommitHash(repoDir)
	if err != nil {
		t.Fatalf("failed to get current commit: %v", err)
	}
	if currentCommit != headCommit {
		t.Fatalf("expected initial commit %s, got %s", headCommit, currentCommit)
	}

	// Store a resolved commit that is different from HEAD (simulating deploy resolution).
	refs.StoreResolvedCommits(map[string]string{
		"test@1.0": olderCommit,
	})

	// Call Clone with the existing repo — it should HardReset to the resolved commit.
	config := buildsystems.BuildConfig{
		Ctx: deployFakeContext{},
		PortConfig: buildsystems.PortConfig{
			LibName:    "test",
			LibVersion: "1.0",
			RepoDir:    repoDir,
		},
	}

	if err := config.Clone(repoUrl, "master", "", 0); err != nil {
		t.Fatalf("Clone failed: %v", err)
	}

	// Verify HEAD is now at the resolved commit.
	currentCommit, err = git.GetCommitHash(repoDir)
	if err != nil {
		t.Fatalf("failed to get commit after Clone: %v", err)
	}
	if currentCommit != olderCommit {
		t.Fatalf("expected resolved commit %s, got %s", olderCommit, currentCommit)
	}
}

func TestDeploy_Clone_ExistingRepo_NoResolvedCommit_NoReset(t *testing.T) {
	dirs.RemoveAllForTest()
	defer refs.StoreResolvedCommits(nil)

	repoUrl, headCommit, _ := setupTestRepo(t)

	// Clone the repo first.
	repoDir := filepath.Join(t.TempDir(), "src")
	if err := git.CloneRepo("[test]", "test@1.0", repoUrl, "master", 0, repoDir); err != nil {
		t.Fatalf("initial clone failed: %v", err)
	}

	// No resolved commit stored — Clone should return without resetting.
	refs.StoreResolvedCommits(nil)

	config := buildsystems.BuildConfig{
		Ctx: deployFakeContext{},
		PortConfig: buildsystems.PortConfig{
			LibName:    "test",
			LibVersion: "1.0",
			RepoDir:    repoDir,
		},
	}

	if err := config.Clone(repoUrl, "master", "", 0); err != nil {
		t.Fatalf("Clone failed: %v", err)
	}

	// Verify HEAD is unchanged.
	currentCommit, err := git.GetCommitHash(repoDir)
	if err != nil {
		t.Fatalf("failed to get commit after Clone: %v", err)
	}
	if currentCommit != headCommit {
		t.Fatalf("expected unchanged commit %s, got %s", headCommit, currentCommit)
	}
}

func TestDeploy_Clone_FreshClone_ResetsToResolvedCommit(t *testing.T) {
	dirs.RemoveAllForTest()
	defer refs.StoreResolvedCommits(nil)

	repoUrl, _, olderCommit := setupTestRepo(t)

	// Store resolved commit before cloning.
	refs.StoreResolvedCommits(map[string]string{
		"test@1.0": olderCommit,
	})

	// RepoDir does not exist — this is a fresh clone.
	repoDir := filepath.Join(t.TempDir(), "new-src")

	cfg := buildsystems.BuildConfig{
		Ctx: deployFakeContext{},
		PortConfig: buildsystems.PortConfig{
			LibName:    "test",
			LibVersion: "1.0",
			RepoDir:    repoDir,
		},
	}

	if err := cfg.Clone(repoUrl, "master", "", 0); err != nil {
		t.Fatalf("Clone failed: %v", err)
	}

	// Verify HEAD is at the resolved commit, not the branch HEAD.
	currentCommit, err := git.GetCommitHash(repoDir)
	if err != nil {
		t.Fatalf("failed to get commit after Clone: %v", err)
	}
	if currentCommit != olderCommit {
		t.Fatalf("expected resolved commit %s, got %s", olderCommit, currentCommit)
	}

	// Verify the branch name is preserved (not detached HEAD).
	branch, err := git.GetCurrentBranch(repoDir)
	if err != nil {
		t.Fatalf("failed to get current branch: %v", err)
	}
	if branch != "master" {
		t.Fatalf("expected branch 'master', got %q (detached HEAD)", branch)
	}
}

func TestDeploy_Clone_FreshClone_NoResolvedCommit_StaysOnBranch(t *testing.T) {
	dirs.RemoveAllForTest()
	defer refs.StoreResolvedCommits(nil)

	repoUrl, headCommit, _ := setupTestRepo(t)

	// No resolved commit stored.
	refs.StoreResolvedCommits(nil)

	repoDir := filepath.Join(t.TempDir(), "new-src")

	cfg := buildsystems.BuildConfig{
		Ctx: deployFakeContext{},
		PortConfig: buildsystems.PortConfig{
			LibName:    "test",
			LibVersion: "1.0",
			RepoDir:    repoDir,
		},
	}

	if err := cfg.Clone(repoUrl, "master", "", 0); err != nil {
		t.Fatalf("Clone failed: %v", err)
	}

	// Verify HEAD is at the branch tip.
	currentCommit, err := git.GetCommitHash(repoDir)
	if err != nil {
		t.Fatalf("failed to get commit after Clone: %v", err)
	}
	if currentCommit != headCommit {
		t.Fatalf("expected branch HEAD commit %s, got %s", headCommit, currentCommit)
	}

	// Verify the branch name is preserved.
	branch, err := git.GetCurrentBranch(repoDir)
	if err != nil {
		t.Fatalf("failed to get current branch: %v", err)
	}
	if branch != "master" {
		t.Fatalf("expected branch 'master', got %q", branch)
	}
}
