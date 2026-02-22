package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"path/filepath"
	"slices"
	"testing"

	"github.com/spf13/cobra"
)

func TestDeployCmd_ArgsValidation(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	tests := []struct {
		name               string
		args               []string
		setExport          bool
		exportValue        string
		expectError        bool
		expectedExportPath string
	}{
		{
			name:        "default_no_export_should_succeed",
			args:        []string{},
			expectError: false,
		},
		{
			name:        "positional_args_should_fail",
			args:        []string{"unexpected"},
			expectError: true,
		},
		{
			name:        "empty_export_should_fail",
			setExport:   true,
			exportValue: "",
			expectError: true,
		},
		{
			name:               "dot_export_should_succeed",
			setExport:          true,
			exportValue:        ".",
			expectError:        false,
			expectedExportPath: filepath.Clean("."),
		},
		{
			name:               "parent_export_should_succeed",
			setExport:          true,
			exportValue:        "..",
			expectError:        false,
			expectedExportPath: filepath.Clean(".."),
		},
		{
			name:               "escape_workspace_should_succeed",
			setExport:          true,
			exportValue:        "../snapshots/2026-02-21",
			expectError:        false,
			expectedExportPath: filepath.Clean("../snapshots/2026-02-21"),
		},
		{
			name:               "absolute_export_should_succeed",
			setExport:          true,
			exportValue:        filepath.Join(dirs.WorkspaceDir, "snapshots", "2026-02-21"),
			expectError:        false,
			expectedExportPath: filepath.Clean(filepath.Join(dirs.WorkspaceDir, "snapshots", "2026-02-21")),
		},
		{
			name:               "valid_relative_export_should_succeed",
			setExport:          true,
			exportValue:        "snapshots/../snapshots/2026-02-21",
			expectError:        false,
			expectedExportPath: filepath.Clean("snapshots/../snapshots/2026-02-21"),
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			deploy := &deployCmd{}
			cmd := deploy.Command(configs.NewCeler())

			if test.setExport {
				if err := cmd.Flags().Set("export", test.exportValue); err != nil {
					t.Fatalf("failed to set --export: %v", err)
				}
			}

			err := cmd.Args(cmd, test.args)
			if test.expectError && err == nil {
				t.Fatal("expected args validation error")
			}
			if !test.expectError && err != nil {
				t.Fatalf("expected args validation success, got: %v", err)
			}

			if test.expectedExportPath != "" && deploy.exportPath != test.expectedExportPath {
				t.Fatalf("expected cleaned export path %s, got %s", test.expectedExportPath, deploy.exportPath)
			}
		})
	}
}

func TestDeployCmd_Completion(t *testing.T) {
	// Cleanup.
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
			name:       "complete_export_flag",
			toComplete: "--exp",
			expected:   []string{"--export"},
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
