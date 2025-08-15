package git

import (
	"bufio"
	"bytes"
	"celer/pkgs/cmd"
	"celer/pkgs/fileio"
	"celer/pkgs/proxy"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"slices"
	"strings"
)

func CloneRepo(title, repoUrl, repoRef, repoDir string) error {
	// Try to hack github repo url with proxy url.
	repoUrl, err := proxy.HackRepoUrl(repoUrl)
	if err != nil {
		return err
	}

	switch {
	case IsRemoteBranch(repoUrl, repoRef), IsRemoteTag(repoUrl, repoRef):
		command := fmt.Sprintf("git clone --branch %s %s --depth 1 --recursive %s", repoRef, repoUrl, repoDir)
		if err := cmd.NewExecutor(title, command).Execute(); err != nil {
			return err
		}

	case IsRemoteCommit(repoUrl, repoRef):
		// Clone repo.
		cloneCmd := fmt.Sprintf("git clone %s %s --depth 1", repoUrl, repoDir)
		if err := cmd.NewExecutor(title, cloneCmd).Execute(); err != nil {
			return err
		}

		// Checkout commit.
		checkoutCmd := fmt.Sprintf("git -C %s checkout %s", repoDir, repoRef)
		if err := cmd.NewExecutor(title, checkoutCmd).Execute(); err != nil {
			return err
		}

		// Update submodules.
		submoduleCmd := fmt.Sprintf("git -C %s submodule update --init --recursive", repoDir)
		if err := cmd.NewExecutor(title, submoduleCmd).Execute(); err != nil {
			return err
		}

	default:
		return fmt.Errorf("ref %s is not a remote branch, tag or commit", repoRef)
	}

	return nil
}

func UpdateRepo(title, repoDir, repoRef string, force bool) error {
	// Check if repo is modified.
	modified, err := IsRepoModified(repoDir)
	if err != nil {
		return err
	}
	if modified {
		if !force {
			return fmt.Errorf("repo file is modified, update is skipped ... ⭐⭐⭐ You can update forcibly with -f/--force ⭐⭐⭐")
		}
	}

	// Get default branch if repoRef is empty.
	if repoRef == "" {
		branch, err := DefaultBranch(repoDir)
		if err != nil {
			return err
		}
		repoRef = branch
	}

	var commands []string
	commands = append(commands, "git reset --hard && git clean -xfd")

	switch {
	case IsRemoteBranch(repoDir, repoRef):
		commands = append(commands, fmt.Sprintf("git fetch origin %s", repoRef))
		commands = append(commands, fmt.Sprintf("git checkout -B %s origin/%s", repoRef, repoRef))
		commands = append(commands, fmt.Sprintf("git pull origin %s", repoRef))

	case IsRemoteTag(repoDir, repoRef):
		commands = append(commands, fmt.Sprintf("git tag -d %s || true", repoRef))
		commands = append(commands, "git fetch --tags origin")
		commands = append(commands, fmt.Sprintf("git checkout %s", repoRef))

	case IsRemoteCommit(repoDir, repoRef):
		commands = append(commands, fmt.Sprintf("git reset --hard %s", repoRef))
	}

	// Execute clone command.
	commandLine := strings.Join(commands, " && ")
	executor := cmd.NewExecutor(title, commandLine)
	executor.SetWorkDir(repoDir)
	return executor.Execute()
}

func CherryPick(title, srcDir string, patches []string) error {
	// Change to source dir to execute git command.
	if err := os.Chdir(srcDir); err != nil {
		return err
	}

	// Execute patch command.
	var commands []string
	commands = append(commands, fmt.Sprintf("git -C %s fetch origin", srcDir))

	for _, patch := range patches {
		commands = append(commands, fmt.Sprintf("git cherry-pick %s", patch))
	}

	commandLine := strings.Join(commands, " && ")
	if err := cmd.NewExecutor(title, commandLine).Execute(); err != nil {
		return err
	}

	return nil
}

func Rebase(title, srcDir, repoRef string, rebaseRefs []string) error {
	// Change to source dir to execute git command.
	if err := os.Chdir(srcDir); err != nil {
		return err
	}

	var commands []string
	commands = append(commands, fmt.Sprintf("git -C %s fetch origin", srcDir))

	for _, ref := range rebaseRefs {
		commands = append(commands, fmt.Sprintf("git checkout %s", ref))
		commands = append(commands, fmt.Sprintf("git rebase %s", repoRef))
	}

	commandLine := strings.Join(commands, " && ")
	if err := cmd.NewExecutor(title, commandLine).Execute(); err != nil {
		return err
	}

	return nil
}

