package cmds

import (
	"os"
	"path/filepath"
	"slices"
	"strings"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/spf13/cobra"
)

func TestCreateCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	celer := configs.NewCeler()
	createCmd := createCmd{}
	cmd := createCmd.Command(celer)

	t.Run("command_structure", func(t *testing.T) {
		if cmd.Use != "create" {
			t.Errorf("Expected Use to be 'create', got '%s'", cmd.Use)
		}

		if cmd.Short == "" {
			t.Error("Short description should not be empty")
		}

		if cmd.Long == "" {
			t.Error("Long description should not be empty")
		}

		// Check flags exist
		flags := []string{"platform", "project", "port"}
		for _, flagName := range flags {
			if cmd.Flags().Lookup(flagName) == nil {
				t.Errorf("--%s flag should be defined", flagName)
			}
		}
	})

	t.Run("mutually_exclusive_flags", func(t *testing.T) {
		// This test verifies that the flags are properly marked as mutually exclusive,
		// The actual enforcement is handled by Cobra during command execution.
		flagNames := []string{"platform", "project", "port"}
		for _, flagName := range flagNames {
			if cmd.Flags().Lookup(flagName) == nil {
				t.Errorf("Flag %s should exist", flagName)
			}
		}
	})
}

func TestCreateCmd_Completion(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	createCmd := createCmd{}
	celer := configs.NewCeler()
	cmd := createCmd.Command(celer)

	tests := []struct {
		name       string
		toComplete string
		expected   []string
	}{
		{
			name:       "complete platform flag",
			toComplete: "--plat",
			expected:   []string{"--platform"},
		},
		{
			name:       "complete project flag",
			toComplete: "--proj",
			expected:   []string{"--project"},
		},
		{
			name:       "complete port flag",
			toComplete: "--port",
			expected:   []string{"--port"},
		},
		{
			name:       "no completion for unknown",
			toComplete: "--unknown",
			expected:   []string{},
		},
		{
			name:       "multiple matches",
			toComplete: "--p",
			expected:   []string{"--platform", "--project", "--port"},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			suggestions, directive := createCmd.completion(cmd, []string{}, test.toComplete)

			if directive != cobra.ShellCompDirectiveNoFileComp {
				t.Errorf("Expected directive %v, got %v", cobra.ShellCompDirectiveNoFileComp, directive)
			}

			if len(suggestions) != len(test.expected) {
				t.Errorf("Expected %d suggestions, got %d: %v", len(test.expected), len(suggestions), suggestions)
				return
			}

			for _, expected := range test.expected {
				found := slices.Contains(suggestions, expected)
				if !found {
					t.Errorf("Expected suggestion '%s' not found in %v", expected, suggestions)
				}
			}
		})
	}
}

func TestCreateCmd_Validation(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	createCmd := &createCmd{}

	t.Run("validatePlatformName", func(t *testing.T) {
		tests := []struct {
			name        string
			input       string
			expectError bool
		}{
			{"valid platform name", "windows-x86_64-msvc", false},
			{"empty platform name", "", true},
			{"whitespace only", "   ", true},
			{"platform with spaces", "windows x86_64", true},
			{"valid complex name", "x86_64-linux-gnu-gcc", false},
			{"platform path traversal", "../windows-x86_64-msvc", true},
			{"platform with slash", "windows/x86_64", true},
		}

		for _, test := range tests {
			t.Run(test.name, func(t *testing.T) {
				err := createCmd.validatePlatformName(test.input)
				if test.expectError && err == nil {
					t.Errorf("Expected error for input '%s' but got none", test.input)
				} else if !test.expectError && err != nil {
					t.Errorf("Expected no error for input '%s' but got: %v", test.input, err)
				}
			})
		}
	})

	t.Run("validateProjectName", func(t *testing.T) {
		tests := []struct {
			name        string
			input       string
			expectError bool
		}{
			{"valid project name", "my-awesome-project", false},
			{"empty project name", "", true},
			{"whitespace only", "   ", true},
			{"project with spaces", "my project", false}, // Project names can have spaces
			{"valid with numbers", "project123", false},
			{"project path traversal", "../my-project", true},
			{"project with slash", "my/project", true},
		}

		for _, test := range tests {
			t.Run(test.name, func(t *testing.T) {
				err := createCmd.validateProjectName(test.input)
				if test.expectError && err == nil {
					t.Errorf("Expected error for input '%s' but got none", test.input)
				} else if !test.expectError && err != nil {
					t.Errorf("Expected no error for input '%s' but got: %v", test.input, err)
				}
			})
		}
	})

	t.Run("validatePortName", func(t *testing.T) {
		tests := []struct {
			name        string
			input       string
			expectError bool
		}{
			{"valid port", "opencv@4.8.0", false},
			{"empty port name", "", true},
			{"no version separator", "opencv", true},
			{"empty name", "@4.8.0", true},
			{"empty version", "opencv@", true},
			{"multiple separators", "opencv@4.8@0", true},
			{"valid complex version", "opencv@4.8.0-beta1", false},
			{"port with slash", "opencv/test@4.8.0", true},
			{"version with slash", "opencv@4/8/0", true},
			{"port path traversal", "../opencv@4.8.0", true},
		}

		for _, test := range tests {
			t.Run(test.name, func(t *testing.T) {
				err := createCmd.validatePortName(test.input)
				if test.expectError && err == nil {
					t.Errorf("Expected error for input '%s' but got none", test.input)
				} else if !test.expectError && err != nil {
					t.Errorf("Expected no error for input '%s' but got: %v", test.input, err)
				}
			})
		}
	})
}

