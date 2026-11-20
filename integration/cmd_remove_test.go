package integration

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

func TestRemoveCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Test command creation
	remove := &removeCmd{}
	celer := &configs.Celer{}

	cmd := remove.Command(celer)
	if cmd.Use != "remove" {
		t.Errorf("Expected Use to be 'remove', got %s", cmd.Use)
	}

	if cmd.Short == "" {
		t.Error("Short description should not be empty")
	}

	if cmd.Long == "" {
		t.Error("Long description should not be empty")
	}

	// Test flags.
	tests := []struct {
		flagName     string
		shorthand    string
		defaultValue string
	}{
		{"build-cache", "c", "false"},
		{"recursive", "r", "false"},
		{"purge", "p", "false"},
		{"dev", "d", "false"},
	}

	for _, test := range tests {
		t.Run(test.flagName, func(t *testing.T) {
			flag := cmd.Flags().Lookup(test.flagName)
			if flag == nil {
				t.Errorf("Flag --%s should be defined", test.flagName)
				return
			}

			if flag.Shorthand != test.shorthand {
				t.Errorf("Expected shorthand %s, got %s", test.shorthand, flag.Shorthand)
			}

			if flag.DefValue != test.defaultValue {
				t.Errorf("Expected default value %s, got %s", test.defaultValue, flag.DefValue)
			}
		})
	}

	// Test minimum args requirement
	if cmd.Args == nil {
		t.Error("Args validation should be set")
	}
}

func TestRemoveCmd_Completion(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	celer := newTestCeler(t, "", testProject)
	remove := &removeCmd{Celer: celer}
	cmd := &cobra.Command{}

	// Mock the getInstalledPackages method behavior
	originalInstalledDir := dirs.InstalledDir
	defer func() { dirs.InstalledDir = originalInstalledDir }()

	// Traces are stored per platform/project/build type under <installed>/infos/traces.
	testDir := t.TempDir()
	dirs.InstalledDir = testDir

	traceDir := filepath.Join(testDir, "infos", "traces", celer.Platform().GetName(), celer.Project().GetName(), celer.BuildType())
	if err := os.MkdirAll(traceDir, 0755); err != nil {
		t.Fatalf("Failed to create test directory: %v", err)
	}

	// Create a test trace file
	tracePath := filepath.Join(traceDir, "boost@1.87.0.trace")
	if err := os.WriteFile(tracePath, []byte("test"), 0644); err != nil {
		t.Fatalf("Failed to create test file: %v", err)
	}

	tests := []struct {
		name             string
		toComplete       string
		expectedPackages []string
		expectedFlags    []string
	}{
		{
			name:             "empty_input",
			toComplete:       "",
			expectedPackages: []string{"boost@1.87.0"},
			expectedFlags:    []string{"--build-cache", "-c", "--recursive", "-r", "--purge", "-p", "--dev", "-d"},
		},
		{
			name:             "flag_prefix",
			toComplete:       "--",
			expectedPackages: []string{},
			expectedFlags:    []string{"--build-cache", "--recursive", "--purge", "--dev"},
		},
		{
			name:             "specific_flag",
			toComplete:       "--rec",
			expectedPackages: []string{},
			expectedFlags:    []string{"--recursive"},
		},
		{
			name:             "short_flag",
			toComplete:       "-r",
			expectedPackages: []string{},
			expectedFlags:    []string{"-r"},
		},
		{
			name:             "package_prefix",
			toComplete:       "boost",
			expectedPackages: []string{"boost@1.87.0"},
			expectedFlags:    []string{},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			suggestions, directive := remove.Completion(cmd, []string{}, test.toComplete)

			// Count expected total suggestions.
			expectedTotal := len(test.expectedPackages) + len(test.expectedFlags)

			if len(suggestions) != expectedTotal {
				t.Errorf("Expected %d suggestions, got %d: %v", expectedTotal, len(suggestions), suggestions)
			}

			// Check that all expected packages are present.
			for _, expectedPkg := range test.expectedPackages {
				found := false
				for _, suggestion := range suggestions {
					if suggestion == expectedPkg {
						found = true
						break
					}
				}
				if !found {
					t.Errorf("Expected package %s not found in suggestions %v", expectedPkg, suggestions)
				}
			}

			// Check that all expected flags are present.
			for _, expectedFlag := range test.expectedFlags {
				found := false
				for _, suggestion := range suggestions {
					if suggestion == expectedFlag {
						found = true
						break
					}
				}
				if !found {
					t.Errorf("Expected flag %s not found in suggestions %v", expectedFlag, suggestions)
				}
			}

			if directive != cobra.ShellCompDirectiveNoFileComp {
				t.Errorf("Expected NoFileComp directive, got %d", directive)
			}
		})
	}
}

