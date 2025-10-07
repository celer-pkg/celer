package git

import (
	"fmt"
	"os/exec"
	"strings"
)

var (
	ProxyAddress string
	ProxyPort    int
)

func gitCmd() string {
	if ProxyAddress != "" && ProxyPort != 0 {
		proxy := fmt.Sprintf("%s:%d", ProxyAddress, ProxyPort)
		return fmt.Sprintf("git -c http.proxy=http://%s -c https.proxy=https://%s", proxy, proxy)
	}
	return "git"
}

// CheckIfRemoteBranch check if repoRef is a branch.
func CheckIfRemoteBranch(repoUrl, repoRef string) (bool, error) {
	cmd := exec.Command(gitCmd(), "ls-remote", "--heads", repoUrl, repoRef)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) != "", nil
}

// CheckIfRemoteTag check if repoRef is a tag.
func CheckIfRemoteTag(repoUrl, repoRef string) (bool, error) {
	cmd := exec.Command(gitCmd(), "ls-remote", "--tags", repoUrl, repoRef+"^{}")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, err
	}
	return strings.TrimSpace(string(output)) != "", nil
}

// ReadLocalCommit read git commit.
func ReadRemoteCommit(repoUrl, repoRef string) (string, error) {
	// Try to get latest commit of branch.
	isBranch, err := CheckIfRemoteBranch(repoUrl, repoRef)
	if err != nil {
		return "", fmt.Errorf("check if remote branch error: %w", err)
	}
	if isBranch {
		cmd := exec.Command(gitCmd(), "ls-remote", repoUrl, "refs/heads/"+repoRef)
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
	isTag, err := CheckIfRemoteTag(repoUrl, repoRef)
	if err != nil {
		return "", fmt.Errorf("check if remote tag error: %w", err)
	}
	if isTag {
		cmd := exec.Command(gitCmd(), "ls-remote", repoUrl, "refs/tags/"+repoRef)
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
