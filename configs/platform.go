package configs

import (
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
	Name string `toml:"-"`
	ctx  Context
}

func (p *Platform) Init(ctx Context, platformName string) error {
	// Init internal fields.
	p.ctx = ctx
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

	// RootFS maybe nil when platform is native.
	if p.RootFS != nil {
		if err := p.RootFS.Validate(); err != nil {
			return err
		}
	}

	// Validate toolchain or detect toolchain if not specified in platform.
	if err := p.detectToolchain(); err != nil {
		return err
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

func (p *Platform) Setup() error {
	// Repair rootfs if not empty.
	if p.RootFS != nil {
		if err := p.RootFS.Validate(); err != nil {
			return err
		}

		if err := p.RootFS.CheckAndRepair(); err != nil {
			return fmt.Errorf("celer.rootfs check and repair error: %w", err)
		}
	}

	if p.Toolchain == nil {
		// Auto detect native toolchain in different os.
		if err := p.detectToolchain(); err != nil {
			return err
		}
	} else {
		// Repair toolchain.
		if err := p.Toolchain.Validate(); err != nil {
			return fmt.Errorf("valid toolchain error: %w", err)
		}

		if err := p.Toolchain.CheckAndRepair(); err != nil {
			return fmt.Errorf("check and repair toolchain error: %w", err)
		}
	}

	return nil
}

func (p *Platform) detectToolchain() error {
	// Detect toolchain.
	var toolchain Toolchain
	if err := toolchain.Detect(); err != nil {
		return fmt.Errorf("detect celer.toolchain: %w", err)
	}
	p.Toolchain = &toolchain

	// Windows kit is only supported in Windows.
	if runtime.GOOS == "windows" && p.WindowsKit == nil {
		var windowsKit WindowsKit
		if err := windowsKit.Detect(&p.Toolchain.msvc); err != nil {
			return fmt.Errorf("detect celer.windows_kit: %w", err)
		}
		p.WindowsKit = &windowsKit
	}

	// Assign standard toolchain name.
	if toolchain.Name == "msvc" {
		p.Name = "x86_64-windows"
	} else {
		p.Name = "x86_64-linux"
	}

	return nil
}
