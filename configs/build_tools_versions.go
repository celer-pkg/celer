package configs

import (
	"celer/buildtools"
	"celer/pkgs/cmd"
	"celer/pkgs/errors"
	"celer/pkgs/fileio"
	"fmt"
	"strings"
)

var toolVersionArgs = map[string][]string{
	"meson":            {"--version"},
	"ninja":            {"--version"},
	"cmake":            {"--version"},
	"make":             {"--version"},
	"python3:meson":    {"-c", "from importlib.metadata import version; print(version('meson'))"},
	"python3:gyp-next": {"-c", "from importlib.metadata import version; print(version('gyp-next'))"},
}

func (p Port) GenBuildToolsVersions(tools []string) (string, error) {
	// Check if repo cloned or downloaded.
	if !fileio.PathExists(p.MatchedConfig.PortConfig.RepoDir) {
		return "", errors.ErrRepoNotExit
	}

	// Ensure tools are validated and their paths are set in PATH,
	if err := buildtools.CheckTools(p.ctx, tools...); err != nil {
		return "", fmt.Errorf("failed to check tools -> %w", err)
	}

	var buffer strings.Builder
	fmt.Fprintf(&buffer, "celer: %s", Version)

	for _, tool := range tools {
		toolName, _, _ := strings.Cut(tool, "@")
		args, ok := toolVersionArgs[toolName]
		if !ok {
			continue
		}

		// For python3:xxx tools, use the Python executable from virtual environment.
		cmdName, _, _ := strings.Cut(toolName, ":")
		if cmdName == "python3" && buildtools.Python3 != nil {
			cmdName = buildtools.Python3.Path
		}

		executor := cmd.NewExecutor("", cmdName, args...)
		out, err := executor.ExecuteOutput()
		if err != nil {
			return "", fmt.Errorf("failed to get tool version of %s", tool)
		}

		line := strings.TrimSpace(strings.Split(out, "\n")[0])
		if line != "" {
			if buffer.Len() > 0 {
				buffer.WriteString("\n")
			}
			buffer.WriteString(tool)
			buffer.WriteString(": ")
			buffer.WriteString(line)
		}
	}

	return buffer.String(), nil
}
