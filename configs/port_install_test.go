package configs

import (
	"fmt"
	"testing"
)

// TestParseInstallPrefer locks down the CLI names of the strict install paths:
// an empty value must keep the default fallback chain, and a typo must fail
// instead of silently falling back.
func TestParseInstallPrefer(t *testing.T) {
	var tests = []struct {
		name string
		want InstallPrefer
		ok   bool
	}{
		{name: "", want: PreferNone, ok: true},
		{name: "none", want: PreferNone, ok: true},
		{name: " source ", want: PreferSource, ok: true},
		{name: "SOURCE", want: PreferSource, ok: true},
		{name: "package", want: PreferPackage, ok: true},
		{name: "pkgcache", want: PreferPkgCache, ok: true},
		{name: "devcache", want: PreferDevCache, ok: true},
		{name: "cache", ok: false},
	}

	for _, tt := range tests {
		t.Run(fmt.Sprintf("%q", tt.name), func(t *testing.T) {
			got, err := ParseInstallPrefer(tt.name)
			if !tt.ok {
				if err == nil {
					t.Fatalf("ParseInstallPrefer(%q) should fail", tt.name)
				}
				return
			}

			if err != nil {
				t.Fatalf("ParseInstallPrefer(%q) error = %v", tt.name, err)
			}
			if got != tt.want {
				t.Fatalf("ParseInstallPrefer(%q) = %v, want %v", tt.name, got, tt.want)
			}
		})
	}
}
