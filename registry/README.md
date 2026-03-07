# Sapphire Package Registry

A lightweight HTTP server for hosting and distributing Sapphire packages.

## Features

- **Package Publishing**: Upload packages to the registry
- **Package Discovery**: Search for packages by name or description
- **Version Management**: Support for multiple package versions
- **Download Tracking**: Track package download statistics
- **Web Interface**: Browse packages in your browser
- **REST API**: Programmatic access to registry

## Quick Start

### Start the Registry Server

```bash
cd registry
python3 server.py
```

The server will start on `http://localhost:8080`

### Browse Packages

Open your browser to `http://localhost:8080` to see the web interface.

### Search for Packages

```bash
curl http://localhost:8080/api/search?q=http
```

### Get Package Info

```bash
# Get latest version
curl http://localhost:8080/api/package/my-package

# Get specific version
curl http://localhost:8080/api/package/my-package/1.0.0
```

### Download a Package

```bash
curl -O http://localhost:8080/download/my-package/1.0.0
```

## API Reference

### GET /

Web interface for browsing packages.

### GET /api/search?q={query}

Search for packages.

**Response:**
```json
[
  {
    "name": "http-client",
    "version": "1.0.0",
    "description": "HTTP client library",
    "downloads": 42
  }
]
```

### GET /api/package/{name}

Get latest version of a package.

**Response:**
```json
{
  "name": "http-client",
  "version": "1.0.0",
  "description": "HTTP client library",
  "author": "John Doe",
  "license": "MIT",
  "created_at": "2026-03-07T10:00:00",
  "downloads": 42
}
```

### GET /api/package/{name}/{version}

Get specific version of a package.

### GET /api/versions/{name}

Get all versions of a package.

**Response:**
```json
[
  {
    "version": "1.0.0",
    "created_at": "2026-03-07T10:00:00"
  },
  {
    "version": "0.9.0",
    "created_at": "2026-03-01T10:00:00"
  }
]
```

### POST /api/publish

Publish a package to the registry.

**Request:**
```json
{
  "name": "my-package",
  "version": "1.0.0",
  "metadata": {
    "description": "My awesome package",
    "author": "John Doe",
    "license": "MIT",
    "dependencies": {
      "other-package": "^2.0.0"
    }
  },
  "content": "base64-encoded-tarball"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Package my-package@1.0.0 published successfully"
}
```

### GET /download/{name}/{version}

Download a package tarball.

## Database Schema

The registry uses SQLite for metadata storage.

### packages table

| Column | Type | Description |
|--------|------|-------------|
| id | INTEGER | Primary key |
| name | TEXT | Package name |
| version | TEXT | Package version |
| description | TEXT | Package description |
| author | TEXT | Package author |
| license | TEXT | Package license |
| created_at | TEXT | Creation timestamp |
| downloads | INTEGER | Download count |

### dependencies table

| Column | Type | Description |
|--------|------|-------------|
| id | INTEGER | Primary key |
| package_id | INTEGER | Foreign key to packages |
| dependency_name | TEXT | Dependency name |
| dependency_version | TEXT | Dependency version |

## Directory Structure

```
registry/
├── server.py           # Registry server
├── README.md          # This file
├── registry.db        # SQLite database (created on first run)
└── packages/          # Package storage (created on first run)
    └── {package-name}/
        ├── 1.0.0.tar.gz
        └── 1.1.0.tar.gz
```

## Configuration

Edit `server.py` to change configuration:

```python
REGISTRY_DIR = 'packages'  # Package storage directory
DB_PATH = 'registry.db'    # Database file
PORT = 8080                # Server port
```

## Integration with spm

The registry is designed to work with the Sapphire Package Manager (spm).

### Publishing

```bash
spm publish --registry http://localhost:8080
```

### Installing

```bash
spm install http-client --registry http://localhost:8080
```

## Security Considerations

**Note:** This is a development registry. For production use, add:

1. **Authentication**: API keys or OAuth
2. **HTTPS**: TLS encryption
3. **Rate Limiting**: Prevent abuse
4. **Package Signing**: Verify package integrity
5. **Malware Scanning**: Check uploaded packages
6. **Access Control**: Private packages

## Deployment

### Local Development

```bash
python3 server.py
```

### Production (with gunicorn)

```bash
pip install gunicorn
gunicorn -w 4 -b 0.0.0.0:8080 server:app
```

### Docker

```dockerfile
FROM python:3.9-slim
WORKDIR /app
COPY server.py .
RUN mkdir -p packages
EXPOSE 8080
CMD ["python3", "server.py"]
```

## Future Enhancements

- [ ] Package signing and verification
- [ ] User authentication
- [ ] Private packages
- [ ] Package statistics dashboard
- [ ] README rendering
- [ ] Dependency graph visualization
- [ ] Package badges
- [ ] Webhook notifications
- [ ] CDN integration
- [ ] Mirror support

## License

MIT License - See LICENSE file in the Sapphire repository.
