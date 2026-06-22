package git

import (
	"bufio"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"slices"
	"strings"

	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/color"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/fileio"
)

// CloneRepo clone git repo.
func CloneRepo(title, target, repoUrl, repoRef string, depth int, repoDir string) error {
	retryExecutor := func(title, command string) error {
		executor := cmd.NewExecutor(title, command)
		if fileio.PathExists(repoDir) {
			executor.SetWorkDir(repoDir)
		}
		return executor.Execute()
	}

	cloneArgsFunc := func(repoRef, repoUrl, repoDir string, depth int) []string {
		args := []string{"clone"}
		if repoRef != "" {
			args = append(args, "--branch", repoRef)
		}
		if depth > 0 {
			args = append(args, "--single-branch")
			args = append(args, "--depth", fmt.Sprint(depth))
		}
		args = append(args, repoUrl, repoDir)
		return args
	}

	cloneWithRetry := func(action string, args []string) error {
		var lastErr error

		for attempt := 1; attempt <= retryMaxAttempts; attempt++ {
			// Failed clones can leave a partial destination behind and poison the
			// next attempt, so always retry from a clean target directory.
			if err := os.RemoveAll(repoDir); err != nil {
				return fmt.Errorf("failed to clean repo dir %s for %s -> %w", repoDir, target, err)
			}

			executor := cmd.NewExecutor(title, "git", args...)
			err := executor.Execute()
			if err == nil {
				return nil
			}

			lastErr = err
			color.Printf(color.Warning, "Git %s failed (attempt %d/%d) for %s: %v\n", action, attempt, retryMaxAttempts, target, err)
			if attempt < retryMaxAttempts {
				retrySleep(attempt)
			}
		}

		return fmt.Errorf("git %s failed after %d attempts for %s -> %w", action, retryMaxAttempts, target, lastErr)
	}

	cloneWithFallback := func(action string, repoRef string, depth int) error {
		args := cloneArgsFunc(repoRef, repoUrl, repoDir, depth)
		if err := cloneWithRetry(action, args); err != nil {
			if depth > 0 {
				color.Printf(color.Warning, "-- Git %s failed with shallow clone for %s, retrying without --depth\n", action, target)
				if fallbackErr := cloneWithRetry(action+" without depth", cloneArgsFunc(repoRef, repoUrl, repoDir, 0)); fallbackErr == nil {
					return nil
				}
			}
			return err
		}
		return nil
	}

	// ============ Clone default branch ============
	if repoRef == "" {
		if err := cloneWithFallback("clone git repo", repoRef, depth); err != nil {
			return fmt.Errorf("failed to clone git repo for %s -> %w", target, err)
		}
		return nil
	}

	// ============ Clone specific branch ============
	isBranch, err := CheckIfRemoteBranch(target, repoUrl, repoRef)
	if err != nil {
		return fmt.Errorf("failed to check if remote branch '%s' for '%s' -> %w", repoRef, target, err)
	}
	if isBranch {
		if err := cloneWithFallback("clone git branch", repoRef, depth); err != nil {
			return fmt.Errorf("failed to clone git branch '%s' for '%s' -> %w", repoRef, target, err)
		}
		return nil
	}

	// ============ Clone specific tag ============
	isTag, err := CheckIfRemoteTag(target, repoUrl, repoRef)
	if err != nil {
		return fmt.Errorf("failed to check if remote tag '%s' for '%s' -> %w", repoRef, target, err)
	}
	if isTag {
		if err := cloneWithFallback("clone git tag", repoRef, depth); err != nil {
			return fmt.Errorf("failed to clone with git tag '%s' for '%s' -> %w", repoRef, target, err)
		}
		return nil
	}

	// ============ Clone and checkout commit ============
	cloneArgs := []string{"clone", repoUrl, repoDir}
	if err := cloneWithRetry("clone git repo", cloneArgs); err != nil {
		return fmt.Errorf("failed to clone with git repo %s for '%s' -> %w", repoUrl, target, err)
	}

	// Fetch all remote refs so the commit object is available locally.
	fetchExecutor := cmd.NewExecutor(title+" (fetch)", "git", "fetch", "origin")
	fetchExecutor.SetWorkDir(repoDir)
	if output, err := fetchExecutor.ExecuteOutputLive(); err != nil {
		return fmt.Errorf("failed to fetch origin after clone for '%s' -> %w: %s", target, err, output)
	}

	// Checkout repo to commit.
	command := fmt.Sprintf("git reset --hard %s", repoRef)
	if err := retryExecutor(title+" (reset to commit)", command); err != nil {
		return fmt.Errorf("failed to reset --hard '%s' to commit '%s' -> %w", target, repoRef, err)
	}

	return nil
}

