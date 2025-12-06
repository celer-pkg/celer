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

func TestInstallCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	t.Cleanup(func() {
		dirs.RemoveAllForTest()
	})

	celer := configs.NewCeler()
	installCmd := installCmd{}
	cmd := installCmd.Command(celer)

	t.Run("command_structure", func(t *testing.T) {
		if cmd.Use != "install" {
			t.Errorf("Expected Use to be 'install', got '%s'", cmd.Use)
		}

		if cmd.Short == "" {
			t.Error("Short description should not be empty")
		}

		if cmd.Long == "" {
			t.Error("Long description should not be empty")
		}

		// Check that it expects exactly one argument
		if cmd.Args == nil {
			t.Error("Args should be set to require package name@version")
		}
	})

	t.Run("flags_configuration", func(t *testing.T) {
		flags := []struct {
			name      string
			shorthand string
		}{
			{"dev", "d"},
			{"force", "f"},
			{"recursive", "r"},
			{"store-cache", "s"},
			{"cache-token", "t"},
			{"jobs", "j"},
			{"verbose", "v"},
		}

		for _, flag := range flags {
			f := cmd.Flags().Lookup(flag.name)
			if f == nil {
				t.Errorf("--%s flag should be defined", flag.name)
			} else if f.Shorthand != flag.shorthand {
				t.Errorf("Expected %s flag shorthand to be '%s', got '%s'", flag.name, flag.shorthand, f.Shorthand)
			}
		}
	})
}

func TestInstallCmd_ValidateAndCleanInput(t *testing.T) {
	// Cleanup.
	t.Cleanup(func() {
		dirs.RemoveAllForTest()
	})

	installCmd := &installCmd{}

	tests := []struct {
		name        string
		input       string
		expected    string
		expectError bool
	}{
		{
			name:        "valid package",
			input:       "opencv@4.8.0",
			expected:    "opencv@4.8.0",
			expectError: false,
		},
		{
			name:        "valid package with spaces",
			input:       " opencv@4.8.0 ",
			expected:    "opencv@4.8.0",
			expectError: false,
		},
		{
			name:        "valid package with windows escape",
			input:       "opencv`@4.8.0",
			expected:    "opencv@4.8.0",
			expectError: false,
		},
		{
			name:        "complex version",
			input:       "boost@1.82.0-beta1",
			expected:    "boost@1.82.0-beta1",
			expectError: false,
		},
		{
			name:        "empty input",
			input:       "",
			expected:    "",
			expectError: true,
		},
		{
			name:        "whitespace only",
			input:       "   ",
			expected:    "",
			expectError: true,
		},
		{
			name:        "missing version",
			input:       "opencv",
			expected:    "",
			expectError: true,
		},
		{
			name:        "missing package name",
			input:       "@4.8.0",
			expected:    "",
			expectError: true,
		},
		{
			name:        "empty package name",
			input:       " @4.8.0",
			expected:    "",
			expectError: true,
		},
		{
			name:        "empty version",
			input:       "opencv@ ",
			expected:    "",
			expectError: true,
		},
		{
			name:        "multiple @ symbols",
			input:       "opencv@4.8@0",
			expected:    "",
			expectError: true,
		},
		{
			name:        "no @ symbol",
			input:       "opencv4.8.0",
			expected:    "",
			expectError: true,
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			result, err := installCmd.validateAndCleanInput(test.input)

			if test.expectError {
				if err == nil {
					t.Errorf("Expected error for input '%s' but got none", test.input)
				}
			} else {
				if err != nil {
					t.Errorf("Expected no error for input '%s' but got: %v", test.input, err)
				}
				if result != test.expected {
					t.Errorf("Expected result '%s' for input '%s' but got '%s'", test.expected, test.input, result)
				}
			}
		})
	}
}

