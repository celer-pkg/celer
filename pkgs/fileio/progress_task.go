package fileio

import (
	"fmt"
	"path/filepath"
	"strings"

	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/expr"
	"github.com/celer-pkg/celer/pkgs/logger"
)

// Op is a lifecycle step shown in status lines.
type Op string

const (
	OpDownload Op = "Download"
	OpExtract  Op = "Extract"
	OpRestore  Op = "Restore"
	OpStore    Op = "Store"

	opLabelWidth    = max(len(OpDownload), len(OpExtract), len(OpRestore), len(OpStore))
	nameColWidth    = 22
	archiveColWidth = 24
)

// ProgressTask prints aligned status lines for Download/Extract/Restore/Store.
// The second column is the archive (or display) name:
//
//	[✔] [ Download ]  m4-1.4.19.tar.xz    1.58 MB  3s
//	[✔] [  Extract ]  m4-1.4.19.tar.xz    buildtrees/m4@1.4.19/src
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

// Start runs fn while showing an in-progress Extract-style line for dest,
// then marks the line complete. dest is shown relative to the workspace.
func (p *ProgressTask) Start(dest string, fn func() error) error {
	detail := WorkspaceRelPath(dest)
	p.start(detail)
	if err := fn(); err != nil {
		// End the in-progress line; callers print the real error separately.
		logger.PrintInline(logger.Hint, "\n")
		return err
	}
	p.complete(detail)
	return nil
}

// start prints an in-progress line (overwritten by complete).
func (p *ProgressTask) start(detail string) {
	logger.PrintInline(logger.Hint, "%s", p.format(false, detail))
}

// complete prints the success line and ends the current in-progress line.
func (p *ProgressTask) complete(detail string) {
	logger.PrintInline(logger.Success, "%s\n", p.format(true, detail))
}

func (p *ProgressTask) format(done bool, detail string) string {
	icon := expr.If(done, "[✔]", "[-]")
	tag := fmt.Sprintf("[%s]", center(string(p.op), opLabelWidth+2))
	if detail == "" {
		return fmt.Sprintf("%s %s  %-*s", icon, tag, nameColWidth, p.name)
	}
	return fmt.Sprintf("%s %s  %-*s%s%s", icon, tag, nameColWidth, p.name, " ", detail)
}

// center pads s to width, centering it with any extra space on the right.
func center(s string, width int) string {
	if n := width - len(s); n > 0 {
		return strings.Repeat(" ", n/2) + s + strings.Repeat(" ", n-n/2)
	}
	return s
}

func transferDetail(name, archive, size, elapsed string) string {
	if archive == "" || archive == name {
		return fmt.Sprintf("%s  %s", size, elapsed)
	}
	return fmt.Sprintf("%-*s %s  %s", archiveColWidth, archive, size, elapsed)
}

// WorkspaceRelPath returns path relative to the workspace root (forward slashes).
func WorkspaceRelPath(absPath string) string {
	rel, err := filepath.Rel(dirs.WorkspaceDir, absPath)
	if err != nil || rel == ".." || strings.HasPrefix(rel, ".."+string(filepath.Separator)) {
		return filepath.ToSlash(absPath)
	}
	return filepath.ToSlash(rel)
}
