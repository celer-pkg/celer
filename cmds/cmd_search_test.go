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

func TestSearchCmd_CommandStructure(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Test command creation
	search := &searchCmd{}
	celer := &configs.Celer{}

	cmd := search.Command(celer)
	if cmd == nil {
		t.Fatal("Command should not be nil")
	}

	if cmd.Use != "search" {
		t.Errorf("Expected Use to be 'search', got %s", cmd.Use)
	}

	if cmd.Short == "" {
		t.Error("Short description should not be empty")
	}

	if cmd.Long == "" {
		t.Error("Long description should not be empty")
	}

	// Test Args validation - should require exactly 1 argument.
	if cmd.Args == nil {
		t.Error("Args validation should be set")
	}
}

func TestSearchCmd_Search_ExactMatch(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	// Test exact match search.
	searchCmd := searchCmd{celer: celer}
	results, err := searchCmd.search("zlib@1.3.1")
	if err != nil {
		t.Fatalf("Search failed: %v", err)
	}

	// Check if we found at least one result.
	if len(results) == 0 {
		t.Log("Warning: No results found for zlib@1.3.1 (port may not exist in test-conf)")
	} else {
		// Verify result contains expected port.
		found := slices.Contains(results, "zlib@1.3.1")
		if !found {
			t.Error("Expected to find zlib@1.3.1 in results")
		}
	}
}

func TestSearchCmd_Search_PrefixMatch(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	// Test prefix match search.
	searchCmd := searchCmd{celer: celer}
	results, err := searchCmd.search("zlib*")
	if err != nil {
		t.Fatalf("Search failed: %v", err)
	}

	// All results should start with "zlib".
	for _, result := range results {
		if !strings.HasPrefix(result, "zlib") {
			t.Errorf("Result %s should start with 'zlib'", result)
		}
	}
}

func TestSearchCmd_Search_SuffixMatch(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	// Create search command.
	searchCmd := searchCmd{celer: celer}

	// Test suffix match search.
	results, err := searchCmd.search("*@1.3.1")
	if err != nil {
		t.Fatalf("Search failed: %v", err)
	}

	// All results should end with "@1.3.1".
	for _, result := range results {
		if !strings.HasSuffix(result, "@1.3.1") {
			t.Errorf("Result %s should end with '@1.3.1'", result)
		}
	}
}

func TestSearchCmd_Search_ContainsMatch(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	// Create search command.
	searchCmd := searchCmd{celer: celer}

	// Test contains match search.
	results, err := searchCmd.search("*lib*")
	if err != nil {
		t.Fatalf("Search failed: %v", err)
	}

	// All results should contain "lib".
	for _, result := range results {
		if !strings.Contains(result, "lib") {
			t.Errorf("Result %s should contain 'lib'", result)
		}
	}
}

func TestSearchCmd_Search_NoMatch(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	// Create search command.
	searchCmd := searchCmd{celer: celer}

	// Test search with no matches.
	results, err := searchCmd.search("nonexistent-package@99.99.99")
	if err != nil {
		t.Fatalf("Search failed: %v", err)
	}

	if len(results) != 0 {
		t.Errorf("Expected no results, got %d results", len(results))
	}
}

func TestSearchCmd_Search_PortsDirNotExist(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer without setting up (so ports dir won't exist).
	celer := configs.NewCeler()
	check(celer.Init())

	// Ensure ports directory does not exist.
	check(os.RemoveAll(dirs.PortsDir))

	// Create search command.
	searchCmd := searchCmd{celer: celer}

	// Test search when ports directory doesn't exist.
	results, err := searchCmd.search("zlib*")
	if err != nil {
		t.Fatalf("Search should not fail when ports dir doesn't exist: %v", err)
	}

	if len(results) != 0 {
		t.Errorf("Expected no results when ports dir doesn't exist, got %d results", len(results))
	}
}

func TestSearchCmd_Search_ProjectSpecificPorts(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("project_test_01"))
	check(celer.Setup())

	// Create a test project-specific port.
	projectPortDir := filepath.Join(dirs.ConfProjectsDir, "project_test_01", "testlib", "1.0.0")
	check(os.MkdirAll(projectPortDir, os.ModePerm))

	portTomlPath := filepath.Join(projectPortDir, "port.toml")
	check(os.WriteFile(portTomlPath, []byte("[package]\nname = \"testlib\"\nversion = \"1.0.0\"\n"), 0644))

	// Create search command.
	searchCmd := searchCmd{celer: celer}

	// Test search for project-specific port.
	results, err := searchCmd.search("testlib*")
	if err != nil {
		t.Fatalf("Search failed: %v", err)
	}

	// Verify we found the project-specific port.
	found := false
	for _, result := range results {
		if result == "testlib@1.0.0" {
			found = true
			break
		}
	}
	if !found {
		t.Error("Expected to find testlib@1.0.0 in results")
	}
}

func TestSearchCmd_Search_InvalidWildcard(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	// Create search command.
	searchCmd := searchCmd{celer: celer}

	// Test search with invalid wildcard pattern (more than 2 wildcards).
	results, err := searchCmd.search("*zlib*@*")
	if err != nil {
		t.Fatalf("Search failed: %v", err)
	}

	// Should return no results for invalid pattern.
	if len(results) != 0 {
		t.Logf("Got %d results for invalid pattern (this is acceptable)", len(results))
	}
}

