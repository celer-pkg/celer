package git

import (
	"fmt"
	"strings"
	"time"

	"github.com/celer-pkg/celer/pkgs/cmd"
)

const retryMaxAttempts = 3

func retrySleep(attempt int) {
	time.Sleep(time.Duration(attempt) * time.Second)
}

// CheckIfRemoteBranch check if repoRef is a branch.
func CheckIfRemoteBranch(target, repoUrl, repoRef string) (bool, error) {
	title := fmt.Sprintf("[query remote branch: %s]", target)
	output, err := cmd.NewExecutor(title, "git", "ls-remote", "--heads", repoUrl, repoRef).
		WithRetry(retryMaxAttempts).ExecuteOutput()
	if err != nil {
		return false, fmt.Errorf("failed to query remote branch %s of %s -> %w", repoRef, repoUrl, err)
	}

	return strings.TrimSpace(output) != "", nil
}

// CheckIfRemoteTag check if repoRef is a tag.
func CheckIfRemoteTag(target, repoUrl, repoRef string) (bool, error) {
	title := fmt.Sprintf("[query remote tag: %s]", target)
	output, err := cmd.NewExecutor(title, "git", "ls-remote", "--tags", repoUrl, repoRef).
		WithRetry(retryMaxAttempts).ExecuteOutput()
	if err != nil {
		return false, fmt.Errorf("failed to query remote tag %s of %s -> %w", repoRef, repoUrl, err)
	}
	return strings.TrimSpace(output) != "", nil
}

// GetRemoteHeadCommit resolves the HEAD commit of a remote repository.
func GetRemoteHeadCommit(target, repoUrl string) (string, error) {
	title := fmt.Sprintf("[resolve remote HEAD: %s]", target)
	output, err := cmd.NewExecutor(title, "git", "ls-remote", repoUrl, "HEAD").
		WithRetry(retryMaxAttempts).ExecuteOutput()
	if err != nil {
		return "", fmt.Errorf("failed to resolve HEAD of %s -> %w", repoUrl, err)
	}

	fields := strings.Fields(output)
	if len(fields) < 1 {
		return "", fmt.Errorf("no HEAD commit found for %s", repoUrl)
	}

	return fields[0], nil
}

// GetRemoteRefCommit read remote git commit hash of specified ref.
func GetRemoteRefCommit(target, repoUrl, repoRef string) (string, error) {
	// Try to get latest commit of branch.
	isBranch, err := CheckIfRemoteBranch(target, repoUrl, repoRef)
	if err != nil {
		return "", fmt.Errorf("failed to check if remote branch -> %w", err)
	}
	if isBranch {
		title := fmt.Sprintf("[read remote branch commit: %s]", target)
		output, err := cmd.NewExecutor(title, "git", "ls-remote", repoUrl, "refs/heads/"+repoRef).
			WithRetry(retryMaxAttempts).ExecuteOutput()
		if err != nil {
			return "", fmt.Errorf("failed to read git commit hash -> %w", err)
		}

		fields := strings.Fields(output)
		if len(fields) < 1 {
			return "", fmt.Errorf("invalid git commit hash: %s", output)
		}

		return fields[0], nil
	}

	// Try to get latest commit of tag.
	isTag, err := CheckIfRemoteTag(target, repoUrl, repoRef)
	if err != nil {
		return "", fmt.Errorf("failed to check if remote tag: %s -> %w", repoRef, err)
	}
	if isTag {
		title := fmt.Sprintf("[read remote tag commit: %s]", target)
		output, err := cmd.NewExecutor(title, "git", "ls-remote", repoUrl, "refs/tags/"+repoRef).
			WithRetry(retryMaxAttempts).ExecuteOutput()
		if err != nil {
			return "", fmt.Errorf("failed to read git commit hash -> %w", err)
		}

		fields := strings.Fields(output)
		if len(fields) < 1 {
			return "", fmt.Errorf("invalid git commit hash: %s", output)
		}

		return fields[0], nil
	}

	// The repoRef may be a commit.
	return repoRef, nil
}
