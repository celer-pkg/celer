package buildtools

import (
	"celer/context"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/env"
	"celer/pkgs/fileio"
	"embed"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"slices"
	"strings"

	"github.com/BurntSushi/toml"
)

var (
	//go:embed static/*
	static embed.FS
)

// CheckTools checks if tools exist and repair them if necessary.
func CheckTools(ctx context.Context, tools ...string) error {
	// Filter duplicated tools.
	var uniqueTools []string
	for _, tool := range tools {
		if !slices.Contains(uniqueTools, tool) {
			uniqueTools = append(uniqueTools, tool)
		}
	}

	// Determine current architecture.
	arch := runtime.GOARCH
	switch arch {
	case "amd64", "x86_64":
		arch = "x86_64"
	case "arm64":
		arch = "aarch64"
	}

	// Read and decode static file.
	staticFile := fmt.Sprintf("static/%s-%s.toml", arch, runtime.GOOS)
	bytes, err := static.ReadFile(staticFile)
	if err != nil {
		return err
	}
	var buildTools BuildTools
	if err := toml.Unmarshal(bytes, &buildTools); err != nil {
		return err
	}

	confToolsFile := filepath.Join(dirs.WorkspaceDir, "conf", "buildtools", arch+"-"+runtime.GOOS+".toml")
	if fileio.PathExists(confToolsFile) {
		bytes, err := os.ReadFile(confToolsFile)
		if err != nil {
			return err
		}
		var confBuildTools BuildTools
		if err := toml.Unmarshal(bytes, &confBuildTools); err != nil {
			return err
		}
		buildTools = buildTools.merge(confBuildTools)
	}

	// Check if need to install python3 and msys2.
	for _, tool := range uniqueTools {
		len := strings.Count(tool, ":")
		if len == 0 {
			continue
		}
		if len != 1 {
			return fmt.Errorf("invalid tool format: %s", tool)
		}

		if strings.HasPrefix(tool, "msys2:") && !slices.Contains(uniqueTools, "msys2") {
			uniqueTools = append(uniqueTools, "msys2")
		}
		if strings.HasPrefix(tool, "python3:") && !slices.Contains(uniqueTools, "python3") {
			uniqueTools = append(uniqueTools, "python3")
		}
	}

	var (
		msys2Tool   *BuildTool
		python3Tool *BuildTool
	)

	// Validate tools in loop.
	for _, tool := range uniqueTools {
		if tool := buildTools.findTool(ctx, tool); tool != nil {
			if err := tool.validate(); err != nil {
				return err
			}

			switch tool.Name {
			case "python3":
				python3Tool = tool

			case "msys2":
				msys2Tool = tool
			}
		}
	}

	// Keep tools that not managed by buildTools.
	uniqueTools = slices.DeleteFunc(uniqueTools, func(element string) bool {
		return buildTools.contains(element)
	})

	// Check if we need to install python3 packages.
	needPython3Packages := slices.ContainsFunc(uniqueTools, func(tool string) bool {
		return strings.HasPrefix(tool, "python3:")
	})

	// Install python3 packages.
	if needPython3Packages {
		if python3Tool != nil {
			setupPython3(python3Tool.rootDir)
		} else if runtime.GOOS == "linux" {
			setupPython3("/usr/bin")
		}
		if err := pipInstall(&uniqueTools); err != nil {
			return err
		}
	}

	// Setup msys2 for windows.
	if msys2Tool != nil && runtime.GOOS == "windows" {
		if err := SetupMSYS2(msys2Tool.rootDir, &uniqueTools); err != nil {
			return err
		}
	}

	// Check if package installed for linux.
	if runtime.GOOS == "linux" {
		if err := CheckSystemTools(uniqueTools); err != nil {
			return err
		}
	}

	return nil
}

type BuildTool struct {
	Name    string   `toml:"name"`
	Version string   `toml:"version"`
	Url     string   `toml:"url"`
	Archive string   `toml:"archive"`
	Paths   []string `toml:"paths"`

	// Internal fields.
	rootDir    string
	fullpaths  []string
	cmakepaths []string
	ctx        context.Context
}

