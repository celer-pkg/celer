package git

import (
	"os/exec"
	"strings"
)

// CheckIfRemoteBranch check if repoRef is a branch.
func CheckIfRemoteBranch(repoUrl, repoRef string) bool {
	cmd := exec.Command("git", "ls-remote", "--heads", repoUrl, repoRef)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false
	}
	return strings.TrimSpace(string(output)) != ""
}

// CheckIfRemoteTag check if repoRef is a tag.
func CheckIfRemoteTag(repoUrl, repoRef string) bool {
	cmd := exec.Command("git", "ls-remote", "--tags", repoUrl, repoRef+"^{}")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false
	}
	return strings.TrimSpace(string(output)) != ""
}

// CheckIfRemoteCommit check if repoRef is a commit.
func CheckIfRemoteCommit(repoUrl, repoRef string) bool {
	cmd := exec.Command("git", "ls-remote", repoUrl)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false
	}
	return strings.Contains(string(output), repoRef)
}
