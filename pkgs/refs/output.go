package refs

import (
	"fmt"
	"os"
	"strings"
	"time"

	"github.com/celer-pkg/celer/pkgs/color"
)

// truncate limits a string to maxLen characters, appending "..." if truncated.
func truncate(s string, maxLen int) string {
	if len(s) <= maxLen {
		return s
	}
	return s[:maxLen-3] + "..."
}

// PrintResolvedRefs prints a formatted list of resolved refs to the terminal.
func PrintResolvedRefs(projectName string, results []ResolvedRef) {
	title := fmt.Sprintf("\n================================ Resolved all dependency refs for %s ================================", projectName)
	color.Println(color.Title, title)

	// Build rows first to calculate dynamic column widths.
	type row struct {
		name      string
		srcType   string
		url       string
		ref       string
		resolved  string
		isError   bool
		isVirtual bool
	}

	rows := make([]row, 0, len(results))
	for _, r := range results {
		resolved := "-"
		if r.ResolvedCommit != "" {
			resolved = r.ResolvedCommit
		}
		rows = append(rows, row{
			name:      r.NameVersion,
			srcType:   string(r.SourceType),
			url:       r.Url,
			ref:       r.OriginalRef,
			resolved:  resolved,
			isError:   r.Error != "",
			isVirtual: r.SourceType == SourceVirtual,
		})
	}

	// Calculate max width per column.
	maxName := len("NAME@VERSION")
	maxType := len("TYPE")
	maxURL := len("URL")
	maxRef := len("REF")
	for _, r := range rows {
		if len(r.name) > maxName {
			maxName = len(r.name)
		}
		if len(r.srcType) > maxType {
			maxType = len(r.srcType)
		}
		if len(r.url) > maxURL {
			maxURL = len(r.url)
		}
		if len(r.ref) > maxRef {
			maxRef = len(r.ref)
		}
	}

	// Cap URL column to avoid excessively wide tables.
	if maxURL > 55 {
		maxURL = 55
	}

	format := fmt.Sprintf("  %%-%ds  %%-%ds  %%-%ds  %%-%ds  %%s\n", maxName, maxType, maxURL, maxRef)

	for _, row := range rows {
		url := truncate(row.url, maxURL)
		switch {
		case row.isError:
			color.Printf(color.Error, format, row.name, row.srcType, url, row.ref, "error: "+row.resolved)
		case row.isVirtual:
			color.Printf(color.Muted, format, row.name, row.srcType, url, row.ref, row.resolved)
		default:
			fmt.Printf(format, row.name, row.srcType, url, row.ref, row.resolved)
		}
	}

	fmt.Println()
}

// SaveResolvedRefs writes resolved refs to a markdown file.
func SaveResolvedRefs(projectName string, results []ResolvedRef, filePath string) error {
	var buffer strings.Builder

	fmt.Fprintf(&buffer, "# Resolved Refs for %s\n\n", projectName)
	fmt.Fprintf(&buffer, "Generated at: %s\n\n", time.Now().Format("2006-01-02 15:04:05"))

	buffer.WriteString("| Name@Version | Type | URL | Ref | Resolved |\n")
	buffer.WriteString("|---|---|---|---|---|\n")

	for _, r := range results {
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

	return os.WriteFile(filePath, []byte(buffer.String()), 0644)
}