func TestRemoveCmd_ValidatePackageNames(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	remove := &removeCmd{}

	tests := []struct {
		name        string
		packages    []string
		expectError bool
		description string
	}{
		{
			name:        "valid_packages",
			packages:    []string{"boost@1.87.0", "openssl@3.5.0"},
			expectError: false,
			description: "Should accept valid package names",
		},
		{
			name:        "empty_string",
			packages:    []string{""},
			expectError: true,
			description: "Should reject empty package names",
		},
		{
			name:        "whitespace_only",
			packages:    []string{"   "},
			expectError: true,
			description: "Should reject whitespace-only package names",
		},
		{
			name:        "missing_version",
			packages:    []string{"boost"},
			expectError: true,
			description: "Should reject package names without version",
		},
		{
			name:        "missing_name",
			packages:    []string{"@1.87.0"},
			expectError: true,
			description: "Should reject version without package name",
		},
		{
			name:        "invalid_format",
			packages:    []string{"boost-1.87.0"},
			expectError: true,
			description: "Should reject packages without @ separator",
		},
		{
			name:        "complex_names",
			packages:    []string{"opencv_contrib@4.11.0", "my-package@1.0.0-beta"},
			expectError: false,
			description: "Should accept complex valid package names",
		},
		{
			name:        "mixed_valid_invalid",
			packages:    []string{"boost@1.87.0", "invalid-package"},
			expectError: true,
			description: "Should reject if any package is invalid",
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			_, err := remove.ValidatePackageNames(test.packages)
			if test.expectError && err == nil {
				t.Errorf("Expected error for %s, got nil", test.description)
			}
			if !test.expectError && err != nil {
				t.Errorf("Expected no error for %s, got: %v", test.description, err)
			}
		})
	}
}

func TestRemoveCmd_NormalizePackageNames_PowerShellEscaped(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	remove := &removeCmd{}
	input := []string{"boost`@1.87.0", "  openssl@3.5.0  "}

	normalized, err := remove.ValidatePackageNames(input)
	if err != nil {
		t.Fatalf("expected normalized package names, got error: %v", err)
	}

	expected := []string{"boost@1.87.0", "openssl@3.5.0"}
	if !slices.Equal(normalized, expected) {
		t.Fatalf("expected %v, got %v", expected, normalized)
	}
}

func TestRemoveCmd_Execute_ValidationBeforeInit(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	remove := &removeCmd{
		Celer: nil, // Should not be touched for invalid input.
	}

	err := remove.Execute([]string{"invalid-package"})
	if err == nil {
		t.Fatal("expected validation error")
	}

	if !strings.Contains(err.Error(), "invalid package names") {
		t.Fatalf("expected validation-first error, got: %v", err)
	}
}

