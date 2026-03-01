package cmds

import (
	"celer/buildtools"
	"celer/configs"
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"celer/pkgs/git"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"testing"

	"github.com/spf13/cobra"
)

func TestCleanCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	cleanCmd := cleanCmd{}
	celer := configs.NewCeler()
	cmd := cleanCmd.Command(celer)

	// Test command basic properties.
	if cmd.Use != "clean" {
		t.Errorf("Expected Use to be 'clean', got '%s'", cmd.Use)
	}

	if cmd.Short == "" {
		t.Error("Short description should not be empty")
	}

	if cmd.Long == "" {
		t.Error("Long description should not be empty")
	}

	// Test flags.
	recursiveFlag := cmd.Flags().Lookup("recursive")
	if recursiveFlag == nil {
		t.Error("--recursive flag should be defined")
	} else {
		if recursiveFlag.Shorthand != "r" {
			t.Errorf("Expected recursive flag shorthand to be 'r', got '%s'", recursiveFlag.Shorthand)
		}
	}

	devFlag := cmd.Flags().Lookup("dev")
	if devFlag == nil {
		t.Error("--dev flag should be defined")
	} else {
		if devFlag.Shorthand != "d" {
			t.Errorf("Expected dev flag shorthand to be 'd', got '%s'", devFlag.Shorthand)
		}
	}

	allFlag := cmd.Flags().Lookup("all")
	if allFlag == nil {
		t.Error("--all flag should be defined")
	} else {
		if allFlag.Shorthand != "a" {
			t.Errorf("Expected all flag shorthand to be 'a', got '%s'", allFlag.Shorthand)
		}
	}
}

func TestCleanCmd_ArgsValidation(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	tests := []struct {
		name        string
		all         bool
		args        []string
		expectError bool
	}{
		{
			name:        "no_targets_without_all_should_fail",
			all:         false,
			args:        []string{},
			expectError: true,
		},
		{
			name:        "targets_without_all_should_succeed",
			all:         false,
			args:        []string{"x264@stable"},
			expectError: false,
		},
		{
			name:        "all_without_targets_should_succeed",
			all:         true,
			args:        []string{},
			expectError: false,
		},
		{
			name:        "all_with_targets_should_fail",
			all:         true,
			args:        []string{"x264@stable"},
			expectError: true,
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			clean := cleanCmd{}
			cmd := clean.Command(configs.NewCeler())

			if err := cmd.Flags().Set("all", expr.If(test.all, "true", "false")); err != nil {
				t.Fatalf("failed to set --all flag: %v", err)
			}

			err := cmd.Args(cmd, test.args)
			if test.expectError && err == nil {
				t.Fatal("expected args validation error")
			}
			if !test.expectError && err != nil {
				t.Fatalf("expected args validation success, got: %v", err)
			}
		})
	}
}

func TestCleanCmd_ValidateTargets(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	tests := []struct {
		name        string
		targets     []string
		expectError bool
	}{
		{
			name:        "no_targets",
			targets:     []string{},
			expectError: true,
		},
		{
			name:        "single_package",
			targets:     []string{"x264@stable"},
			expectError: false,
		},
		{
			name:        "multiple_packages",
			targets:     []string{"x264@stable", "ffmpeg@3.4.13"},
			expectError: false,
		},
		{
			name:        "project_target",
			targets:     []string{"my-project"},
			expectError: false,
		},
		{
			name:        "mixed_targets",
			targets:     []string{"x264@stable", "my-project"},
			expectError: false,
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			cleanCmd := cleanCmd{}
			err := cleanCmd.validateTargets(test.targets)

			if test.expectError && err == nil {
				t.Error("Expected error but got none")
			} else if !test.expectError && err != nil {
				t.Errorf("Expected no error but got: %v", err)
			}
		})
	}
}

