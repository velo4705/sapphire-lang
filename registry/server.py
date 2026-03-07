#!/usr/bin/env python3
"""
Sapphire Package Registry Server

A simple HTTP server for hosting Sapphire packages.
Supports package upload, download, search, and version management.
"""

import os
import json
import hashlib
import shutil
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import urlparse, parse_qs
from datetime import datetime
import sqlite3

# Configuration
REGISTRY_DIR = os.path.join(os.path.dirname(__file__), 'packages')
DB_PATH = os.path.join(os.path.dirname(__file__), 'registry.db')
PORT = 8080

# Ensure directories exist
os.makedirs(REGISTRY_DIR, exist_ok=True)

class RegistryDB:
    """Database for package metadata"""
    
    def __init__(self, db_path):
        self.db_path = db_path
        self.init_db()
    
    def init_db(self):
        """Initialize database schema"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Packages table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS packages (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT NOT NULL,
                version TEXT NOT NULL,
                description TEXT,
                author TEXT,
                license TEXT,
                created_at TEXT NOT NULL,
                downloads INTEGER DEFAULT 0,
                UNIQUE(name, version)
            )
        ''')
        
        # Dependencies table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS dependencies (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                package_id INTEGER NOT NULL,
                dependency_name TEXT NOT NULL,
                dependency_version TEXT NOT NULL,
                FOREIGN KEY (package_id) REFERENCES packages(id)
            )
        ''')
        
        conn.commit()
        conn.close()
    
    def add_package(self, name, version, metadata):
        """Add a package to the registry"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        try:
            cursor.execute('''
                INSERT INTO packages (name, version, description, author, license, created_at)
                VALUES (?, ?, ?, ?, ?, ?)
            ''', (
                name,
                version,
                metadata.get('description', ''),
                metadata.get('author', ''),
                metadata.get('license', ''),
                datetime.now().isoformat()
            ))
            
            package_id = cursor.lastrowid
            
            # Add dependencies
            for dep_name, dep_version in metadata.get('dependencies', {}).items():
                cursor.execute('''
                    INSERT INTO dependencies (package_id, dependency_name, dependency_version)
                    VALUES (?, ?, ?)
                ''', (package_id, dep_name, dep_version))
            
            conn.commit()
            return True
        except sqlite3.IntegrityError:
            return False
        finally:
            conn.close()
    
    def get_package(self, name, version=None):
        """Get package metadata"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        if version:
            cursor.execute('''
                SELECT name, version, description, author, license, created_at, downloads
                FROM packages
                WHERE name = ? AND version = ?
            ''', (name, version))
        else:
            cursor.execute('''
                SELECT name, version, description, author, license, created_at, downloads
                FROM packages
                WHERE name = ?
                ORDER BY created_at DESC
                LIMIT 1
            ''', (name,))
        
        row = cursor.fetchone()
        conn.close()
        
        if row:
            return {
                'name': row[0],
                'version': row[1],
                'description': row[2],
                'author': row[3],
                'license': row[4],
                'created_at': row[5],
                'downloads': row[6]
            }
        return None
    
    def get_versions(self, name):
        """Get all versions of a package"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            SELECT version, created_at
            FROM packages
            WHERE name = ?
            ORDER BY created_at DESC
        ''', (name,))
        
        versions = [{'version': row[0], 'created_at': row[1]} for row in cursor.fetchall()]
        conn.close()
        return versions
    
    def search_packages(self, query):
        """Search for packages"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            SELECT DISTINCT name, MAX(version) as version, description, downloads
            FROM packages
            WHERE name LIKE ? OR description LIKE ?
            GROUP BY name
            ORDER BY downloads DESC
            LIMIT 50
        ''', (f'%{query}%', f'%{query}%'))
        
        results = []
        for row in cursor.fetchall():
            results.append({
                'name': row[0],
                'version': row[1],
                'description': row[2],
                'downloads': row[3]
            })
        
        conn.close()
        return results
    
    def increment_downloads(self, name, version):
        """Increment download counter"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            UPDATE packages
            SET downloads = downloads + 1
            WHERE name = ? AND version = ?
        ''', (name, version))
        
        conn.commit()
        conn.close()

class RegistryHandler(BaseHTTPRequestHandler):
    """HTTP request handler for package registry"""
    
    db = RegistryDB(DB_PATH)
    
    def do_GET(self):
        """Handle GET requests"""
        parsed = urlparse(self.path)
        path = parsed.path
        query = parse_qs(parsed.query)
        
        if path == '/':
            self.send_html_response(self.get_index_html())
        
        elif path == '/api/search':
            q = query.get('q', [''])[0]
            results = self.db.search_packages(q)
            self.send_json_response(results)
        
        elif path.startswith('/api/package/'):
            # /api/package/{name} or /api/package/{name}/{version}
            parts = path.split('/')[3:]
            if len(parts) == 1:
                # Get latest version
                package = self.db.get_package(parts[0])
                if package:
                    self.send_json_response(package)
                else:
                    self.send_error_response(404, 'Package not found')
            elif len(parts) == 2:
                # Get specific version
                package = self.db.get_package(parts[0], parts[1])
                if package:
                    self.send_json_response(package)
                else:
                    self.send_error_response(404, 'Package version not found')
        
        elif path.startswith('/api/versions/'):
            # /api/versions/{name}
            name = path.split('/')[3]
            versions = self.db.get_versions(name)
            self.send_json_response(versions)
        
        elif path.startswith('/download/'):
            # /download/{name}/{version}
            parts = path.split('/')[2:]
            if len(parts) == 2:
                self.download_package(parts[0], parts[1])
            else:
                self.send_error_response(400, 'Invalid download path')
        
        else:
            self.send_error_response(404, 'Not found')
    
    def do_POST(self):
        """Handle POST requests"""
        if self.path == '/api/publish':
            self.publish_package()
        else:
            self.send_error_response(404, 'Not found')
    
    def publish_package(self):
        """Handle package publication"""
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        
        try:
            data = json.loads(post_data.decode('utf-8'))
            name = data.get('name')
            version = data.get('version')
            metadata = data.get('metadata', {})
            content = data.get('content', '')
            
            if not name or not version:
                self.send_error_response(400, 'Missing name or version')
                return
            
            # Save package file
            package_dir = os.path.join(REGISTRY_DIR, name)
            os.makedirs(package_dir, exist_ok=True)
            
            package_file = os.path.join(package_dir, f'{version}.tar.gz')
            with open(package_file, 'w') as f:
                f.write(content)
            
            # Add to database
            if self.db.add_package(name, version, metadata):
                self.send_json_response({
                    'success': True,
                    'message': f'Package {name}@{version} published successfully'
                })
            else:
                self.send_error_response(409, 'Package version already exists')
        
        except Exception as e:
            self.send_error_response(500, str(e))
    
    def download_package(self, name, version):
        """Handle package download"""
        package_file = os.path.join(REGISTRY_DIR, name, f'{version}.tar.gz')
        
        if os.path.exists(package_file):
            self.db.increment_downloads(name, version)
            
            with open(package_file, 'rb') as f:
                content = f.read()
            
            self.send_response(200)
            self.send_header('Content-Type', 'application/gzip')
            self.send_header('Content-Length', len(content))
            self.end_headers()
            self.wfile.write(content)
        else:
            self.send_error_response(404, 'Package file not found')
    
    def send_json_response(self, data):
        """Send JSON response"""
        response = json.dumps(data, indent=2)
        self.send_response(200)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', len(response))
        self.end_headers()
        self.wfile.write(response.encode('utf-8'))
    
    def send_html_response(self, html):
        """Send HTML response"""
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.send_header('Content-Length', len(html))
        self.end_headers()
        self.wfile.write(html.encode('utf-8'))
    
    def send_error_response(self, code, message):
        """Send error response"""
        response = json.dumps({'error': message})
        self.send_response(code)
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', len(response))
        self.end_headers()
        self.wfile.write(response.encode('utf-8'))
    
    def get_index_html(self):
        """Get index page HTML"""
        return '''
<!DOCTYPE html>
<html>
<head>
    <title>Sapphire Package Registry</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 1200px; margin: 0 auto; padding: 20px; }
        h1 { color: #4A90E2; }
        .search-box { width: 100%; padding: 10px; font-size: 16px; margin: 20px 0; }
        .package { border: 1px solid #ddd; padding: 15px; margin: 10px 0; border-radius: 5px; }
        .package h3 { margin: 0 0 10px 0; color: #333; }
        .package .version { color: #666; font-size: 14px; }
        .package .description { color: #888; margin: 10px 0; }
        .package .downloads { color: #4A90E2; font-size: 12px; }
    </style>
</head>
<body>
    <h1>💎 Sapphire Package Registry</h1>
    <p>Discover and share Sapphire packages</p>
    
    <input type="text" class="search-box" id="search" placeholder="Search packages...">
    
    <div id="results"></div>
    
    <script>
        const searchBox = document.getElementById('search');
        const resultsDiv = document.getElementById('results');
        
        searchBox.addEventListener('input', async (e) => {
            const query = e.target.value;
            if (query.length < 2) {
                resultsDiv.innerHTML = '';
                return;
            }
            
            const response = await fetch(`/api/search?q=${encodeURIComponent(query)}`);
            const packages = await response.json();
            
            resultsDiv.innerHTML = packages.map(pkg => `
                <div class="package">
                    <h3>${pkg.name}</h3>
                    <div class="version">v${pkg.version}</div>
                    <div class="description">${pkg.description || 'No description'}</div>
                    <div class="downloads">📥 ${pkg.downloads} downloads</div>
                </div>
            `).join('');
        });
    </script>
</body>
</html>
        '''
    
    def log_message(self, format, *args):
        """Override to customize logging"""
        print(f"[{datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] {format % args}")

def run_server(port=PORT):
    """Run the registry server"""
    server_address = ('', port)
    httpd = HTTPServer(server_address, RegistryHandler)
    print(f'Sapphire Package Registry running on http://localhost:{port}')
    print(f'Database: {DB_PATH}')
    print(f'Packages: {REGISTRY_DIR}')
    print('Press Ctrl+C to stop')
    
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print('\nShutting down server...')
        httpd.shutdown()

if __name__ == '__main__':
    run_server()
