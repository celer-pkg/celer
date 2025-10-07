package git

import (
	"fmt"
	"os/exec"
	"strings"
)

// CheckIfRemoteBranch check if repoRef is a branch.
func (g Git) CheckIfRemoteBranch(repoUrl, repoRef string) (bool, error) {
	args := append(g.proxyArgs(), "ls-remote", "--heads", repoUrl, repoRef)
	cmd := exec.Command("git", args...)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) != "", nil
}

// CheckIfRemoteTag check if repoRef is a tag.
func (g Git) CheckIfRemoteTag(repoUrl, repoRef string) (bool, error) {
	args := append(g.proxyArgs(), "ls-remote", "--tags", repoUrl, repoRef+"^{}")
	cmd := exec.Command("git", args...)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) != "", nil
}

// ReadLocalCommit read git commit.
func (g Git) ReadRemoteCommit(repoUrl, repoRef string) (string, error) {
	// Try to get latest commit of branch.
	isBranch, err := g.CheckIfRemoteBranch(repoUrl, repoRef)
	if err != nil {
		return "", fmt.Errorf("check if remote branch error: %w", err)
	}
	if isBranch {
		args := append(g.proxyArgs(), "ls-remote", repoUrl, "refs/heads/"+repoRef)
		cmd := exec.Command("git", args...)
		output, err := cmd.CombinedOutput()
		if err != nil {
			return "", fmt.Errorf("read git commit hash error: %w", err)
		}

		fields := strings.Fields(string(output))
		if len(fields) < 1 {
			return "", fmt.Errorf("invalid git commit hash: %s", string(output))
		}

		return fields[0], nil
	}

	// Try to get latest commit of tag.
	isTag, err := g.CheckIfRemoteTag(repoUrl, repoRef)
	if err != nil {
		return "", fmt.Errorf("check if remote tag error: %w", err)
	}
	if isTag {
		args := append(g.proxyArgs(), "ls-remote", repoUrl, "refs/tags/"+repoRef)
		cmd := exec.Command("git", args...)
		output, err := cmd.CombinedOutput()
		if err != nil {
			return "", fmt.Errorf("read git commit hash error: %w", err)
		}

		fields := strings.Fields(string(output))
		if len(fields) < 1 {
			return "", fmt.Errorf("invalid git commit hash: %s", string(output))
		}

		return fields[0], nil
	}

	// The repoRef may be a commit.
	return repoRef, nil
}
