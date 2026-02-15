package configs

import (
	"celer/pkgs/cmd"
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
	var buffer strings.Builder
	for _, tool := range tools {
		args, ok := toolVersionArgs[tool]
		if !ok {
			continue
		}

		cmdName, _, _ := strings.Cut(tool, ":")
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
