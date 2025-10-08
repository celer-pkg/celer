package git

import (
	"bufio"
	"celer/pkgs/cmd"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"strings"
)

// CloneRepo clone git repo.
func CloneRepo(title, repoUrl, repoRef, repoDir string) error {
	// ============ Clone default branch ============
	if repoRef == "" {
		command := fmt.Sprintf("git clone --recursive %s %s", repoUrl, repoDir)
		return cmd.NewExecutor(title, command).Execute()
	}

	// ============ Clone specific branch ============
	isBranch, err := CheckIfRemoteBranch(repoUrl, repoRef)
	if err != nil {
		return fmt.Errorf("check if remote branch error: %w", err)
	}
	if isBranch {
		command := fmt.Sprintf("git clone --branch %s --recursive %s %s", repoRef, repoUrl, repoDir)
		return cmd.NewExecutor(title, command).Execute()
	}

	// ============ Clone specific tag ============
	isTag, err := CheckIfRemoteTag(repoUrl, repoRef)
	if err != nil {
		return fmt.Errorf("check if remote tag error: %w", err)
	}
	if isTag {
		command := fmt.Sprintf("git clone --branch %s %s --recursive %s", repoRef, repoUrl, repoDir)
		return cmd.NewExecutor(title, command).Execute()
	}
	// ============ Clone and checkout commit ============
	command := fmt.Sprintf("git clone %s %s", repoUrl, repoDir)
	if err := cmd.NewExecutor(title, command).Execute(); err != nil {
		return fmt.Errorf("clone git repo error: %w", err)
	}

	// Checkout repo to commit.
	command = fmt.Sprintf("git reset --hard %s", repoRef)
	executor := cmd.NewExecutor(title+" (reset to commit)", command)
	executor.SetWorkDir(repoDir)
	if err := executor.Execute(); err != nil {
		return fmt.Errorf("reset --hard error: %w", err)
	}

	// Update submodules.
	if pathExists(filepath.Join(repoDir, ".gitmodules")) {
		command = "git submodule update --init --recursive"
		executor = cmd.NewExecutor(title+" (clone submodule)", command)
		executor.SetWorkDir(repoDir)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("update submodules error: %w", err)
		}
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

	// Update to branch.
	isBranch, err := CheckIfRemoteBranch(repoDir, repoRef)
	if err != nil {
		return err
	}
	if isBranch {
		var commands []string
		commands = append(commands, "git reset --hard")
		commands = append(commands, "git clean -xfd")
		commands = append(commands, "git fetch origin "+repoRef)
		commands = append(commands, "git checkout -B "+repoRef+" origin/"+repoRef)
		commands = append(commands, "git pull origin "+repoRef)

		commandLine := strings.Join(commands, " && ")
		executor := cmd.NewExecutor(title, commandLine)
		executor.SetWorkDir(repoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	// Update to tag.
	isTag, err := CheckIfRemoteTag(repoDir, repoRef)
	if err != nil {
		return err
	}
	if isTag {
		var commands []string
		commands = append(commands, "git reset --hard")
		commands = append(commands, "git clean -xfd")
		commands = append(commands, "git tag -d "+repoRef+" || true")
		commands = append(commands, "git fetch --tags origin")
		commands = append(commands, "git checkout "+repoRef)

		commandLine := strings.Join(commands, " && ")
		executor := cmd.NewExecutor(title, commandLine)
		executor.SetWorkDir(repoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return fmt.Errorf("invalid repoRef: %s", repoRef)
}

// CherryPick cherry-pick patches.
func CherryPick(title, srcDir string, patches []string) error {
	// Change to source dir to execute git command.
	if err := os.Chdir(srcDir); err != nil {
		return err
	}

	var commands []string
	commands = append(commands, "git fetch origin")

	for _, patch := range patches {
		commands = append(commands, "git cherry-pick "+patch)
	}

	commandLine := strings.Join(commands, " && ")
	executor := cmd.NewExecutor(title, commandLine)
	executor.SetWorkDir(srcDir)
	if err := executor.Execute(); err != nil {
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
	commands = append(commands, "git fetch origin")

	for _, rebaseRef := range rebaseRefs {
		commands = append(commands, "git checkout "+rebaseRef)
		commands = append(commands, "git rebase "+repoRef)
	}

	commandLine := strings.Join(commands, " && ")
	executor := cmd.NewExecutor(title, commandLine)
	executor.SetWorkDir(srcDir)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

// Clean clean git repo.
func Clean(title, repoDir string) error {
	var commands []string
	commands = append(commands, "git reset --hard")
	commands = append(commands, "git clean -xfd")

	commandLine := strings.Join(commands, " && ")
	executor := cmd.NewExecutor(title, commandLine)
	executor.SetWorkDir(repoDir)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

// ApplyPatch apply git patch.
func ApplyPatch(port, repoDir, patchFile string) error {
	patchFileName := filepath.Base(patchFile)

	// Check if patched already.
	recordFilePath := filepath.Join(repoDir, ".patched")
	if pathExists(recordFilePath) {
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
		title := fmt.Sprintf("[patch %s]", port)
		args := []string{"apply", "--ignore-space-change", "--ignore-whitespace", "-v", patchFile}
		executor := cmd.NewExecutor(title, "git", args...)
		executor.SetWorkDir(repoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	} else {
		// Others, assume it's a regular patch file.
		title := fmt.Sprintf("[patch %s]", port)
		args := []string{"-Np1", "-i", patchFile}
		executor := cmd.NewExecutor(title, "patch", args...)
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

func pathExists(path string) bool {
	_, err := os.Stat(path)
	if err == nil {
		return true
	}
	return !os.IsNotExist(err)
}
