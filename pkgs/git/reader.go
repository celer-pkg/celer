package git

import (
	"bytes"
	"celer/pkgs/fileio"
	"fmt"
	"os/exec"
	"strings"
)

// IsBranch check if ref is a branch.
func IsBranch(repoUrl, repoRef string) bool {
	cmd := exec.Command("git", "ls-remote", "--heads", repoUrl, repoRef)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false
	}
	return strings.TrimSpace(string(output)) != ""
}

// IsTag check if ref is a tag.
func IsTag(repoUrl, repoRef string) bool {
	cmd := exec.Command("git", "ls-remote", "--tags", repoUrl, repoRef+"^{}")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false
	}
	return strings.TrimSpace(string(output)) != ""
}

// IsCommit check if ref is a commit.
func IsCommit(repoUrl, repoRef string) bool {
	cmd := exec.Command("git", "ls-remote", repoUrl)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false
	}
	return strings.Contains(string(output), repoRef)
}

// IsModified check if repo is modified.
func IsModified(repoDir string) (bool, error) {
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

// ReadCommit read git commit hash.
func ReadCommit(repoDir string) (string, error) {
	if !fileio.PathExists(repoDir) {
		return "", fmt.Errorf("repo dir %s is not exist", repoDir)
	}
	command := exec.Command("git", "-C", repoDir, "rev-parse", "HEAD")

	var out bytes.Buffer
	command.Stdout = &out
	command.Stderr = &out

	if err := command.Run(); err != nil {
		return "", fmt.Errorf("read git commit hash: %w", err)
	}

	return strings.TrimSpace(out.String()), nil
}

// DefaultBranch read git default branch.
func DefaultBranch(repoDir string) (string, error) {
	command := exec.Command("git", "remote", "show", "origin")
	output, err := command.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("read git default branch: %w", err)
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
