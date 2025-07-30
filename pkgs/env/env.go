package env

import (
	"os"
	"slices"
	"strings"
)

// JoinPaths it'll filter the paths that are already in the runtimePaths,
// and append them to the end of the finalPaths.
func JoinPaths(envkey string, paths ...string) string {
	if strings.TrimSpace(envkey) == "" {
		panic("envkey is empty when JoinPaths.")
	}
	separator := string(os.PathListSeparator)

	var finalPaths []string
	var currentPaths []string

	path := os.Getenv(envkey)
	if path != "" {
		currentPaths = strings.Split(path, separator)
	}

	// Filter paths that are not in runtimePaths.
	for _, path := range paths {
		if !slices.Contains(currentPaths, path) {
			finalPaths = append(finalPaths, path)
		}
	}

	// Merge runtimePaths to the end of finalPaths.
	if len(currentPaths) > 0 {
		finalPaths = append(finalPaths, currentPaths...)
	}
	return strings.Join(finalPaths, separator)
}

// JoinSpace Joins the paths with space.
func JoinSpace(paths ...string) string {
	filtered := make([]string, 0, len(paths))
	for _, s := range paths {
		if strings.TrimSpace(s) != "" {
			filtered = append(filtered, s)
		}
	}
	return strings.Join(filtered, " ")
}
