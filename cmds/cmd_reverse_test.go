package cmds

import (
	"celer/buildsystems"
	"celer/configs"
	"celer/pkgs/dirs"
	"os"
	"path/filepath"
	"slices"
	"strings"
	"testing"
)

// TestReverse_ValidatePackageName tests input validation
func TestReverse_ValidatePackageName(t *testing.T) {
	cmd := reverseCmd{}

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

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			err := cmd.validatePackageName(tt.packageName)
			if (err != nil) != tt.wantError {
				t.Errorf("validatePackageName() error = %v, wantError %v", err, tt.wantError)
			}
		})
	}
}

// TestReverse_EmptyPorts tests behavior when no ports directory exists
func TestReverse_EmptyPorts(t *testing.T) {
	// Create a temporary directory structure without ports
	tempDir := t.TempDir()
	originalPortsDir := dirs.PortsDir
	defer func() {
		dirs.PortsDir = originalPortsDir
	}()
	dirs.PortsDir = filepath.Join(tempDir, "nonexistent")

	cmd := reverseCmd{}

	// Test with non-existent ports directory
	results, err := cmd.query("nonexistent@1.0.0")
	if err != nil {
		t.Fatalf("expected no error, got %v", err)
	}
	if len(results) != 0 {
		t.Fatalf("expected empty results, got %v", results)
	}
}

func TestReverse_Without_Dev(t *testing.T) {
	// Check error.
	check := func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Cleanup function.
	cleanup := func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
		check(os.RemoveAll(dirs.ConfDir))
	}
	t.Cleanup(cleanup)

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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	cmdReverse := reverseCmd{celer: celer}
	dependencies, err := cmdReverse.query("eigen@3.4.0")
	check(err)

	expected := []string{
		"ceres-solver@2.1.0",
		"gstreamer@1.26.0",
		"gtsam@4.2.0",
		"lbfgspp@0.3.0",
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

func TestReverse_With_Dev(t *testing.T) {
	// Check error.
	check := func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Cleanup function.
	cleanup := func() {
		check(os.RemoveAll(filepath.Join(dirs.WorkspaceDir, "celer.toml")))
		check(os.RemoveAll(dirs.TmpDir))
		check(os.RemoveAll(dirs.TestCacheDir))
		check(os.RemoveAll(dirs.ConfDir))
	}
	t.Cleanup(cleanup)

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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", false))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	// Search as default mode.
	cmdReverse := reverseCmd{celer: celer}
	dependencies, err := cmdReverse.query("nasm@2.16.03")
	check(err)
	if len(dependencies) > 0 {
		t.Fatalf("expected no dependencies, but got %s", dependencies)
	}

	// Search as dev mode.
	cmdReverse.dev = true
	dependencies, err = cmdReverse.query("nasm@2.16.03")
	check(err)
	expected := []string{
		"ffmpeg@3.4.13",
		"ffmpeg@5.1.6",
		"openssl@3.5.0",
		"x264@stable",
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

// TestReverse_HasDependency tests the dependency checking logic
func TestReverse_HasDependency(t *testing.T) {
	cmd := reverseCmd{}

	// Mock port with dependencies
	port := configs.Port{
		MatchedConfig: &buildsystems.BuildConfig{
			Dependencies:    []string{"dep1@1.0.0", "dep2@2.0.0"},
			DevDependencies: []string{"devdep1@1.0.0", "devdep2@2.0.0"},
		},
	}

	// Test regular dependencies
	if !cmd.hasDependency(port, "dep1@1.0.0") {
		t.Error("expected to find regular dependency")
	}

	if cmd.hasDependency(port, "nonexistent@1.0.0") {
		t.Error("expected not to find nonexistent dependency")
	}

	// Test dev dependencies (should not be found in non-dev mode)
	if cmd.hasDependency(port, "devdep1@1.0.0") {
		t.Error("expected not to find dev dependency in regular mode")
	}

	// Test dev dependencies in dev mode
	cmd.dev = true
	if !cmd.hasDependency(port, "devdep1@1.0.0") {
		t.Error("expected to find dev dependency in dev mode")
	}

	if !cmd.hasDependency(port, "dep1@1.0.0") {
		t.Error("expected to find regular dependency in dev mode")
	}
}

// TestReverse_Completion tests the autocompletion functionality
func TestReverse_Completion(t *testing.T) {
	cmd := reverseCmd{}

	suggestions, directive := cmd.completion(nil, []string{}, "test")

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