func CleanRepo(repoDir string) error {
	commandLine := "git reset --hard && git clean -xfd"
	executor := cmd.NewExecutor("", commandLine)
	executor.SetWorkDir(repoDir)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func ApplyPatch(port, repoDir, patchFile string) error {
	patchFileName := filepath.Base(patchFile)

	// Check if patched already.
	recordFilePath := filepath.Join(repoDir, ".patched")
	if fileio.PathExists(recordFilePath) {
		bytes, err := os.ReadFile(recordFilePath)
		if err != nil {
			return fmt.Errorf("cannot read .patched: %w", err)
		}

		lines := strings.Split(string(bytes), "\n")
		if slices.Contains(lines, patchFileName) {
			return nil
		}
	}

	// Read the first few lines of the file to check for Git patch features.
	file, err := os.Open(patchFile)
	if err != nil {
		return err
	}
	defer file.Close()

	var gitBatch bool
	scanner := bufio.NewScanner(file)
	for range 10 {
		if !scanner.Scan() {
			break
		}
		line := scanner.Text()

		// If you find Git patch features such as "From " or "Subject: "
		if strings.HasPrefix(line, "diff --git ") {
			gitBatch = true
			break
		}
	}

	if gitBatch {
		command := fmt.Sprintf("git apply --ignore-space-change --ignore-whitespace -v %s", patchFile)
		title := fmt.Sprintf("[patch %s]", port)
		executor := cmd.NewExecutor(title, command)
		executor.SetWorkDir(repoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	} else {
		// Others, assume it's a regular patch file.
		command := fmt.Sprintf("patch -Np1 -i %s", patchFile)
		title := fmt.Sprintf("[patch %s]", port)
		executor := cmd.NewExecutor(title, command)
		executor.SetWorkDir(repoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	// Create a flag file to indicated that patch already applied.
	recordFile, err := os.OpenFile(recordFilePath, os.O_CREATE|os.O_RDWR|os.O_APPEND, os.ModePerm)
	if err != nil {
		return fmt.Errorf("cannot open or create .patched: %w", err)
	}
	defer recordFile.Close()

	if _, err := recordFile.WriteString(patchFileName + "\n"); err != nil {
		return fmt.Errorf("cannot write %s into .patched: %w", patchFileName, err)
	}
	return nil
}

func IsPatched(repoDir, patchFile string) (gitBatch, patched bool, err error) {
	file, err := os.Open(patchFile)
	if err != nil {
		return false, false, err
	}
	defer file.Close()

	// Read the first few lines of the file to check for Git patch features.
	scanner := bufio.NewScanner(file)
	for range 10 {
		if !scanner.Scan() {
			break
		}
		line := scanner.Text()

		// If you find Git patch features such as "From " or "Subject: "
		if strings.HasPrefix(line, "diff --git ") {
			gitBatch = true
			break
		}
	}

	if gitBatch {
		command := fmt.Sprintf("git apply --check %s", patchFile)
		exector := cmd.NewExecutor("", command)
		exector.SetWorkDir(repoDir)
		outout, err := exector.ExecuteOutput()

		if err != nil {
			return true, false, fmt.Errorf("run git command: %w", err)
		}

		if strings.TrimSpace(outout) == "" {
			return true, true, nil
		}

		return true, false, nil
	} else {
		command := fmt.Sprintf("patch --dry-run -p1 < %s", patchFile)
		exector := cmd.NewExecutor("", command)
		exector.SetWorkDir(repoDir)
		outout, err := exector.ExecuteOutput()

		if err != nil {
			return false, false, fmt.Errorf("run git command: %w", err)
		}

		if strings.Contains(outout, "patch detected") {
			return false, true, nil
		}

		return false, false, nil
	}
}

func IsRepoModified(repoDir string) (bool, error) {
	cmd := exec.Command("git", "-C", repoDir, "status", "--porcelain")

	var out bytes.Buffer
	cmd.Stdout = &out
	cmd.Stderr = &out

	if err := cmd.Run(); err != nil {
		return false, fmt.Errorf("run git command: %w", err)
	}

	status := strings.TrimSpace(out.String())
	return status != "", nil
}

func ReadGitCommit(repoDir string) (string, error) {
	if !fileio.PathExists(repoDir) {
		return "", fmt.Errorf("repo dir %s is not exist", repoDir)
	}
	cmd := exec.Command("git", "-C", repoDir, "rev-parse", "HEAD")

	var out bytes.Buffer
	cmd.Stdout = &out
	cmd.Stderr = &out

	if err := cmd.Run(); err != nil {
		return "", fmt.Errorf("read git commit hash: %w", err)
	}

	return strings.TrimSpace(out.String()), nil
}

func DefaultBranch(repoDir string) (string, error) {
	exector := cmd.NewExecutor("", "git remote show origin")
	outout, err := exector.ExecuteOutput()
	if err != nil {
		return "", fmt.Errorf("read git default branch: %w", err)
	}

	lines := strings.Split(outout, "\n")
	for _, line := range lines {
		if strings.Contains(line, "HEAD branch") {
			parts := strings.Split(line, ":")
			if len(parts) > 1 {
				return strings.TrimSpace(parts[1]), nil
			}
		}
	}

	return "", fmt.Errorf("default branch not found")
}
