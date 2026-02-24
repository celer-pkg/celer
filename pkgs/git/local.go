package git

import (
	"fmt"
	"os"
	"os/exec"
	"strings"
)

// GetRepoUrl get git repo origin URL.
func GetRepoUrl(repoDir string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "remote", "get-url", "origin")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get repo url -> %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

// GetCurrentBranch read current branch of repo.
func GetCurrentBranch(repoDir string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "rev-parse", "--abbrev-ref", "HEAD")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get current branch -> %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

// GetCurrentTag read current tag of repo.
func GetCurrentTag(repoDir string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "describe", "--exact-match", "--tags", "HEAD")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get current tag -> %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

// IsModified check if repo is modified.
func IsModified(repoDir string) (bool, error) {
	cmd := exec.Command("git", "-C", repoDir, "status", "--porcelain")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, fmt.Errorf("check if repo is modified -> %s", output)
	}

	return strings.TrimSpace(string(output)) != "", nil
}

// GetCurrentCommit read git commit hash.
func GetCurrentCommit(repoDir string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "rev-parse", "HEAD")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("read git commit hash: %s", output)
	}

	return strings.TrimSpace(string(output)), nil
}

// GetDefaultBranch read git default branch.
func GetDefaultBranch(repoDir string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "remote", "show", "origin")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("read git default branch: %s", output)
	}

	lines := strings.SplitSeq(string(output), "\n")
	for line := range lines {
		if strings.Contains(line, "HEAD branch") {
			parts := strings.Split(line, ":")
			if len(parts) > 1 {
				return strings.TrimSpace(parts[1]), nil
			}
		}
	}

	return "", fmt.Errorf("default branch not found of %s", repoDir)
}

func gitRevParse(repoDir, ref string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "rev-parse", ref)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("git rev-parse %s -> %s", ref, output)
	}
	return strings.TrimSpace(string(output)), nil
}

// CheckIfUpToDate check if repo matches remote state for current branch or tag.
func CheckIfUpToDate(repoDir string) (bool, error) {
	// Get current commit hash.
	currentCommit, err := GetCurrentCommit(repoDir)
	if err != nil {
		return false, err
	}

	// Check if there's any remote configured.
	cmd := exec.Command("git", "-C", repoDir, "remote")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return false, fmt.Errorf("git remote failed: %v", err)
	}

	// No remote - local repo is up to date.
	if strings.TrimSpace(string(output)) == "" {
		return true, nil
	}

	// Fetch remote updates.
	cmd = exec.Command("git", "-C", repoDir, "fetch", "--tags", "origin")
	if err := cmd.Run(); err != nil {
		return false, fmt.Errorf("git fetch failed: %v", err)
	}

	// If current checkout is on a branch, compare with origin/<branch>.
	branch, err := GetCurrentBranch(repoDir)
	if err == nil && branch != "" && branch != "HEAD" {
		if remoteCommit, err := gitRevParse(repoDir, "origin/"+branch); err == nil {
			return currentCommit == remoteCommit, nil
		}
	}

	// If current checkout is exactly at a tag, ensure this tag exists on origin.
	tag, err := GetCurrentTag(repoDir)
	if err == nil && tag != "" {
		cmd = exec.Command("git", "-C", repoDir, "ls-remote", "--tags", "origin", "refs/tags/"+tag, "refs/tags/"+tag+"^{}")
		output, err := cmd.CombinedOutput()
		if err != nil {
			return false, fmt.Errorf("git ls-remote --tags %s -> %s", tag, output)
		}
		for line := range strings.SplitSeq(strings.TrimSpace(string(output)), "\n") {
			if line == "" {
				continue
			}

			hash := strings.Fields(line)[0]
			if hash == currentCommit {
				return true, nil
			}
		}
		return false, nil
	}

	// Fallback to remote default branch head.
	remoteCommit, err := gitRevParse(repoDir, "origin/HEAD")
	if err == nil {
		return currentCommit == remoteCommit, nil
	}
	if defaultBranch, err := GetDefaultBranch(repoDir); err == nil && defaultBranch != "" {
		if remoteCommit, err := gitRevParse(repoDir, "origin/"+defaultBranch); err == nil {
			return currentCommit == remoteCommit, nil
		}
	}

	// Cannot resolve a meaningful remote target.
	return false, nil
}

// InitAsLocalRepo init folder as a local repo.
func InitAsLocalRepo(repoDir, message string) error {
	cmd := exec.Command("git", "-C", repoDir, "init")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to git init repo -> %s", output)
	}

	cmd = exec.Command("git", "-C", repoDir, "add", "-A")
	output, err = cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to git add -A -> %s", output)
	}

	cmd = exec.Command("git", "-C", repoDir, "commit", "-m", message)
	cmd.Env = append(os.Environ(),
		"GIT_AUTHOR_NAME=CI Robot",
		"GIT_AUTHOR_EMAIL=ci@celer.com",
		"GIT_COMMITTER_NAME=CI Robot",
		"GIT_COMMITTER_EMAIL=ci@celer.com",
	)
	output, err = cmd.CombinedOutput()
	if err != nil {
		return fmt.Errorf("failed to git commit repo -> %s", output)
	}

	return nil
}
