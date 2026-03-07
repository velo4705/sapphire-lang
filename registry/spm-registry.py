#!/usr/bin/env python3
"""
SPM Registry Client

Extends spm with remote registry support for publishing and installing packages.
"""

import os
import sys
import json
import subprocess
import requests
import tarfile
import tempfile
import shutil
from pathlib import Path

DEFAULT_REGISTRY = 'http://localhost:8080'

def read_spm_toml():
    """Read spm.toml file"""
    if not os.path.exists('spm.toml'):
        print('Error: spm.toml not found')
        sys.exit(1)
    
    with open('spm.toml', 'r') as f:
        content = f.read()
    
    # Simple TOML parsing (just extract key values)
    metadata = {}
    for line in content.split('\n'):
        if '=' in line:
            key, value = line.split('=', 1)
            key = key.strip()
            value = value.strip().strip('"')
            metadata[key] = value
    
    return metadata

def create_package_tarball():
    """Create a tarball of the package"""
    with tempfile.NamedTemporaryFile(suffix='.tar.gz', delete=False) as tmp:
        with tarfile.open(tmp.name, 'w:gz') as tar:
            # Add source files
            if os.path.exists('src'):
                tar.add('src', arcname='src')
            
            # Add spm.toml
            if os.path.exists('spm.toml'):
                tar.add('spm.toml')
            
            # Add README if exists
            if os.path.exists('README.md'):
                tar.add('README.md')
        
        return tmp.name

def publish_package(registry_url=DEFAULT_REGISTRY):
    """Publish package to registry"""
    print('📦 Publishing package to registry...')
    
    # Read metadata
    metadata = read_spm_toml()
    name = metadata.get('name', 'unknown')
    version = metadata.get('version', '0.1.0')
    
    print(f'Package: {name}@{version}')
    
    # Create tarball
    print('Creating package tarball...')
    tarball_path = create_package_tarball()
    
    try:
        # Read tarball content
        with open(tarball_path, 'rb') as f:
            content = f.read()
        
        # Prepare request
        data = {
            'name': name,
            'version': version,
            'metadata': {
                'description': metadata.get('description', ''),
                'author': metadata.get('author', ''),
                'license': metadata.get('license', 'MIT'),
                'dependencies': {}
            },
            'content': content.hex()  # Send as hex string
        }
        
        # Publish to registry
        print(f'Uploading to {registry_url}...')
        response = requests.post(f'{registry_url}/api/publish', json=data)
        
        if response.status_code == 200:
            result = response.json()
            print(f'✓ {result["message"]}')
        else:
            error = response.json()
            print(f'✗ Error: {error.get("error", "Unknown error")}')
            sys.exit(1)
    
    finally:
        # Clean up tarball
        os.unlink(tarball_path)

def install_package(package_spec, registry_url=DEFAULT_REGISTRY):
    """Install package from registry"""
    # Parse package spec (name or name@version)
    if '@' in package_spec:
        name, version = package_spec.split('@', 1)
    else:
        name = package_spec
        version = None
    
    print(f'📥 Installing {name}...')
    
    # Get package info
    if version:
        url = f'{registry_url}/api/package/{name}/{version}'
    else:
        url = f'{registry_url}/api/package/{name}'
    
    response = requests.get(url)
    if response.status_code != 200:
        print(f'✗ Package not found: {name}')
        sys.exit(1)
    
    package_info = response.json()
    version = package_info['version']
    
    print(f'Found {name}@{version}')
    
    # Download package
    download_url = f'{registry_url}/download/{name}/{version}'
    response = requests.get(download_url)
    
    if response.status_code != 200:
        print(f'✗ Failed to download package')
        sys.exit(1)
    
    # Extract to dependencies directory
    deps_dir = Path('dependencies') / name
    deps_dir.mkdir(parents=True, exist_ok=True)
    
    # Save and extract tarball
    with tempfile.NamedTemporaryFile(suffix='.tar.gz', delete=False) as tmp:
        tmp.write(response.content)
        tmp_path = tmp.name
    
    try:
        with tarfile.open(tmp_path, 'r:gz') as tar:
            tar.extractall(deps_dir)
        
        print(f'✓ Installed {name}@{version} to dependencies/{name}')
    finally:
        os.unlink(tmp_path)

def search_packages(query, registry_url=DEFAULT_REGISTRY):
    """Search for packages"""
    print(f'🔍 Searching for "{query}"...\n')
    
    response = requests.get(f'{registry_url}/api/search', params={'q': query})
    
    if response.status_code != 200:
        print('✗ Search failed')
        sys.exit(1)
    
    results = response.json()
    
    if not results:
        print('No packages found')
        return
    
    for pkg in results:
        print(f"📦 {pkg['name']} v{pkg['version']}")
        if pkg.get('description'):
            print(f"   {pkg['description']}")
        print(f"   📥 {pkg['downloads']} downloads\n")

def show_package_info(name, registry_url=DEFAULT_REGISTRY):
    """Show package information"""
    response = requests.get(f'{registry_url}/api/package/{name}')
    
    if response.status_code != 200:
        print(f'✗ Package not found: {name}')
        sys.exit(1)
    
    pkg = response.json()
    
    print(f"\n📦 {pkg['name']} v{pkg['version']}")
    print(f"   {pkg.get('description', 'No description')}")
    print(f"\n   Author: {pkg.get('author', 'Unknown')}")
    print(f"   License: {pkg.get('license', 'Unknown')}")
    print(f"   Downloads: {pkg['downloads']}")
    print(f"   Published: {pkg['created_at']}\n")
    
    # Get versions
    response = requests.get(f'{registry_url}/api/versions/{name}')
    if response.status_code == 200:
        versions = response.json()
        print(f"   Versions: {', '.join(v['version'] for v in versions)}\n")

def main():
    if len(sys.argv) < 2:
        print('Usage: spm-registry <command> [options]')
        print('\nCommands:')
        print('  publish [--registry URL]       Publish package to registry')
        print('  install <package> [--registry URL]  Install package from registry')
        print('  search <query> [--registry URL]     Search for packages')
        print('  info <package> [--registry URL]     Show package information')
        sys.exit(1)
    
    command = sys.argv[1]
    args = sys.argv[2:]
    
    # Extract registry URL if provided
    registry_url = DEFAULT_REGISTRY
    if '--registry' in args:
        idx = args.index('--registry')
        if idx + 1 < len(args):
            registry_url = args[idx + 1]
            args = args[:idx] + args[idx+2:]
    
    try:
        if command == 'publish':
            publish_package(registry_url)
        elif command == 'install':
            if not args:
                print('Error: package name required')
                sys.exit(1)
            install_package(args[0], registry_url)
        elif command == 'search':
            if not args:
                print('Error: search query required')
                sys.exit(1)
            search_packages(args[0], registry_url)
        elif command == 'info':
            if not args:
                print('Error: package name required')
                sys.exit(1)
            show_package_info(args[0], registry_url)
        else:
            print(f'Unknown command: {command}')
            sys.exit(1)
    except requests.exceptions.ConnectionError:
        print(f'✗ Could not connect to registry at {registry_url}')
        print('  Make sure the registry server is running')
        sys.exit(1)
    except Exception as e:
        print(f'✗ Error: {e}')
        sys.exit(1)

if __name__ == '__main__':
    main()
