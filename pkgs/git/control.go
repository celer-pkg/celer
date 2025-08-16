package git

import (
	"bufio"
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

// CloneRepo clone git repo.
func CloneRepo(title, repoUrl, repoRef, repoDir string) error {
	// Try to hack github repo url with proxy url.
	repoUrl, err := proxy.HackRepoUrl(repoUrl)
	if err != nil {
		return err
	}

	switch {
	case repoRef == "":
		// Clone default branch.
		command := fmt.Sprintf("git clone %s --recursive %s", repoUrl, repoDir)
		if err := cmd.NewExecutor(title, command).Execute(); err != nil {
			return err
		}

	case CheckIfRemoteBranch(repoUrl, repoRef), CheckIfRemoteTag(repoUrl, repoRef):
		// Clone specific branch or tag.
		command := fmt.Sprintf("git clone --branch %s %s --depth 1 --recursive %s", repoRef, repoUrl, repoDir)
		if err := cmd.NewExecutor(title, command).Execute(); err != nil {
			return err
		}

	case CheckIfRemoteCommit(repoUrl, repoRef):
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

// UpdateRepo update git repo.
func UpdateRepo(title, repoRef, repoDir string, force bool) error {
	// Check if repo is modified.
	modified, err := IsModified(repoDir)
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
	case CheckIfRemoteBranch(repoDir, repoRef):
		commands = append(commands, fmt.Sprintf("git fetch origin %s", repoRef))
		commands = append(commands, fmt.Sprintf("git checkout -B %s origin/%s", repoRef, repoRef))
		commands = append(commands, fmt.Sprintf("git pull origin %s", repoRef))

	case CheckIfRemoteTag(repoDir, repoRef):
		commands = append(commands, fmt.Sprintf("git tag -d %s || true", repoRef))
		commands = append(commands, "git fetch --tags origin")
		commands = append(commands, fmt.Sprintf("git checkout %s", repoRef))

	case CheckIfRemoteCommit(repoDir, repoRef):
		commands = append(commands, fmt.Sprintf("git reset --hard %s", repoRef))
	}

	// Execute clone command.
	commandLine := strings.Join(commands, " && ")
	executor := cmd.NewExecutor(title, commandLine)
	executor.SetWorkDir(repoDir)
	return executor.Execute()
}

// CherryPick cherry-pick patches.
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

// Rebase rebase patches.
func Rebase(title, repoRef, srcDir string, rebaseRefs []string) error {
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

// CleanRepo clean git repo.
func CleanRepo(repoDir string) error {
	// git reset --hard
	resetCmd := exec.Command("git", "reset", "--hard")
	resetCmd.Stdout = os.Stdout
	resetCmd.Stderr = os.Stderr
	if err := resetCmd.Run(); err != nil {
		return err
	}

	// git clean -xfd
	cleanCmd := exec.Command("git", "clean", "-xfd")
	cleanCmd.Stdout = os.Stdout
	cleanCmd.Stderr = os.Stderr
	if err := cleanCmd.Run(); err != nil {
		return err
	}

	return nil
}

// ApplyPatch apply git patch.
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