func TestRemoveCmd_GetInstalledPackages(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	celer := newTestCeler(t, "", testProject)
	remove := &removeCmd{Celer: celer}

	// Create temporary test directory structure.
	testDir := t.TempDir()

	// Backup original dirs.InstalledDir.
	originalInstalledDir := dirs.InstalledDir
	dirs.InstalledDir = testDir
	defer func() { dirs.InstalledDir = originalInstalledDir }()

	// Traces are stored per platform/project/build type under <installed>/infos/traces.
	traceDir := filepath.Join(testDir, "infos", "traces", celer.Platform().GetName(), celer.Project().GetName(), celer.BuildType())
	if err := os.MkdirAll(traceDir, 0755); err != nil {
		t.Fatalf("Failed to create test directory: %v", err)
	}

	// Create test trace files
	testFiles := []string{
		"boost@1.87.0.trace",
		"openssl@3.5.0.trace",
		"opencv@4.11.0.trace",
		"invalid-file.txt",
		"no-extension",
	}

	for _, filename := range testFiles {
		filePath := filepath.Join(traceDir, filename)
		if err := os.WriteFile(filePath, []byte("test content"), 0644); err != nil {
			t.Fatalf("Failed to create test file %s: %v", filename, err)
		}
	}

	// Create a subdirectory to test directory filtering.
	subDir := filepath.Join(traceDir, "subdir")
	if err := os.MkdirAll(subDir, 0755); err != nil {
		t.Fatalf("Failed to create test subdirectory: %v", err)
	}

	tests := []struct {
		name       string
		toComplete string
		expected   []string
	}{
		{
			name:       "empty_prefix",
			toComplete: "",
			expected:   []string{"boost@1.87.0", "openssl@3.5.0", "opencv@4.11.0"},
		},
		{
			name:       "boost_prefix",
			toComplete: "boost",
			expected:   []string{"boost@1.87.0"},
		},
		{
			name:       "opencv_prefix",
			toComplete: "opencv",
			expected:   []string{"opencv@4.11.0"},
		},
		{
			name:       "nonexistent_prefix",
			toComplete: "nonexistent",
			expected:   []string{},
		},
		{
			name:       "partial_name",
			toComplete: "op",
			expected:   []string{"openssl@3.5.0", "opencv@4.11.0"},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			suggestions := remove.GetSuggestions(test.toComplete)

			if len(suggestions) != len(test.expected) {
				t.Errorf("Expected %d packages, got %d: %v", len(test.expected), len(suggestions), suggestions)
				return
			}

			for _, expected := range test.expected {
				found := slices.Contains(suggestions, expected)
				if !found {
					t.Errorf("Expected to find %s in result %v", expected, suggestions)
				}
			}
		})
	}
}

func TestRemoveCmd_Completion_DeduplicateInstalledPackages(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	celer := newTestCeler(t, "", testProject)
	remove := &removeCmd{Celer: celer}
	cmd := &cobra.Command{}

	originalInstalledDir := dirs.InstalledDir
	defer func() { dirs.InstalledDir = originalInstalledDir }()

	testDir := t.TempDir()
	dirs.InstalledDir = testDir

	traceDir := filepath.Join(testDir, "infos", "traces", celer.Platform().GetName(), celer.Project().GetName(), celer.BuildType())
	if err := os.MkdirAll(traceDir, 0755); err != nil {
		t.Fatalf("failed to create trace dir: %v", err)
	}

	// A trace named like a flag yields the same suggestion as the flag itself, which must
	// be listed only once.
	testFiles := []string{
		"boost@1.87.0.trace",
		"--purge.trace",
	}
	for _, filename := range testFiles {
		if err := os.WriteFile(filepath.Join(traceDir, filename), []byte("test"), 0644); err != nil {
			t.Fatalf("failed to create trace file %s: %v", filename, err)
		}
	}

	suggestions, directive := remove.Completion(cmd, nil, "")
	if directive != cobra.ShellCompDirectiveNoFileComp {
		t.Fatalf("expected NoFileComp directive, got: %v", directive)
	}

	var purgeCount int
	for _, suggestion := range suggestions {
		if suggestion == "--purge" {
			purgeCount++
		}
	}
	if purgeCount != 1 {
		t.Fatalf("expected deduplicated suggestion count 1, got %d (all: %v)", purgeCount, suggestions)
	}
}

func TestRemoveCmd_GetInstalledPackages_NoTraceDir(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	celer := newTestCeler(t, "", testProject)
	remove := &removeCmd{Celer: celer}

	// Backup original dirs.InstalledDir.
	originalInstalledDir := dirs.InstalledDir
	dirs.InstalledDir = "/nonexistent/directory"
	defer func() { dirs.InstalledDir = originalInstalledDir }()

	result := remove.GetSuggestions("any")
	if len(result) != 0 {
		t.Errorf("Expected empty result when trace directory doesn't exist, got %v", result)
	}
}

