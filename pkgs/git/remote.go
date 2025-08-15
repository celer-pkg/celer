package git

import (
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
