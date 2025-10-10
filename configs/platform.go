package configs

import (
	"celer/pkgs/dirs"
	"celer/pkgs/expr"
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
	Name string `toml:"-"`
	ctx  Context
}

func (p *Platform) Init(platformName string) error {
	// Init internal fields.
	p.Name = platformName

	// Check if platform name is empty.
	platformName = strings.TrimSpace(platformName)
	if platformName == "" {
		return fmt.Errorf("platform name is empty")
	}

	// Check if platform file exists.
	platformPath := filepath.Join(dirs.ConfPlatformsDir, platformName+".toml")
	if !fileio.PathExists(platformPath) {
		return fmt.Errorf("platform %s does not exists", platformName)
	}

	// Read conf/celer.toml
	bytes, err := os.ReadFile(platformPath)
	if err != nil {
		return err
	}
	if err := toml.Unmarshal(bytes, p); err != nil {
		return fmt.Errorf("read error: %w", err)
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

func (p Platform) HostName() string {
	var arch string
	switch runtime.GOARCH {
	case "amd64":
		arch = "x86_64"
	case "386":
		arch = "x86"
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

func (p Platform) Write(platformPath string) error {
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

// Setup build envs.
func (p *Platform) Setup() error {
	// Repair rootfs if not empty.
	if p.RootFS != nil {
		if err := p.RootFS.CheckAndRepair(); err != nil {
			return fmt.Errorf("check and repair rootfs error: %w", err)
		}
	}

	// WindowsKit can be detected automatically.
	if runtime.GOOS == "windows" && p.WindowsKit == nil {
		var windowsKit WindowsKit
		if err := windowsKit.Detect(&p.Toolchain.MSVC); err != nil {
			return fmt.Errorf("detect celer.windows_kit error: %w", err)
		}
		p.WindowsKit = &windowsKit
	}

	// Repair toolchain.
	if p.Toolchain == nil {
		panic("Toolchain should not be empty, it may specified in platform or automatically detected.")
	}
	if err := p.Toolchain.CheckAndRepair(false); err != nil {
		return fmt.Errorf("check and repair toolchain error: %w", err)
	}

	// Only for Windows MSVC.
	if p.Toolchain.Name == "msvc" {
		p.Toolchain.MSVC.VCVars = filepath.Join(p.Toolchain.rootDir, "VC", "Auxiliary", "Build", "vcvarsall.bat")
	}

	// Generate toolchain file.
	if err := p.ctx.GenerateToolchainFile(); err != nil {
		return fmt.Errorf("generate toolchain file error: %w", err)
	}

	return nil
}

func (p *Platform) detectToolchain() error {
	// Detect toolchain.
	var toolchain = Toolchain{ctx: p.ctx}
	if err := toolchain.Detect(); err != nil {
		return fmt.Errorf("detect celer.toolchain: %w", err)
	}
	p.Toolchain = &toolchain
	p.Toolchain.SystemName = runtime.GOOS
	p.Toolchain.SystemProcessor = runtime.GOARCH

	// WindowsKit can be detected automatically.
	if runtime.GOOS == "windows" && p.WindowsKit == nil {
		var windowsKit WindowsKit
		if err := windowsKit.Detect(&p.Toolchain.MSVC); err != nil {
			return fmt.Errorf("detect celer.windows_kit error: %w", err)
		}
		p.WindowsKit = &windowsKit
	}

	// Assign standard toolchain name.
	p.Name = expr.If(p.Toolchain.Name == "msvc", "x86_64-windows", "x86_64-linux")

	return nil
}