func TestCleanCmd_Completion(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Setup test environment.
	os.MkdirAll(dirs.BuildtreesDir, os.ModePerm)
	os.MkdirAll(dirs.ConfProjectsDir, os.ModePerm)

	// Create test buildtrees.
	os.MkdirAll(filepath.Join(dirs.BuildtreesDir, "x264@stable"), os.ModePerm)
	os.MkdirAll(filepath.Join(dirs.BuildtreesDir, "ffmpeg@3.4.13"), os.ModePerm)

	// Create test projects.
	os.WriteFile(filepath.Join(dirs.ConfProjectsDir, "project1.toml"), []byte{}, 0644)
	os.WriteFile(filepath.Join(dirs.ConfProjectsDir, "project2.toml"), []byte{}, 0644)

	cleanCmd := cleanCmd{}
	celer := configs.NewCeler()
	cmd := cleanCmd.Command(celer)

	tests := []struct {
		name          string
		toComplete    string
		expectContain []string
	}{
		{
			name:          "complete_package_prefix",
			toComplete:    "x264",
			expectContain: []string{"x264@stable"},
		},
		{
			name:          "complete_project_prefix",
			toComplete:    "project",
			expectContain: []string{"project1", "project2"},
		},
		{
			name:          "complete_dev_flag",
			toComplete:    "--d",
			expectContain: []string{"--dev"},
		},
		{
			name:          "complete_dev_short_flag",
			toComplete:    "-d",
			expectContain: []string{"-d"},
		},
		{
			name:          "complete_recursive_flag",
			toComplete:    "--r",
			expectContain: []string{"--recursive"},
		},
		{
			name:          "complete_all_flag",
			toComplete:    "--a",
			expectContain: []string{"--all"},
		},
		{
			name:          "no_match",
			toComplete:    "nonexistent",
			expectContain: []string{},
		},
	}

	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			suggestions, directive := cleanCmd.completion(cmd, []string{}, test.toComplete)

			if directive != cobra.ShellCompDirectiveNoFileComp {
				t.Errorf("Expected directive %v, got %v", cobra.ShellCompDirectiveNoFileComp, directive)
			}

			for _, expected := range test.expectContain {
				found := false
				for _, suggestion := range suggestions {
					if suggestion == expected {
						found = true
						break
					}
				}
				if !found {
					t.Errorf("Expected to find '%s' in suggestions %v", expected, suggestions)
				}
			}
		})
	}
}

