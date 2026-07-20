//go:build ignore

// Generate root README.md from docs/en-US/README.md by adjusting relative link paths.
//
// The en-US README uses paths relative to docs/en-US/ (e.g. ./quick_start.md).
// The root README needs paths relative to repo root (e.g. ./docs/en-US/quick_start.md).
//
// Usage:
//
//	go run generate_readme.go
package main

import (
	"fmt"
	"os"
	"strings"
)

func main() {
	src, err := os.ReadFile("docs/en-US/README.md")
	if err != nil {
		fmt.Fprintf(os.Stderr, "read source: %v\n", err)
		os.Exit(1)
	}

	content := string(src)

	// Language switcher: point back to en-US README, zh-CN stays.
	content = strings.ReplaceAll(content,
		"[English](../../README.md)",
		"[English](./README.md)")
	content = strings.ReplaceAll(content,
		"[🌍 中文](../zh-CN/README.md)",
		"[🌍 中文](./docs/zh-CN/README.md)")

	// Assets: lift one level up from docs/en-US/.
	content = strings.ReplaceAll(content,
		"](../../assets/",
		"](./docs/assets/")

	// Root-level files: lift out of docs/en-US/.
	content = strings.ReplaceAll(content,
		"](../../LICENSE)",
		"](./LICENSE)")

	// Doc links: ./xxx.md -> ./docs/en-US/xxx.md
	// Only rewrite relative links that are NOT already under docs/en-US/.
	// We do a simple prefix check: replace "./quick_" with "./docs/en-US/quick_" etc.
	content = strings.ReplaceAll(content,
		"](./quick_",
		"](./docs/en-US/quick_")
	content = strings.ReplaceAll(content,
		"](./cmd_",
		"](./docs/en-US/cmd_")
	content = strings.ReplaceAll(content,
		"](./article_",
		"](./docs/en-US/article_")
	content = strings.ReplaceAll(content,
		"](./why_",
		"](./docs/en-US/why_")

	if err := os.WriteFile("README.md", []byte(content), 0644); err != nil {
		fmt.Fprintf(os.Stderr, "write README.md: %v\n", err)
		os.Exit(1)
	}

	fmt.Println("README.md generated from docs/en-US/README.md")
}
