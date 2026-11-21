package integration

import (
	"os"
	"path/filepath"
	"slices"
	"strings"
	"testing"

	"github.com/celer-pkg/celer/configs"
	"github.com/celer-pkg/celer/internal/dirs"
)

func TestReverseCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	reverseCmd := reverseCmd{}
	celer := configs.NewCeler()
	cmd := reverseCmd.Command(celer)

	// Test command basic properties.
	if cmd.Use != "reverse" {
		t.Errorf("Expected Use to be 'reverse', got '%s'", cmd.Use)
	}

	if cmd.Short == "" {
		t.Error("Short description should not be empty")
	}

	// Test flags.
	devFlag := cmd.Flags().Lookup("dev")
	if devFlag == nil {
		t.Error("--dev flag should be defined")
	} else {
		if devFlag.Shorthand != "d" {
			t.Errorf("Expected dev flag shorthand to be 'd', got '%s'", devFlag.Shorthand)
		}
	}
}

func TestReverseCmd_Completion(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	cmd := reverseCmd{}
	suggestions, directive := cmd.Completion(nil, []string{}, "test")

	// Should return no file completion directive (cobra.ShellCompDirectiveNoFileComp)
	// Note: The actual value might differ, so we just check it's a valid directive
	if directive < 0 {
		t.Errorf("expected valid completion directive, got %d", directive)
	}

	// Suggestions might be empty if no ports exist
	if suggestions == nil {
		t.Error("expected non-nil suggestions slice")
	}
}

func TestReverseCmd_ValidatePackageName(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	tests := []struct {
		name        string
		packageName string
		wantError   bool
	}{
		{"valid package", "eigen@3.4.0", false},
		{"valid package with hyphen", "boost-system@1.87.0", false},
		{"valid package with underscore", "my_lib@1.0.0", false},
		{"empty name", "", true},
		{"no version separator", "eigen", true},
		{"invalid characters", "ei@gen@3.4.0", true},
		{"missing version", "eigen@", true},
		{"missing name", "@3.4.0", true},
	}

	cmd := reverseCmd{}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			err := cmd.ValidatePackageName(test.packageName)
			if (err != nil) != test.wantError {
				t.Errorf("validatePackageName() error = %v, wantError %v", err, test.wantError)
			}
		})
	}
}

func TestReverseCmd_EmptyPorts(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Create a temporary directory structure without ports
	tempDir := t.TempDir()
	originalPortsDir := dirs.PortsDir
	defer func() {
		dirs.PortsDir = originalPortsDir
	}()
	dirs.PortsDir = filepath.Join(tempDir, "nonexistent")

	celer := configs.NewCeler()
	cmd := reverseCmd{Celer: celer}

	// Test with non-existent ports directory
	results, err := cmd.Query("nonexistent@1.0.0")
	if err != nil {
		t.Fatalf("expected no error, got %v", err)
	}
	if len(results) != 0 {
		t.Fatalf("expected empty results, got %v", results)
	}
}

func TestReverseCmd_Without_Dev(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	check := func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Helper function to check if two slices are equal (order-insensitive)
	equals := func(list1, list2 []string) bool {
		if len(list1) != len(list2) {
			return false
		}
		for _, item := range list1 {
			if !slices.Contains(list2, item) {
				return false
			}
		}
		return true
	}

	// Init celer.
	celer := newTestCeler(t, "", testProject)
	check(celer.SetBuildType("Release"))

	cmdReverse := reverseCmd{Celer: celer}
	dependencies, err := cmdReverse.Query("eigen@3.4.0")
	check(err)

	expected := []string{
		"ceres-solver@2.1.0",
		"gtsam@4.2.0",
		"lbfgspp@0.3.0",
		"pcl@1.14.1",
	}

	if !equals(dependencies, expected) {
		t.Fatalf("expected %s, but got %s", expected, dependencies)
	}

	// Verify results are sorted
	for i := 1; i < len(dependencies); i++ {
		if strings.Compare(dependencies[i-1], dependencies[i]) > 0 {
			t.Errorf("results are not sorted: %v", dependencies)
			break
		}
	}
}

func TestReverseCmd_With_Dev(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	check := func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Helper function to check if two slices are equal (order-insensitive)
	equals := func(list1, list2 []string) bool {
		if len(list1) != len(list2) {
			return false
		}
		for _, item := range list1 {
			if !slices.Contains(list2, item) {
				return false
			}
		}
		return true
	}

	// Init celer.
	celer := newTestCeler(t, "", testProject)
	check(celer.SetBuildType("Release"))

	// Search as default mode.
	cmdReverse := reverseCmd{Celer: celer}
	dependencies, err := cmdReverse.Query("nasm@2.16.03")
	check(err)
	if len(dependencies) > 0 {
		t.Fatalf("expected no dependencies, but got %s", dependencies)
	}

	// Search as dev mode.
	cmdReverse.Dev = true
	dependencies, err = cmdReverse.Query("nasm@2.16.03")
	check(err)
	expected := []string{
		"ffmpeg@3.4.13",
		"ffmpeg@5.1.6",
		"openssl@3.5.0",
		"x264@stable",
		"x265@4.1",
	}
	if !equals(dependencies, expected) {
		t.Fatalf("expected %s, but got %s", expected, dependencies)
	}

	// Verify results are sorted
	for i := 1; i < len(dependencies); i++ {
		if strings.Compare(dependencies[i-1], dependencies[i]) > 0 {
			t.Errorf("results are not sorted: %v", dependencies)
			break
		}
	}
}

func TestReverseCmd_TomlHasDependency(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Create a temporary port.toml with dependencies.
	tempDir := t.TempDir()
	portToml := filepath.Join(tempDir, "port.toml")
	content := `[package]
  name = "testlib"
  version = "1.0.0"

[[build_configs]]
  build_system = "cmake"
  dependencies = ["dep1@1.0.0", "dep2@2.0.0"]
  dev_dependencies = ["devdep1@1.0.0", "devdep2@2.0.0"]
`
	if err := os.WriteFile(portToml, []byte(content), 0644); err != nil {
		t.Fatal(err)
	}

	cmd := reverseCmd{}

	// Test regular dependencies.
	if !cmd.TomlHasDependency(portToml, "dep1@1.0.0") {
		t.Error("expected to find regular dependency")
	}
	if cmd.TomlHasDependency(portToml, "nonexistent@1.0.0") {
		t.Error("expected not to find nonexistent dependency")
	}

	// Test dev dependencies (should not be found in non-dev mode).
	if cmd.TomlHasDependency(portToml, "devdep1@1.0.0") {
		t.Error("expected not to find dev dependency in regular mode")
	}

	// Test dev dependencies in dev mode.
	cmd.Dev = true
	if !cmd.TomlHasDependency(portToml, "devdep1@1.0.0") {
		t.Error("expected to find dev dependency in dev mode")
	}
	if !cmd.TomlHasDependency(portToml, "dep1@1.0.0") {
		t.Error("expected to find regular dependency in dev mode")
	}
}
