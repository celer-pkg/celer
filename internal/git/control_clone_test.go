package git

import (
	"testing"
)

const testRepo = "https://gitlab.com/bzip2/bzip2.git"

func TestCloneRepo_NoDefaultBranch(t *testing.T) {
	repoDir := t.TempDir()

	if err := NewClone(testRepo, "", repoDir).IgnoreSubmodule(true).Clone("[test clone repo]", "bzip2@master"); err != nil {
		t.Fatal(err)
	}
}

func TestCloneRepo_Branch(t *testing.T) {
	repoDir := t.TempDir()

	branch := "master"
	if err := NewClone(testRepo, branch, repoDir).
		IgnoreSubmodule(true).
		Clone("[test clone repo]", "bzip2@master"); err != nil {
		t.Fatal(err)
	}

	result, err := GetCurrentBranch(repoDir)
	if err != nil {
		t.Fatal(err)
	}
	if result != branch {
		t.Fatalf("branch %s not found", branch)
	}
}

func TestCloneRepo_Tag(t *testing.T) {
	repoDir := t.TempDir()

	tag := "bzip2-1.0.7"
	if err := NewClone(testRepo, tag, repoDir).
		IgnoreSubmodule(true).
		Clone("[test clone repo]", "bzip2@bzip2-1.0.7"); err != nil {
		t.Fatal(err)
	}

	result, err := GetCurrentTag(repoDir)
	if err != nil {
		t.Fatal(err)
	}
	if result != tag {
		t.Fatalf("tag %s not found", tag)
	}
}

func TestCloneRepo_Commit(t *testing.T) {
	repoDir := t.TempDir()

	commit := "1ea1ac188ad4b9cb662e3f8314673c63df95a589"
	if err := NewClone(testRepo, commit, repoDir).
		IgnoreSubmodule(true).
		Clone("[test clone repo]", "bzip2@1ea1ac188ad4b9cb662e3f8314673c63df95a589"); err != nil {
		t.Fatal(err)
	}

	result, err := GetCommitHash(repoDir)
	if err != nil {
		t.Fatal(err)
	}
	if result != commit {
		t.Fatalf("commit %s not found", commit)
	}
}
