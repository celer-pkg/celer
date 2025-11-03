package buildsystems

import (
	"celer/pkgs/cmd"
	"fmt"
	"runtime"
	"strings"
)

type eventHook interface {
	preConfigure() error
	postConfigure() error
	preBuild() error
	postBuild() error
	preInstall() error
	postInstall() error

	// Some third-parties need extra steps
	// to fix build, for example: nspr.
	fixBuild() error
}

func (b BuildConfig) preConfigure() error {
	for _, script := range b.PreConfigure {
		script = strings.TrimSpace(script)
		if script == "" {
			continue
		}

		title := fmt.Sprintf("[pre configure %s]", b.PortConfig.nameVersionDesc())
		script = b.replaceHolders(script)
		executor := cmd.NewExecutor(title, script)
		executor.SetWorkDir(b.PortConfig.RepoDir)
		executor.MSYS2Env(runtime.GOOS == "windows")
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

func (b BuildConfig) postConfigure() error {
	for _, script := range b.PostConfigure {
		script = strings.TrimSpace(script)
		if script == "" {
			continue
		}

		title := fmt.Sprintf("[post confiure %s]", b.PortConfig.nameVersionDesc())
		script = b.replaceHolders(script)
		executor := cmd.NewExecutor(title, script)
		executor.SetWorkDir(b.PortConfig.RepoDir)
		executor.MSYS2Env(runtime.GOOS == "windows")
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

func (b BuildConfig) preBuild() error {
	for _, script := range b.PreBuild {
		script = strings.TrimSpace(script)
		if script == "" {
			continue
		}

		title := fmt.Sprintf("[pre build %s]", b.PortConfig.nameVersionDesc())
		script = b.replaceHolders(script)
		executor := cmd.NewExecutor(title, script)
		executor.SetWorkDir(b.PortConfig.RepoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

func (b BuildConfig) postBuild() error {
	for _, script := range b.PostBuild {
		script = strings.TrimSpace(script)
		if script == "" {
			continue
		}

		title := fmt.Sprintf("[post build %s]", b.PortConfig.nameVersionDesc())
		script = b.replaceHolders(script)
		executor := cmd.NewExecutor(title, script)
		executor.SetWorkDir(b.PortConfig.RepoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

func (b BuildConfig) preInstall() error {
	for _, script := range b.PreInstall {
		script = strings.TrimSpace(script)
		if script == "" {
			continue
		}

		title := fmt.Sprintf("[pre install %s]", b.PortConfig.nameVersionDesc())
		script = b.replaceHolders(script)
		executor := cmd.NewExecutor(title, script)
		executor.SetWorkDir(b.PortConfig.RepoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

func (b BuildConfig) postInstall() error {
	for _, script := range b.PostInstall {
		script = strings.TrimSpace(script)
		if script == "" {
			continue
		}

		title := fmt.Sprintf("[post install %s]", b.PortConfig.nameVersionDesc())
		script = b.replaceHolders(script)
		executor := cmd.NewExecutor(title, script)
		executor.SetWorkDir(b.PortConfig.RepoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

func (b BuildConfig) fixBuild() error {
	for _, script := range b.FixBuild {
		script = strings.TrimSpace(script)
		if script == "" {
			continue
		}

		title := fmt.Sprintf("[fix build %s]", b.PortConfig.nameVersionDesc())
		script = b.replaceHolders(script)
		executor := cmd.NewExecutor(title, script)
		executor.SetWorkDir(b.PortConfig.RepoDir)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}
