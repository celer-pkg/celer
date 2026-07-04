package git

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/cmd"
	"github.com/celer-pkg/celer/pkgs/color"
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
	lines, err := statusLines(repoDir)
	if err != nil {
		return false, err
	}
	return len(lines) > 0, nil
}

// CleanRepo clean local changes of a repo to HEAD.
func CleanRepo(repoDir string) error {
	// git clean
	cmd1 := exec.Command("git", "-C", repoDir, "clean", "-ffdx")
	output1, err := cmd1.CombinedOutput()
	if err != nil {
		return fmt.Errorf("git clean failed: %s", output1)
	}

	// git reset
	cmd2 := exec.Command("git", "-C", repoDir, "reset", "--hard")
	output2, err := cmd2.CombinedOutput()
	if err != nil {
		return fmt.Errorf("git reset --hard failed: %s", output2)
	}

	return nil
}

// StatusSummary returns a concise git-status summary for diagnostics.
func StatusSummary(repoDir string, maxEntries int) (string, error) {
	lines, err := statusLines(repoDir)
	if err != nil {
		return "", err
	}
	if len(lines) == 0 {
		return "", nil
	}

	if maxEntries <= 0 || maxEntries > len(lines) {
		maxEntries = len(lines)
	}

	summary := strings.Join(lines[:maxEntries], "; ")
	if len(lines) > maxEntries {
		summary += "; ..."
	}
	return summary, nil
}

func statusLines(repoDir string) ([]string, error) {
	cmd := exec.Command("git", "-C", repoDir, "status", "--porcelain")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return nil, fmt.Errorf("check if repo is modified -> %s", output)
	}

	var lines []string
	for line := range strings.SplitSeq(strings.TrimSpace(string(output)), "\n") {
		line = strings.TrimSpace(line)
		if line != "" {
			if shouldIgnoreStatusLine(repoDir, line) {
				continue
			}
			lines = append(lines, line)
		}
	}
	return lines, nil
}

func shouldIgnoreStatusLine(repoDir, line string) bool {
	if !strings.HasPrefix(line, "?? ") {
		return false
	}

	path := strings.TrimSpace(line[3:])
	path = strings.Trim(path, "\"")
	path = strings.TrimSuffix(filepath.ToSlash(path), "/")
	if !strings.HasPrefix(path, "subprojects/") {
		return false
	}

	base := filepath.Base(path)
	if base == ".wraplock" || strings.HasSuffix(base, ".wrap") {
		return true
	}

	absPath := filepath.Join(repoDir, filepath.FromSlash(path))
	if pathExists(filepath.Join(absPath, ".git")) || pathExists(filepath.Join(absPath, ".meson-subproject-wrap-hash.txt")) {
		return true
	}

	if pathExists(filepath.Join(repoDir, "subprojects", base+".wrap")) {
		return true
	}

	return false
}

// GetCommitHash read git commit hash.
func GetCommitHash(repoDir string) (string, error) {
	// Check if repo exists.
	if _, err := os.Stat(repoDir); err != nil {
		return "", fmt.Errorf("directory error -> %w", err)
	}

	cmd := exec.Command("git", "-C", repoDir, "rev-parse", "HEAD")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("read git commit hash: %s", output)
	}

	return strings.TrimSpace(string(output)), nil
}

