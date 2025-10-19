package depcheck

import (
	"celer/configs"
	"celer/pkgs/dirs"
	"os"
	"path/filepath"
	"testing"
)

func TestDepCheck_CheckConflict_Conflict(t *testing.T) {
	// Convenient function to check error.
	var checkError = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Set test workspace dir.
	currentDir, err := os.Getwd()
	checkError(err)
	dirs.Init(filepath.Join(currentDir, "testdata/depcheck/conflict"))

	celer := configs.NewCeler()
	checkError(celer.Init())

	var project configs.Project
	checkError(project.Init(celer, "project_001"))

	depcheck := NewDepCheck()
	var ports []configs.Port
	for _, nameVersion := range project.Ports {
		var port configs.Port
		if err := port.Init(celer, nameVersion); err != nil {
			t.FailNow()
		}
		ports = append(ports, port)
	}

	if err := depcheck.CheckConflict(celer, ports...); err != nil {
		t.Log(err.Error())
	} else {
		t.Fatal("conflict should be found here.")
	}
}

func TestDepCheck_CheckConflict_Normal(t *testing.T) {
	// Convenient function to check error.
	var checkError = func(err error) {
		t.Helper()
		if err != nil {
			t.Fatal(err)
		}
	}

	// Set test workspace dir.
	currentDir, err := os.Getwd()
	checkError(err)
	dirs.Init(filepath.Join(currentDir, "testdata/depcheck/conflict"))

	celer := configs.NewCeler()
	checkError(celer.Init())

	var project configs.Project
	checkError(project.Init(celer, "project_002"))

	depcheck := NewDepCheck()
	var ports []configs.Port

	for _, nameVersion := range project.Ports {
		var port configs.Port
		if err := port.Init(celer, nameVersion); err != nil {
			t.FailNow()
		}
		ports = append(ports, port)
	}

	if err := depcheck.CheckConflict(celer, ports...); err != nil {
		t.Fatal(err)
	}
}
