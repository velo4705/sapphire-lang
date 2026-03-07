# Sapphire Language Server Protocol (LSP)

The Sapphire LSP provides IDE features for Sapphire code.

## Features

- **Auto-completion**: Keyword and symbol completion
- **Go to Definition**: Jump to function/class/variable definitions
- **Find References**: Find all uses of a symbol
- **Diagnostics**: Real-time syntax error checking
- **Document Symbols**: Outline view of functions and classes

## Building

```bash
make sapphire-lsp
```

## Usage

The LSP server communicates over stdin/stdout using the JSON-RPC protocol.

### With VSCode

1. Install the Sapphire VSCode extension from `editors/vscode/`
2. The extension will automatically start the LSP server
3. Configure the path to `sapphire-lsp` in VSCode settings if needed

### Manual Testing

You can test the LSP server manually:

```bash
echo 'Content-Length: 52\r\n\r\n{"jsonrpc":"2.0","id":1,"method":"initialize"}' | ./sapphire-lsp
```

## Architecture

```
┌─────────────┐         ┌─────────────┐
│   VSCode    │ ←JSON→  │  LSP Server │
│  Extension  │  RPC    │   (C++)     │
└─────────────┘         └─────────────┘
                              ↓
                        ┌─────────────┐
                        │  Sapphire   │
                        │   Parser    │
                        └─────────────┘
```

## Implementation

- **main.cpp**: LSP server implementation
- **symbol_table.h**: Symbol tracking for definitions and references
- **json_helper.h**: Simple JSON parsing utilities

## Supported LSP Methods

- `initialize` - Initialize server capabilities
- `textDocument/didOpen` - Document opened
- `textDocument/didChange` - Document changed
- `textDocument/completion` - Auto-completion
- `textDocument/definition` - Go to definition
- `textDocument/references` - Find references
- `textDocument/publishDiagnostics` - Error diagnostics

## Future Enhancements

- Hover documentation
- Signature help
- Rename refactoring
- Code actions
- Semantic highlighting
