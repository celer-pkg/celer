package git

import (
	"fmt"
	"strings"
)

// CheckIfRemoteBranch check if repoRef is a branch.
func CheckIfRemoteBranch(target, repoUrl, repoRef string) (bool, error) {
	output, err := runWithRetry("query remote branch of "+target, "", "ls-remote", "--heads", repoUrl, repoRef)
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) != "", nil
}

// CheckIfRemoteTag check if repoRef is a tag.
func CheckIfRemoteTag(target, repoUrl, repoRef string) (bool, error) {
	output, err := runWithRetry("query remote tag of "+target, "", "ls-remote", "--tags", repoUrl, repoRef)
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) != "", nil
}

// GetRemoteCommit read git commit.
func GetRemoteCommit(target, repoUrl, repoRef string) (string, error) {
	// Try to get latest commit of branch.
	isBranch, err := CheckIfRemoteBranch(target, repoUrl, repoRef)
	if err != nil {
		return "", fmt.Errorf("failed to check if remote branch -> %w", err)
	}
	if isBranch {
		output, err := runWithRetry("read remote branch commit", "", "ls-remote", repoUrl, "refs/heads/"+repoRef)
		if err != nil {
			return "", fmt.Errorf("failed to read git commit hash -> %w", err)
		}

		fields := strings.Fields(string(output))
		if len(fields) < 1 {
			return "", fmt.Errorf("invalid git commit hash: %s", string(output))
		}

		return fields[0], nil
	}

	// Try to get latest commit of tag.
	isTag, err := CheckIfRemoteTag(target, repoUrl, repoRef)
	if err != nil {
		return "", fmt.Errorf("failed to check if remote tag: %s -> %w", repoRef, err)
	}
	if isTag {
		output, err := runWithRetry("read remote tag commit", "", "ls-remote", repoUrl, "refs/tags/"+repoRef)
		if err != nil {
			return "", fmt.Errorf("failed to read git commit hash -> %w", err)
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