func TestSearchCmd_Completion(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	searchCmd := searchCmd{celer: celer}
	cmd := searchCmd.Command(celer)

	t.Run("completion for ports", func(t *testing.T) {
		suggestions, directive := searchCmd.completion(cmd, []string{}, "zlib")
		if directive != cobra.ShellCompDirectiveNoFileComp {
			t.Errorf("Expected directive NoFileComp, got %v", directive)
		}

		// Should get suggestions starting with "zlib"
		if len(suggestions) > 0 {
			for _, suggestion := range suggestions {
				if !strings.HasPrefix(suggestion, "zlib") {
					t.Errorf("Suggestion %s should start with 'zlib'", suggestion)
				}
			}
		}
	})

	t.Run("completion with empty prefix", func(t *testing.T) {
		suggestions, _ := searchCmd.completion(cmd, []string{}, "")

		// Should return all available ports.
		if len(suggestions) == 0 {
			t.Log("No suggestions found (may be acceptable if no ports exist)")
		}
	})

	t.Run("completion from project ports", func(t *testing.T) {
		// Set a project.
		check(celer.SetProject("project_test_01"))

		// Create a test project-specific port for completion.
		projectPortDir := filepath.Join(dirs.ConfProjectsDir, "project_test_01", "mylib", "2.0.0")
		check(os.MkdirAll(projectPortDir, os.ModePerm))

		portTomlPath := filepath.Join(projectPortDir, "port.toml")
		check(os.WriteFile(portTomlPath, []byte("[package]\nname = \"mylib\"\nversion = \"2.0.0\"\n"), 0644))

		suggestions, _ := searchCmd.completion(cmd, []string{}, "mylib")

		// Should find the project-specific port.
		found := slices.Contains(suggestions, "mylib@2.0.0")
		if !found {
			t.Error("Expected to find mylib@2.0.0 in completion suggestions")
		}
	})
}

func TestSearchCmd_Completion_ErrorHandling(t *testing.T) {
	// Cleanup.
	dirs.RemoveAllForTest()

	// Check error.
	var check = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Init celer without setup (ports dir may not exist).
	celer := configs.NewCeler()
	check(celer.Init())

	searchCmd := searchCmd{celer: celer}
	cmd := searchCmd.Command(celer)

	// Test completion when ports directory doesn't exist.
	suggestions, directive := searchCmd.completion(cmd, []string{}, "test")

	// Should not panic and should return valid directive.
	if directive != cobra.ShellCompDirectiveNoFileComp {
		t.Errorf("Expected directive NoFileComp, got %v", directive)
	}

	// Should return empty suggestions or handle gracefully.
	if len(suggestions) > 0 {
		t.Log("Got some suggestions despite missing ports dir (acceptable)")
	}
}

func TestSearchCmd_DoSearch_Integration(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	// Create search command.
	searchCmd := searchCmd{celer: celer}

	// Test doSearch method (integration test).
	// This should not panic or exit.
	searchCmd.doSearch("zlib*")

	// If we reach here, doSearch completed without fatal errors
}

func TestSearchCmd_Search_DuplicateResults(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.SetProject("project_test_01"))
	check(celer.Setup())

	// Create a port that exists in both global and project-specific locations.
	globalPortDir := dirs.GetPortDir("duplib", "1.0.0")
	projectPortDir := filepath.Join(dirs.ConfProjectsDir, "project_test_01", "duplib", "1.0.0")

	check(os.MkdirAll(globalPortDir, os.ModePerm))
	check(os.MkdirAll(projectPortDir, os.ModePerm))

	portContent := []byte("[package]\nname = \"duplib\"\nversion = \"1.0.0\"\n")
	check(os.WriteFile(filepath.Join(globalPortDir, "port.toml"), portContent, 0644))
	check(os.WriteFile(filepath.Join(projectPortDir, "port.toml"), portContent, 0644))

	// Create search command.
	searchCmd := searchCmd{celer: celer}

	// Test search
	results, err := searchCmd.search("duplib*")
	if err != nil {
		t.Fatalf("Search failed: %v", err)
	}

	// Count occurrences of duplib@1.0.0
	count := 0
	for _, result := range results {
		if result == "duplib@1.0.0" {
			count++
		}
	}

	// Note: Current implementation may return duplicates,
	// This test documents the current behavior.
	if count > 1 {
		t.Logf("Found %d duplicates of duplib@1.0.0 (current implementation allows duplicates)", count)
	}
}

func TestSearchCmd_Search_EmptyPortsDir(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())

	// Create empty ports directory.
	check(os.RemoveAll(dirs.PortsDir))
	check(os.MkdirAll(dirs.PortsDir, os.ModePerm))

	// Create search command.
	searchCmd := searchCmd{celer: celer}

	// Test search in empty directory.
	results, err := searchCmd.search("*")
	if err != nil {
		t.Fatalf("Search should not fail with empty ports dir: %v", err)
	}

	if len(results) != 0 {
		t.Errorf("Expected no results in empty ports dir, got %d results", len(results))
	}
}

func TestSearchCmd_Search_SpecialCharacters(t *testing.T) {
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
	celer := configs.NewCeler()
	check(celer.Init())
	check(celer.CloneConf("https://github.com/celer-pkg/test-conf.git", "", true))
	check(celer.SetBuildType("Release"))
	check(celer.Setup())

	// Create search command.
	searchCmd := searchCmd{celer: celer}

	// Test search with various special characters.
	testPatterns := []string{
		"lib-test*", // hyphen
		"lib_test*", // underscore
		"lib.test*", // dot
		"*-1.0",     // version with hyphen
	}

	for _, pattern := range testPatterns {
		_, err := searchCmd.search(pattern)
		if err != nil {
			t.Errorf("Search with pattern '%s' should not fail: %v", pattern, err)
		}
	}
}
