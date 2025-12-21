package cmds

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
	"testing"

	"github.com/spf13/cobra"
)

func TestRemoveCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Test command creation
	remove := &removeCmd{}
	celer := &configs.Celer{}

	cmd := remove.Command(celer)
	if cmd == nil {
		t.Fatal("Command should not be nil")
	}

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

	remove := &removeCmd{}
	cmd := &cobra.Command{}

	// Mock the getInstalledPackages method behavior
	originalInstalledDir := dirs.InstalledDir
	defer func() { dirs.InstalledDir = originalInstalledDir }()

	// Create temporary test directory
	testDir := filepath.Join(os.TempDir(), "celer-test-completion")
	defer os.RemoveAll(testDir)
	dirs.InstalledDir = testDir

	traceDir := filepath.Join(testDir, "celer", "trace")
	if err := os.MkdirAll(traceDir, 0755); err != nil {
		t.Fatalf("Failed to create test directory: %v", err)
	}

	// Create a test trace file
	tracePath := filepath.Join(traceDir, "boost@1.87.0@x86_64-linux.trace")
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
			suggestions, directive := remove.completion(cmd, []string{}, test.toComplete)

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
			err := remove.validatePackageNames(test.packages)
			if test.expectError && err == nil {
				t.Errorf("Expected error for %s, got nil", test.description)
			}
			if !test.expectError && err != nil {
				t.Errorf("Expected no error for %s, got: %v", test.description, err)
			}
		})
	}
}

func TestRemoveCmd_GetInstalledPackages(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	remove := &removeCmd{}

	// Create temporary test directory structure.
	testDir := filepath.Join(os.TempDir(), "celer-test-remove")
	defer os.RemoveAll(testDir)

	// Backup original dirs.InstalledDir.
	originalInstalledDir := dirs.InstalledDir
	dirs.InstalledDir = testDir
	defer func() { dirs.InstalledDir = originalInstalledDir }()

	traceDir := filepath.Join(testDir, "celer", "trace")
	if err := os.MkdirAll(traceDir, 0755); err != nil {
		t.Fatalf("Failed to create test directory: %v", err)
	}

	// Create test trace files
	testFiles := []string{
		"boost@1.87.0@x86_64-linux.trace",
		"openssl@3.5.0@x86_64-linux-dev.trace",
		"opencv@4.11.0@arm64-linux.trace",
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
			result := remove.getInstalledPackages(test.toComplete)

			if len(result) != len(test.expected) {
				t.Errorf("Expected %d packages, got %d: %v", len(test.expected), len(result), result)
				return
			}

			for _, expected := range test.expected {
				found := slices.Contains(result, expected)
				if !found {
					t.Errorf("Expected to find %s in result %v", expected, result)
				}
			}
		})
	}
}

func TestRemoveCmd_GetInstalledPackages_NoTraceDir(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	remove := &removeCmd{}

	// Backup original dirs.InstalledDir.
	originalInstalledDir := dirs.InstalledDir
	dirs.InstalledDir = "/nonexistent/directory"
	defer func() { dirs.InstalledDir = originalInstalledDir }()

	result := remove.getInstalledPackages("any")
	if len(result) != 0 {
		t.Errorf("Expected empty result when trace directory doesn't exist, got %v", result)
	}
}

func TestRemoveCmd_Execute_ValidationError(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	remove := &removeCmd{
		celer: &configs.Celer{},
	}

	// Test with invalid package name.
	err := remove.execute([]string{"invalid-package"})
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
	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Cleanup workspace.
	dirs.RemoveAllForTest()

	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14.44", "x86_64-windows-msvc-community-14.44")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_remove"
	)

	// Init celer.
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))
	check(celer.Setup())

	var (
		packageFolder = fmt.Sprintf("%s@%s@%s@%s", nameVersion, platform, project, celer.BuildType())
		port          configs.Port
		options       configs.InstallOptions
	)

	check(port.Init(celer, nameVersion))
	check(port.InstallFromSource(options))

	// Check if package dir exists.
	packageDir := filepath.Join(dirs.PackagesDir, packageFolder)
	if !fileio.PathExists(packageDir) {
		t.Fatalf("package dir cannot found : %s", packageDir)
	}

	// Check if installed.
	installed, err := port.Installed()
	check(err)
	if !installed {
		t.Fatal("package is not installed")
	}

	check(port.Remove(option))
	return port
}
