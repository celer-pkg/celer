package git

import (
	"fmt"
	"os"
	"os/exec"
	"strings"
)

// CheckIfLocalBranch check if repoRef is a branch.
func (g Git) CheckIfLocalBranch(repoDir, repoRef string) (bool, error) {
	// Also can call `git symbolic-ref --short HEAD`
	cmd := exec.Command("git", "rev-parse", "--abbrev-ref", "HEAD")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) == repoRef, nil
}

// CheckIfLocalTag check if repoRef is a tag.
func (g Git) CheckIfLocalTag(repoDir, repoRef string) (bool, error) {
	cmd := exec.Command("git", "describe", "--exact-match", "--tags", "HEAD")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) == repoRef, nil
}

// CheckIfLocalCommit check if repoRef is a commit.
func (g Git) CheckIfLocalCommit(repoDir, repoRef string) (bool, error) {
	cmd := exec.Command("git", "rev-parse", "HEAD")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) == repoRef, nil
}

// IsModified check if repo is modified.
func (g Git) IsModified(repoDir string) (bool, error) {
	cmd := exec.Command("git", "-C", repoDir, "status", "--porcelain")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, fmt.Errorf("check if repo is modified error: %s", output)
	}

	return strings.TrimSpace(string(output)) != "", nil
}

// ReadLocalCommit read git commit hash.
func (g Git) ReadLocalCommit(repoDir string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "rev-parse", "HEAD")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("read git commit hash: %s", output)
	}

	return strings.TrimSpace(string(output)), nil
}

// DefaultBranch read git default branch.
func (g Git) DefaultBranch(repoDir string) (string, error) {
	cmd := exec.Command("git", "remote", "show", "origin")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("read git default branch: %s", output)
	}

	lines := strings.Split(string(output), "\n")
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

func (g Git) BranchOfLocal(repoDir string) (string, error) {
	command := exec.Command("git", "-C", repoDir, "rev-parse", "--abbrev-ref", "HEAD")
	output, err := command.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get current branch name: %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

func (g Git) InitRepo(repoDir, message string) error {
	cmd := exec.Command("git", "init")
	cmd.Dir = repoDir
	output, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("git init error: %s", output)
	}

	cmd = exec.Command("git", "add", "-A")
	cmd.Dir = repoDir
	output, err = cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("git add -A error: %s", output)
	}

	cmd = exec.Command("git", "commit", "-m", message)
	cmd.Dir = repoDir
	cmd.Env = append(os.Environ(),
		"GIT_AUTHOR_NAME=CI Robot",
		"GIT_AUTHOR_EMAIL=ci@celer.com",
		"GIT_COMMITTER_NAME=CI Robot",
		"GIT_COMMITTER_EMAIL=ci@celer.com",
	)
	output, err = cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("git commit error: %s", output)
	}

	return nil
}
