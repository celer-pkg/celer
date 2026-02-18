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
body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; line-height: 1.5; margin: 24px; }
table { border-collapse: collapse; width: 100%; }
th, td { border: 1px solid #d0d7de; padding: 6px 10px; text-align: left; }
th { background: #f6f8fa; }
code { background: #f6f8fa; padding: 1px 4px; border-radius: 4px; }
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
	toYesNo := func(value bool) string {
		if value {
			return "yes"
		}
		return "no"
	}
	normalize := func(value string) string {
		if strings.TrimSpace(value) == "" {
			return "-"
		}
		return value
	}

	var lines []string
	lines = append(lines, "# Install Report")
	lines = append(lines, "")
	lines = append(lines, "Summary:")
	lines = append(lines, "--------")
	lines = append(lines, fmt.Sprintf("Root: `%s`  ", i.rootNameVersion))
	lines = append(lines, fmt.Sprintf("Platform: `%s`  ", p.ctx.Platform().GetName()))
	lines = append(lines, fmt.Sprintf("Project: `%s`  ", p.ctx.Project().GetName()))
	lines = append(lines, fmt.Sprintf("Build Type: `%s`  ", p.ctx.BuildType()))
	lines = append(lines, fmt.Sprintf("Time: `%s`  ", time.Now().Local().Format("2006-01-02 15:04:05")))
	lines = append(lines, "")
	lines = append(lines, "| NameVersion | DevDep | Native | Parent | BuildSystem | InstalledFrom |")
	lines = append(lines, "| --- | --- | --- | --- | --- | --- |")

	orderedEntries := i.orderedEntries()
	for _, entry := range orderedEntries {
		lines = append(lines, fmt.Sprintf("| `%s` | %s | %s | `%s` | `%s` | %s |",
			normalize(entry.NameVersion),
			toYesNo(entry.DevDep),
			toYesNo(entry.Native),
			normalize(entry.Parent),
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
