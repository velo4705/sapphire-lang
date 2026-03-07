# Sapphire Documentation Generator

Automatically generate HTML documentation from Sapphire source code with doc comments.

## Features

- **Doc Comment Parsing**: Extract documentation from `///` or `#!` comments
- **Function Documentation**: Parameters, return types, descriptions
- **Class Documentation**: Class descriptions and methods
- **HTML Generation**: Clean, modern HTML output
- **Markdown Support**: Simple markdown in doc comments
- **Syntax Highlighting**: Code blocks with syntax highlighting
- **Navigation**: Easy navigation between pages
- **Search Ready**: Structure ready for search integration

## Quick Start

### Generate Documentation

```bash
python3 tools/doc/sapphire-doc.py src/ docs/
```

This will:
1. Parse all `.spp` files in `src/`
2. Extract doc comments
3. Generate HTML in `docs/`

### View Documentation

Open `docs/index.html` in your browser.

## Doc Comment Syntax

### Functions

```sapphire
/// Calculate the factorial of a number
///
/// This function uses recursion to calculate n!
///
/// ## Example
/// ```
/// let result = factorial(5)
/// print(result)  # 120
/// ```
fn factorial(n: int) -> int:
    if n <= 1:
        return 1
    return n * factorial(n - 1)
```

### Classes

```sapphire
/// Represents a point in 2D space
///
/// The Point class provides methods for geometric operations.
class Point:
    fn init(x: int, y: int):
        self.x = x
        self.y = y
```

## Markdown Support

Doc comments support simple markdown:

- **Headers**: `# H1`, `## H2`, `### H3`
- **Code**: `` `inline code` ``
- **Code Blocks**: ` ``` ... ``` `
- **Lists**: `- item`
- **Paragraphs**: Blank lines separate paragraphs

## Output Structure

```
docs/
├── index.html       # Main page with overview
├── functions.html   # All functions
└── classes.html     # All classes
```

## Integration with spm

Add to your `spm.toml`:

```toml
[scripts]
docs = "python3 tools/doc/sapphire-doc.py src/ docs/"
```

Then run:

```bash
spm run docs
```

## Future Enhancements

- [ ] Search functionality
- [ ] Module organization
- [ ] Cross-references
- [ ] Type links
- [ ] Example extraction
- [ ] Test coverage
- [ ] Changelog generation
- [ ] PDF export

## License

MIT License - See LICENSE file in the Sapphire repository.