// GetDefaultBranch read git default branch.
func GetDefaultBranch(nameVersion, repoDir string) (string, error) {
	title := fmt.Sprintf("[read default branch: %s]", nameVersion)
	executor := cmd.NewExecutor(title, "git", "remote", "show", "origin")
	executor.SetWorkDir(repoDir)
	output, err := executor.ExecuteOutput()
	if err != nil {
		return "", fmt.Errorf("read git default branch -> %w", err)
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

// shortHash returns a concise git hash format: first 7 chars...last 7 chars
func shortHash(hash string) string {
	if len(hash) <= 14 {
		return hash
	}
	return hash[:7] + "..." + hash[len(hash)-7:]
}

// IsFullCommitHash checks if a string looks like a full git commit hash (40 hex chars)
func IsFullCommitHash(ref string) bool {
	if len(ref) != 40 {
		return false
	}

	for _, b := range []byte(ref) {
		if !((b >= '0' && b <= '9') || (b >= 'a' && b <= 'f')) {
			return false
		}
	}

	return true
}

// CheckIfRefMatches checks whether the local checkout matches the expected ref.
// It returns an empty string on match, or a human-readable mismatch reason when
// the checkout does not match. If expectedRef is empty, it falls back to
// comparing the current branch HEAD with its upstream branch.
func CheckIfRefMatches(ctx context.Context, nameVersion, repoDir, expectedRef string) (string, error) {
	currentCommit, err := GetCommitHash(repoDir)
	if err != nil {
		return "", err
	}

	// Fast path: if expectedRef is already a commit hash, do local comparison without git fetch
	expectedRef = strings.TrimSpace(expectedRef)
	if IsFullCommitHash(expectedRef) {
		if currentCommit != expectedRef {
			return fmt.Sprintf("hash mismatch (local:%s vs expected:%s)", shortHash(currentCommit), shortHash(expectedRef)), nil
		}
		return "", nil // Match found, no need for git fetch.
	}

	if expectedRef != "" {
		expectedCommit, parseErr := RevParseRepoRef(ctx, nameVersion, repoDir, expectedRef)
		if parseErr != nil {
			return "", parseErr
		}
		if currentCommit == expectedCommit {
			return "", nil
		}
		return fmt.Sprintf("hash mismatch (remote:%s vs local:%s)", shortHash(expectedCommit), shortHash(currentCommit)), nil
	}

	// No configured ref means "is my current branch still aligned with its upstream?".
	branch, err := GetCurrentBranch(repoDir)
	if err != nil {
		return "", err
	}
	if branch == "" || branch == "HEAD" {
		return "", fmt.Errorf("expect upstream, got detached HEAD @ %s", currentCommit)
	}

	upstreamBranch, err := getUpstreamBranch(repoDir)
	if err != nil {
		return "", fmt.Errorf("expect upstream for %q, got none (HEAD @ %s)", branch, currentCommit)
	}

	if !ctx.Offline() {
		// Upstream branches are reported as <remote>/<branch>, so peel out the remote
		// name and branch name before fetching the latest remote state.
		remoteName := upstreamBranch
		branchName := upstreamBranch
		if index := strings.IndexByte(upstreamBranch, '/'); index > 0 {
			remoteName = upstreamBranch[:index]
			branchName = upstreamBranch[index+1:]
		}

		if err := fetchRemoteRef(nameVersion, repoDir, remoteName, branchName); err != nil {
			return "", fmt.Errorf("git fetch branch %s failed for %s -> %w", branchName, repoDir, err)
		}
	}

	upstreamCommit, err := revParseCommit(repoDir, upstreamBranch)
	if err != nil {
		return "", err
	}
	if currentCommit == upstreamCommit {
		return "", nil
	}
	return fmt.Sprintf("hash mismatch on %q (remote:%s vs local:%s)", upstreamBranch, shortHash(upstreamCommit), shortHash(currentCommit)), nil
}

func getRemoteNames(repoDir string) ([]string, error) {
	cmd := exec.Command("git", "-C", repoDir, "remote")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return nil, fmt.Errorf("git remote failed -> %w", err)
	}

	var remotes []string
	for line := range strings.SplitSeq(strings.TrimSpace(string(output)), "\n") {
		line = strings.TrimSpace(line)
		if line != "" {
			remotes = append(remotes, line)
		}
	}
	return remotes, nil
}

func getPrimaryRemote(repoDir string) (string, error) {
	remotes, err := getRemoteNames(repoDir)
	if err != nil {
		return "", err
	}
	if len(remotes) == 0 {
		return "", nil
	}

	for _, remote := range remotes {
		if remote == "origin" {
			return remote, nil
		}
	}
	return remotes[0], nil
}

func getUpstreamBranch(repoDir string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "rev-parse", "--abbrev-ref", "--symbolic-full-name", "@{upstream}")
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("get upstream branch -> %s", output)
	}
	return strings.TrimSpace(string(output)), nil
}

