package configs

import (
	"bytes"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"html"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"time"

	"github.com/yuin/goldmark"
	"github.com/yuin/goldmark/extension"
)

type installReportEntry struct {
	Port          string
	Parent        string
	BuildSystem   string
	InstalledFrom string
	DevDep        bool
	HostDev       bool
}

type installReport struct {
	rootPort string
	entries  map[string]installReportEntry
}

func (i installReport) dependencyTypeOf(entry installReportEntry) string {
	switch {
	case entry.DevDep && entry.HostDev:
		return "buildtime - host"
	case entry.DevDep:
		return "buildtime"
	case entry.HostDev:
		return "buildtime - host"
	default:
		return "runtime"
	}
}

func (i installReport) dependencyTypeRank(depType string) int {
	switch depType {
	case "runtime":
		return 0
	case "buildtime":
		return 1
	case "buildtime - host":
		return 2
	default:
		return 99
	}
}

const reportHTMLStyle = `<style>
:root {
  --bg: #f7f9fc;
  --text: #1f2937;
  --muted: #4b5563;
  --card: #ffffff;
  --line: #d9e1ec;
  --head: #eef3fa;
  --code-bg: #f3f6fb;
}
* { box-sizing: border-box; }
body {
  margin: 0;
  padding: 28px;
  background: var(--bg);
  color: var(--text);
  font-family: "Segoe UI", Tahoma, sans-serif;
  line-height: 1.55;
}
h1, h2 { margin: 0 0 12px; }
h1 { font-size: 26px; }
h2 { margin-top: 24px; font-size: 18px; }
p { margin: 10px 0; color: var(--muted); }
ul { margin-top: 6px; }
hr { border: 0; border-top: 1px solid var(--line); margin: 18px 0; }
table {
  width: 100%;
  border-collapse: collapse;
  background: var(--card);
  border: 1px solid var(--line);
}
th, td {
  border: 1px solid var(--line);
  padding: 8px 10px;
  text-align: left;
  vertical-align: top;
}
th {
  background: var(--head);
  font-weight: 600;
}
tr:nth-child(even) td { background: #fbfdff; }
code {
  background: var(--code-bg);
  padding: 1px 5px;
  border-radius: 4px;
}
</style>`

func newInstallReport(rootNameVersion string) *installReport {
	return &installReport{
		rootPort: rootNameVersion,
		entries:  make(map[string]installReportEntry),
	}
}

func (i *installReport) add(port *Port, installedFrom string) {
	if i == nil || port == nil || port.Name == "" || port.Version == "" {
		return
	}

	key := port.NameVersion() + "|" + fmt.Sprintf("%t|%t", port.DevDep, port.HostDep)
	entry := installReportEntry{
		Port:          port.NameVersion(),
		Parent:        port.Parent,
		BuildSystem:   port.MatchedConfig.BuildSystem,
		InstalledFrom: installedFrom,
		DevDep:        port.DevDep,
		HostDev:       port.HostDep,
	}

	old, ok := i.entries[key]
	if !ok {
		i.entries[key] = entry
		return
	}

	// Prefer richer information over "preinstalled" records.
	if old.InstalledFrom == "preinstalled" && entry.InstalledFrom != "preinstalled" {
		i.entries[key] = entry
	}
}

