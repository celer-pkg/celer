//go:build ignore

// Generate root README.md from docs/en-US/README.md by adjusting relative link paths.
//
// Usage:
//
//	go run generate_readme.go
package main

import (
	"fmt"
	"os"
	"regexp"
)

func main() {
	src, err := os.ReadFile("docs/en-US/README.md")
	if err != nil {
		fmt.Fprintf(os.Stderr, "read source: %v\n", err)
		os.Exit(1)
	}

	content := string(src)

	// 1. Fix language switcher: ../zh-CN/README.md -> ./docs/zh-CN/README.md
	content = regexp.MustCompile(`\[🌍 中文\]\(\.\./zh-CN/README\.md\)`).
		ReplaceAllString(content, `[🌍 中文](./docs/zh-CN/README.md)`)

	// 2. Fix image path: ](../assets/ -> ](./docs/assets/
	content = regexp.MustCompile(`\]\(\.\./assets/`).
		ReplaceAllString(content, `](./docs/assets/`)

	// 3. Fix doc links: ](./xxx.md) -> ](./docs/en-US/xxx.md)
	//    Skip links already under ./docs/ and the self-reference ](./README.md).
	//    Handles optional #anchor fragments.
	re := regexp.MustCompile(`\]\(\./([^)]*\.md(?:#[^)]*)?)\)`)
	content = re.ReplaceAllStringFunc(content, func(m string) string {
		// Keep unchanged: self-reference or already under docs/.
		if m == "](./README.md)" || regexp.MustCompile(`^\]\(\./docs/`).MatchString(m) {
			return m
		}
		return re.ReplaceAllString(m, `](./docs/en-US/$1)`)
	})

	// 4. Fix LICENSE path: ../../LICENSE -> ./LICENSE (root README is at repo root).
	content = regexp.MustCompile(`\]\(\.\./\.\./LICENSE\)`).
		ReplaceAllString(content, `](./LICENSE)`)

	if err := os.WriteFile("README.md", []byte(content), 0644); err != nil {
		fmt.Fprintf(os.Stderr, "write README.md: %v\n", err)
		os.Exit(1)
	}

	fmt.Println("README.md generated from docs/en-US/README.md")
}
