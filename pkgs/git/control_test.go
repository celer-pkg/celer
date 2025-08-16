package git

import (
	"os"
	"testing"
)

const testRepo = "https://gitlab.gnome.org/GNOME/libxml2.git"

func TestCloneRepo_NoDefaultBranch(t *testing.T) {
	if err := os.RemoveAll("testdata"); err != nil {
		t.Fatal(err)
	}

	if err := CloneRepo("[test clone repo]", testRepo, "", "testdata"); err != nil {
		t.Fatal(err)
	}
}

func TestCloneRepo_Branch(t *testing.T) {
	if err := os.RemoveAll("testdata"); err != nil {
		t.Fatal(err)
	}

	if err := CloneRepo("[test clone repo]", testRepo, "", "testdata"); err != nil {
		t.Fatal(err)
	}

	result, err := CheckIfLocalBranch("testdata", "master")
	if err != nil {
		t.Fatal(err)
	}
	if !result {
		t.Fatal("master branch not found")
	}
}

func TestCloneRepo_Tag(t *testing.T) {
	if err := os.RemoveAll("testdata"); err != nil {
		t.Fatal(err)
	}

	if err := CloneRepo("[test clone repo]", testRepo, "v2.14.5", "testdata"); err != nil {
		t.Fatal(err)
	}

	result, err := CheckIfLocalTag("testdata", "v2.14.5")
	if err != nil {
		t.Fatal(err)
	}
	if !result {
		t.Fatal("tag v2.14.5 not found")
	}
}