func TestRemoveCmd_Execute_ValidationError(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	remove := &removeCmd{
		Celer: &configs.Celer{},
	}

	// Test with invalid package name.
	err := remove.Execute([]string{"invalid-package"})
	if err == nil {
		t.Error("Expected error for invalid package name, got nil")
	}

	if !strings.Contains(err.Error(), "invalid package names") {
		t.Errorf("Expected validation error, got: %v", err)
	}
}

func TestRemoveCmd_Default(t *testing.T) {
	installedPort := installForTestRemove(t, "glog@0.6.0", configs.RemoveOptions{})
	if installed, err := installedPort.Installed(); err != nil {
		t.Fatalf("failed to check installation status of glog@0.6.0: %v", err)
	} else if installed {
		t.Fatal("glog@0.6.0 should have been removed")
	}
}

func TestRemoveCmd_BuildCache(t *testing.T) {
	installedPort := installForTestRemove(t, "glog@0.6.0", configs.RemoveOptions{
		BuildCache: true,
	})
	if installed, err := installedPort.Installed(); err != nil {
		t.Fatalf("failed to check installation status of glog@0.6.0: %v", err)
	} else if installed {
		t.Fatal("glog@0.6.0 should have been removed")
	}

	if fileio.PathExists(installedPort.MatchedConfig.PortConfig.BuildDir) {
		t.Fatalf("build cache for glog@0.6.0 should have been removed")
	}
}

func TestRemoveCmd_Purge(t *testing.T) {
	installedPort := installForTestRemove(t, "glog@0.6.0", configs.RemoveOptions{
		Purge: true,
	})

	// Check if still installed.
	if installed, err := installedPort.Installed(); err != nil {
		t.Fatalf("failed to check installation status of glog@0.6.0: %v", err)
	} else if installed {
		t.Fatal("glog@0.6.0 should have been removed")
	}

	// Check if package files are removed.
	if fileio.PathExists(installedPort.PackageDir) {
		t.Fatalf("package files for glog@0.6.0 should have been purged")
	}
}

func TestRemoveCmd_Recursive(t *testing.T) {
	installedPort := installForTestRemove(t, "glog@0.6.0", configs.RemoveOptions{
		Recursive: true,
	})

	// Check if still installed.
	if installed, err := installedPort.Installed(); err != nil {
		t.Fatalf("failed to check installation status of glog@0.6.0: %v", err)
	} else if installed {
		t.Fatal("glog@0.6.0 should have been removed")
	}

	// Check if dependency gflags@2.2.2 is also removed.
	gflagsPort := configs.Port{}
	if err := gflagsPort.Init(installedPort.MatchedConfig.Ctx, "gflags@2.2.2"); err != nil {
		t.Fatalf("failed to initialize gflags@2.2.2 port: %v", err)
	}
	if installed, err := gflagsPort.Installed(); err != nil {
		t.Fatalf("failed to check installation status of gflags@2.2.2: %v", err)
	} else if installed {
		t.Fatal("gflags@2.2.2 should have been removed")
	}
}

func installForTestRemove(t *testing.T, nameVersion string, option configs.RemoveOptions) configs.Port {
	// Cleanup workspace.
	dirs.RemoveAllForTest()

	var (
		platform = buildTestPlatform()
		project  = testProjectName
	)

	// Init celer.
	celer := newTestCeler(t, platform, testProject)
	check(t, celer.SetBuildType("Release"))
	check(t, celer.SetPlatform(platform))
	check(t, celer.SetProject(project))

	var (
		packageFolder = filepath.Join(platform, project, celer.BuildType(), nameVersion)
		port          configs.Port
		options       = configs.InstallOptions{Prefer: configs.PreferSource}
	)

	check(t, port.Init(celer, nameVersion))
	if _, err := port.Install(options); err != nil {
		t.Fatal(err)
	}

	// Check if package dir exists.
	packageDir := filepath.Join(dirs.PackagesDir, packageFolder)
	if !fileio.PathExists(packageDir) {
		t.Fatalf("package dir cannot found : %s", packageDir)
	}

	// Check if installed.
	installed, err := port.Installed()
	check(t, err)
	if !installed {
		t.Fatal("package is not installed")
	}

	check(t, port.Remove(option))
	return port
}
