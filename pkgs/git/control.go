package git

import (
	"bufio"
	"bytes"
	"celer/pkgs/cmd"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"strings"
)

// CloneRepo clone git repo.
func CloneRepo(title, repoUrl, repoRef string, depth int, repoDir string) error {
	cloneRepo := func(repoRef, repoUrl, repoDir string, depth int) string {
		var buffer bytes.Buffer
		buffer.WriteString("git clone")
		if repoRef != "" {
			buffer.WriteString(" --branch " + repoRef)
		}
		if depth > 0 {
			fmt.Fprintf(&buffer, " --depth %d", depth)
		}
		fmt.Fprintf(&buffer, " %s %s", repoUrl, repoDir)
		return buffer.String()
	}

	// ============ Clone default branch ============
	if repoRef == "" {
		command := cloneRepo(repoRef, repoUrl, repoDir, depth)
		return cmd.NewExecutor(title, command).Execute()
	}

	// ============ Clone specific branch ============
	isBranch, err := CheckIfRemoteBranch(repoUrl, repoRef)
	if err != nil {
		return fmt.Errorf("failed to check if remote branch -> %w", err)
	}
	if isBranch {
		command := cloneRepo(repoRef, repoUrl, repoDir, depth)
		return cmd.NewExecutor(title, command).Execute()
	}

	// ============ Clone specific tag ============
	isTag, err := CheckIfRemoteTag(repoUrl, repoRef)
	if err != nil {
		return fmt.Errorf("failed to check if remote tag: %s -> %w", repoRef, err)
	}
	if isTag {
		command := cloneRepo(repoRef, repoUrl, repoDir, depth)
		return cmd.NewExecutor(title, command).Execute()
	}

	// ============ Clone and checkout commit ============
	command := fmt.Sprintf("git clone %s %s", repoUrl, repoDir)
	if err := cmd.NewExecutor(title, command).Execute(); err != nil {
		return fmt.Errorf("faield to clone git repo -> %w", err)
	}

	// Checkout repo to commit.
	command = fmt.Sprintf("git reset --hard %s", repoRef)
	executor := cmd.NewExecutor(title+" (reset to commit)", command)
	executor.SetWorkDir(repoDir)
	if err := executor.Execute(); err != nil {
		return fmt.Errorf("failed to reset --hard -> %w", err)
	}

	return nil
}

// UpdateSubmodules update git submodules.
func UpdateSubmodules(title, repoDir string) error {
	if !fileio.PathExists(filepath.Join(repoDir, ".gitmodules")) {
		return nil
	}

	command := "git submodule update --init --recursive"
	executor := cmd.NewExecutor(title, command)
	executor.SetWorkDir(repoDir)
	if err := executor.Execute(); err != nil {
		return fmt.Errorf("failed to update submodules -> %w", err)
	}
	return nil
}

// UpdateRepo update git repo.
func UpdateRepo(title, repoRef, repoDir string, force bool) error {
	if !fileio.PathExists(repoDir) {
		return nil
	}
	if !fileio.PathExists(filepath.Join(repoDir, ".git")) {
		return fmt.Errorf("refuse to run git commands in non-repo dir: %s", repoDir)
	}

	// Check if repo is modified.
	modified, err := IsModified(repoDir)
	if err != nil {
		return err
	}
	if modified {
		if !force {
			return fmt.Errorf("repository has local modifications, update is skipped ... ⭐⭐⭐ you can update forcibly with -f/--force ⭐⭐⭐")
		}
	}

	// Get default branch if repoRef is empty.
	if repoRef == "" {
		branch, err := GetDefaultBranch(repoDir)
		if err != nil {
			return err
		}
		repoRef = branch
	}

	// Get repo URL.
	repoUrl, err := GetRepoUrl(repoDir)
	if err != nil {
		return err
	}

	// Update to branch.
	isBranch, err := CheckIfRemoteBranch(repoUrl, repoRef)
	if err != nil {
		return err
	}
	if isBranch {
		commands := []string{
			"git reset --hard",
			"git clean -xfd",
			"git fetch origin " + repoRef,
			"git checkout -B " + repoRef + " origin/" + repoRef,
			"git pull origin " + repoRef,
		}
		commandLine := strings.Join(commands, " && ")
		executor := cmd.NewExecutor(title, commandLine)
		executor.SetWorkDir(repoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
		return nil
	}

	// Update to tag.
	isTag, err := CheckIfRemoteTag(repoUrl, repoRef)
	if err != nil {
		return err
	}
	if isTag {
		// Delete local tag if exists (ignore error if not exists)
		deleteTagCmd := "git tag -d " + repoRef
		deleteExecutor := cmd.NewExecutor("", deleteTagCmd)
		deleteExecutor.SetWorkDir(repoDir)
		deleteExecutor.Execute() // Ignore error, tag may not exist.

		// Fetch and checkout tag
		commands := []string{
			"git reset --hard",
			"git clean -xfd",
			"git fetch --tags origin",
			"git checkout " + repoRef,
		}

		commandLine := strings.Join(commands, " && ")
		executor := cmd.NewExecutor(title, commandLine)
		executor.SetWorkDir(repoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
		return nil
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
	if !fileio.PathExists(filepath.Join(repoDir, ".git")) {
		return fmt.Errorf("refuse to run git commands in non-repo dir: %s", repoDir)
	}

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
			return fmt.Errorf("cannot read .patched -> %w", err)
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
		executor := cmd.NewExecutor(title, "patch", "-Np1", "-i", patchFile)
		executor.SetWorkDir(repoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	// Create a flag file to indicated that patch already applied.
	recordFile, err := os.OpenFile(recordFilePath, os.O_CREATE|os.O_RDWR|os.O_APPEND, os.ModePerm)
	if err != nil {
		return fmt.Errorf("cannot open or create .patched -> %w", err)
	}
	defer recordFile.Close()

	if _, err := recordFile.WriteString(patchFileName + "\n"); err != nil {
		return fmt.Errorf("cannot write %s into .patched -> %w", patchFileName, err)
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
