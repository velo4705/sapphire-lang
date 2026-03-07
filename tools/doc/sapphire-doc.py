#!/usr/bin/env python3
"""
Sapphire Documentation Generator

Generates HTML documentation from Sapphire source code with doc comments.
"""

import os
import sys
import re
import json
from pathlib import Path
from datetime import datetime

class DocParser:
    """Parse Sapphire source code and extract documentation"""
    
    def __init__(self):
        self.functions = []
        self.classes = []
        self.current_doc = []
    
    def parse_file(self, filepath):
        """Parse a single Sapphire file"""
        with open(filepath, 'r') as f:
            lines = f.readlines()
        
        i = 0
        while i < len(lines):
            line = lines[i].strip()
            
            # Check for doc comment
            if line.startswith('///') or line.startswith('#!'):
                doc_line = line[3:].strip() if line.startswith('///') else line[2:].strip()
                self.current_doc.append(doc_line)
                i += 1
                continue
            
            # Check for function definition
            if line.startswith('fn '):
                func = self.parse_function(line)
                if func:
                    func['doc'] = '\n'.join(self.current_doc)
                    func['file'] = str(filepath)
                    self.functions.append(func)
                self.current_doc = []
            
            # Check for class definition
            elif line.startswith('class '):
                cls = self.parse_class(line)
                if cls:
                    cls['doc'] = '\n'.join(self.current_doc)
                    cls['file'] = str(filepath)
                    self.classes.append(cls)
                self.current_doc = []
            
            else:
                # Reset doc comments if we hit non-doc, non-definition line
                if line and not line.startswith('//'):
                    self.current_doc = []
            
            i += 1
    
    def parse_function(self, line):
        """Parse function definition"""
        # fn name(params) -> return_type:
        match = re.match(r'fn\s+(\w+)\s*\((.*?)\)(?:\s*->\s*(\w+))?', line)
        if match:
            name = match.group(1)
            params_str = match.group(2)
            return_type = match.group(3) or 'void'
            
            # Parse parameters
            params = []
            if params_str.strip():
                for param in params_str.split(','):
                    param = param.strip()
                    if ':' in param:
                        param_name, param_type = param.split(':', 1)
                        params.append({
                            'name': param_name.strip(),
                            'type': param_type.strip()
                        })
            
            return {
                'name': name,
                'params': params,
                'return_type': return_type,
                'type': 'function'
            }
        return None
    
    def parse_class(self, line):
        """Parse class definition"""
        # class Name:
        match = re.match(r'class\s+(\w+)', line)
        if match:
            return {
                'name': match.group(1),
                'type': 'class',
                'methods': []
            }
        return None
    
    def parse_directory(self, directory):
        """Parse all Sapphire files in a directory"""
        for filepath in Path(directory).rglob('*.spp'):
            self.parse_file(filepath)

