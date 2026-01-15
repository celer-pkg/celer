package buildtools

import (
	"celer/pkgs/cmd"
	"celer/pkgs/env"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"
)

var Python3 *python3

// setupPython3 setup python3 in windows only, since linux always has builtin python always.
func setupPython3() error {
	if Python3 != nil {
		return nil
	}

	// Init python3.
	switch runtime.GOOS {
	case "windows":
		var python python3
		if err := python.validate(); err != nil {
			return err
		}
		Python3 = &python

	case "linux":
		Python3 = &python3{
			Path:    "/usr/bin/python3",
			rootDir: "/usr/bin",
		}
	}

	// Make sure Python is available in PATH.
	os.Setenv("PATH", env.JoinPaths("PATH", Python3.rootDir))
	return nil
}

// pipInstall checks if the python3 is installed.
func pipInstall(libraries *[]string) error {
	// Remove none python3:xxx from list.
	*libraries = slices.DeleteFunc(*libraries, func(element string) bool {
		return !strings.HasPrefix(element, "python3:")
	})

	defer func() {
		// Remove python3 related after setted up.
		*libraries = slices.DeleteFunc(*libraries, func(tool string) bool {
			return strings.HasPrefix(tool, "python3:")
		})
	}()

	// Install extra tools if not exist.
	for _, library := range *libraries {
		library = strings.TrimPrefix(library, "python3:")

		// Check if the tool is already installed.
		installed, err := Python3.checkIfInstalled(library)
		if err != nil {
			return fmt.Errorf("check if %s is installed: %s", library, err)
		}
		if installed {
			continue
		}

		// Use Python3.Path instead of "python3" command to ensure it works on Windows.
		title := "[python3 install tool]"
		var command string
		if Python3 != nil && Python3.Path != "" {
			command = fmt.Sprintf("\"%s\" -m pip install %s", Python3.Path, library)
		} else {
			command = fmt.Sprintf("python3 -m pip install %s", library)
		}
		executor := cmd.NewExecutor(title, command)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

type python3 struct {
	// It must be converted to "/" format path or "\\" format path.
	Path    string
	rootDir string
	version string
}

func (p *python3) validate() error {
	if err := p.findInstalledVersion(); err != nil {
		return err
	}

	// os.Setenv("PATH", env.JoinPaths("PATH", p.rootDir, filepath.Join(p.rootDir, "Scripts")))
	return nil
}

func (p *python3) findInstalledVersion() error {
	var notFoundErr = fmt.Errorf("no python3 found in your windows os")

	// Find py.exe
	executor := cmd.NewExecutor("", "where py.exe")
	output, err := executor.ExecuteOutput()
	if err != nil {
		return fmt.Errorf("failed to find py.exe\n %w", notFoundErr)
	}
	if strings.Contains(output, "INFO: Could not find files for the given pattern(s).") {
		return fmt.Errorf("no py.exe found in your windows os")
	}
	pypath := strings.Split(output, "\r\n")[0]

	// Find python3 with py.exe
	executor = cmd.NewExecutor("", fmt.Sprintf("%s -0p", pypath))
	output, err = executor.ExecuteOutput()
	if err != nil {
		return err
	}

	list := strings.Split(strings.TrimSpace(output), "\r\n")
	if len(list) == 0 {
		return notFoundErr
	}

	for _, path := range list {
		parts := strings.Split(path, "*")
		if len(parts) != 2 {
			continue
		}

		// python3 is suggested.
		version := strings.TrimSpace(strings.TrimPrefix(parts[0], "-V:"))
		if strings.HasPrefix(version, "3.") {
			p.version = version
			p.Path = filepath.ToSlash(strings.TrimSpace(parts[1]))
			p.rootDir = filepath.Dir(p.Path)
			break
		}
	}

	if p.version == "" || p.rootDir == "" {
		return notFoundErr
	}

	// python3.exe may not exist.
	if err := p.createSymlink(p.rootDir); err != nil {
		return err
	}

	return nil
}

func (p python3) checkIfInstalled(target string) (bool, error) {
	// Redirect output to null device.
	nullDevice := expr.If(runtime.GOOS == "windows", "nul", "/dev/null")
	command := fmt.Sprintf("pip show %s >%s 2>&1 && echo yes || echo no", target, nullDevice)
	executor := cmd.NewExecutor("", command)
	output, err := executor.ExecuteOutput()
	if err != nil {
		return false, err
	}

	if strings.TrimSpace(output) == "yes" {
		return true, nil
	}

	return false, nil
}

func (p python3) createSymlink(rootDir string) error {
	src := filepath.Join(rootDir, "python.exe")
	dst := filepath.Join(rootDir, "python3.exe")

	if !fileio.PathExists(dst) {
		if err := os.Link(src, dst); err != nil {
			return fmt.Errorf("create symlink: %s", err)
		}

		fmt.Println("-- symlink python3 is created.")
	}

	return nil
}
