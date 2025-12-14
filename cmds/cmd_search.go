package cmds

import (
	"celer/configs"
	"celer/pkgs/color"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"io/fs"
	"path/filepath"
	"strings"

	"github.com/spf13/cobra"
)

type searchCmd struct {
	celer *configs.Celer
}

func (s *searchCmd) Command(celer *configs.Celer) *cobra.Command {
	s.celer = celer
	command := &cobra.Command{
		Use:   "search",
		Short: "Search available ports from ports repository.",
		Long: `Search available ports from ports repository.

This command searches for ports by name and version pattern. It supports
wildcard matching for flexible searches.

Pattern matching rules:
  - Exact match:     zlib@1.3.1
  - Prefix match:    zlib*
  - Suffix match:    *@1.3.1
  - Contains match:  *lib*

Examples:
  celer search zlib@1.3.1    # Search for exact match
  celer search zlib*         # Search for all zlib versions
  celer search *@1.3.1       # Search for all ports with version 1.3.1
  celer search *ffmpeg*      # Search for all ports containing 'ffmpeg'`,
		Args: cobra.ExactArgs(1),
		Run: func(cmd *cobra.Command, args []string) {
			s.doSearch(args[0])
		},
		ValidArgsFunction: s.completion,
	}

	return command
}

func (s *searchCmd) doSearch(pattern string) {
	// Initialize celer configuration.
	if err := s.celer.Init(); err != nil {
		configs.PrintError(err, "Failed to initialize celer.")
		return
	}

	// Perform search.
	libraries, err := s.search(pattern)
	if err != nil {
		configs.PrintError(err, "Failed to search available ports.")
		return
	}

	// Display results.
	color.Printf(color.Title, "Search results\n")
	color.Printf(color.Line, "-----------------------------------\n")
	if len(libraries) > 0 {
		for _, lib := range libraries {
			color.Println(color.List, lib)
		}
		color.Printf(color.Line, "-----------------------------------\n")
		color.Printf(color.Bottom, "Total: %d port(s)\n", len(libraries))
	} else {
		color.Println(color.Error, "no matched port found.")
	}
}

func (s *searchCmd) search(pattern string) ([]string, error) {
	var results []string

	// Helper function to search in a directory.
	searchInDir := func(dir string) error {
		if !fileio.PathExists(dir) {
			return nil
		}

		return filepath.WalkDir(dir, func(path string, entity fs.DirEntry, err error) error {
			if err != nil {
				return err
			}

			if !entity.IsDir() && entity.Name() == "port.toml" {
				libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
				libVersion := filepath.Base(filepath.Dir(path))
				nameVersion := libName + "@" + libVersion

				switch {
				case !strings.Contains(pattern, "*"):
					if nameVersion == pattern {
						results = append(results, nameVersion)
					}

				case strings.HasPrefix(pattern, "*") && strings.Count(pattern, "*") == 1:
					if strings.HasSuffix(nameVersion, pattern[1:]) {
						results = append(results, nameVersion)
					}

				case strings.HasSuffix(pattern, "*") && strings.Count(pattern, "*") == 1:
					if strings.HasPrefix(nameVersion, pattern[:len(pattern)-1]) {
						results = append(results, nameVersion)
					}

				case strings.Count(pattern, "*") == 2 && strings.HasPrefix(pattern, "*") && strings.HasSuffix(pattern, "*"):
					content := pattern[1 : len(pattern)-1]
					if strings.Contains(nameVersion, content) {
						results = append(results, nameVersion)
					}

				default:
					return nil
				}
			}

			return nil
		})
	}

	// Search in global ports.
	if err := searchInDir(dirs.PortsDir); err != nil {
		return nil, err
	}

	// Search in project-specific ports.
	projectPortsDir := filepath.Join(dirs.ConfProjectsDir, s.celer.Project().GetName())
	if err := searchInDir(projectPortsDir); err != nil {
		return nil, err
	}

	return results, nil
}

func (s *searchCmd) completion(cmd *cobra.Command, args []string, toComplete string) ([]string, cobra.ShellCompDirective) {
	var suggestions []string

	// Completion from global ports.
	if fileio.PathExists(dirs.PortsDir) {
		filepath.WalkDir(dirs.PortsDir, func(path string, d fs.DirEntry, err error) error {
			if err != nil {
				return nil // Skip errors in completion.
			}

			if !d.IsDir() && d.Name() == "port.toml" {
				libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
				libVersion := filepath.Base(filepath.Dir(path))
				nameVersion := libName + "@" + libVersion

				if strings.HasPrefix(nameVersion, toComplete) {
					suggestions = append(suggestions, nameVersion)
				}
			}

			return nil
		})
	}

	// Completion from project-specific ports.
	projectPortsDir := filepath.Join(dirs.ConfProjectsDir, s.celer.Project().GetName())
	if fileio.PathExists(projectPortsDir) {
		filepath.WalkDir(projectPortsDir, func(path string, d fs.DirEntry, err error) error {
			if err != nil {
				return nil // Skip errors in completion.
			}

			if !d.IsDir() && d.Name() == "port.toml" {
				libName := filepath.Base(filepath.Dir(filepath.Dir(path)))
				libVersion := filepath.Base(filepath.Dir(path))
				nameVersion := libName + "@" + libVersion

				if strings.HasPrefix(nameVersion, toComplete) {
					suggestions = append(suggestions, nameVersion)
				}
			}

			return nil
		})
	}

	return suggestions, cobra.ShellCompDirectiveNoFileComp
}