func TestInstallCmd_Completion(t *testing.T) {
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

	// Create mock port structure for testing completion.
	portsDir := dirs.PortsDir
	testPortDir := filepath.Join(portsDir, "testlib", "1.0.0")
	check(os.MkdirAll(testPortDir, os.ModePerm))

	// Create a port.toml file
	portFile := filepath.Join(testPortDir, "port.toml")
	f, err := os.Create(portFile)
	check(err)
	f.Close()

	celer := configs.NewCeler()
	installCmd := installCmd{celer: celer}
	cmd := installCmd.Command(celer)

	tests := []struct {
		name           string
		toComplete     string
		expectContains []string
	}{
		{
			name:           "package completion",
			toComplete:     "testlib",
			expectContains: []string{"testlib@1.0.0"},
		},
		{
			name:           "flag completion - dev",
			toComplete:     "--dev",
			expectContains: []string{"--dev"},
		},
		{
			name:           "flag completion - force",
			toComplete:     "--force",
			expectContains: []string{"--force"},
		},
		{
			name:           "flag completion - short dev",
			toComplete:     "-d",
			expectContains: []string{"-d"},
		},
		{
			name:           "flag completion - jobs",
			toComplete:     "--jobs",
			expectContains: []string{"--jobs"},
		},
		{
			name:           "no completion for unknown",
			toComplete:     "unknown",
			expectContains: []string{},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			suggestions, directive := installCmd.completion(cmd, []string{}, test.toComplete)

			if directive != cobra.ShellCompDirectiveNoFileComp {
				t.Errorf("Expected directive %v, got %v", cobra.ShellCompDirectiveNoFileComp, directive)
			}

			for _, expected := range test.expectContains {
				found := slices.Contains(suggestions, expected)
				if !found {
					t.Errorf("Expected suggestion '%s' not found in %v", expected, suggestions)
				}
			}
		})
	}
}

func TestInstallCmd_BuildSuggestions(t *testing.T) {
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

	// Setup test environment
	tmpDir := t.TempDir()

	// Create mock port structure
	testPorts := []struct {
		name    string
		version string
	}{
		{"opencv", "4.8.0"},
		{"opencv", "4.7.0"},
		{"boost", "1.82.0"},
		{"eigen", "3.4.0"},
	}

	for _, port := range testPorts {
		portDir := filepath.Join(tmpDir, port.name, port.version)
		check(os.MkdirAll(portDir, os.ModePerm))

		portFile := filepath.Join(portDir, "port.toml")
		f, err := os.Create(portFile)
		check(err)
		f.Close()
	}

	celer := configs.NewCeler()
	installCmd := installCmd{celer: celer}

	tests := []struct {
		name       string
		toComplete string
		expected   []string
	}{
		{
			name:       "complete opencv",
			toComplete: "opencv",
			expected:   []string{"opencv@4.8.0", "opencv@4.7.0"},
		},
		{
			name:       "complete boost",
			toComplete: "boost",
			expected:   []string{"boost@1.82.0"},
		},
		{
			name:       "complete partial name - no match",
			toComplete: "lib",
			expected:   []string{}, // No matches since no packages start with "lib"
		},
		{
			name:       "complete with version",
			toComplete: "opencv@4.8",
			expected:   []string{"opencv@4.8.0"},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			var suggestions []string
			installCmd.buildSuggestions(&suggestions, tmpDir, test.toComplete)

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

func TestInstallCmd_ErrorHandling(t *testing.T) {
	// Cleanup.
	t.Cleanup(func() {
		dirs.RemoveAllForTest()
	})

	tests := []struct {
		name        string
		input       string
		expectError string
	}{
		{
			name:        "empty package name",
			input:       "",
			expectError: "package name cannot be empty",
		},
		{
			name:        "invalid format - no version",
			input:       "opencv",
			expectError: "package must be specified in name@version format",
		},
		{
			name:        "invalid format - empty name",
			input:       "@4.8.0",
			expectError: "package name cannot be empty",
		},
		{
			name:        "invalid format - empty version",
			input:       "opencv@",
			expectError: "package version cannot be empty",
		},
	}

	installCmd := &installCmd{}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			_, err := installCmd.validateAndCleanInput(test.input)
			if err == nil {
				t.Errorf("Expected error for input '%s' but got none", test.input)
				return
			}

			if !strings.Contains(err.Error(), test.expectError) {
				t.Errorf("Expected error containing '%s' for input '%s', got: %v", test.expectError, test.input, err)
			}
		})
	}
}
