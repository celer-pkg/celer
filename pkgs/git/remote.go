package git

import (
	"celer/pkgs/cmd"
	"fmt"
	"strings"
)

// CheckIfRemoteBranch check if repoRef is a branch.
func CheckIfRemoteBranch(repoUrl, repoRef string) (bool, error) {
	command := fmt.Sprintf("git ls-remote --heads %s %s", repoUrl, repoRef)
	executor := cmd.NewExecutor("", command)
	output, err := executor.ExecuteOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(output) != "", nil
}

// CheckIfRemoteTag check if repoRef is a tag.
func CheckIfRemoteTag(repoUrl, repoRef string) (bool, error) {
	command := fmt.Sprintf("git ls-remote --tags %s %s", repoUrl, repoRef)
	executor := cmd.NewExecutor("", command)
	output, err := executor.ExecuteOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(output) != "", nil
}

// ReadLocalCommit read git commit.
func ReadRemoteCommit(repoUrl, repoRef string) (string, error) {
	// Try to get latest commit of branch.
	isBranch, err := CheckIfRemoteBranch(repoUrl, repoRef)
	if err != nil {
		return "", fmt.Errorf("failed to check if remote branch -> %w", err)
	}
	if isBranch {
		command := fmt.Sprintf("git ls-remote %s %s", repoUrl, "refs/heads/"+repoRef)
		executor := cmd.NewExecutor("", command)
		output, err := executor.ExecuteOutput()
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
	isTag, err := CheckIfRemoteTag(repoUrl, repoRef)
	if err != nil {
		return "", fmt.Errorf("failed to check if remote tag: %s -> %w", repoRef, err)
	}
	if isTag {
		command := fmt.Sprintf("git ls-remote %s %s", repoUrl, "refs/tags/"+repoRef)
		executor := cmd.NewExecutor("", command)
		output, err := executor.ExecuteOutput()
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
