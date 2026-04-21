package git

import (
	"os"
	"testing"
)

const testRepo = "https://gitlab.com/bzip2/bzip2.git"

func TestCloneRepo_NoDefaultBranch(t *testing.T) {
	if err := os.RemoveAll("testdata"); err != nil {
		t.Fatal(err)
	}

	if err := CloneRepo("[test clone repo]", "bzip2@master", testRepo, "", 0, "testdata"); err != nil {
		t.Fatal(err)
	}
}

func TestCloneRepo_Branch(t *testing.T) {
	if err := os.RemoveAll("testdata"); err != nil {
		t.Fatal(err)
	}

	branch := "bzip2-1.0.8"
	if err := CloneRepo("[test clone repo]", "bzip2@bzip2-1.0.8", testRepo, branch, 0, "testdata"); err != nil {
		t.Fatal(err)
	}

	result, err := GetCurrentBranch("testdata")
	if err != nil {
		t.Fatal(err)
	}
	if result != branch {
		t.Fatalf("branch %s not found", branch)
	}
}

func TestCloneRepo_Tag(t *testing.T) {
	if err := os.RemoveAll("testdata"); err != nil {
		t.Fatal(err)
	}

	tag := "bzip2-1.0.7"
	if err := CloneRepo("[test clone repo]", "bzip2@bzip2-1.0.7", testRepo, tag, 0, "testdata"); err != nil {
		t.Fatal(err)
	}

	result, err := GetCurrentTag("testdata")
	if err != nil {
		t.Fatal(err)
	}
	if result != tag {
		t.Fatalf("tag %s not found", tag)
	}
}

func TestCloneRepo_Commit(t *testing.T) {
	if err := os.RemoveAll("testdata"); err != nil {
		t.Fatal(err)
	}

	commit := "1ea1ac188ad4b9cb662e3f8314673c63df95a589"
	if err := CloneRepo("[test clone repo]", "bzip2@1ea1ac188ad4b9cb662e3f8314673c63df95a589", testRepo, commit, 0, "testdata"); err != nil {
		t.Fatal(err)
	}

	result, err := GetCommitHash("testdata")
	if err != nil {
		t.Fatal(err)
	}
	if result != commit {
		t.Fatalf("commit %s not found", commit)
	}
}
