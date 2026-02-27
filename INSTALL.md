# Sapphire Installation Guide

## Quick Install (One Line)

```bash
curl -fsSL https://raw.githubusercontent.com/Velocity4705/sapphire-lang/main/install.sh | bash
```

This is the easiest way to install Sapphire. It will:
1. Check prerequisites (g++, make, git)
2. Clone the repository
3. Build Sapphire automatically
4. Ask if you want system-wide installation

## Prerequisites

Before installing, make sure you have:

- **C++20 compiler** (g++ 10+ or clang++ 10+)
- **Make**
- **Git**
- **LLVM** (optional, for native compilation)

### Install Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install g++ make git
```

**Fedora:**
```bash
sudo dnf install gcc-c++ make git
```

**macOS:**
```bash
brew install gcc make git
```

## Build from Source

### Step 1: Clone

```bash
git clone https://github.com/Velocity4705/sapphire-lang.git
cd sapphire-lang
```

### Step 2: Build

```bash
# Build the compiler
make quick

# Build additional tools (optional)
make spm           # Package manager
make sapphire-fmt  # Code formatter
```

This creates:
- `./sapp` - Sapphire compiler/interpreter
- `./sapphire` - Alias for sapp
- `./spm` - Package manager (if built)
- `./sapphire-fmt` - Code formatter (if built)

### Step 3: Test

```bash
./sapp --version
./sapp examples/hello.spp
```

## System-Wide Installation

To use Sapphire from anywhere:

```bash
sudo cp sapp /usr/local/bin/
sudo cp sapphire /usr/local/bin/
```

Optional tools (if available):
```bash
sudo cp spm /usr/local/bin/           # Package manager
sudo cp sapphire-fmt /usr/local/bin/  # Code formatter
```

Now you can run:
```bash
sapp --version
sapp myprogram.spp
```

## Update

Sapphire includes a built-in update system:

```bash
# Check for updates
sapp --check-updates

# Update to latest version
sapp --update
```

This will:
- Pull the latest code from GitHub
- Rebuild automatically
- Preserve your local changes

### Update Requirements

The auto-update feature requires:
- Installation via git (using the one-line installer or manual git clone)
- Internet connection
- Git installed on your system

### If Not Installed via Git

If you installed Sapphire without git, you'll see:
```
ℹ Not installed via git. To update, run:
  curl -fsSL https://raw.githubusercontent.com/Velocity4705/sapphire-lang/main/install.sh | bash
```

Simply run the one-line installer again - it will update your installation.

## Manual Update

If auto-update doesn't work:

```bash
cd sapphire-lang
git pull origin main
make quick
```

## Uninstall

### Remove System Installation

```bash
sudo rm /usr/local/bin/sapp
sudo rm /usr/local/bin/sapphire

# If you installed optional tools
sudo rm /usr/local/bin/spm 2>/dev/null
sudo rm /usr/local/bin/sapphire-fmt 2>/dev/null
```

### Remove Source

```bash
rm -rf sapphire-lang
```

## Troubleshooting

### "g++ not found"

Install C++ compiler:
```bash
sudo apt install g++  # Ubuntu/Debian
sudo dnf install gcc-c++  # Fedora
```

### "make: command not found"

Install make:
```bash
sudo apt install make  # Ubuntu/Debian
sudo dnf install make  # Fedora
```

### "git: command not found"

Install git:
```bash
sudo apt install git  # Ubuntu/Debian
sudo dnf install git  # Fedora
```

### Build Errors

Make sure you have C++20 support:
```bash
g++ --version  # Should be 10.0 or higher
```

### Permission Denied

When installing system-wide, use sudo:
```bash
sudo cp sapp /usr/local/bin/
```

## Verify Installation

```bash
# Check version
sapp --version

# Run hello world
sapp examples/hello.spp

# Check all tools
sapp --help
spm --help
sapphire-fmt --help
```

## Next Steps

After installation:

1. **Read the docs**: [Getting Started](docs/GETTING_STARTED.md)
2. **Try examples**: `ls examples/`
3. **Create a project**: `spm init my-project`
4. **Join the community**: [GitHub Discussions](https://github.com/Velocity4705/sapphire-lang/discussions)

## Platform Support

Sapphire works on:
- ✅ Linux (primary)
- ✅ macOS
- ✅ Windows (via WSL)
- ✅ Web (via WASM)
- ✅ iOS (native)
- ✅ Android (native)

## Getting Help

- **Documentation**: [docs/](docs/)
- **Issues**: [GitHub Issues](https://github.com/Velocity4705/sapphire-lang/issues)
- **Discussions**: [GitHub Discussions](https://github.com/Velocity4705/sapphire-lang/discussions)
- **Quick Fix**: [QUICK_FIX.md](QUICK_FIX.md)

## Advanced Installation

### Custom Build Options

```bash
# Debug build
make debug

# Release build with optimizations
make release

# Build with LLVM support
make llvm

# Build everything
make all
```

### Install to Custom Location

```bash
# Install to ~/bin
cp sapp ~/bin/
export PATH="$PATH:$HOME/bin"
```

### Docker Installation

```dockerfile
FROM ubuntu:22.04
RUN apt update && apt install -y g++ make git
RUN git clone https://github.com/Velocity4705/sapphire-lang.git
WORKDIR /sapphire-lang
RUN make quick
CMD ["./sapp"]
```

## Development Installation

For contributing to Sapphire:

```bash
git clone https://github.com/Velocity4705/sapphire-lang.git
cd sapphire-lang
make debug  # Build with debug symbols
make test   # Run tests
```

See [CONTRIBUTING.md](CONTRIBUTING.md) for more details.
