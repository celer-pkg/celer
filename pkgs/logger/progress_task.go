package logger

import (
	"fmt"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/pkgs/dirs"
)

// Op is a lifecycle step shown in status lines.
type Op string

const (
	OpDownload Op = "Download"
	OpExtract  Op = "Extract"
	OpRestore  Op = "Restore"
	OpStore    Op = "Store"
	// Ops for one-off steps that do not transfer an archive.
	OpCreate Op = "Create"
	OpGit    Op = "Git"

	opLabelWidth    = max(len(OpDownload), len(OpExtract), len(OpRestore), len(OpStore), len(OpCreate), len(OpGit))
	nameColWidth    = 22
	archiveColWidth = 24
)

// opStyles color the op tag so each step is recognizable at a glance.
//
// The palette groups steps by what they do, so a glance at the hue tells you
// which kind of work the line belongs to:
//
//	incoming    cyan   Download (from network), Restore (from cache)
//	outgoing    magenta Store (into cache)
//	local work  yellow Extract, Create
//	VCS         blue   Git
//
// The status icon shares the op hue, so each line reads as one color.
var opStyles = map[Op]*Style{
	OpDownload: New(Cyan, Bold),
	OpRestore:  New(BrightCyan, Bold),
	OpStore:    New(Magenta, Bold),
	OpExtract:  New(Yellow, Bold),
	OpCreate:   New(BrightYellow, Bold),
	OpGit:      New(Red, Bold),
}

// ProgressTask prints aligned status lines for Download/Extract/Restore/Store
// and other one-off steps. The second column is the archive (or display) name:
//
//	[✔] [ Download ]  m4-1.4.19.tar.xz    1.58 MB  3s
//	[✔] [  Extract ]  m4-1.4.19.tar.xz    buildtrees/m4@1.4.19/src
//	[✔] [  Create  ]  py311               python 3.11 · conda-forge
type ProgressTask struct {
	op   Op
	name string
}

func NewProgressTask(op Op, name string) *ProgressTask {
	return &ProgressTask{op: op, name: name}
}

// Complete finishes a Download/Store/Restore line with size and elapsed time.
// archive is an optional extra column; omit it (or pass name) when the second
// column already is the archive name.
func (p *ProgressTask) Complete(archive, size, elapsed string) {
	p.complete(transferDetail(p.name, archive, size, elapsed))
}

// Start runs fn while showing dest in the detail column, then marks the line
// complete. dest is shown relative to the workspace.
func (p *ProgressTask) Start(dest string, fn func() error) error {
	return p.Run(WorkspaceRelPath(dest), fn)
}

// Run runs fn while showing detail in the trailing column, then marks the line
// complete. On failure the in-progress line is ended (callers print the real
// error separately) and fn's error is returned unchanged.
func (p *ProgressTask) Run(detail string, fn func() error) error {
	p.start(detail)
	if err := fn(); err != nil {
		PrintInline(Hint, "\n")
		return err
	}
	p.complete(detail)
	return nil
}

// start prints an in-progress line (overwritten by complete).
func (p *ProgressTask) start(detail string) {
	PrintInline(Hint, "%s", p.format(false, detail))
}

// complete prints the success line and ends the current in-progress line.
func (p *ProgressTask) complete(detail string) {
	PrintInline(Pass, "%s\n", p.format(true, detail))
}

func (p *ProgressTask) format(done bool, detail string) string {
	tagStyle := opStyles[p.op]
	if tagStyle == nil {
		tagStyle = Summary
	}
	icon := tagStyle.Apply("[·]")
	if done {
		icon = tagStyle.Apply("[✔]")
	}
	tag := tagStyle.Apply(fmt.Sprintf("[%s]", center(string(p.op), opLabelWidth+2)))
	name := tagStyle.Apply(fmt.Sprintf("%-*s", nameColWidth, p.name))
	if detail == "" {
		return fmt.Sprintf("%s %s  %s", icon, tag, name)
	}
	return fmt.Sprintf("%s %s  %s %s", icon, tag, name, Muted.Apply(detail))
}

// center pads s to width, centering it with any extra space on the right.
func center(s string, width int) string {
	if n := width - len(s); n > 0 {
		return strings.Repeat(" ", n/2) + s + strings.Repeat(" ", n-n/2)
	}
	return s
}

func transferDetail(name, archive, size, elapsed string) string {
	meta := fmt.Sprintf("%s · %s", size, elapsed)
	if archive == "" || archive == name {
		return meta
	}
	return fmt.Sprintf("%-*s %s", archiveColWidth, archive, meta)
}

// WorkspaceRelPath returns path relative to the workspace root (forward slashes).
func WorkspaceRelPath(absPath string) string {
	rel, err := filepath.Rel(dirs.WorkspaceDir, absPath)
	if err != nil || rel == ".." || strings.HasPrefix(rel, ".."+string(filepath.Separator)) {
		return filepath.ToSlash(absPath)
	}
	return filepath.ToSlash(rel)
}