// InitAsLocalRepo init folder as a local repo.
func InitAsLocalRepo(repoDir, message string) error {
	// Check if repo directory exists
	if _, err := os.Stat(repoDir); err != nil {
		return fmt.Errorf("directory error -> %w", err)
	}

	// Set up environment variables for git commits
	gitEnv := append(os.Environ(),
		"GIT_AUTHOR_NAME=CI Robot",
		"GIT_AUTHOR_EMAIL=ci@celer.com",
		"GIT_COMMITTER_NAME=CI Robot",
		"GIT_COMMITTER_EMAIL=ci@celer.com",
	)

	// git init
	color.Printf(color.Hint, "- git -C %s init", repoDir)
	cmd := exec.Command("git", "-C", repoDir, "init")
	cmd.Env = gitEnv
	if output, err := cmd.CombinedOutput(); err != nil {
		return fmt.Errorf("failed to git init repo: %s -> %w", output, err)
	}
	color.PrintInline(color.Hint, "✔ git -C %s init\n", repoDir)

	// git add
	color.Printf(color.Hint, "- git -C %s add -A", repoDir)
	cmd = exec.Command("git", "-C", repoDir, "add", "-A")
	cmd.Env = gitEnv
	if output, err := cmd.CombinedOutput(); err != nil {
		return fmt.Errorf("failed to git add -A: %w (output: %s)", err, output)
	}
	color.PrintInline(color.Hint, "✔ git -C %s add -A\n", repoDir)

	// git commit
	color.Printf(color.Hint, "- git -C %s commit -m %s", repoDir, message)
	cmd = exec.Command("git", "-C", repoDir, "commit", "-m", message)
	cmd.Env = gitEnv
	if output, err := cmd.CombinedOutput(); err != nil {
		return fmt.Errorf("failed to git commit: %w (output: %s)", err, output)
	}
	color.PrintInline(color.Hint, "✔ git -C %s commit -m %s\n", repoDir, message)

	return nil
}

// RevParseRepoRef return full commit hash with repo ref, if repo ref is not found in remote,
// then find it in local repo, the ref can be any valid git revision (branch, tag, HEAD, commit hash, etc.).
func RevParseRepoRef(ctx context.Context, nameVersion, repoDir, repoRef string) (string, error) {
	repoRef = strings.TrimSpace(repoRef)
	if repoRef == "" {
		return "", nil
	}

	// Prefer remote branch heads when the expected ref names a branch.
	if !ctx.Offline() {
		remoteName, err := getPrimaryRemote(repoDir)
		if err != nil {
			return "", err
		}
		if remoteName != "" {
			// Fetch the specific remote ref to ensure we can resolve it.
			if err := fetchRemoteRef(nameVersion, repoDir, remoteName, repoRef); err != nil {
				return "", err
			}
			if remoteCommit, err := revParseCommit(repoDir, remoteName+"/"+repoRef); err == nil {
				return remoteCommit, nil
			}
		}
	}

	// Fall back to any locally resolvable ref: commit hash, tag, branch, etc.
	return revParseCommit(repoDir, repoRef)
}

// revParseCommit returns the full commit hash for the given repo ref.
func revParseCommit(repoDir, repoRef string) (string, error) {
	repoRef = strings.TrimSpace(repoRef)
	if repoRef == "" {
		return "", nil
	}
	// Force refs like annotated tags to resolve to the commit they point at.
	return revParse(repoDir, repoRef+"^{commit}")
}

// revParse returns the full commit hash for the given repo ref from local repo.
// The ref can be any valid git revision (branch, tag, HEAD, commit hash, etc.).
func revParse(repoDir, repoRef string) (string, error) {
	cmd := exec.Command("git", "-C", repoDir, "rev-parse", repoRef)
	output, err := cmd.CombinedOutput()
	if err != nil {
		return "", fmt.Errorf("failed to git rev-parse %q -> %s", repoRef, output)
	}
	return strings.TrimSpace(string(output)), nil
}

// fetchRemoteRef fetch a specific remote ref (branch, tag, or commit).
func fetchRemoteRef(nameVersion, repoDir, remoteName, refName string) error {
	title := fmt.Sprintf("[git fetch remote ref: %s]", nameVersion)
	executor := cmd.NewExecutor(title, "git", "fetch", remoteName, refName)
	executor.SetWorkDir(repoDir)
	if output, err := executor.ExecuteOutput(); err != nil {
		return fmt.Errorf("failed to git fetch remote ref %s for %s: %s -> %w", refName, nameVersion, output, err)
	}

	return nil
}
