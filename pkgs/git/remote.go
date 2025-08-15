package git

import (
	"os/exec"
	"strings"
)

// IsRemoteBranch check if ref is a remote branch.
func IsRemoteBranch(repoURL, ref string) bool {
	cmd := exec.Command("git", "ls-remote", "--heads", repoURL, ref)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false
	}
	return strings.TrimSpace(string(output)) != ""
}

// IsRemoteTag check if ref is a remote tag.
func IsRemoteTag(repoURL, ref string) bool {
	cmd := exec.Command("git", "ls-remote", "--tags", repoURL, ref+"^{}")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false
	}
	return strings.TrimSpace(string(output)) != ""
}

// IsRemoteCommit check if ref is a remote commit.
func IsRemoteCommit(repoURL, ref string) bool {
	cmd := exec.Command("git", "ls-remote", repoURL)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false
	}
	return strings.Contains(string(output), ref)
}
