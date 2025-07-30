package buildsystems

import (
	"celer/buildtools"
	"celer/pkgs/cmd"
	"celer/pkgs/expr"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"slices"
	"strings"
)

func NewQMake(config *BuildConfig) *qmake {
	return &qmake{BuildConfig: config}
}

type qmake struct {
	*BuildConfig
}

func (qmake) Name() string {
	return "qmake"
}

func (m *qmake) CheckTools() error {
	m.BuildConfig.BuildTools = append(m.BuildConfig.BuildTools, "git", "cmake")
	return buildtools.CheckTools(m.BuildConfig.BuildTools...)
}

func (q qmake) CleanRepo() error {
	// We do not configure qmake project in source folder.
	return nil
}

func (m qmake) preConfigure() error {
	// Execute pre configure scripts.
	for _, script := range m.PreConfigure {
		script = strings.TrimSpace(script)
		if script == "" {
			continue
		}

		title := fmt.Sprintf("[post confiure %s]", m.PortConfig.nameVersionDesc())
		script = m.replaceHolders(script)
		executor := cmd.NewExecutor(title, script)
		if err := executor.Execute(); err != nil {
			return err
		}
	}

	return nil
}

func (m qmake) configureOptions() ([]string, error) {
	var options = slices.Clone(m.Options)

	// Remove common cross compile args for native build.
	if m.PortConfig.CrossTools.Native || m.BuildConfig.DevDep {
		options = slices.DeleteFunc(options, func(element string) bool {
			return strings.Contains(element, "-sysroot=")
		})
	}

	// Set build library type.
	libraryType := m.libraryType("-shared", "-static")
	switch m.BuildConfig.LibraryType {
	case "shared", "": // default is `shared`.
		options = append(options, libraryType.enableShared)
		if libraryType.disableStatic != "" {
			options = append(options, libraryType.disableStatic)
		}
	case "static":
		options = append(options, libraryType.enableStatic)
		if libraryType.disableShared != "" {
			options = append(options, libraryType.disableShared)
		}
	}

	options = append(options, fmt.Sprintf("--prefix=%s", m.PortConfig.PackageDir))

	// Replace placeholders.
	for index, value := range options {
		options[index] = m.replaceHolders(value)
	}

	return options, nil
}

func (m qmake) configured() bool {
	makeFile := filepath.Join(m.PortConfig.BuildDir, "Makefile")
	return fileio.PathExists(makeFile)
}

func (m qmake) Configure(options []string) error {
	// In windows, we set msvc related environments.
	if m.DevDep && m.PortConfig.CrossTools.Name != "msvc" {
		m.PortConfig.CrossTools.ClearEnvs()
	} else {
		m.PortConfig.CrossTools.SetEnvs(m.BuildConfig)
	}

	// Different Makefile projects set the build_type in inconsistent ways,
	// Fortunately, it can be configured through CFLAGS and CXXFLAGS.
	m.setBuildType(m.BuildType)

	// Create build dir if not exists.
	if !m.BuildInSource {
		if err := os.MkdirAll(m.PortConfig.BuildDir, os.ModePerm); err != nil {
			return err
		}
	}

	// Asssemble configure command.
	joinedOptions := strings.Join(options, " ")
	command := fmt.Sprintf("%s/configure %s", m.PortConfig.SrcDir, joinedOptions)
	title := fmt.Sprintf("[configure %s]", m.PortConfig.nameVersionDesc())
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(m.getLogPath("configure"))
	executor.SetWorkDir(expr.If(m.BuildInSource, m.PortConfig.SrcDir, m.PortConfig.BuildDir))
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (m qmake) buildOptions() ([]string, error) {
	return nil, nil
}

func (m qmake) Build(options []string) error {
	// Assemble command.
	command := fmt.Sprintf("make -j %d", m.PortConfig.JobNum)

	// Execute build.
	title := fmt.Sprintf("[build %s]", m.PortConfig.nameVersionDesc())
	executor := cmd.NewExecutor(title, command)
	executor.SetLogPath(m.getLogPath("build"))
	executor.SetWorkDir(m.PortConfig.BuildDir)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}

func (m qmake) Install(options []string) error {
	// Execute install.
	title := fmt.Sprintf("[install %s]", m.PortConfig.nameVersionDesc())
	executor := cmd.NewExecutor(title, "make install")
	executor.SetLogPath(m.getLogPath("install"))
	executor.SetWorkDir(m.PortConfig.BuildDir)
	if err := executor.Execute(); err != nil {
		return err
	}

	return nil
}
