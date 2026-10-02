package dag

import (
	"os"
	"path/filepath"
	"reflect"
	"testing"
)

// TestParseDAG locks down the checks done before a build follows an exported
// DAG: the file must parse, and it must have been exported by this celer
// version, since the plan's own workspace identity is adopted by the caller.
func TestParseDAG(t *testing.T) {
	dir := t.TempDir()

	valid := filepath.Join(dir, "dag.json")
	content := `{
  "celer_version": "v0.0.0",
  "platform": "x86_64-linux-ubuntu-22.04-gcc-11.5.0",
  "project": "project_01",
  "build_type": "release",
  "scheduled_nodes": [
    {"name_version": "zlib@1.3.1", "build_hash": "aa", "checksum": "bb", "dependencies": []},
    {"name_version": "fontconfig@2.16.0", "build_hash": "cc", "checksum": "dd", "dependencies": ["zlib@1.3.1"]}
  ]
}`
	if err := os.WriteFile(valid, []byte(content), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	invalid := filepath.Join(dir, "invalid.json")
	if err := os.WriteFile(invalid, []byte("{"), os.ModePerm); err != nil {
		t.Fatal(err)
	}

	var tests = []struct {
		name         string
		path         string
		celerVersion string
		nodes        int
		wantErr      bool
	}{
		{
			name:         "same celer version",
			path:         valid,
			celerVersion: "v0.0.0",
			nodes:        2,
		},
		{
			name:         "another celer version",
			path:         valid,
			celerVersion: "v0.4.2",
			wantErr:      true,
		},
		{
			name:         "malformed json",
			path:         invalid,
			celerVersion: "v0.0.0",
			wantErr:      true,
		},
		{
			name:         "missing file",
			path:         filepath.Join(dir, "not-exist.json"),
			celerVersion: "v0.0.0",
			wantErr:      true,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			report, err := Parse(tt.path, tt.celerVersion)
			if tt.wantErr {
				if err == nil {
					t.Fatal("Parse() should fail")
				}
				return
			}

			if err != nil {
				t.Fatalf("Parse() error = %v", err)
			}
			if len(report.ScheduledNodes) != tt.nodes {
				t.Fatalf("Parse() returned %d nodes, want %d", len(report.ScheduledNodes), tt.nodes)
			}
		})
	}
}

// TestReportPlan locks down what a build takes from a plan: the source revision
// pinned per port, the build hash expected per scheduled node, and the nodes
// pkgcache supplies instead of building them.
func TestReportPlan(t *testing.T) {
	report := Report{
		ScheduledNodes: []node{
			{NameVersion: "fontconfig@2.16.0", BuildHash: "aa", Checksum: "bb", Dependencies: []string{"zlib@1.3.1"}},
			{NameVersion: "zlib@1.3.1", BuildHash: "cc"},
		},
		Cached:     []string{"libpng@1.6.49"},
		LocalNodes: []localNode{{NameVersion: "nasm@2.16.03", Checksum: "dd"}, {NameVersion: "m4@1.4.19"}},
	}

	// Only ports with a checksum are pinned: an empty revision would undo a pin
	// set by another source, and a port without one is never cloned anyway.
	pins := report.PinSources()
	wantPins := map[string]string{"fontconfig@2.16.0": "bb", "nasm@2.16.03": "dd"}
	if !reflect.DeepEqual(pins, wantPins) {
		t.Fatalf("Pins() = %v, want %v", pins, wantPins)
	}

	hashes := report.DagHashs()
	wantHashes := map[string]string{"fontconfig@2.16.0": "aa", "zlib@1.3.1": "cc"}
	if !reflect.DeepEqual(hashes, wantHashes) {
		t.Fatalf("BuildHashes() = %v, want %v", hashes, wantHashes)
	}
	cached := report.DagCached()
	wantCached := map[string]bool{"libpng@1.6.49": true}
	if !reflect.DeepEqual(cached, wantCached) {
		t.Fatalf("CachedSet() = %v, want %v", cached, wantCached)
	}
}