// UpdateSubmodules update git submodules.
func UpdateSubmodules(title, repoDir string) error {
	if !fileio.PathExists(repoDir) {
		return errors.ErrDirNotExist
	}
	if !fileio.PathExists(filepath.Join(repoDir, ".git")) {
		return errors.ErrNotGitDir
	}

	if !fileio.PathExists(filepath.Join(repoDir, ".gitmodules")) {
		return nil
	}

	command := "git submodule update --init --recursive"
	executor := cmd.NewExecutor(title, command)
	executor.SetWorkDir(repoDir)
	if output, err := executor.ExecuteOutputLive(); err != nil {
		return fmt.Errorf("failed to update submodules: %s -> %w", output, err)
	}
	return nil
}

// UpdateRepo update git repo.
func UpdateRepo(target, repoRef, repoDir string, force bool) error {
	if !fileio.PathExists(repoDir) {
		return errors.ErrDirNotExist
	}
	if !fileio.PathExists(filepath.Join(repoDir, ".git")) {
		return errors.ErrNotGitDir
	}

	// Check if repo is modified.
	modified, err := IsModified(repoDir)
	if err != nil {
		return err
	}
	if modified {
		if !force {
			return fmt.Errorf("repository has local modifications, update is skipped - you can update forcibly with -f/--force")
		}
	}

	// Get default branch if repoRef is empty.
	if repoRef == "" {
		branch, err := GetDefaultBranch(target, repoDir)
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

	// Update to a specific commit hash: fetch then hard-reset.
	if CheckIsCommitHash(repoRef) {
		if err := HardReset(repoDir, repoRef); err != nil {
			return fmt.Errorf("failed to update %s to commit '%s' -> %w", target, repoRef, err)
		}
		return nil
	}

	// Update to branch.
	isBranch, err := CheckIfRemoteBranch(target, repoUrl, repoRef)
	if err != nil {
		return err
	}
	if isBranch {
		commands := []string{
			"git reset --hard",
			"git clean -ffdx",
			"git fetch origin " + repoRef,
			"git checkout -B " + repoRef + " origin/" + repoRef,
			"git pull origin " + repoRef,
		}
		commandLine := strings.Join(commands, " && ")
		executor := cmd.NewExecutor("[update "+target+"]", commandLine)
		executor.SetWorkDir(repoDir)
		if output, err := executor.ExecuteOutputLive(); err != nil {
			return fmt.Errorf("failed to update '%s' to '%s' -> %s -> %w", target, repoRef, output, err)
		}
		return nil
	}

	// Update to tag.
	isTag, err := CheckIfRemoteTag(target, repoUrl, repoRef)
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
			"git clean -ffdx",
			"git fetch origin tag " + repoRef,
			"git checkout -f " + repoRef,
		}

		commandLine := strings.Join(commands, " && ")
		executor := cmd.NewExecutor("[update "+target+"]", commandLine)
		executor.SetWorkDir(repoDir)
		if output, err := executor.ExecuteOutputLive(); err != nil {
			return fmt.Errorf("failed to update '%s' to '%s' -> %s -> %w", target, repoRef, output, err)
		}
		return nil
	}

	return fmt.Errorf("invalid repo ref %s for %s", repoRef, target)
}

// CherryPick cherry-pick patches.
func CherryPick(title, repoDir string, patches []string) error {
	if !fileio.PathExists(repoDir) {
		return errors.ErrDirNotExist
	}
	if !fileio.PathExists(filepath.Join(repoDir, ".git")) {
		return errors.ErrNotGitDir
	}

	// Change to source dir to execute git command.
	if err := os.Chdir(repoDir); err != nil {
		return err
	}

	var commands []string
	commands = append(commands, "git fetch origin")

	for _, patch := range patches {
		commands = append(commands, "git cherry-pick "+patch)
	}

	commandLine := strings.Join(commands, " && ")
	executor := cmd.NewExecutor(title, commandLine)
	executor.SetWorkDir(repoDir)
	if output, err := executor.ExecuteOutputLive(); err != nil {
		return fmt.Errorf("%s -> %w", output, err)
	}

	return nil
}

