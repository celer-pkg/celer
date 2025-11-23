package binarycache

import (
	"bytes"
	"fmt"
)

type divider struct{}

func (d divider) string(parents []string, args ...string) string {
	var buffer bytes.Buffer

	for index, parent := range parents {
		if index == 0 {
			buffer.WriteString(parent)
		} else {
			buffer.WriteString(" <<< " + parent)
		}
	}

	for _, arg := range args {
		if arg == "" {
			continue
		}

		if buffer.Len() == 0 {
			buffer.WriteString(arg)
		} else {
			buffer.WriteString(" <<< " + arg)
		}
	}
	return fmt.Sprintf("# -------- %s --------\n", buffer.String())
}

func newDivider(parents []string, args ...string) string {
	return divider{}.string(parents, args...)
}
