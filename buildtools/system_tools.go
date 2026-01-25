package buildtools

import (
	"celer/pkgs/cmd"
	"fmt"
	"runtime"
	"strings"
)

// checkSystemTools checks if the system tools are installed.
func checkSystemTools(packageNames []string) error {
	osType, err := getOSType()
	if err != nil {
		return err
	}

	var missing []string
	for _, packageName := range packageNames {
		packageName = strings.TrimSpace(packageName)
		if packageName == "" {
			continue
		}

		var (
			installed bool
			err       error
		)

		// Check installed based on OS type.
		switch osType {
		case "debian", "ubuntu":
			installed, err = checkDebianLibraryInstalled(packageName)
		case "centos", "fedora", "rhel":
			installed, err = checkRedHatLibraryInstalled(packageName)
		default:
			return fmt.Errorf("unsupported package manager prefix in package name: %s", packageName)
		}

		// Check error from isLibraryInstalled.
		if err != nil {
			return err
		}
		if !installed {
			switch osType {
			case "debian", "ubuntu":
				packageName = strings.TrimPrefix(packageName, "apt:")
			case "centos", "fedora", "rhel":
				packageName = strings.TrimPrefix(packageName, "yum:")
			default:
				return fmt.Errorf("unsupported OS type: %s", osType)
			}

			missing = append(missing, packageName)
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
			switch osType {
			case "debian", "ubuntu":
				return fmt.Errorf("%s.\n please install it with `sudo apt install %s`", summary, joined)
			case "centos", "fedora", "rhel":
				return fmt.Errorf("%s.\n please install it with `sudo yum install %s`", summary, joined)
			}

		case "darwin":
			return fmt.Errorf("%s.\n please install it with `brew install %s`", joined, joined)
		}
	}

	return nil
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

func checkDebianLibraryInstalled(packageName string) (bool, error) {
	// Use dpkg -l to check if the library is installed.
	packageName = strings.TrimPrefix(packageName, "apt:")
	executor := cmd.NewExecutor("", "dpkg", "-l", packageName)
	out, err := executor.ExecuteOutput()
	if err != nil {
		// If not installed, dpkg -l will return exit status 1.
		return false, nil
	}

	// Check if the library is installed.
	lines := strings.SplitSeq(string(out), "\n")
	for line := range lines {
		if strings.HasPrefix(line, "ii") && strings.Contains(line, packageName) {
			return true, nil
		}
	}

	return false, nil
}

func checkRedHatLibraryInstalled(libraryName string) (bool, error) {
	// Use rpm -q to check if the library is installed.
	libraryName = strings.TrimPrefix(libraryName, "yum:")
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
