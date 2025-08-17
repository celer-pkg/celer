package git

import (
	"os/exec"
	"strings"
)

// CheckIfRemoteBranch check if repoRef is a branch.
func CheckIfRemoteBranch(repoUrl, repoRef string) (bool, error) {
	cmd := exec.Command("git", "ls-remote", "--heads", repoUrl, repoRef)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) != "", nil
}

// CheckIfRemoteTag check if repoRef is a tag.
func CheckIfRemoteTag(repoUrl, repoRef string) (bool, error) {
	cmd := exec.Command("git", "ls-remote", "--tags", repoUrl, repoRef+"^{}")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) != "", nil
}
