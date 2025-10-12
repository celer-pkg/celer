package buildtools

import (
	"celer/pkgs/cmd"
	"fmt"
	"runtime"
	"strings"
)

// CheckSystemTools checks if the system tools are installed.
func CheckSystemTools(tools []string) error {
	var missing []string
	for _, tool := range tools {
		tool = strings.TrimSpace(tool)
		if tool == "" {
			continue
		}

		installed, err := isLibraryInstalled(tool)
		if err != nil {
			return err
		}

		if !installed {
			missing = append(missing, tool)
		}
	}

	if len(missing) > 0 {
		var summary string
		if len(missing) == 1 {
			summary = fmt.Sprintf("`%s` is not installed", missing[0])
		} else if len(missing) == 2 {
			summary = fmt.Sprintf("`%s` and `%s` are not installed", missing[0], missing[1])
		} else {
			summary = "`" + strings.Join(missing[:len(missing)-1], "`, `") + " and `" + missing[len(missing)-1] + "` are not installed"
		}

		joined := strings.Join(missing, " ")
		switch runtime.GOOS {
		case "linux":
			return fmt.Errorf("%s, please install it with `sudo apt install %s`", summary, joined)
		case "darwin":
			return fmt.Errorf("%s, please install it with `brew install %s`", joined, joined)
		}
	}

	return nil
}

// isLibraryInstalled checks if a library is installed on the system.
func isLibraryInstalled(libraryName string) (bool, error) {
	osType, err := getOSType()
	if err != nil {
		return false, err
	}

	switch osType {
	case "debian", "ubuntu":
		return checkDebianLibrary(libraryName)
	case "centos", "fedora", "rhel":
		return checkRedHatLibrary(libraryName)
	default:
		return false, fmt.Errorf("unsupported OS type: %s", osType)
	}
}

func getOSType() (string, error) {
	executor := cmd.NewExecutor("", "cat", "/etc/os-release")
	out, err := executor.ExecuteOutput()
	if err != nil {
		return "", fmt.Errorf("failed to read /etc/os-release\n %w", err)
	}

	lines := strings.SplitSeq(string(out), "\n")
	for line := range lines {
		if after, ok := strings.CutPrefix(line, "ID="); ok {
			id := after
			id = strings.Trim(id, `"`)
			return id, nil
		}
	}

	return "", fmt.Errorf("can not determine OS type")
}

func checkDebianLibrary(libraryName string) (bool, error) {
	// Use dpkg -l to check if the library is installed.
	executor := cmd.NewExecutor("", "dpkg", "-l", libraryName)
	out, err := executor.ExecuteOutput()
	if err != nil {
		// If not installed, dpkg -l will return exit status 1.
		return false, nil
	}

	// Check if the library is installed.
	lines := strings.SplitSeq(string(out), "\n")
	for line := range lines {
		if strings.HasPrefix(line, "ii") && strings.Contains(line, libraryName) {
			return true, nil
		}
	}

	return false, nil
}

func checkRedHatLibrary(libraryName string) (bool, error) {
	// Use rpm -q to check if the library is installed
	executor := cmd.NewExecutor("", "rpm", "-q", libraryName)
	out, err := executor.ExecuteOutput()
	if err != nil {
		return false, fmt.Errorf("failed to run rpm -q: %v", err)
	}

	// Check if the library is installed.
	if !strings.Contains(string(out), "not installed") {
		return true, nil
	}

	return false, nil
}
