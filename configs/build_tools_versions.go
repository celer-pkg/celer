package configs

import (
	"celer/pkgs/cmd"
	"strings"
)

// GenBuildToolsVersions returns version strings for build tools used by the given build system.
// Used in package cache meta so cache key changes when meson/cmake/ninja etc. are upgraded.
func (p Port) GenBuildToolsVersions(buildSystem string) (string, error) {
	type toolVersion struct {
		name string
		args []string
	}

	var toolVersions []toolVersion

	switch buildSystem {
	case "meson":
		toolVersions = []toolVersion{
			{"meson", []string{"--version"}},
			{"ninja", []string{"--version"}},
		}
	case "cmake":
		toolVersions = []toolVersion{
			{"cmake", []string{"--version"}},
			{"ninja", []string{"--version"}},
			{"make", []string{"--version"}},
		}
	case "makefiles":
		toolVersions = []toolVersion{
			{"make", []string{"--version"}},
		}
	case "b2":
		toolVersions = []toolVersion{
			{"b2", []string{"--version"}},
		}
	case "qmake":
		toolVersions = []toolVersion{
			{"qmake", []string{"-v"}},
			{"cmake", []string{"--version"}},
		}
	case "gyp":
		toolVersions = []toolVersion{
			{"python3", []string{"-c", "import gyp; print(gyp.__version__)"}},
		}
	default:
		return "", nil // nobuild, prebuilt, custom: no build tools
	}

	var buffer strings.Builder
	for _, version := range toolVersions {
		executor := cmd.NewExecutor("", version.name, version.args...)
		out, err := executor.ExecuteOutput()
		if err != nil {
			continue // Skip if tool not found (e.g. ninja when using Unix Makefiles)
		}

		line := strings.TrimSpace(strings.Split(out, "\n")[0])
		if line != "" {
			if buffer.Len() > 0 {
				buffer.WriteString("\n")
			}
			buffer.WriteString(version.name)
			buffer.WriteString(": ")
			buffer.WriteString(line)
		}
	}

	return buffer.String(), nil
}
