package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"os"
	"path/filepath"
	"slices"
	"strings"
	"testing"

	"github.com/spf13/cobra"
)

func TestTreeCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	treeCmd := treeCmd{}
	celer := configs.NewCeler()
	cmd := treeCmd.Command(celer)

	// Test command basic properties.
	if cmd.Use != "tree" {
		t.Errorf("Expected Use to be 'tree', got '%s'", cmd.Use)
	}

	if cmd.Short == "" {
		t.Error("Short description should not be empty")
	}

	if cmd.Long == "" {
		t.Error("Long description should not be empty")
	}

	// Verify Long description contains examples
	if !strings.Contains(cmd.Long, "Examples:") {
		t.Error("Long description should contain examples")
	}

	// Test that command requires exactly 1 argument
	if cmd.Args == nil {
		t.Error("Args validation should be set")
	}

	// Test flags.
	hideDevFlag := cmd.Flags().Lookup("hide-dev")
	if hideDevFlag == nil {
		t.Error("--hide-dev flag should be defined")
	} else {
		if hideDevFlag.DefValue != "false" {
			t.Errorf("Expected hide-dev default to be 'false', got '%s'", hideDevFlag.DefValue)
		}
	}
}

func TestTreeCmd_ValidateTarget(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	treeCmd := treeCmd{}

	tests := []struct {
		name        string
		target      string
		expectError bool
		description string
	}{
		{
			name:        "valid_package",
			target:      "boost@1.87.0",
			expectError: false,
			description: "Should accept valid package format",
		},
		{
			name:        "valid_project",
			target:      "my_project",
			expectError: false,
			description: "Should accept valid project name",
		},
		{
			name:        "empty_target",
			target:      "",
			expectError: true,
			description: "Should reject empty target",
		},
		{
			name:        "whitespace_target",
			target:      "   ",
			expectError: true,
			description: "Should reject whitespace-only target",
		},
		{
			name:        "complex_package_name",
			target:      "opencv_contrib@4.11.0",
			expectError: false,
			description: "Should accept complex package names",
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			err := treeCmd.validateTarget(test.target)
			if test.expectError && err == nil {
				t.Errorf("Expected error for %s, got nil", test.description)
			}
			if !test.expectError && err != nil {
				t.Errorf("Expected no error for %s, got: %v", test.description, err)
			}
		})
	}
}

func TestTreeCmd_Completion(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Setup test environment
	setupTestEnvironment(t)

	treeCmd := treeCmd{}
	celer := configs.NewCeler()
	treeCmd.celer = celer

	tests := []struct {
		name             string
		toComplete       string
		expectContains   []string
		expectNotContain []string
	}{
		{
			name:           "complete_flag",
			toComplete:     "--hide",
			expectContains: []string{"--hide-dev"},
		},
		{
			name:           "complete_package",
			toComplete:     "boost",
			expectContains: []string{"boost@1.87.0"},
		},
		{
			name:           "complete_project",
			toComplete:     "test",
			expectContains: []string{"test_project"},
		},
		{
			name:             "no_match",
			toComplete:       "nonexistent",
			expectNotContain: []string{"boost@1.87.0"},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			suggestions, directive := treeCmd.completion(nil, []string{}, test.toComplete)

			if directive != cobra.ShellCompDirectiveNoFileComp {
				t.Errorf("Expected NoFileComp directive, got %d", directive)
			}

			// Check expected suggestions.
			for _, expected := range test.expectContains {
				found := slices.Contains(suggestions, expected)
				if !found {
					t.Errorf("Expected to find '%s' in suggestions, got: %v", expected, suggestions)
				}
			}

			// Check suggestions that should not be present.
			for _, notExpected := range test.expectNotContain {
				for _, suggestion := range suggestions {
					if suggestion == notExpected {
						t.Errorf("Did not expect to find '%s' in suggestions", notExpected)
					}
				}
			}
		})
	}
}

func TestTreeCmd_PrintTree(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	treeCmd := treeCmd{}

	// Create a simple dependency tree
	root := &portInfo{
		nameVersion: "root@1.0.0",
		depth:       0,
		devDep:      false,
	}

	dep1 := &portInfo{
		parent:      root,
		nameVersion: "dep1@1.0.0",
		depth:       1,
		devDep:      false,
	}

	dep2 := &portInfo{
		parent:      root,
		nameVersion: "dep2@2.0.0",
		depth:       1,
		devDep:      false,
	}

	devDep := &portInfo{
		parent:      root,
		nameVersion: "devdep@1.0.0",
		depth:       1,
		devDep:      true,
	}

	root.depedencies = []*portInfo{dep1, dep2}
	root.devDependencies = []*portInfo{devDep}

	// Test without hiding dev dependencies.
	treeCmd.hideDevDep = false
	treeCmd.printTree(root) // Should not panic

	// Test with hiding dev dependencies.
	treeCmd.hideDevDep = true
	treeCmd.printTree(root) // Should not panic
}

func TestTreeCmd_CollectPortInfos(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// This test would require actual port configuration files
	// For now, we test that the method signature is correct
	treeCmd := treeCmd{}
	celer := configs.NewCeler()
	treeCmd.celer = celer

	parent := &portInfo{
		nameVersion: "test@1.0.0",
		depth:       0,
		devDep:      false,
	}

	// This will fail without proper setup, but we're testing the error handling
	if err := treeCmd.collectPortInfos(parent, "nonexistent@1.0.0"); err == nil {
		t.Error("Expected error for nonexistent package")
	}
}

func TestTreeCmd_Execute_InvalidTarget(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	treeCmd := treeCmd{}
	celer := configs.NewCeler()
	treeCmd.celer = celer

	// Test with empty target
	err := treeCmd.tree("")
	if err == nil {
		t.Error("Expected error for empty target")
	}

	if !strings.Contains(err.Error(), "empty") {
		t.Errorf("Expected error message to contain 'empty', got: %v", err)
	}
}

// Helper function to setup test environment
func setupTestEnvironment(t *testing.T) {
	// Create test ports directory.
	portsDir := dirs.PortsDir
	boostDir := filepath.Join(portsDir, "boost", "1.87.0")
	if err := os.MkdirAll(boostDir, 0755); err != nil {
		t.Fatalf("Failed to create test ports directory: %v", err)
	}

	// Create a dummy port.toml file.
	portToml := filepath.Join(boostDir, "port.toml")
	if err := os.WriteFile(portToml, []byte("# Test port"), 0644); err != nil {
		t.Fatalf("Failed to create test port.toml: %v", err)
	}

	// Create test projects directory.
	projectsDir := dirs.ConfProjectsDir
	if err := os.MkdirAll(projectsDir, 0755); err != nil {
		t.Fatalf("Failed to create test projects directory: %v", err)
	}

	// Create a dummy project file.
	projectToml := filepath.Join(projectsDir, "test_project.toml")
	if err := os.WriteFile(projectToml, []byte("# Test project"), 0644); err != nil {
		t.Fatalf("Failed to create test project.toml: %v", err)
	}
}
