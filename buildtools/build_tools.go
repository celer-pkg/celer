package buildtools

import (
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

	// In offline mode, tools would not be downloaded.
	Offline bool
)

// CheckTools checks if tools exist and repair them if necessary.
func CheckTools(requiredTools ...string) error {
	tools := slices.Clone(requiredTools)

	// Read toml file.
	bytes, err := static.ReadFile(fmt.Sprintf("static/x86_64-%s.toml", runtime.GOOS))
	if err != nil {
		return err
	}

	// Decode toml file.
	var buildTools buildTools
	if err := toml.Unmarshal(bytes, &buildTools); err != nil {
		return err
	}

	// Validate tool also if tool's package is inside.
	for _, tool := range tools {
		if strings.Count(tool, ":") == 1 && strings.HasPrefix(tool, "msys2:") &&
			!slices.Contains(tools, "msys2") {
			tools = append(tools, "msys2")
		}
		if strings.Count(tool, ":") == 1 && strings.HasPrefix(tool, "python3:") &&
			!slices.Contains(tools, "python3") {
			tools = append(tools, "python3")
		}
	}

	var (
		msys2Tool       *buildTool
		python3Required bool
	)

	// Validate tools in loop.
	for _, tool := range tools {
		// Skip python3 now, since it's not portable tool, we will validate it later.
		if strings.HasPrefix(tool, "python3") {
			python3Required = true
			continue
		}

		// Find tool and validate it.
		if tool := buildTools.findTool(tool); tool != nil {
			if err := tool.validate(); err != nil {
				return err
			}

			if tool.Name == "msys2" {
				msys2Tool = tool
			}
		}
	}

	// Remove builtin tools from requiredTools.
	tools = slices.DeleteFunc(tools, func(element string) bool {
		return buildTools.contains(element)
	})

	// Validate python3 and install python3 libraries.
	if python3Required {
		if err := SetupPython3(&tools); err != nil {
			return err
		}
	}

	switch runtime.GOOS {
	case "windows":
		if msys2Tool != nil {
			if err := SetupMSYS2(msys2Tool.rootDir, &tools); err != nil {
				return err
			}
		}
	case "linux":
		if err := CheckSystemTools(tools); err != nil {
			return err
		}
	}

	return nil
}

type buildTool struct {
	Name    string   `toml:"name"`
	Version string   `toml:"version"`
	Url     string   `toml:"url"`
	Archive string   `toml:"archive"`
	Paths   []string `toml:"paths"`

	// Internal fields.
	rootDir    string
	fullpaths  []string
	cmakepaths []string
}

func (b *buildTool) validate() error {
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

	// Validate paths.
	if len(b.Paths) == 0 {
		return fmt.Errorf("path of %s is empty", b.Name)
	}

	// Set rootDir.
	folderName := strings.Split(b.Paths[0], "/")[0]
	b.rootDir = filepath.Join(dirs.DownloadedToolsDir, folderName)

	// Assemble fullpaths and cmakepaths.
	for _, path := range b.Paths {
		b.fullpaths = append(b.fullpaths, filepath.Join(dirs.DownloadedToolsDir, path))
		b.cmakepaths = append(b.cmakepaths, "${WORKSPACE_DIR}/downloads/tools/"+filepath.ToSlash(path))
	}
	os.Setenv("PATH", env.JoinPaths("PATH", b.fullpaths...))

	// Check and fix tool.
	if err := b.checkAndFix(); err != nil {
		return err
	}

	return nil
}

func (b *buildTool) checkAndFix() error {
	// Use archive name as download file name if specified.
	archiveName := filepath.Base(b.Url)
	if b.Archive != "" {
		archiveName = b.Archive
	}

	// Default folder name would be the first folder of path, it also can be specified by archiveName.
	folderName := strings.Split(b.Paths[0], "/")[0]

	// Check and repair resource.
	location := filepath.Join(dirs.DownloadedToolsDir, b.Name)
	repair := fileio.NewRepair(b.Url, archiveName, folderName, dirs.DownloadedToolsDir)

	if err := repair.CheckAndRepair(Offline); err != nil {
		return err
	}

	// Print download & extract info.
	title := color.Sprintf(color.Green, "\n[✔] ---- Tool: %s\n", fileio.FileBaseName(b.Url))
	fmt.Printf("%sLocation: %s\n", title, location)

	return nil
}

type buildTools struct {
	BuildTools []buildTool `toml:"build_tools"`
}

func (b buildTools) findTool(name string) *buildTool {
	for index, tool := range b.BuildTools {
		if tool.Name == name {
			return &b.BuildTools[index]
		}
	}
	return nil
}

func (b buildTools) contains(name string) bool {
	for _, tool := range b.BuildTools {
		if tool.Name == name {
			return true
		}
	}
	return false
}