func (b *BuildTool) validate() error {
	// Validate download url.
	if b.Url == "" {
		return fmt.Errorf("url of %s is empty", b.Name)
	}

	// Validate version.
	if b.Version == "" {
		return fmt.Errorf("version of %s is empty", b.Name)
	}

	// Validate url.
	if b.Url == "" {
		return fmt.Errorf("url of %s is empty", b.Name)
	}

	// Set rootDir and paths based on tool type.
	// Single-file tools (has archive, no paths) are placed in versioned subdirectories: {name}-{version}/
	// Archive tools (has paths) are extracted to subdirectories.
	if len(b.Paths) > 0 {
		// Archive with paths specified: extract to subdirectory
		folderName := strings.Split(b.Paths[0], "/")[0]
		b.rootDir = filepath.Join(dirs.DownloadedToolsDir, folderName)
		for _, path := range b.Paths {
			b.fullpaths = append(b.fullpaths, filepath.Join(dirs.DownloadedToolsDir, path))
			b.cmakepaths = append(b.cmakepaths, "${CELER_ROOT}/downloads/tools/"+filepath.ToSlash(path))
		}
	} else {
		// Single-file tool, place in subdirectory downloads/tools/{name}-{version}/
		toolDir := fmt.Sprintf("%s-%s", b.Name, b.Version)
		b.rootDir = filepath.Join(dirs.DownloadedToolsDir, toolDir)
		if !slices.Contains(b.fullpaths, b.rootDir) {
			b.fullpaths = append(b.fullpaths, b.rootDir)
		}
		cmakePath := fmt.Sprintf("${CELER_ROOT}/downloads/tools/%s", toolDir)
		if !slices.Contains(b.cmakepaths, cmakePath) {
			b.cmakepaths = append(b.cmakepaths, cmakePath)
		}
	}

	os.Setenv("PATH", env.JoinPaths("PATH", b.fullpaths...))

	// Check and fix tool.
	if err := b.checkAndFix(); err != nil {
		return err
	}

	return nil
}

func (b *BuildTool) checkAndFix() error {
	// Determine folder name and location based on tool type
	var folderName string
	var archiveName string
	var location string

	if len(b.Paths) > 0 {
		// Archive with paths: extract to subdirectory.
		folderName = strings.Split(b.Paths[0], "/")[0]
		location = filepath.Join(dirs.DownloadedToolsDir, b.Name)
		// Use archive name as download file name if specified.
		archiveName = filepath.Base(b.Url)
		if b.Archive != "" {
			archiveName = b.Archive
		}
	} else {
		// Single-file tool: place in subdirectory downloads/tools/{name}-{version}/
		folderName = fmt.Sprintf("%s-%s", b.Name, b.Version)
		location = filepath.Join(dirs.DownloadedToolsDir, folderName)
		// For single-file tools: download with original filename, but pass Archive for symlink creation
		archiveName = "" // Empty means use original URL filename for download
	}

	// Check and repair resource.
	// For single-file tools, use Archive as the archive name for target file naming
	if len(b.Paths) == 0 && b.Archive != "" {
		archiveName = b.Archive
	}
	repair := fileio.NewRepair(b.Url, archiveName, folderName, dirs.DownloadedToolsDir)
	if err := repair.CheckAndRepair(b.ctx); err != nil {
		return err
	}

	// Print download & extract info.
	color.Printf(color.List, "\n[✔] -- tool: %s\n", fileio.FileBaseName(b.Url))
	color.Printf(color.Hint, "Location: %s\n", location)

	return nil
}

type BuildTools struct {
	BuildTools []BuildTool `toml:"build_tools"`
}

func (b BuildTools) findTool(ctx context.Context, name string) *BuildTool {
	for index, tool := range b.BuildTools {
		if tool.Name == name {
			b.BuildTools[index].ctx = ctx
			return &b.BuildTools[index]
		}
	}
	return nil
}

func (b BuildTools) contains(name string) bool {
	for _, tool := range b.BuildTools {
		if tool.Name == name {
			return true
		}
	}
	return false
}

func (b BuildTools) merge(buildTools BuildTools) BuildTools {
	for index, tool := range buildTools.BuildTools {
		if !b.contains(tool.Name) {
			b.BuildTools = append(b.BuildTools, tool)
		} else {
			b.BuildTools[index] = tool
		}
	}

	return b
}
