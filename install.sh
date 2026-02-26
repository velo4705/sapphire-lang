#!/bin/bash
# Sapphire One-Line Installer
# Usage: curl -fsSL https://raw.githubusercontent.com/Velocity4705/sapphire-lang/main/install.sh | bash

set -e

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║         Sapphire Programming Language Installer              ║"
echo "║                    Version 1.0.0                              ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Check prerequisites
echo "Checking prerequisites..."
command -v g++ >/dev/null 2>&1 || { echo "✗ g++ not found. Install: sudo apt install g++"; exit 1; }
command -v make >/dev/null 2>&1 || { echo "✗ make not found. Install: sudo apt install make"; exit 1; }
command -v git >/dev/null 2>&1 || { echo "✗ git not found. Install: sudo apt install git"; exit 1; }
echo "✓ All prerequisites found"
echo ""

# Clone repository
echo "Cloning Sapphire..."
if [ -d "sapphire-lang" ]; then
    echo "Directory exists, updating..."
    cd sapphire-lang
    git pull
else
    git clone https://github.com/Velocity4705/sapphire-lang.git
    cd sapphire-lang
fi

# Build
echo ""
echo "Building Sapphire..."
make quick

echo ""
echo "Building additional tools..."
make spm 2>/dev/null && echo "  ✓ spm built" || echo "  ⚠ spm build skipped"
make sapphire-fmt 2>/dev/null && echo "  ✓ sapphire-fmt built" || echo "  ⚠ sapphire-fmt build skipped"

# Install to system (optional)
echo ""
read -p "Install to /usr/local/bin? (y/N): " install_system

if [[ $install_system =~ ^[Yy]$ ]]; then
    echo "Installing..."
    sudo cp sapp /usr/local/bin/
    sudo cp sapphire /usr/local/bin/
    
    if [ -f "spm" ]; then
        sudo cp spm /usr/local/bin/
    fi
    
    if [ -f "sapphire-fmt" ]; then
        sudo cp sapphire-fmt /usr/local/bin/
    fi
    
    echo "✓ Installed to /usr/local/bin"
    echo ""
    echo "You can now run 'sapp' from anywhere!"
else
    echo ""
    echo "To use Sapphire, run: ./sapp"
    echo "Or add to PATH: export PATH=\"\$PATH:$(pwd)\""
fi

# Test
echo ""
echo "Testing installation..."
./sapp --version

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  ✓ Sapphire installed successfully!                          ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""
echo "Try it out:"
echo "  ./sapp examples/hello.spp"
echo ""
echo "Update anytime:"
echo "  sapp --update"
echo ""
echo "Documentation: https://github.com/Velocity4705/sapphire-lang"