func (i *installReport) renderMarkdown(p *Port) string {
	normalize := func(value string) string {
		if strings.TrimSpace(value) == "" {
			return "-"
		}
		return value
	}
	percent := func(part, total int) string {
		if total <= 0 {
			return "0.0%"
		}
		return fmt.Sprintf("%.1f%%", float64(part)*100.0/float64(total))
	}

	var (
		lines             []string
		buildtimeCount    int
		runtimeCount      int
		preinstalledCount int
	)
	installedFromCount := make(map[string]int)

	orderedEntries := i.orderedEntries()
	for _, entry := range orderedEntries {
		if entry.DevDep || entry.HostDev {
			buildtimeCount++
		} else {
			runtimeCount++
		}

		if entry.InstalledFrom == "preinstalled" {
			preinstalledCount++
		}

		installedFrom := normalize(entry.InstalledFrom)
		installedFromCount[installedFrom]++
	}
	freshInstallCount := len(orderedEntries) - preinstalledCount

	lines = append(lines, "# Install Report")
	lines = append(lines, "")
	lines = append(lines, fmt.Sprintf("Generated at `%s`", time.Now().Local().Format("2006-01-02 15:04:05")))
	lines = append(lines, "")
	lines = append(lines, "## Overview")
	lines = append(lines, "")
	lines = append(lines, fmt.Sprintf("- Total packages: `%d`", len(orderedEntries)))
	lines = append(lines, fmt.Sprintf("- Fresh installed: `%d`", freshInstallCount))
	lines = append(lines, fmt.Sprintf("- Preinstalled: `%d`", preinstalledCount))
	lines = append(lines, fmt.Sprintf("- Buildtime dependencies: `%d`", buildtimeCount))
	lines = append(lines, fmt.Sprintf("- Runtime dependencies: `%d`", runtimeCount))
	lines = append(lines, "")
	lines = append(lines, "### Installed From Distribution")
	lines = append(lines, "")
	lines = append(lines, "| Installed From | Count | Percentage |")
	lines = append(lines, "| --- | --- | --- |")

	type installedFromStat struct {
		name  string
		count int
	}
	stats := make([]installedFromStat, 0, len(installedFromCount))
	for name, count := range installedFromCount {
		stats = append(stats, installedFromStat{name: name, count: count})
	}
	sort.SliceStable(stats, func(a, b int) bool {
		if stats[a].count != stats[b].count {
			return stats[a].count > stats[b].count
		}
		return stats[a].name < stats[b].name
	})
	for _, stat := range stats {
		lines = append(lines, fmt.Sprintf("| %s | `%d` | `%s` |",
			stat.name,
			stat.count,
			percent(stat.count, len(orderedEntries)),
		))
	}

	lines = append(lines, "")
	lines = append(lines, "## Workspace Configuration")
	lines = append(lines, "")
	lines = append(lines, "| Name | Value |")
	lines = append(lines, "| --- | --- |")
	lines = append(lines, fmt.Sprintf("| Platform | `%s` |", normalize(p.ctx.Platform().GetName())))
	lines = append(lines, fmt.Sprintf("| Project | `%s` |", normalize(p.ctx.Project().GetName())))
	lines = append(lines, fmt.Sprintf("| Build Type | `%s` |", normalize(p.ctx.BuildType())))
	lines = append(lines, "")
	lines = append(lines, "## Dependencies List")
	lines = append(lines, "")
	lines = append(lines, "| Name Version | Parent | Dependency Type | Build System | Installed From |")
	lines = append(lines, "| --- | --- | --- | --- | --- |")

	for _, entry := range orderedEntries {
		lines = append(lines, fmt.Sprintf("| `%s` | `%s` | %s | `%s` | %s |",
			normalize(entry.Port),
			normalize(entry.Parent),
			i.dependencyTypeOf(entry),
			normalize(entry.BuildSystem),
			normalize(entry.InstalledFrom),
		))
	}

	return strings.Join(lines, "\n") + "\n"
}

func (i *installReport) orderedEntries() []installReportEntry {
	// Convert map to slice for deterministic ordering in report output.
	ordered := make([]installReportEntry, 0, len(i.entries))
	for _, entry := range i.entries {
		ordered = append(ordered, entry)
	}

	// Multi-key sort (high priority -> low priority):
	// 1) dependency type group: runtime, buildtime, native
	// 2) parent package
	// 3) package itself
	// 4) build system
	// 5) install source
	sort.SliceStable(ordered, func(first, second int) bool {
		left := ordered[first]
		right := ordered[second]

		// Keep type groups together and in configured rank order.
		leftRank := i.dependencyTypeRank(i.dependencyTypeOf(left))
		rightRank := i.dependencyTypeRank(i.dependencyTypeOf(right))
		if leftRank != rightRank {
			return leftRank < rightRank
		}

		// Normalize empty parent to "-" so root entries can be compared consistently.
		leftParent := left.Parent
		rightParent := right.Parent
		if strings.TrimSpace(leftParent) == "" {
			leftParent = "-"
		}
		if strings.TrimSpace(rightParent) == "" {
			rightParent = "-"
		}
		if leftParent != rightParent {
			return leftParent < rightParent
		}

		// Then order by package identity and remaining metadata for stable output.
		if left.Port != right.Port {
			return left.Port < right.Port
		}

		if left.BuildSystem != right.BuildSystem {
			return left.BuildSystem < right.BuildSystem
		}
		return left.InstalledFrom < right.InstalledFrom
	})

	return ordered
}

func (i *installReport) write(p *Port) (string, error) {
	if i == nil || p == nil {
		return "", nil
	}

	reportDir := filepath.Join(dirs.InstalledDir, "celer", "report")
	if err := fileio.MkdirAll(reportDir, os.ModePerm); err != nil {
		return "", err
	}

	fileBase := fmt.Sprintf("%s@%s@%s@%s",
		strings.ReplaceAll(i.rootPort, "@", "_"),
		p.ctx.Platform().GetName(),
		p.ctx.Project().GetName(),
		p.ctx.BuildType(),
	)

	mdPath := filepath.Join(reportDir, fileBase+".md")
	htmlPath := filepath.Join(reportDir, fileBase+".html")

	// Generate markdown report.
	markdown := i.renderMarkdown(p)
	if err := os.WriteFile(mdPath, []byte(markdown), os.ModePerm); err != nil {
		return "", err
	}
	defer os.Remove(mdPath)

	// Convert to html report.
	var htmlBuf bytes.Buffer
	md := goldmark.New(goldmark.WithExtensions(extension.Table))
	if err := md.Convert([]byte(markdown), &htmlBuf); err != nil {
		return "", err
	}

	page := fmt.Sprintf("<!doctype html><html><head><meta charset=\"utf-8\"><title>%s</title>%s</head><body>%s</body></html>",
		html.EscapeString("Install Report"),
		reportHTMLStyle,
		htmlBuf.String(),
	)
	if err := os.WriteFile(htmlPath, []byte(page), os.ModePerm); err != nil {
		return "", err
	}

	return htmlPath, nil
}
