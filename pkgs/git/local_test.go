package git

import (
	"os"
	"path/filepath"
	"testing"
)

func TestCleanRepo(t *testing.T) {
	repoDir := t.TempDir()
	filePath := filepath.Join(repoDir, "tool.txt")

	if err := os.WriteFile(filePath, []byte("origin\n"), 0o644); err != nil {
		t.Fatal(err)
	}
	if err := InitAsLocalRepo(repoDir, "init test repo"); err != nil {
		t.Fatal(err)
	}

	if err := os.WriteFile(filePath, []byte("changed\n"), 0o644); err != nil {
		t.Fatal(err)
	}

	modified, err := IsModified(repoDir)
	if err != nil {
		t.Fatal(err)
	}
	if !modified {
		t.Fatal("expected repo to be modified")
	}

	if err := CleanRepo(repoDir); err != nil {
		t.Fatal(err)
	}

	modified, err = IsModified(repoDir)
	if err != nil {
		t.Fatal(err)
	}
	if modified {
		t.Fatal("expected repo to be clean after reset --hard")
	}

	content, err := os.ReadFile(filePath)
	if err != nil {
		t.Fatal(err)
	}
	if string(content) != "origin\n" {
		t.Fatalf("unexpected file content after reset: %s", string(content))
	}
}
