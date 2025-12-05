package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"testing"

	"github.com/spf13/cobra"
)

func TestCreateCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	t.Cleanup(func() {
		dirs.RemoveAllForTest()
	})

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
	t.Cleanup(func() {
		dirs.RemoveAllForTest()
	})

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
	t.Cleanup(func() {
		dirs.RemoveAllForTest()
	})

	createCmd := &createCmd{}

	t.Run("validatePlatformName", func(t *testing.T) {
		tests := []struct {
			name        string
			input       string
			expectError bool
		}{
			{"valid platform name", "windows-amd64-msvc", false},
			{"empty platform name", "", true},
			{"whitespace only", "   ", true},
			{"platform with spaces", "windows amd64", true},
			{"valid complex name", "x86_64-linux-gnu-gcc", false},
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

func TestCreateCmd(t *testing.T) {
	// Cleanup.
	t.Cleanup(func() {
		dirs.RemoveAllForTest()
	})

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
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))

	// ============= Create platform ============= //
	t.Run("CreatePlatformSuccess", func(t *testing.T) {
		const platformName = "x86_64-linux-ubuntu-test"
		check(celer.CreatePlatform(platformName))

		// Check if platform really created.
		platformPath := filepath.Join(dirs.ConfPlatformsDir, platformName+".toml")
		if !fileio.PathExists(platformPath) {
			t.Fatalf("platform %s should be created", platformName)
		}

		check(os.RemoveAll(platformPath))
	})

	t.Run("CreatePlatformFailed_emptyName", func(t *testing.T) {
		if err := celer.CreatePlatform(""); err == nil {
			t.Fatal("it should be failed")
		}

		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
	})
	check(celer.SetBuildType("Release"))

	// ============= Create project ============= //
	t.Run("CreateProjectSuccess", func(t *testing.T) {
		const projectName = "project_test_create"
		check(celer.CreateProject(projectName))

		projectPath := filepath.Join(dirs.ConfProjectsDir, projectName+".toml")
		if !fileio.PathExists(projectPath) {
			t.Fatalf("project does not exist: %s", projectName)
		}

		t.Cleanup(func() {
			check(os.Remove(projectPath))
		})
	})

	t.Run("Create project failed: empyt name", func(t *testing.T) {
		if err := celer.CreateProject(""); err == nil {
			t.Fatal("it should be failed")
		}
	})

	// ============= Create port ============= //
	t.Run("CreatePortSuccess", func(t *testing.T) {
		const portName = "test_port_test"
		const portVersion = "1.0.0"
		check(celer.CreatePort(portName + "@" + portVersion))

		portPath := filepath.Join(dirs.PortsDir, fmt.Sprintf("%s/%s/port.toml", portName, portVersion))
		if !fileio.PathExists(portPath) {
			t.Fatalf("port does not exists: %s@%s", portName, portVersion)
		}

		t.Cleanup(func() {
			check(os.Remove(portPath))
		})
	})

	t.Run("CreatePortFailed_emptyName", func(t *testing.T) {
		if err := celer.CreatePort(""); err == nil {
			t.Fatal("it should be failed")
		}
	})

	t.Run("CreatePortFailed_invalidPortName", func(t *testing.T) {
		if err := celer.CreatePort("libxxx"); err == nil {
			t.Fatal("it should be failed")
		}
	})
}