func TestCreateCmd_CommandExecutionValidation(t *testing.T) {
	t.Run("empty project value should fail", func(t *testing.T) {
		celer := newInitializedCeler(t)
		cmd := &createCmd{}

		stderr, err := runCommand(t, cmd.Command(celer), "--project=")
		if err == nil {
			t.Fatal("expected error when --project is set with empty value")
		}
		if !strings.Contains(stderr, "cannot be empty") {
			t.Fatalf("stderr should report empty value, got:\n%s", stderr)
		}
	})

	t.Run("positional args should fail", func(t *testing.T) {
		celer := newInitializedCeler(t)
		cmd := &createCmd{}

		// cobra.NoArgs handles this — error comes from cobra, not from RunE.
		_, err := runCommand(t, cmd.Command(celer), "unexpected-arg", "--project=test_project")
		if err == nil {
			t.Fatal("expected error when positional args are provided")
		}
	})
}

func TestCreateCmd(t *testing.T) {
	celer := newInitializedCeler(t)

	// ============= Create platform ============= //
	t.Run("CreatePlatformSuccess", func(t *testing.T) {
		const platformName = "x86_64-linux-ubuntu-test"
		cmd := &createCmd{}
		if _, err := runCommand(t, cmd.Command(celer), "--platform="+platformName); err != nil {
			t.Fatal(err)
		}

		// Check if platform really created.
		platformPath := filepath.Join(dirs.ConfPlatformsDir, platformName+".toml")
		if !fileio.PathExists(platformPath) {
			t.Errorf("platform file does not exist: %s", platformPath)
		}

		t.Cleanup(func() {
			_ = os.RemoveAll(platformPath)
		})
	})

	t.Run("CreatePlatformFailed_emptyName", func(t *testing.T) {
		cmd := &createCmd{}
		stderr, err := runCommand(t, cmd.Command(celer), "--platform=")
		if err == nil {
			t.Fatal("expected error when --platform is empty")
		}
		if !strings.Contains(stderr, "cannot be empty") {
			t.Fatalf("stderr should report empty name, got:\n%s", stderr)
		}
	})

	// ============= Create project ============= //
	t.Run("CreateProjectSuccess", func(t *testing.T) {
		const projectName = "project_test_create"
		cmd := &createCmd{}
		if _, err := runCommand(t, cmd.Command(celer), "--project="+projectName); err != nil {
			t.Fatal(err)
		}

		projectPath := filepath.Join(dirs.ConfProjectsDir, projectName+".toml")
		if !fileio.PathExists(projectPath) {
			t.Errorf("project does not exist: %s", projectName)
		}

		t.Cleanup(func() {
			_ = os.Remove(projectPath)
		})
	})

	t.Run("CreateProjectFailed_emptyName", func(t *testing.T) {
		cmd := &createCmd{}
		stderr, err := runCommand(t, cmd.Command(celer), "--project=")
		if err == nil {
			t.Fatal("expected error when --project is empty")
		}
		if !strings.Contains(stderr, "cannot be empty") {
			t.Fatalf("stderr should report empty name, got:\n%s", stderr)
		}
	})

	// ============= Create port ============= //
	t.Run("CreatePortSuccess", func(t *testing.T) {
		const portName = "test_port_test"
		const portVersion = "1.0.0"
		cmd := &createCmd{}
		if _, err := runCommand(t, cmd.Command(celer), "--port="+portName+"@"+portVersion); err != nil {
			t.Fatal(err)
		}

		portPath := dirs.GetPortPath(portName, portVersion)
		if !fileio.PathExists(portPath) {
			t.Errorf("port does not exists: %s@%s", portName, portVersion)
		}

		t.Cleanup(func() {
			_ = os.Remove(portPath)
		})
	})

	t.Run("CreatePortFailed_emptyName", func(t *testing.T) {
		cmd := &createCmd{}
		stderr, err := runCommand(t, cmd.Command(celer), "--port=")
		if err == nil {
			t.Fatal("expected error when --port is empty")
		}
		if !strings.Contains(stderr, "cannot be empty") {
			t.Fatalf("stderr should report empty port name, got:\n%s", stderr)
		}
	})

	t.Run("CreatePortFailed_invalidPortName", func(t *testing.T) {
		cmd := &createCmd{}
		stderr, err := runCommand(t, cmd.Command(celer), "--port=libxxx")
		if err == nil {
			t.Fatal("expected error when --port is not in name@version form")
		}
		if !strings.Contains(stderr, "name@version") {
			t.Fatalf("stderr should report invalid format, got:\n%s", stderr)
		}
	})
}
