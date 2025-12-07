package configs

import (
	"celer/buildtools"
	"celer/context"
	"celer/pkgs/dirs"
	"celer/pkgs/fileio"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"strings"

	"github.com/BurntSushi/toml"
)

type Platform struct {
	RootFS     *RootFS     `toml:"rootfs"`
	Toolchain  *Toolchain  `toml:"toolchain"`
	WindowsKit *WindowsKit `toml:"windows_kit"`

	// Internal fields.
	Name string          `toml:"-"`
	ctx  context.Context `toml:"-"`
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
		return fmt.Errorf("platform does not exist: %s", platformName)
	}
	p.Name = platformName

	// Read conf/celer.toml
	bytes, err := os.ReadFile(platformPath)
	if err != nil {
		return err
	}
	if err := toml.Unmarshal(bytes, p); err != nil {
		return fmt.Errorf("failed to read %s.\n %w", platformPath, err)
	}

	if p.RootFS != nil {
		p.RootFS.ctx = p.ctx
		if err := p.RootFS.Validate(); err != nil {
			return err
		}
	}
	if p.Toolchain != nil {
		p.Toolchain.ctx = p.ctx
		if err := p.Toolchain.Validate(); err != nil {
			return err
		}
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
func (p *Platform) setup() error {
	// Repair rootfs if not empty.
	if p.RootFS != nil {
		if err := p.RootFS.CheckAndRepair(); err != nil {
			return fmt.Errorf("failed to check and repair rootfs.\n %w", err)
		}
	}

	if err := p.Toolchain.CheckAndRepair(false); err != nil {
		return fmt.Errorf("failed to check and repair toolchain.\n %w", err)
	}

	// Repaire ccache.
	if err := buildtools.CheckTools(p.ctx, "ccache"); err != nil {
		return fmt.Errorf("failed to check and repair ccache.\n %w", err)
	}

	// Generate toolchain file.
	if err := p.ctx.GenerateToolchainFile(); err != nil {
		return fmt.Errorf("failed to generate toolchain file.\n %w", err)
	}

	return nil
}
