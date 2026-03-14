#!/bin/bash
# Sapphire One-Line Installer
# Usage: curl -fsSL https://raw.githubusercontent.com/velo4705/sapphire-lang/main/install.sh | bash

set -e

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║         Sapphire Programming Language Installer              ║"
echo "║                    Version 1.0-beta.4                        ║"
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
    git clone https://github.com/velo4705/sapphire-lang.git
    cd sapphire-lang
fi

# Build
echo ""
echo "Building Sapphire..."
make clean
make quick

echo ""
echo "Building additional tools..."
make spm 2>/dev/null && echo "  ✓ spm built" || echo "  ⚠ spm build skipped"
make sapphire-fmt 2>/dev/null && echo "  ✓ sapphire-fmt built" || echo "  ⚠ sapphire-fmt build skipped"

# Install to system (recommended)
echo ""
echo "Choose installation method:"
echo "1) System-wide install (recommended) - requires sudo"
echo "2) User install - adds to your PATH permanently"
echo "3) Local only - run with ./sapp"
echo ""
read -p "Choose option (1/2/3) [1]: " install_choice
install_choice=${install_choice:-1}

if [[ $install_choice == "1" ]]; then
    echo "Installing system-wide..."
    sudo cp sapp /usr/local/bin/
    sudo cp sapphire /usr/local/bin/
    
    if [ -f "spm" ]; then
        sudo cp spm /usr/local/bin/
    fi
    
    if [ -f "sapphire-fmt" ]; then
        sudo cp sapphire-fmt /usr/local/bin/
    fi
    
    echo "✓ Installed to /usr/local/bin"
    echo "You can now run 'sapp' from anywhere!"
    
elif [[ $install_choice == "2" ]]; then
    echo "Setting up user installation..."
    SAPPHIRE_DIR="$(pwd)"
    
    # Detect shell and add to appropriate config file
    if [[ $SHELL == *"zsh"* ]]; then
        SHELL_CONFIG="$HOME/.zshrc"
    elif [[ $SHELL == *"bash"* ]]; then
        SHELL_CONFIG="$HOME/.bashrc"
    else
        SHELL_CONFIG="$HOME/.profile"
    fi
    
    # Add PATH export to shell config if not already present
    if ! grep -q "sapphire-lang" "$SHELL_CONFIG" 2>/dev/null; then
        echo "" >> "$SHELL_CONFIG"
        echo "# Sapphire Programming Language" >> "$SHELL_CONFIG"
        echo "export PATH=\"\$PATH:$SAPPHIRE_DIR\"" >> "$SHELL_CONFIG"
        echo "✓ Added Sapphire to PATH in $SHELL_CONFIG"
        echo ""
        echo "To use immediately, run: source $SHELL_CONFIG"
        echo "Or open a new terminal session"
    else
        echo "✓ Sapphire already in PATH"
    fi
    
    # Also export for current session
    export PATH="$PATH:$SAPPHIRE_DIR"
    echo "✓ Sapphire available in current session"
    
else
    echo "Local installation - use ./sapp to run"
    echo ""
    echo "To make it globally available later:"
    echo "  Option 1: sudo cp sapp /usr/local/bin/"
    echo "  Option 2: Add to PATH: echo 'export PATH=\"\$PATH:$(pwd)\"' >> ~/.bashrc"
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
echo "Quick start:"
if [[ $install_choice == "1" ]]; then
    echo "  sapp examples/hello.spp"
elif [[ $install_choice == "2" ]]; then
    echo "  sapp examples/hello.spp  (after: source $SHELL_CONFIG)"
else
    echo "  ./sapp examples/hello.spp"
fi
echo ""
echo "Documentation: https://github.com/velo4705/sapphire-lang"
echo "Update anytime: Re-run this installer"
