package git

import (
	"bufio"
	"celer/pkgs/cmd"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"strings"
)

// CloneRepo clone git repo.
func (g Git) CloneRepo(title, repoUrl, repoRef, repoDir string) error {
	// ============ Clone default branch ============
	if repoRef == "" {
		args := append(g.proxyArgs(), "clone", "--recursive", repoUrl, repoDir)
		return cmd.NewExecutor(title, "git", args...).Execute()
	}

	// ============ Clone specific branch ============
	isBranch, err := g.CheckIfRemoteBranch(repoUrl, repoRef)
	if err != nil {
		return fmt.Errorf("check if remote branch error: %w", err)
	}
	if isBranch {
		args := append(g.proxyArgs(), "clone", "--branch", repoRef, "--recursive", repoUrl, repoDir)
		return cmd.NewExecutor(title, "git", args...).Execute()
	}

	// ============ Clone specific tag ============
	isTag, err := g.CheckIfRemoteTag(repoUrl, repoRef)
	if err != nil {
		return fmt.Errorf("check if remote tag error: %w", err)
	}
	if isTag {
		cloneArgs := append(g.proxyArgs(), "clone", "--branch", repoRef, repoUrl, "--recursive", repoDir)
		return cmd.NewExecutor(title, "git", cloneArgs...).Execute()
	}
	// ============ Clone and checkout commit ============
	cloneArgs := append(g.proxyArgs(), "clone", repoUrl, repoDir)
	if err := cmd.NewExecutor(title, "git", cloneArgs...).Execute(); err != nil {
		return fmt.Errorf("clone git repo error: %w", err)
	}

	// Checkout repo to commit.
	resetArgs := append(g.proxyArgs(), "reset", "--hard", repoRef)
	executor := cmd.NewExecutor(title+" (reset to commit)", "git", resetArgs...)
	executor.SetWorkDir(repoDir)
	if err := executor.Execute(); err != nil {
		return fmt.Errorf("reset --hard error: %w", err)
	}

	// Update submodules.
	if pathExists(filepath.Join(repoDir, ".gitmodules")) {
		updateArgs := append(g.proxyArgs(), "submodule", "update", "--init", "--recursive")
		executor = cmd.NewExecutor(title+" (clone submodule)", "git", updateArgs...)
		executor.SetWorkDir(repoDir)
		if err := executor.Execute(); err != nil {
			return fmt.Errorf("update submodules error: %w", err)
		}
	}

	return nil
}

// UpdateRepo update git repo.
func (g Git) UpdateRepo(title, repoRef, repoDir string, force bool) error {
	// Check if repo is modified.
	modified, err := g.IsModified(repoDir)
	if err != nil {
		return err
	}
	if modified {
		if !force {
			return fmt.Errorf("repo file is modified, update is skipped ... ⭐⭐⭐ You can update forcibly with -f/--force ⭐⭐⭐")
		}
	}

	// Get default branch if repoRef is empty.
	if repoRef == "" {
		branch, err := g.DefaultBranch(repoDir)
		if err != nil {
			return err
		}
		repoRef = branch
	}

	// Update to branch.
	isBranch, err := g.CheckIfRemoteBranch(repoDir, repoRef)
	if err != nil {
		return err
	}
	if isBranch {
		if err := g.Execute(title, repoDir, "reset", "--hard"); err != nil {
			return err
		}
		if err := g.Execute(title, repoDir, "clean", "-xfd"); err != nil {
			return err
		}
		if err := g.Execute(title, repoDir, "fetch", "origin", repoRef); err != nil {
			return err
		}
		if err := g.Execute(title, repoDir, "checkout", "-B", repoRef, "origin/"+repoRef); err != nil {
			return err
		}
		if err := g.Execute(title, repoDir, "pull", "origin", repoRef); err != nil {
			return err
		}
	}

	// Update to tag.
	isTag, err := g.CheckIfRemoteTag(repoDir, repoRef)
	if err != nil {
		return err
	}
	if isTag {
		if err := g.Execute(title, repoDir, "reset", "--hard"); err != nil {
			return err
		}
		if err := g.Execute(title, repoDir, "clean", "-xfd"); err != nil {
			return err
		}
		if err := g.Execute(title, repoDir, "tag", "-d", repoRef, "||", "true"); err != nil {
			return err
		}
		if err := g.Execute(title, repoDir, "fetch", "--tags", "origin"); err != nil {
			return err
		}
		if err := g.Execute(title, repoDir, "checkout", repoRef); err != nil {
			return err
		}
	}

	return fmt.Errorf("invalid repoRef: %s", repoRef)
}

