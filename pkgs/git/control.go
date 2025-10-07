package git

import (
	"bufio"
	"celer/pkgs/cmd"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"slices"
	"strings"
)

// CloneRepo clone git repo.
func CloneRepo(title, repoUrl, repoRef, repoDir string) error {
	// ============ Clone default branch ============
	if repoRef == "" {
		command := fmt.Sprintf("%s clone %s --recursive %s", gitCmd(), repoUrl, repoDir)
		return cmd.NewExecutor(title, command).Execute()
	}

	// ============ Clone specific branch ============
	isBranch, err := CheckIfRemoteBranch(repoUrl, repoRef)
	if err != nil {
		return fmt.Errorf("check if remote branch error: %w", err)
	}
	if isBranch {
		command := fmt.Sprintf("%s clone --branch %s %s --recursive %s", gitCmd(), repoRef, repoUrl, repoDir)
		return cmd.NewExecutor(title, command).Execute()
	}

	// ============ Clone specific tag ============
	isTag, err := CheckIfRemoteTag(repoUrl, repoRef)
	if err != nil {
		return fmt.Errorf("check if remote tag error: %w", err)
	}
	if isTag {
		command := fmt.Sprintf("%s clone --tag %s %s --recursive %s", gitCmd(), repoRef, repoUrl, repoDir)
		return cmd.NewExecutor(title, command).Execute()
	}

	// ============ Clone and checkout commit ============
	command := fmt.Sprintf("%s clone %s %s", gitCmd(), repoUrl, repoDir)
	if err := cmd.NewExecutor(title, command).Execute(); err != nil {
		return fmt.Errorf("clone git repo error: %w", err)
	}

	// Checkout repo to commit.
	command = fmt.Sprintf("%s reset --hard %s", gitCmd(), repoRef)
	executor := cmd.NewExecutor(title+" (reset to commit)", command)
	executor.SetWorkDir(repoDir)
	if err := executor.Execute(); err != nil {
		return fmt.Errorf("reset --hard error: %w", err)
	}

	// Update submodules.
	if fileio.PathExists(filepath.Join(repoDir, ".gitmodules")) {
		command = fmt.Sprintf("%s submodule update --init --recursive", gitCmd())
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

	var commands []string
	commands = append(commands, fmt.Sprintf("%s reset --hard && git clean -xfd", gitCmd()))

	updateRepo := func(commands []string) error {
		commandLine := strings.Join(commands, " && ")
		executor := cmd.NewExecutor(title, commandLine)
		executor.SetWorkDir(repoDir)
		return executor.Execute()
	}

	// Update to branch.
	isBranch, err := CheckIfRemoteBranch(repoDir, repoRef)
	if err != nil {
		return err
	}
	if isBranch {
		commands = append(commands, fmt.Sprintf("%s fetch origin %s", gitCmd(), repoRef))
		commands = append(commands, fmt.Sprintf("%s checkout -B %s origin/%s", gitCmd(), repoRef, repoRef))
		commands = append(commands, fmt.Sprintf("%s pull origin %s", gitCmd(), repoRef))
		return updateRepo(commands)
	}

	// update to tag.
	isTag, err := CheckIfRemoteTag(repoDir, repoRef)
	if err != nil {
		return err
	}
	if isTag {
		commands = append(commands, fmt.Sprintf("%s tag -d %s || true", gitCmd(), repoRef))
		commands = append(commands, fmt.Sprintf("%s fetch --tags origin", gitCmd()))
		commands = append(commands, fmt.Sprintf("%s checkout %s", gitCmd(), repoRef))
		return updateRepo(commands)
	}

	return fmt.Errorf("invalid repoRef: %s", repoRef)
}

// CherryPick cherry-pick patches.
func CherryPick(title, srcDir string, patches []string) error {
	// Change to source dir to execute git command.
	if err := os.Chdir(srcDir); err != nil {
		return err
	}

	// Execute patch command.
	var commands []string
	commands = append(commands, fmt.Sprintf("%s -C %s fetch origin", gitCmd(), srcDir))

	for _, patch := range patches {
		commands = append(commands, fmt.Sprintf("%s cherry-pick %s", gitCmd(), patch))
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
	commands = append(commands, fmt.Sprintf("%s -C %s fetch origin", gitCmd(), srcDir))

	for _, ref := range rebaseRefs {
		commands = append(commands, fmt.Sprintf("%s checkout %s", gitCmd(), ref))
		commands = append(commands, fmt.Sprintf("%s rebase %s", gitCmd(), repoRef))
	}

	commandLine := strings.Join(commands, " && ")
	if err := cmd.NewExecutor(title, commandLine).Execute(); err != nil {
		return err
	}

	return nil
}

// Clean clean git repo.
func Clean(repoDir string) error {
	// git reset --hard
	resetCmd := exec.Command(gitCmd(), "reset", "--hard")
	resetCmd.Stdout = os.Stdout
	resetCmd.Stderr = os.Stderr
	if err := resetCmd.Run(); err != nil {
		return err
	}

	// git clean -xfd
	cleanCmd := exec.Command(gitCmd(), "clean", "-xfd")
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
		command := fmt.Sprintf("%s apply --ignore-space-change --ignore-whitespace -v %s", gitCmd(), patchFile)
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
