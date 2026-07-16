package configs

import (
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/celer-pkg/celer/context"
	"github.com/celer-pkg/celer/pkgs/dirs"
	"github.com/celer-pkg/celer/pkgs/errors"
	"github.com/celer-pkg/celer/pkgs/fileio"

	"github.com/BurntSushi/toml"
)

type Platform struct {
	Toolchain  *Toolchain  `toml:"toolchain"`
	WindowsKit *WindowsKit `toml:"windows_kit"`
	RootFS     *RootFS     `toml:"rootfs"`

	// Internal fields.
	Name      string          `toml:"-"`
	ctx       context.Context `toml:"-"`
	setupDone bool            `toml:"-"`
}

func (p *Platform) Init(platformName string) error {
	// Check if platform name is empty.
	platformName = strings.TrimSpace(platformName)
	if platformName == "" {
		return fmt.Errorf("platform name is empty")
	}

	// Check if platform file exists.
	platformPath := filepath.Join(dirs.ConfPlatformsDir, platformName+".toml")
	if !fileio.PathExists(platformPath) {
		return fmt.Errorf("%w: %s", errors.ErrPlatformNotExist, platformName)
	}
	p.Name = platformName

	// Read conf/celer.toml
	bytes, err := os.ReadFile(platformPath)
	if err != nil {
		return err
	}
	if err := toml.Unmarshal(bytes, p); err != nil {
		return fmt.Errorf("failed to read %s -> %w", platformPath, err)
	}

	exrVars := p.ctx.ExprVars()
	if p.Toolchain != nil {
		p.Toolchain.ctx = p.ctx
		if err := p.Toolchain.Validate(); err != nil {
			return err
		}

		// Store toolchain releated express vars.
		exrVars.Put("HOST", p.Toolchain.GetHost())
		exrVars.Put("BUILD_HOST", p.buildHost())
		exrVars.Put("SYSTEM_NAME", strings.ToLower(p.Toolchain.GetHost()))
		exrVars.Put("SYSTEM_VERSION", p.Toolchain.GetSystemVersion())
		exrVars.Put("SYSTEM_PROCESSOR", p.Toolchain.GetSystemProcessor())
		exrVars.Put("CROSSTOOL_PREFIX", p.Toolchain.GetCrosstoolPrefix())
		exrVars.Put("TOOLCHAIN", p.Toolchain.rootDir)
		exrVars.Put("BUILD_HOST", p.buildHost())
		exrVars.Put("CC", p.Toolchain.CC)
		exrVars.Put("CXX", p.Toolchain.CXX)
		exrVars.Put("CPP", p.Toolchain.CPP)
		exrVars.Put("AR", p.Toolchain.AR)
		exrVars.Put("LD", p.Toolchain.LD)
		exrVars.Put("AS", p.Toolchain.AS)
		exrVars.Put("OBJCOPY", p.Toolchain.OBJCOPY)
		exrVars.Put("OBJDUMP", p.Toolchain.OBJDUMP)
		exrVars.Put("STRIP", p.Toolchain.STRIP)
		exrVars.Put("READELF", p.Toolchain.READELF)
		exrVars.Put("SIZE", p.Toolchain.SIZE)
		exrVars.Put("STRINGS", p.Toolchain.STRINGS)
		exrVars.Put("NM", p.Toolchain.NM)
		exrVars.Put("RANLIB", p.Toolchain.RANLIB)
		exrVars.Put("GCOV", p.Toolchain.GCOV)
		exrVars.Put("ADDR2LINE", p.Toolchain.ADDR2LINE)
		exrVars.Put("CXXFILT", p.Toolchain.CXXFILT)
		exrVars.Put("FC", p.Toolchain.FC)
	}

	if p.RootFS != nil {
		p.RootFS.ctx = p.ctx
		if err := p.RootFS.Validate(); err != nil {
			return err
		}

		// Store toolchain releated express vars.
		exrVars.Put("SYSROOT", p.RootFS.GetAbsDir())
	}

	return nil
}

func (p Platform) GetName() string {
	return p.Name
}

func (p Platform) GetHostName() string {
	var arch string
	switch runtime.GOARCH {
	case "amd64":
		arch = "x86_64"
	case "386":
		arch = "i386"
	case "arm":
		arch = "arm"
	case "arm64":
		arch = "aarch64"
	default:
		panic("unsupported architecture: " + runtime.GOARCH)
	}

	switch runtime.GOOS {
	case "windows":
		return arch + "-windows"

	case "darwin":
		return arch + "-darwin"

	case "linux":
		return arch + "-linux"

	default:
		panic("unsupported operating system: " + runtime.GOOS)
	}
}

func (p Platform) GetToolchain() context.Toolchain {
	return p.Toolchain
}

func (p Platform) GetRootFS() context.RootFS {
	if p.RootFS == nil {
		return nil
	}
	return p.RootFS
}

func (p *Platform) Write(platformPath string) error {
	// Create empty platform.
	p.RootFS = &RootFS{
		PkgConfigPath: []string{},
		IncludeDirs:   []string{},
		LibDirs:       []string{},
	}
	p.Toolchain = &Toolchain{}

	bytes, err := toml.Marshal(p)
	if err != nil {
		return err
	}

	// Check if conf/celer.toml exists.
	if fileio.PathExists(platformPath) {
		return fmt.Errorf("%s is already exists", platformPath)
	}

	// Make sure the parent directory exists.
	parentDir := filepath.Dir(platformPath)
	if err := os.MkdirAll(parentDir, os.ModePerm); err != nil {
		return err
	}
	return os.WriteFile(platformPath, bytes, os.ModePerm)
}

// setup rootfs and toolchain
func (p *Platform) Setup() error {
	// Check if setup is done.
	if p.setupDone {
		return nil
	}

	// Repair rootfs if not empty.
	if p.RootFS != nil {
		if err := p.RootFS.CheckAndRepair(); err != nil {
			return fmt.Errorf("failed to check and repair rootfs -> %w", err)
		}
	}

	// Repair toolchain.
	if err := p.Toolchain.CheckAndRepair(false); err != nil {
		return fmt.Errorf("failed to check and repair toolchain -> %w", err)
	}

	// Generate toolchain file.
	if err := p.ctx.GenerateToolchainFile(); err != nil {
		return fmt.Errorf("failed to generate toolchain file -> %w", err)
	}

	p.setupDone = true
	return nil
}

// buildHost returns the build machine triplet (e.g., "x86_64-linux-gnu").
// This is the machine where the compiler is running, not the target machine.
func (p *Platform) buildHost() string {
	// Get the processor architecture.
	var processor string
	switch runtime.GOARCH {
	case "amd64":
		processor = "x86_64"
	case "arm64":
		processor = "aarch64"
	case "386":
		processor = "i686"
	case "arm":
		processor = "arm"
	default:
		processor = runtime.GOARCH
	}

	// Get the OS.
	var os string
	switch runtime.GOOS {
	case "linux":
		os = "linux"
	case "windows":
		os = "windows"
	case "darwin":
		os = "apple"
	default:
		os = runtime.GOOS
	}

	// Return triplet format.
	switch runtime.GOOS {
	case "linux":
		return fmt.Sprintf("%s-%s-gnu", processor, os)
	case "darwin":
		return fmt.Sprintf("%s-%s-darwin", processor, os)
	default:
		return fmt.Sprintf("%s-%s", processor, os)
	}
}
