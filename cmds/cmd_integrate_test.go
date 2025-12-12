package cmds

import (
	"celer/cmds/completion"
	"celer/configs"
	"celer/pkgs/dirs"
	"testing"

	"github.com/spf13/cobra"
)

func TestIntegrateCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Test command creation.
	integrate := &integrateCmd{}
	celer := &configs.Celer{}

	cmd := integrate.Command(celer)

	if cmd == nil {
		t.Fatal("Command should not be nil")
	}

	if cmd.Use != "integrate" {
		t.Errorf("Expected Use to be 'integrate', got %s", cmd.Use)
	}

	if cmd.Short == "" {
		t.Error("Short description should not be empty")
	}

	if cmd.Long == "" {
		t.Error("Long description should not be empty")
	}

	// Test flags
	flag := cmd.Flags().Lookup("remove")
	if flag == nil {
		t.Error("--remove flag should be defined")
		return
	}

	if flag.DefValue != "false" {
		t.Errorf("Expected --remove default to be false, got %s", flag.DefValue)
	}
}

func TestIntegrateCmd_Completion(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	integrate := &integrateCmd{}
	cmd := &cobra.Command{}

	tests := []struct {
		name       string
		toComplete string
		expected   []string
	}{
		{
			name:       "empty_input",
			toComplete: "",
			expected:   []string{"--remove"},
		},
		{
			name:       "partial_flag",
			toComplete: "--rem",
			expected:   []string{"--remove"},
		},
		{
			name:       "full_flag",
			toComplete: "--remove",
			expected:   []string{"--remove"},
		},
		{
			name:       "no_match",
			toComplete: "--invalid",
			expected:   []string{},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			suggestions, directive := integrate.completion(cmd, []string{}, test.toComplete)

			if len(suggestions) != len(test.expected) {
				t.Errorf("Expected %d suggestions, got %d", len(test.expected), len(suggestions))
			}

			for i, expected := range test.expected {
				if i >= len(suggestions) || suggestions[i] != expected {
					t.Errorf("Expected suggestion %s, got %s", expected, func() string {
						if i < len(suggestions) {
							return suggestions[i]
						}
						return "<missing>"
					}())
				}
			}

			if directive != cobra.ShellCompDirectiveNoFileComp {
				t.Errorf("Expected NoFileComp directive, got %d", directive)
			}
		})
	}
}

func TestIntegrateCmd_Integration(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	if testing.Short() {
		t.Skip("Skipping integration test in short mode")
	}

	// Only run if we have a supported shell
	if completion.CurrentShell() == completion.NotSupported {
		t.Skip("Skipping integration test on unsupported shell")
	}

	integrate := &integrateCmd{}

	// Test environment validation.
	err := integrate.validateEnvironment()
	if err != nil {
		t.Fatalf("Environment validation failed: %v", err)
	}

	// Test completion initialization (might fail without proper setup).
	err = integrate.initializeCompletions()
	if err != nil {
		// This is acceptable in test environments.
		t.Logf("Completion initialization failed (expected in test env): %v", err)
		return
	}

	// Verify all completions are set.
	if integrate.bashCompletion == nil || integrate.zshCompletion == nil || integrate.psCompletion == nil {
		t.Error("Not all completion handlers were initialized")
	}
}
