package timemachine

import (
	"encoding/json"
	"os"
	"path/filepath"
	"time"
)

// Snapshot represents the metadata of an exported workspace.
type Snapshot struct {
	ExportedAt   time.Time      `json:"exported_at"`
	CelerVersion string         `json:"celer_version"`
	Platform     string         `json:"platform"`
	Project      string         `json:"project"`
	Dependencies []PortSnapshot `json:"dependencies"`
	Notes        string         `json:"notes,omitempty"`
}

// PortSnapshot represents a snapshot of a port with fixed commit.
type PortSnapshot struct {
	Name    string `json:"name"`
	Version string `json:"version"`
	Commit  string `json:"commit"`
	URL     string `json:"url"`
}

// Save saves the snapshot to a JSON file.
func (s *Snapshot) Save(exportDir string) error {
	snapshotPath := filepath.Join(exportDir, "snapshot.json")

	data, err := json.MarshalIndent(s, "", "  ")
	if err != nil {
		return err
	}

	return os.WriteFile(snapshotPath, data, 0644)
}