func TestCleanCmd_Completion_FilterAndShortFlags(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Setup test environment.
	if err := os.MkdirAll(dirs.BuildtreesDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.MkdirAll(dirs.ConfProjectsDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	// Buildtree entries.
	if err := os.MkdirAll(filepath.Join(dirs.BuildtreesDir, "zlib@1.3.1"), os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(dirs.BuildtreesDir, "not-a-dir"), []byte("x"), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	// Project entries.
	if err := os.WriteFile(filepath.Join(dirs.ConfProjectsDir, "proj1.toml"), []byte{}, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(dirs.ConfProjectsDir, "README.md"), []byte{}, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.MkdirAll(filepath.Join(dirs.ConfProjectsDir, "nested"), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	clean := cleanCmd{}
	cmd := clean.Command(configs.NewCeler())

	t.Run("short recursive flag", func(t *testing.T) {
		suggestions, _ := clean.completion(cmd, nil, "-r")
		found := slices.Contains(suggestions, "-r")
		if !found {
			t.Fatalf("expected -r in suggestions: %v", suggestions)
		}
	})

	t.Run("short all flag", func(t *testing.T) {
		suggestions, _ := clean.completion(cmd, nil, "-a")
		found := slices.Contains(suggestions, "-a")
		if !found {
			t.Fatalf("expected -a in suggestions: %v", suggestions)
		}
	})

	t.Run("ignore non toml project files", func(t *testing.T) {
		suggestions, _ := clean.completion(cmd, nil, "R")
		for _, suggestion := range suggestions {
			if suggestion == "README" {
				t.Fatalf("README should not be suggested: %v", suggestions)
			}
		}
	})
}

func TestCleanCmd_CleanAll_BuildtreesNotExist(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	clean := cleanCmd{celer: &configs.Celer{}}
	if err := clean.cleanAll(); err != nil {
		t.Fatalf("cleanAll should return nil when buildtrees does not exist: %v", err)
	}
}

func TestCleanCmd_CleanAll_NonDirEntryReturnsError(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	if err := os.MkdirAll(dirs.BuildtreesDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	nonDir := filepath.Join(dirs.BuildtreesDir, "some-file.txt")
	if err := os.WriteFile(nonDir, []byte("data"), 0644); err != nil {
		t.Fatal(err)
	}

	clean := cleanCmd{celer: &configs.Celer{}}
	if err := clean.cleanAll(); err == nil {
		t.Fatal("cleanAll should return error for non-directory entry in buildtrees")
	}
}

func TestCleanCmd_CleanAll_PortNotFound_RemovesNonSrcAndKeepsSrc(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	portDir := filepath.Join(dirs.BuildtreesDir, "unknown@0.0.1")
	srcDir := filepath.Join(portDir, "src")
	buildDir := filepath.Join(portDir, "a-build-dir")

	if err := os.MkdirAll(srcDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}
	if err := os.MkdirAll(buildDir, os.ModePerm); err != nil {
		t.Fatal(err)
	}

	clean := cleanCmd{celer: &configs.Celer{}}
	if err := clean.cleanAll(); err != nil {
		t.Fatalf("cleanAll should continue when port is not found: %v", err)
	}

	if !fileio.PathExists(srcDir) {
		t.Fatalf("src dir should be kept: %s", srcDir)
	}
	if fileio.PathExists(buildDir) {
		t.Fatalf("non-src build dir should be removed: %s", buildDir)
	}
}

func TestCleanCmd_Execute_ValidateTargetsError(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Avoid ports cloning in celer.Init by making ports dir non-empty.
	if err := os.MkdirAll(filepath.Join(dirs.PortsDir, "dummy"), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	clean := cleanCmd{
		celer: configs.NewCeler(),
		all:   false,
	}

	if err := clean.celer.Init(); err != nil {
		t.Fatalf("init celer: %v", err)
	}

	if err := buildtools.CheckTools(clean.celer, "git"); err != nil {
		t.Fatalf("failed to check git: %v", err)
	}

	err := clean.execute([]string{})
	if err == nil {
		t.Fatal("expected validation error for empty targets")
	}
	if err != configs.ErrSilent {
		t.Fatalf("expected ErrSilent, got: %#v", err)
	}
}

func TestCleanCmd_Execute_AllWithoutTargets(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Avoid ports cloning in celer.Init by making ports dir non-empty.
	if err := os.MkdirAll(filepath.Join(dirs.PortsDir, "dummy"), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	clean := cleanCmd{
		celer: configs.NewCeler(),
		all:   true,
	}

	if err := clean.execute([]string{}); err != nil {
		t.Fatalf("execute should succeed when --all is set with empty targets: %v", err)
	}
}

func TestClean(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer.
	var (
		windowsPlatform = expr.If(os.Getenv("GITHUB_ACTIONS") == "true", "x86_64-windows-msvc-enterprise-14", "x86_64-windows-msvc-community-14")
		platform        = expr.If(runtime.GOOS == "windows", windowsPlatform, "x86_64-linux-ubuntu-22.04-gcc-11.5.0")
		project         = "project_test_clean"
	)

	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf(test_conf_repo_url, test_conf_repo_branch, true))
	check(celer.SetBuildType("Release"))
	check(celer.SetPlatform(platform))
	check(celer.SetProject(project))
	check(celer.Deploy(true))

	cleanCmd := cleanCmd{celer: celer}

	var (
		buildDir = func(nameVersion string, dev bool) string {
			if dev {
				hostPlatform := celer.Platform().GetHostName() + "-dev"
				return fmt.Sprintf("%s/%s/%s", dirs.BuildtreesDir, nameVersion, hostPlatform)
			} else {
				return fmt.Sprintf("%s/%s/%s-%s-%s", dirs.BuildtreesDir, nameVersion, platform, project, celer.BuildType())
			}
		}
	)

	t.Run("clean port", func(t *testing.T) {
		cleanCmd.dev = false
		cleanCmd.recursive = false
		cleanCmd.all = false
		check(cleanCmd.clean("x264@stable"))
		if fileio.PathExists(buildDir("x264@stable", false)) {
			t.Fatal("x264@stable build dir should be removed")
		}
	})

	t.Run("clean port for dev", func(t *testing.T) {
		cleanCmd.dev = true
		cleanCmd.recursive = false
		cleanCmd.all = false
		check(cleanCmd.clean("m4@1.4.19"))
		if fileio.PathExists(buildDir("m4@1.4.19", true)) {
			t.Fatal("m4@1.4.19 build dir should be removed")
		}
	})

	t.Run("clean recursive", func(t *testing.T) {
		cleanCmd.dev = true
		cleanCmd.recursive = true
		cleanCmd.all = false
		check(cleanCmd.clean("automake@1.18"))

		checkList := map[string]bool{
			"nasm@2.16.03":  true,
			"automake@1.18": true,
			"autoconf@2.72": true,
			"m4@1.4.19":     true,
		}
		for nameVersion, dev := range checkList {
			if fileio.PathExists(buildDir(nameVersion, dev)) {
				t.Fatal(nameVersion + " build dir should be removed")
			}
		}
	})

	t.Run("clean all", func(t *testing.T) {
		cleanCmd.dev = true
		cleanCmd.recursive = true
		cleanCmd.all = true

		check(os.RemoveAll(dirs.InstalledDir))
		check(os.RemoveAll(dirs.PackagesDir))

		check(celer.Deploy(true))
		check(cleanCmd.cleanAll())

		checkList := map[string]bool{
			"x264@stable":   false,
			"automake@1.18": true,
			"autoconf@2.72": true,
			"m4@1.4.19":     true,
		}
		for nameVersion, dev := range checkList {
			if fileio.PathExists(buildDir(nameVersion, dev)) {
				t.Fatal(nameVersion + " build dir should be removed")
			}
		}

		// In windows, nasm is a prebuilt port, its source code is not copied to buildtrees.
		if runtime.GOOS != "windows" {
			// nasm is build in source, it should not be cleaned.
			modified, err := git.IsModified(filepath.Join(dirs.BuildtreesDir, "nasm@2.16.03", "src"))
			check(err)
			if modified {
				t.Fatal("nasm@2.16.03 src dir should be cleaned")
			}
		}
	})
}
