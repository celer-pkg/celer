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

	if err := CloneRepo("[test clone repo]", testRepo, "", false, 0, "testdata"); err != nil {
		t.Fatal(err)
	}
}

func TestCloneRepo_Branch(t *testing.T) {
	if err := os.RemoveAll("testdata"); err != nil {
		t.Fatal(err)
	}

	branch := "master"
	if err := CloneRepo("[test clone repo]", testRepo, branch, false, 0, "testdata"); err != nil {
		t.Fatal(err)
	}

	result, err := CheckIfLocalBranch("testdata", branch)
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

	tag := "bzip2-1.0.8"
	if err := CloneRepo("[test clone repo]", testRepo, tag, false, 0, "testdata"); err != nil {
		t.Fatal(err)
	}

	result, err := CheckIfLocalTag("testdata", tag)
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

	commit := "1ea1ac188ad4b9cb662e3f8314673c63df95a589"
	if err := CloneRepo("[test clone repo]", testRepo, commit, false, 0, "testdata"); err != nil {
		t.Fatal(err)
	}

	result, err := CheckIfLocalCommit("testdata", commit)
	if err != nil {
		t.Fatal(err)
	}
	if !result {
		t.Fatalf("commit %s not found", commit)
	}
}
