package snapshot

import (
	"fmt"
	"os"
	"strings"
	"time"

	"github.com/celer-pkg/celer/pkgs/refs"
)

// BuildEnv holds the build environment metadata for a snapshot.
type BuildEnv struct {
	ExportedAt   time.Time
	CelerVersion string
	Platform     string
	Project      string
}

// SaveSnapshotMarkdown writes a snapshot markdown file.
func SaveSnapshotMarkdown(filePath string, env BuildEnv, resolvedRefs []refs.ResolvedRef) error {
	var buffer strings.Builder

	fmt.Fprintf(&buffer, "# Build snapshot\n\n")

	buffer.WriteString("## Build environment\n\n")
	fmt.Fprintf(&buffer, "- exported_at: %s\n", env.ExportedAt.Format(time.RFC3339Nano))
	fmt.Fprintf(&buffer, "- celer_version: %s\n", env.CelerVersion)
	fmt.Fprintf(&buffer, "- platform: %s\n", env.Platform)
	fmt.Fprintf(&buffer, "- project: %s\n\n", env.Project)

	buffer.WriteString("## Resolved commits\n\n")
	buffer.WriteString("| Name@Version | Type | URL | Ref | Resolved |\n")
	buffer.WriteString("|---|---|---|---|---|\n")

	for _, r := range resolvedRefs {
		resolved := "-"
		if r.ResolvedCommit != "" {
			resolved = r.ResolvedCommit
		}

		url := r.Url
		ref := r.OriginalRef

		line := fmt.Sprintf("| %s | %s | %s | %s | %s |",
			r.NameVersion, r.SourceType, url, ref, resolved)

		if r.Error != "" {
			line += " error: " + r.Error
		}

		buffer.WriteString(line)
		buffer.WriteString("\n")
	}

	return os.WriteFile(filePath, []byte(buffer.String()), os.ModePerm)
}
