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

	var git Git
	if err := git.CloneRepo("[test clone repo]", testRepo, "", "testdata"); err != nil {
		t.Fatal(err)
	}
}

func TestCloneRepo_Branch(t *testing.T) {
	if err := os.RemoveAll("testdata"); err != nil {
		t.Fatal(err)
	}

	branch := "master"
	var git Git
	if err := git.CloneRepo("[test clone repo]", testRepo, branch, "testdata"); err != nil {
		t.Fatal(err)
	}

	result, err := git.CheckIfLocalBranch("testdata", branch)
	if err != nil {
		t.Fatal(err)
	}
	if !result {
		t.Fatalf("branch %s not found", branch)
	}
}

func TestCloneRepo_Tag(t *testing.T) {
	if err := os.RemoveAll("testdata"); err != nil {
		t.Fatal(err)
	}

	var git Git
	tag := "bzip2-1.0.8"
	if err := git.CloneRepo("[test clone repo]", testRepo, tag, "testdata"); err != nil {
		t.Fatal(err)
	}

	result, err := git.CheckIfLocalTag("testdata", tag)
	if err != nil {
		t.Fatal(err)
	}
	if !result {
		t.Fatalf("tag %s not found", tag)
	}
}

func TestCloneRepo_Commit(t *testing.T) {
	if err := os.RemoveAll("testdata"); err != nil {
		t.Fatal(err)
	}

	var git Git
	commit := "1ea1ac188ad4b9cb662e3f8314673c63df95a589"
	if err := git.CloneRepo("[test clone repo]", testRepo, commit, "testdata"); err != nil {
		t.Fatal(err)
	}

	result, err := git.CheckIfLocalCommit("testdata", commit)
	if err != nil {
		t.Fatal(err)
	}
	if !result {
		t.Fatalf("commit %s not found", commit)
	}
}
