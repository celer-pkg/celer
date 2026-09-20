package git

import (
	"fmt"
	"os"
	"path/filepath"

	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/fileio"
	"github.com/celer-pkg/celer/pkgs/logger"
)

const gitLFSSkipSmudgeEnv = "GIT_LFS_SKIP_SMUDGE"

// Clone describes one repository to clone.
type Clone struct {
	repoUrl         string // repo url to clone
	repoRef         string // branch, tag or commit; empty means the remote default branch
	repoDir         string // local destination, recreated before every clone attempt
	depth           int    // shallow clone depth; 0 means a full clone
	ignoreSubmodule bool   // skip Git clone submodule
	ignoreLFS       bool   // skip Git LFS downloads (GIT_LFS_SKIP_SMUDGE=1)
}

// NewClone creates a clone task for a single repository.
// Options default to the zero value: full clone, submodules included, Git LFS enabled.
func NewClone(repoUrl, repoRef, repoDir string) *Clone {
	return &Clone{
		repoUrl: repoUrl,
		repoRef: repoRef,
		repoDir: repoDir,
	}
}

func (c Clone) Clone(title, targetName string) error {
	// Without a ref, clone whatever the remote's default branch points at.
	if c.repoRef == "" {
		if err := c.cloneWithFallback(title, targetName, "clone git repo"); err != nil {
			return fmt.Errorf("failed to clone git repo for %s -> %w", targetName, err)
		}
		return nil
	}

	// `git clone --branch` only accepts a branch or a tag, so probe both and fall
	// back to a full clone plus checkout when the ref turns out to be a commit.
	isBranch, err := CheckIfRemoteBranch(targetName, c.repoUrl, c.repoRef)
	if err != nil {
		return fmt.Errorf("failed to check if remote branch '%s' for '%s' -> %w", c.repoRef, targetName, err)
	}
	if isBranch {
		return c.cloneRef(title, targetName, "branch")
	}

	isTag, err := CheckIfRemoteTag(targetName, c.repoUrl, c.repoRef)
	if err != nil {
		return fmt.Errorf("failed to check if remote tag '%s' for '%s' -> %w", c.repoRef, targetName, err)
	}
	if isTag {
		return c.cloneRef(title, targetName, "tag")
	}

	return c.cloneCommit(title, targetName)
}

func (c *Clone) SetDepth(depth int) *Clone {
	c.depth = depth
	return c
}

func (c *Clone) IgnoreSubmodule(ignoreSubmodule bool) *Clone {
	c.ignoreSubmodule = ignoreSubmodule
	return c
}

func (c *Clone) IgnoreLFS(ignoreLFS bool) *Clone {
	c.ignoreLFS = ignoreLFS
	return c
}

func (c Clone) execGit(title, command string) error {
	executor := cmd.NewExecutor(title, command)
	if c.ignoreLFS {
		executor.SetEnv(gitLFSSkipSmudgeEnv, "1")
	}
	if fileio.PathExists(c.repoDir) {
		executor.SetWorkDir(c.repoDir)
	}
	return executor.Execute()
}

func (c Clone) cloneArgs(depth int) []string {
	args := []string{"clone"}
	if c.repoRef != "" {
		args = append(args, "--branch", c.repoRef)
	}
	if depth > 0 {
		args = append(args, "--single-branch", "--depth", fmt.Sprint(depth))
	}
	if !c.ignoreSubmodule {
		args = append(args, "--recurse-submodules")
	}
	args = append(args, c.repoUrl, c.repoDir)
	return args
}

func (c Clone) cloneWithRetry(title, object, action string, args []string) error {
	var lastErr error

	for attempt := 1; attempt <= retryMaxAttempts; attempt++ {
		if err := os.RemoveAll(c.repoDir); err != nil {
			return fmt.Errorf("failed to clean repo dir %s for %s -> %w", c.repoDir, object, err)
		}

		executor := cmd.NewExecutor(title, "git", args...)
		if c.ignoreLFS {
			executor.SetEnv(gitLFSSkipSmudgeEnv, "1")
		}
		err := executor.Execute()
		if err == nil {
			return nil
		}

		lastErr = err
		logger.Printf(logger.Warning, "Git %s failed (attempt %d/%d) for %s: %v\n", action, attempt, retryMaxAttempts, object, err)
		if attempt < retryMaxAttempts {
			retrySleep(attempt)
		}
	}

	return fmt.Errorf("git %s failed after %d attempts for %s -> %w", action, retryMaxAttempts, object, lastErr)
}

func (c Clone) cloneWithFallback(title, targetName, action string) error {
	if err := c.cloneWithRetry(title, targetName, action, c.cloneArgs(c.depth)); err != nil {
		if c.depth <= 0 {
			return err
		}

		logger.Printf(logger.Warning, "-- Git %s failed with shallow clone for %s, retrying without --depth\n", action, targetName)
		if fallbackErr := c.cloneWithRetry(title, targetName, action+" without depth", c.cloneArgs(0)); fallbackErr == nil {
			return nil
		}

		return err
	}
	return nil
}

func (c Clone) cloneRef(title, targetName, kind string) error {
	if err := c.cloneWithFallback(title, targetName, "clone git "+kind); err != nil {
		return fmt.Errorf("failed to clone git %s '%s' for '%s' -> %w", kind, c.repoRef, targetName, err)
	}
	return nil
}

func (c Clone) cloneCommit(title, targetName string) error {
	if err := c.cloneWithRetry(title, targetName, "clone git repo", []string{"clone", c.repoUrl, c.repoDir}); err != nil {
		return fmt.Errorf("failed to clone with git repo %s for '%s' -> %w", c.repoUrl, targetName, err)
	}

	// Fetch all remote refs so the target commit object is available locally.
	fetchExecutor := cmd.NewExecutor(title+" (fetch)", "git", "fetch", "origin")
	if c.ignoreLFS {
		fetchExecutor.SetEnv(gitLFSSkipSmudgeEnv, "1")
	}
	fetchExecutor.SetWorkDir(c.repoDir)
	if output, err := fetchExecutor.ExecuteOutputLive(); err != nil {
		return fmt.Errorf("failed to fetch origin after clone for '%s' -> %s -> %w", targetName, output, err)
	}

	// Checkout the requested commit.
	command := fmt.Sprintf("git reset --hard %s", c.repoRef)
	if err := c.execGit(title+" (reset to commit)", command); err != nil {
		return fmt.Errorf("failed to reset --hard '%s' to commit '%s' -> %w", targetName, c.repoRef, err)
	}

	// This path clones without recursing into submodules, and the reset changed
	// which submodule revisions are needed, so update them last.
	if !c.ignoreSubmodule {
		if err := c.updateSubmodules(title, targetName); err != nil {
			return err
		}
	}

	return nil
}

func (c Clone) updateSubmodules(title, targetName string) error {
	if !fileio.PathExists(filepath.Join(c.repoDir, ".gitmodules")) {
		return nil
	}

	if err := c.execGit(title+" (clone submodule)", "git submodule update --init --recursive"); err != nil {
		return fmt.Errorf("failed to update submodules for '%s' -> %w", targetName, err)
	}

	return nil
}