// CherryPick cherry-pick patches.
func (g Git) CherryPick(title, srcDir string, patches []string) error {
	// Change to source dir to execute git command.
	if err := os.Chdir(srcDir); err != nil {
		return err
	}

	// Execute patch command.
	if err := g.Execute(title, srcDir, "fetch"); err != nil {
		return err
	}

	for _, patch := range patches {
		if err := g.Execute(title, srcDir, "cherry-pick", patch); err != nil {
			return err
		}
	}

	return nil
}

// Rebase rebase patches.
func (g Git) Rebase(title, repoRef, srcDir string, rebaseRefs []string) error {
	// Change to source dir to execute git command.
	if err := os.Chdir(srcDir); err != nil {
		return err
	}

	if err := g.Execute(title, srcDir, "fetch"); err != nil {
		return err
	}

	for _, rebaseRef := range rebaseRefs {
		if err := g.Execute(title, srcDir, "checkout", rebaseRef); err != nil {
			return err
		}

		if err := g.Execute(title, srcDir, "rebase", repoRef); err != nil {
			return err
		}
	}

	return nil
}

// Clean clean git repo.
func (g Git) Clean(title, repoDir string) error {
	if err := g.Execute(title, repoDir, "reset", "--hard"); err != nil {
		return err
	}
	if err := g.Execute(title, repoDir, "clean", "-xfd"); err != nil {
		return err
	}

	return nil
}

// ApplyPatch apply git patch.
func (g Git) ApplyPatch(port, repoDir, patchFile string) error {
	patchFileName := filepath.Base(patchFile)

	// Check if patched already.
	recordFilePath := filepath.Join(repoDir, ".patched")
	if pathExists(recordFilePath) {
		bytes, err := os.ReadFile(recordFilePath)
		if err != nil {
			return fmt.Errorf("cannot read .patched: %w", err)
		}

		lines := strings.Split(string(bytes), "\n")
		if slices.Contains(lines, patchFileName) {
			return nil
		}
	}

	// Read the first few lines of the file to check for Git patch features.
	file, err := os.Open(patchFile)
	if err != nil {
		return err
	}
	defer file.Close()

	var gitBatch bool
	scanner := bufio.NewScanner(file)
	for range 10 {
		if !scanner.Scan() {
			break
		}
		line := scanner.Text()

		// If you find Git patch features such as "From " or "Subject: "
		if strings.HasPrefix(line, "diff --git ") {
			gitBatch = true
			break
		}
	}

	if gitBatch {
		title := fmt.Sprintf("[patch %s]", port)
		args := []string{"apply", "--ignore-space-change", "--ignore-whitespace", "-v", patchFile}
		executor := cmd.NewExecutor(title, "git", args...)
		executor.SetWorkDir(repoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	} else {
		// Others, assume it's a regular patch file.
		title := fmt.Sprintf("[patch %s]", port)
		args := []string{"-Np1", "-i", patchFile}
		executor := cmd.NewExecutor(title, "patch", args...)
		executor.SetWorkDir(repoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	// Create a flag file to indicated that patch already applied.
	recordFile, err := os.OpenFile(recordFilePath, os.O_CREATE|os.O_RDWR|os.O_APPEND, os.ModePerm)
	if err != nil {
		return fmt.Errorf("cannot open or create .patched: %w", err)
	}
	defer recordFile.Close()

	if _, err := recordFile.WriteString(patchFileName + "\n"); err != nil {
		return fmt.Errorf("cannot write %s into .patched: %w", patchFileName, err)
	}
	return nil
}

func pathExists(path string) bool {
	_, err := os.Stat(path)
	if err == nil {
		return true
	}
	return !os.IsNotExist(err)
}
