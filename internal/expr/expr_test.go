package expr

import "testing"

func TestGetMinorVersion(t *testing.T) {
	tests := map[string]string{
		"":            "",
		"5":           "5",
		"5.11":        "5.11",
		"5.11.13":     "5.11",
		"5.11.13.4":   "5.11",
		"v1.2.3-rc.1": "v1.2",
	}

	for in, want := range tests {
		if got := GetMinorVersion(in); got != want {
			t.Errorf("GetMinorVersion(%q) = %q, want %q", in, got, want)
		}
	}
}