class HTMLGenerator:
    """Generate HTML documentation"""
    
    def __init__(self, parser, output_dir='docs'):
        self.parser = parser
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(exist_ok=True)
    
    def generate(self):
        """Generate all documentation"""
        self.generate_index()
        self.generate_functions()
        self.generate_classes()
        self.copy_static_files()
    
    def generate_index(self):
        """Generate index page"""
        html = self.get_template()
        
        content = '<h1>📚 Sapphire Documentation</h1>\n'
        content += '<p>Generated on ' + datetime.now().strftime('%Y-%m-%d %H:%M:%S') + '</p>\n'
        
        # Functions section
        if self.parser.functions:
            content += '<h2>Functions</h2>\n'
            content += '<div class="item-list">\n'
            for func in sorted(self.parser.functions, key=lambda x: x['name']):
                content += f'<div class="item">\n'
                content += f'  <a href="functions.html#{func["name"]}">\n'
                content += f'    <code>{func["name"]}()</code>\n'
                content += f'  </a>\n'
                if func.get('doc'):
                    first_line = func['doc'].split('\n')[0]
                    content += f'  <p>{first_line}</p>\n'
                content += '</div>\n'
            content += '</div>\n'
        
        # Classes section
        if self.parser.classes:
            content += '<h2>Classes</h2>\n'
            content += '<div class="item-list">\n'
            for cls in sorted(self.parser.classes, key=lambda x: x['name']):
                content += f'<div class="item">\n'
                content += f'  <a href="classes.html#{cls["name"]}">\n'
                content += f'    <code>{cls["name"]}</code>\n'
                content += f'  </a>\n'
                if cls.get('doc'):
                    first_line = cls['doc'].split('\n')[0]
                    content += f'  <p>{first_line}</p>\n'
                content += '</div>\n'
            content += '</div>\n'
        
        html = html.replace('{{CONTENT}}', content)
        html = html.replace('{{TITLE}}', 'Sapphire Documentation')
        
        with open(self.output_dir / 'index.html', 'w') as f:
            f.write(html)
    
    def generate_functions(self):
        """Generate functions page"""
        html = self.get_template()
        
        content = '<h1>Functions</h1>\n'
        
        for func in sorted(self.parser.functions, key=lambda x: x['name']):
            content += f'<div class="doc-item" id="{func["name"]}">\n'
            content += f'  <h2>{func["name"]}</h2>\n'
            
            # Signature
            params_str = ', '.join(f'{p["name"]}: {p["type"]}' for p in func['params'])
            content += f'  <div class="signature">\n'
            content += f'    <code>fn {func["name"]}({params_str}) -> {func["return_type"]}</code>\n'
            content += f'  </div>\n'
            
            # Documentation
            if func.get('doc'):
                content += f'  <div class="description">\n'
                content += self.markdown_to_html(func['doc'])
                content += f'  </div>\n'
            
            # Parameters
            if func['params']:
                content += f'  <h3>Parameters</h3>\n'
                content += f'  <ul>\n'
                for param in func['params']:
                    content += f'    <li><code>{param["name"]}</code>: {param["type"]}</li>\n'
                content += f'  </ul>\n'
            
            # Return type
            if func['return_type'] != 'void':
                content += f'  <h3>Returns</h3>\n'
                content += f'  <p><code>{func["return_type"]}</code></p>\n'
            
            # Source file
            content += f'  <div class="source-link">\n'
            content += f'    <small>Defined in <code>{func["file"]}</code></small>\n'
            content += f'  </div>\n'
            
            content += '</div>\n'
        
        html = html.replace('{{CONTENT}}', content)
        html = html.replace('{{TITLE}}', 'Functions - Sapphire Documentation')
        
        with open(self.output_dir / 'functions.html', 'w') as f:
            f.write(html)
    
    def generate_classes(self):
        """Generate classes page"""
        html = self.get_template()
        
        content = '<h1>Classes</h1>\n'
        
        for cls in sorted(self.parser.classes, key=lambda x: x['name']):
            content += f'<div class="doc-item" id="{cls["name"]}">\n'
            content += f'  <h2>{cls["name"]}</h2>\n'
            
            # Documentation
            if cls.get('doc'):
                content += f'  <div class="description">\n'
                content += self.markdown_to_html(cls['doc'])
                content += f'  </div>\n'
            
            # Source file
            content += f'  <div class="source-link">\n'
            content += f'    <small>Defined in <code>{cls["file"]}</code></small>\n'
            content += f'  </div>\n'
            
            content += '</div>\n'
        
        html = html.replace('{{CONTENT}}', content)
        html = html.replace('{{TITLE}}', 'Classes - Sapphire Documentation')
        
        with open(self.output_dir / 'classes.html', 'w') as f:
            f.write(html)
    
    def markdown_to_html(self, text):
        """Convert simple markdown to HTML"""
        html = ''
        in_code_block = False
        
        for line in text.split('\n'):
            # Code blocks
            if line.strip().startswith('```'):
                if in_code_block:
                    html += '</code></pre>\n'
                    in_code_block = False
                else:
                    html += '<pre><code>\n'
                    in_code_block = True
                continue
            
            if in_code_block:
                html += line + '\n'
                continue
            
            # Headers
            if line.startswith('### '):
                html += f'<h4>{line[4:]}</h4>\n'
            elif line.startswith('## '):
                html += f'<h3>{line[3:]}</h3>\n'
            elif line.startswith('# '):
                html += f'<h2>{line[2:]}</h2>\n'
            # Lists
            elif line.strip().startswith('- '):
                html += f'<li>{line.strip()[2:]}</li>\n'
            # Paragraphs
            elif line.strip():
                # Inline code
                line = re.sub(r'`([^`]+)`', r'<code>\1</code>', line)
                html += f'<p>{line}</p>\n'
            else:
                html += '\n'
        
        return html
    
    def get_template(self):
        """Get HTML template"""
        return '''<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{{TITLE}}</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            line-height: 1.6;
            color: #333;
            background: #f5f5f5;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
            background: white;
            min-height: 100vh;
        }
        h1 {
            color: #4A90E2;
            margin-bottom: 20px;
            padding-bottom: 10px;
            border-bottom: 2px solid #4A90E2;
        }
        h2 {
            color: #333;
            margin-top: 30px;
            margin-bottom: 15px;
        }
        h3 {
            color: #666;
            margin-top: 20px;
            margin-bottom: 10px;
            font-size: 1.1em;
        }
        h4 {
            color: #888;
            margin-top: 15px;
            margin-bottom: 8px;
            font-size: 1em;
        }
        .item-list {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
            gap: 15px;
            margin: 20px 0;
        }
        .item {
            border: 1px solid #ddd;
            padding: 15px;
            border-radius: 5px;
            background: #fafafa;
            transition: all 0.2s;
        }
        .item:hover {
            border-color: #4A90E2;
            box-shadow: 0 2px 8px rgba(74, 144, 226, 0.2);
        }
        .item a {
            color: #4A90E2;
            text-decoration: none;
            font-weight: 500;
        }
        .item a:hover {
            text-decoration: underline;
        }
        .item p {
            margin-top: 8px;
            color: #666;
            font-size: 0.9em;
        }
        .doc-item {
            margin: 30px 0;
            padding: 20px;
            border: 1px solid #ddd;
            border-radius: 5px;
            background: #fafafa;
        }
        .signature {
            background: #2d2d2d;
            color: #f8f8f2;
            padding: 15px;
            border-radius: 5px;
            margin: 15px 0;
            overflow-x: auto;
        }
        .signature code {
            color: #f8f8f2;
            font-family: 'Courier New', monospace;
        }
        .description {
            margin: 15px 0;
        }
        .description p {
            margin: 10px 0;
        }
        code {
            background: #f4f4f4;
            padding: 2px 6px;
            border-radius: 3px;
            font-family: 'Courier New', monospace;
            font-size: 0.9em;
            color: #e83e8c;
        }
        pre {
            background: #2d2d2d;
            color: #f8f8f2;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
            margin: 15px 0;
        }
        pre code {
            background: none;
            color: #f8f8f2;
            padding: 0;
        }
        ul {
            margin: 10px 0 10px 30px;
        }
        li {
            margin: 5px 0;
        }
        .source-link {
            margin-top: 15px;
            padding-top: 15px;
            border-top: 1px solid #ddd;
        }
        .source-link small {
            color: #888;
        }
        nav {
            background: #4A90E2;
            padding: 15px 0;
            margin-bottom: 30px;
        }
        nav .container {
            background: transparent;
            padding: 0 20px;
            min-height: auto;
        }
        nav a {
            color: white;
            text-decoration: none;
            margin-right: 20px;
            font-weight: 500;
        }
        nav a:hover {
            text-decoration: underline;
        }
    </style>
</head>
<body>
    <nav>
        <div class="container">
            <a href="index.html">Home</a>
            <a href="functions.html">Functions</a>
            <a href="classes.html">Classes</a>
        </div>
    </nav>
    <div class="container">
        {{CONTENT}}
    </div>
</body>
</html>'''
    
    def copy_static_files(self):
        """Copy any static files (placeholder)"""
        pass

def main():
    if len(sys.argv) < 2:
        print('Usage: sapphire-doc <source-directory> [output-directory]')
        print('\nGenerates HTML documentation from Sapphire source code.')
        print('\nExample:')
        print('  sapphire-doc src/ docs/')
        sys.exit(1)
    
    source_dir = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else 'docs'
    
    if not os.path.exists(source_dir):
        print(f'Error: Source directory not found: {source_dir}')
        sys.exit(1)
    
    print(f'📚 Generating documentation...')
    print(f'   Source: {source_dir}')
    print(f'   Output: {output_dir}')
    
    # Parse source files
    parser = DocParser()
    parser.parse_directory(source_dir)
    
    print(f'   Found {len(parser.functions)} functions')
    print(f'   Found {len(parser.classes)} classes')
    
    # Generate HTML
    generator = HTMLGenerator(parser, output_dir)
    generator.generate()
    
    print(f'✓ Documentation generated in {output_dir}/')
    print(f'   Open {output_dir}/index.html in your browser')

if __name__ == '__main__':
    main()
