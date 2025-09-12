package depcheck

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"os"
	"path/filepath"
	"testing"
)

func TestDepCheck_CheckCircular_Normal(t *testing.T) {
	// Convenient checkError function.
	var checkError = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Set test workspace dir.
	currentDir, err := os.Getwd()
	if err != nil {
		t.Fatal(err)
	}
	dirs.Init(filepath.Join(currentDir, "testdata/depcheck/circular/normal"))

	celer := configs.NewCeler()
	checkError(celer.Init())

	var port configs.Port
	checkError(port.Init(celer, "aaa@1.0.0", celer.BuildType()))

	depcheck := NewDepCheck()
	if err := depcheck.CheckCircular(celer, port); err != nil {
		t.FailNow()
	}
}

func TestDepCheck_CheckCircular_Dependencies(t *testing.T) {
	// Convenient checkError function.
	var checkError = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Set test workspace dir.
	currentDir, err := os.Getwd()
	if err != nil {
		t.Fatal(err)
	}
	dirs.Init(filepath.Join(currentDir, "testdata/depcheck/circular/dependencies"))

	celer := configs.NewCeler()
	checkError(celer.Init())

	var port configs.Port
	checkError(port.Init(celer, "aaa@1.0.0", celer.BuildType()))

	depcheck := NewDepCheck()
	if err := depcheck.CheckCircular(celer, port); err != nil {
		t.Log(err.Error())
	} else {
		t.FailNow()
	}
}

func TestDepCheck_CheckCircular_DevDependencies(t *testing.T) {
	// Convenient checkError function.
	var checkError = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Set test workspace dir.
	currentDir, err := os.Getwd()
	if err != nil {
		t.Fatal(err)
	}
	dirs.Init(filepath.Join(currentDir, "testdata/depcheck/circular/dev_dependencies"))

	celer := configs.NewCeler()
	checkError(celer.Init())

	var port configs.Port
	checkError(port.Init(celer, "aaa@1.0.0", celer.BuildType()))

	depcheck := NewDepCheck()
	if err := depcheck.CheckCircular(celer, port); err != nil {
		t.Log(err.Error())
	} else {
		t.FailNow()
	}
}