// Rebase rebase patches.
func Rebase(title, repoRef, repoDir string, rebaseRefs []string) error {
	if !fileio.PathExists(repoDir) {
		return errors.ErrDirNotExist
	}
	if !fileio.PathExists(filepath.Join(repoDir, ".git")) {
		return errors.ErrNotGitDir
	}

	// Change to source dir to execute git command.
	if err := os.Chdir(repoDir); err != nil {
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
	executor.SetWorkDir(repoDir)
	if output, err := executor.ExecuteOutputLive(); err != nil {
		return fmt.Errorf("%s -> %w", output, err)
	}

	return nil
}

// Clean clean git repo.
func Clean(target, repoDir string) error {
	if !fileio.PathExists(repoDir) {
		return errors.ErrDirNotExist
	}
	if !fileio.PathExists(filepath.Join(repoDir, ".git")) {
		return errors.ErrNotGitDir
	}

	var commands []string
	commands = append(commands, "git reset --hard")
	commands = append(commands, "git clean -ffdx")

	title := fmt.Sprintf("[clean %s]", target)
	commandLine := strings.Join(commands, " && ")
	executor := cmd.NewExecutor(title, commandLine)
	executor.SetWorkDir(repoDir)
	if output, err := executor.ExecuteOutputLive(); err != nil {
		return fmt.Errorf("failed to clean '%s' -> %s -> %w", target, output, err)
	}

	return nil
}

// HardReset hard resets the repo to the given commit and cleans untracked files.
// If the commit is not available locally, it fetches from origin first.
func HardReset(repoDir, commit string) error {
	if !fileio.PathExists(repoDir) {
		return errors.ErrDirNotExist
	}
	if !fileio.PathExists(filepath.Join(repoDir, ".git")) {
		return errors.ErrNotGitDir
	}

	nameVersion := filepath.Base(filepath.Dir(repoDir))
	title := fmt.Sprintf("[reset %s]", nameVersion)

	execute := func(args ...string) error {
		executor := cmd.NewExecutor(title, "git", args...)
		executor.SetWorkDir(repoDir)
		if output, err := executor.ExecuteOutputLive(); err != nil {
			return fmt.Errorf("%s -> %w", output, err)
		}
		return nil
	}

	// Try reset directly — commit may already be local.
	if err := execute("reset", "--hard", commit); err != nil {
		// Commit not found locally, fetch and retry.
		if err := execute("fetch", "origin"); err != nil {
			return fmt.Errorf("failed to fetch origin for '%s' -> %w", nameVersion, err)
		}
		if err := execute("reset", "--hard", commit); err != nil {
			return fmt.Errorf("failed to reset --hard '%s' to '%s' -> %w", nameVersion, commit, err)
		}
	}

	if err := execute("clean", "-ffdx"); err != nil {
		return fmt.Errorf("failed to clean '%s' -> %w", nameVersion, err)
	}

	return nil
}

// ApplyPatch apply git patch.
func ApplyPatch(nameVersion, repoDir, patchFile string) error {
	if !fileio.PathExists(repoDir) {
		return errors.ErrDirNotExist
	}
	if !fileio.PathExists(filepath.Join(repoDir, ".git")) {
		return errors.ErrNotGitDir
	}

	patchFileName := filepath.Base(patchFile)

	// Check if patched already.
	recordFilePath := filepath.Join(repoDir, ".patched")
	if pathExists(recordFilePath) {
		bytes, err := os.ReadFile(recordFilePath)
		if err != nil {
			return fmt.Errorf("failed to read .patched for '%s' -> %w", nameVersion, err)
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
	if err := scanner.Err(); err != nil {
		return err
	}

	if gitBatch {
		title := fmt.Sprintf("[apply patch: %s]", nameVersion)
		args := []string{"apply", "--ignore-space-change", "--ignore-whitespace", "-v", patchFile}
		executor := cmd.NewExecutor(title, "git", args...)
		executor.SetWorkDir(repoDir)
		if output, err := executor.ExecuteOutputLive(); err != nil {
			return fmt.Errorf("failed to apply patch for '%s' -> %s -> %w", nameVersion, output, err)
		}
	} else {
		// Others, assume it's a regular patch file.
		title := fmt.Sprintf("[apply patch: %s]", nameVersion)
		executor := cmd.NewExecutor(title, "patch", "-Np1", "-i", patchFile)
		executor.SetWorkDir(repoDir)
		if output, err := executor.ExecuteOutputLive(); err != nil {
			return fmt.Errorf("failed to apply patch for '%s' -> %s -> %w", nameVersion, output, err)
		}
	}

	// Create a flag file to indicated that patch already applied.
	recordFile, err := os.OpenFile(recordFilePath, os.O_CREATE|os.O_RDWR|os.O_APPEND, os.ModePerm)
	if err != nil {
		return fmt.Errorf("failed to open or create .patched for '%s' -> %w", nameVersion, err)
	}
	defer recordFile.Close()

	if _, err := recordFile.WriteString(patchFileName + "\n"); err != nil {
		return fmt.Errorf("failed to write %s into .patched for '%s' -> %w", patchFileName, nameVersion, err)
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

var commitHashPattern = regexp.MustCompile(`^[a-fA-F0-9]{7,40}$`)

// CheckIsCommitHash check if a valid git commit hash, booth short hash and long hash can be supported.
func CheckIsCommitHash(hash string) bool {
	return commitHashPattern.MatchString(strings.TrimSpace(hash))
}
