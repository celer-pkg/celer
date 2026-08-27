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
	output, err := cmd.NewExecutor(title, "git", "ls-remote", repoUrl, "refs/heads/"+repoRef).
		WithRetry(retryMaxAttempts).ExecuteOutput()
	if err != nil {
		return false, fmt.Errorf("failed to query remote branch %s of %s -> %s -> %w", repoRef, repoUrl, output, err)
	}

	return strings.TrimSpace(output) != "", nil
}

// CheckIfRemoteTag check if repoRef is a tag.
func CheckIfRemoteTag(target, repoUrl, repoRef string) (bool, error) {
	title := fmt.Sprintf("[query remote tag: %s]", target)
	output, err := cmd.NewExecutor(title, "git", "ls-remote", repoUrl, "refs/tags/"+repoRef).
		WithRetry(retryMaxAttempts).ExecuteOutput()
	if err != nil {
		return false, fmt.Errorf("failed to query remote tag %s of %s -> %s -> %w", repoRef, repoUrl, output, err)
	}
	return strings.TrimSpace(output) != "", nil
}

// GetRemoteHeadCommit resolves the HEAD commit of a remote repository.
func GetRemoteHeadCommit(target, repoUrl string) (string, error) {
	title := fmt.Sprintf("[resolve remote HEAD: %s]", target)
	output, err := cmd.NewExecutor(title, "git", "ls-remote", repoUrl, "HEAD").
		WithRetry(retryMaxAttempts).ExecuteOutput()
	if err != nil {
		return "", fmt.Errorf("failed to resolve HEAD of %s -> %s -> %w", repoUrl, output, err)
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
			return "", fmt.Errorf("failed to read git commit hash -> %s -> %w", output, err)
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
		ref := "refs/tags/" + repoRef
		// Query both the tag reference and its peeled form, prefer the commit pointed
		// to by ref^{} when available.
		output, err := cmd.NewExecutor(title, "git", "ls-remote", repoUrl, ref, ref+"^{}").
			WithRetry(retryMaxAttempts).ExecuteOutput()
		if err != nil {
			return "", fmt.Errorf("failed to read git commit hash -> %s -> %w", output, err)
		}

		var commit string
		for line := range strings.SplitSeq(output, "\n") {
			fields := strings.Fields(line)
			if len(fields) < 2 {
				continue
			}

			// Prefer the commit pointed to by ref^{} if available.
			if strings.HasSuffix(fields[1], "^{}") {
				return fields[0], nil
			}

			// This is the fallback commit pointed to by ref.
			commit = fields[0]
		}
		if commit == "" {
			return "", fmt.Errorf("invalid git commit hash: %s", output)
		}
		return commit, nil
	}

	// The repoRef may be a commit.
	return repoRef, nil
}
