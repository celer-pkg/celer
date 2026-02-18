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
	NameVersion   string
	Parent        string
	BuildSystem   string
	InstalledFrom string
	DevDep        bool
	Native        bool
}

type installReport struct {
	rootNameVersion string
	entries         map[string]installReportEntry
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
		rootNameVersion: rootNameVersion,
		entries:         make(map[string]installReportEntry),
	}
}

func (i *installReport) add(port *Port, installedFrom string) {
	if i == nil || port == nil || port.Name == "" || port.Version == "" {
		return
	}

	key := port.NameVersion() + "|" + fmt.Sprintf("%t|%t", port.DevDep, port.Native)
	entry := installReportEntry{
		NameVersion:   port.NameVersion(),
		Parent:        port.Parent,
		BuildSystem:   port.MatchedConfig.BuildSystem,
		InstalledFrom: installedFrom,
		DevDep:        port.DevDep,
		Native:        port.Native,
	}

	old, ok := i.entries[key]
	if !ok {
		i.entries[key] = entry
		return
	}

	// Prefer richer information over "already installed" records.
	if old.InstalledFrom == "already installed" && entry.InstalledFrom != "already installed" {
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

	dependencyType := func(entry installReportEntry) string {
		if entry.DevDep && entry.Native {
			return "native"
		}
		if entry.DevDep {
			return "buildtime"
		}
		if entry.Native {
			return "native"
		}
		return "runtime"
	}

	var lines []string
	orderedEntries := i.orderedEntries()
	devDepCount := 0
	nativeCount := 0
	alreadyInstalledCount := 0
	for _, entry := range orderedEntries {
		if entry.DevDep {
			devDepCount++
		}
		if entry.Native {
			nativeCount++
		}
		if entry.InstalledFrom == "already installed" {
			alreadyInstalledCount++
		}
	}
	freshInstallCount := len(orderedEntries) - alreadyInstalledCount

	lines = append(lines, "# Install Report")
	lines = append(lines, "")
	lines = append(lines, fmt.Sprintf("Generated at `%s`.", time.Now().Local().Format("2006-01-02 15:04:05")))
	lines = append(lines, "")
	lines = append(lines, "## Overview")
	lines = append(lines, "")
	lines = append(lines, fmt.Sprintf("- Total packages: `%d`", len(orderedEntries)))
	lines = append(lines, fmt.Sprintf("- Fresh installs: `%d`", freshInstallCount))
	lines = append(lines, fmt.Sprintf("- Already installed: `%d`", alreadyInstalledCount))
	lines = append(lines, fmt.Sprintf("- Buildtime dependencies: `%d`", devDepCount))
	lines = append(lines, fmt.Sprintf("- Native dependencies: `%d`", nativeCount))
	lines = append(lines, "")
	lines = append(lines, "## Build Environment")
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
			normalize(entry.NameVersion),
			normalize(entry.Parent),
			dependencyType(entry),
			normalize(entry.BuildSystem),
			normalize(entry.InstalledFrom),
		))
	}

	return strings.Join(lines, "\n") + "\n"
}

func (i *installReport) orderedEntries() []installReportEntry {
	keys := make([]string, 0, len(i.entries))
	for key := range i.entries {
		keys = append(keys, key)
	}
	sort.Strings(keys)

	entryByKey := make(map[string]installReportEntry, len(keys))
	childrenByParent := make(map[string][]string)
	rootKeys := make([]string, 0, 2)
	for _, key := range keys {
		entry := i.entries[key]
		entryByKey[key] = entry
		childrenByParent[entry.Parent] = append(childrenByParent[entry.Parent], key)
		if entry.NameVersion == i.rootNameVersion && strings.TrimSpace(entry.Parent) == "" {
			rootKeys = append(rootKeys, key)
		}
	}
	for parent := range childrenByParent {
		sort.Strings(childrenByParent[parent])
	}
	sort.Strings(rootKeys)

	visited := make(map[string]bool, len(keys))
	var ordered []installReportEntry
	var walk func(key string)
	walk = func(key string) {
		if visited[key] {
			return
		}
		visited[key] = true

		entry, ok := entryByKey[key]
		if !ok {
			return
		}
		ordered = append(ordered, entry)

		for _, childKey := range childrenByParent[entry.NameVersion] {
			walk(childKey)
		}
	}

	// Root port first.
	for _, key := range rootKeys {
		walk(key)
	}

	// Then append any leftover entries deterministically.
	for _, key := range keys {
		walk(key)
	}

	return ordered
}

func (i *installReport) write(p *Port) (string, string, error) {
	if i == nil || p == nil {
		return "", "", nil
	}

	reportDir := filepath.Join(dirs.InstalledDir, "celer", "report")
	if err := fileio.MkdirAll(reportDir, os.ModePerm); err != nil {
		return "", "", err
	}

	timestamp := time.Now().Format("20060102-150405")
	fileBase := fmt.Sprintf("%s@%s@%s@%s-%s",
		strings.ReplaceAll(i.rootNameVersion, "@", "_"),
		p.ctx.Platform().GetName(),
		p.ctx.Project().GetName(),
		p.ctx.BuildType(),
		timestamp,
	)

	mdPath := filepath.Join(reportDir, fileBase+".md")
	htmlPath := filepath.Join(reportDir, fileBase+".html")

	markdown := i.renderMarkdown(p)
	if err := os.WriteFile(mdPath, []byte(markdown), os.ModePerm); err != nil {
		return "", "", err
	}

	var htmlBuf bytes.Buffer
	md := goldmark.New(
		goldmark.WithExtensions(
			extension.Table,
		),
	)
	if err := md.Convert([]byte(markdown), &htmlBuf); err != nil {
		return "", "", err
	}

	page := fmt.Sprintf("<!doctype html><html><head><meta charset=\"utf-8\"><title>%s</title>%s</head><body>%s</body></html>",
		html.EscapeString("Install Report"),
		reportHTMLStyle,
		htmlBuf.String(),
	)
	if err := os.WriteFile(htmlPath, []byte(page), os.ModePerm); err != nil {
		return "", "", err
	}

	return mdPath, htmlPath, nil
}
