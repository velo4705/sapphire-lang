# Sapphire Language Support for VSCode

Official VSCode extension for the Sapphire programming language.

## Features

- **Syntax Highlighting** - Full syntax highlighting for Sapphire code
- **Auto-completion** - Intelligent code completion (requires LSP)
- **Go to Definition** - Jump to function/class definitions (requires LSP)
- **Error Checking** - Real-time syntax error detection (requires LSP)
- **Code Formatting** - Format code with sapphire-fmt

## Requirements

- Sapphire compiler installed
- `sapphire-lsp` in PATH (for LSP features)
- `sapphire-fmt` in PATH (for formatting)

## Installation

### From Source

1. Clone the Sapphire repository
2. Navigate to `editors/vscode/`
3. Run `npm install`
4. Run `npm run compile`
5. Press F5 to launch extension development host

### From VSIX

1. Download the `.vsix` file
2. Open VSCode
3. Go to Extensions
4. Click "..." menu
5. Select "Install from VSIX..."

## Configuration

Configure the extension in VSCode settings:

```json
{
  "sapphire.lsp.enabled": true,
  "sapphire.lsp.path": "sapphire-lsp"
}
```

## Usage

1. Open a `.spp` file
2. Start typing - auto-completion will appear
3. Hover over symbols for documentation
4. Right-click and select "Go to Definition"
5. Format code with Shift+Alt+F

## Building the LSP Server

```bash
cd /path/to/sapphire-lang
make sapphire-lsp
```

This creates the `sapphire-lsp` executable.

## Troubleshooting

### LSP not working

1. Check that `sapphire-lsp` is in your PATH
2. Check VSCode output panel (View > Output > Sapphire Language Server)
3. Try restarting VSCode

### Syntax highlighting not working

1. Make sure file extension is `.spp`
2. Try reloading VSCode window (Ctrl+Shift+P > Reload Window)

## Contributing

Contributions welcome! See [CONTRIBUTING.md](../../CONTRIBUTING.md)

## License

MIT License - see [LICENSE](../../LICENSE)
