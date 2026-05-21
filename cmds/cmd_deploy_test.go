package cmds

import (
	"path/filepath"
	"slices"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/dirs"

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
