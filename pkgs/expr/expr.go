package expr

import (
	"fmt"
	"strings"
)

func FormatSize(byteSize int64) string {
	const (
		KB = 1024
		MB = KB * 1024
		GB = MB * 1024
	)

	var size float64
	var unit string

	switch {
	case byteSize < KB:
		size = float64(byteSize)
		unit = "B"
	case byteSize < MB:
		size = float64(byteSize) / KB
		unit = "KB"
	case byteSize < GB:
		size = float64(byteSize) / MB
		unit = "MB"
	default:
		size = float64(byteSize) / GB
		unit = "GB"
	}

	return fmt.Sprintf("%.2f%s", size, unit)
}

func If[T any](condition bool, first T, second T) T {
	if condition {
		return first
	}
	return second
}

func UpperFirst(text string) string {
	return strings.ToUpper(text[:1]) + strings.ToLower(text[1:])
}

// GetMinorVersion convert a version string to its major.minor part
// (e.g. "5.11.13" -> "5.11", "1.11" -> "1.11"). 
func GetMinorVersion(version string) string {
	parts := strings.Split(version, ".")
	if len(parts) >= 2 {
		return parts[0] + "." + parts[1]
	}
	return version
}