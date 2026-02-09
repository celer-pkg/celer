# CCache - Compiler Cache

> **Accelerate C/C++ compilation by caching object files**

## 🎯 What is CCache?

CCache (Compiler Cache) is a compiler cache that speeds up recompilation by caching previous compilations and detecting when the same compilation is being done again. Unlike [package cache](article_package_cache.md) which caches entire built libraries, CCache works at the **object file level** during compilation.

**Key Benefits:**
- ⚡ **Faster incremental builds** - Reuse cached object files instead of recompiling
- 🔄 **Smart invalidation** - Automatically detects source code and header changes
- 🌐 **Remote storage** - Share compilation cache across teams via HTTP
- 💾 **Space efficient** - Configurable cache size with automatic cleanup

## 💡 CCache vs Package Cache

| Feature | CCache | Package Cache |
|---------|--------|--------------|
| **Granularity** | Individual object files (`.o`) | Complete built libraries |
| **Use Case** | Incremental builds, small changes | Full dependency rebuilds |
| **Speed Gain** | High for recompilation | Highest for complete rebuilds |
| **Storage** | Local + Remote HTTP | Local directory |
| **Sharing** | Team-wide via HTTP server | Team-wide via network folder |

**When to use both:**
- **CCache**: Speeds up daily development when making small changes
- **Package Cache**: Eliminates full dependency rebuilds when switching branches or setting up new environments

## 🚀 Quick Start

### Step 1: Enable CCache

Add the `[ccache]` section to your `celer.toml`:

```toml
[global]
platform = "aarch64-linux-ubuntu-22.04-gcc-11.5.0"
project = "my_project"
jobs = 6

[ccache]
enabled = true
maxsize = "10G"
dir = "/home/user/ccache"
```

**What happens:**
- ✅ CCache wraps your compiler calls (`gcc`, `g++`, `clang`, etc.)
- ✅ Compiled object files are cached in the specified directory
- ✅ Subsequent compilations of unchanged files use cached results

### Step 2: Verify CCache is Working

After building, check CCache statistics:

```bash
ccache -s
```

**Example output:**
```
cache directory                     /home/user/ccache
primary config                      /home/user/ccache/ccache.conf
secondary config      (readonly)    /etc/ccache.conf
cache hit (direct)                  1234
cache hit (preprocessed)             567
cache miss                           89
cache hit rate                     95.29 %
```

## 🌐 Remote Storage

### Share Compilation Cache Across Your Team

CCache supports **HTTP-based remote storage**, allowing teams to share compilation cache via a web server. This is particularly useful for CI/CD environments and distributed development teams.

### Setup HTTP Remote Storage

#### 1. Configure Nginx WebDAV Server

Create a simple Nginx configuration to serve as the remote cache storage:

```nginx
server {
    listen 8080;
    server_name localhost;

    location /ccache/ {
        alias /mnt/data/ccache-storage/;
        dav_methods PUT DELETE;
        create_full_put_path on;
        client_max_body_size 100M;
        dav_access user:rw group:rw all:r;
        autoindex off;
    }
    
    location /health {
        return 200 "OK\n";
        add_header Content-Type text/plain;
    }
}
```

**Create the storage directory:**
```bash
sudo mkdir -p /mnt/data/ccache-storage
sudo chown www-data:www-data /mnt/data/ccache-storage
sudo chmod 775 /mnt/data/ccache-storage
```

**Enable and restart Nginx:**
```bash
sudo ln -s /etc/nginx/sites-available/ccache-server /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl restart nginx
```

#### 2. Configure Celer to Use Remote Storage

Update your `celer.toml`:

```toml
[ccache]
enabled = true
maxsize = "10G"
dir = "/home/user/ccache"
remote_storage = "http://your-server:8080/ccache"
remote_only = false
```

**Configuration Options:**

- **`remote_storage`**: HTTP URL of your CCache remote storage server
- **`remote_only`**: 
  - `false` (default): Use both local and remote storage (recommended)
  - `true`: Use only remote storage, local dir stores metadata only

**How it works:**
1. CCache first checks the remote storage for cached objects
2. If found, downloads and uses the cached object file
3. If not found, compiles and uploads to remote storage
4. With `remote_only = false`, also keeps a local cache copy for faster access

### Remote-Only Mode

When `remote_only = true`, CCache operates in **remote-only mode**:

```toml
[ccache]
enabled = true
maxsize = "10G"
dir = "/home/user/ccache-metadata"  # Only stores stats and metadata (~10MB)
remote_storage = "http://your-server:8080/ccache"
remote_only = true
```

**What happens:**
- ✅ All compilation cache is stored on the remote HTTP server
- ✅ Local directory only contains metadata files (`stats`, `inode-cache`)
- ✅ Multiple developers share the same cache instantly
- ⚠️ Requires stable network connection to the HTTP server

**When to use remote-only:**
- CI/CD pipelines with ephemeral build agents
- Docker-based builds where local cache is lost between runs
- Development teams with reliable internal network

## ⚙️ Configuration Reference

### All CCache Options

```toml
[ccache]
# Enable/disable CCache
enabled = true

# Maximum cache size (supports M/G suffixes)
maxsize = "10G"

# Local cache directory
# - With remote_only=false: stores full cache
# - With remote_only=true: stores only metadata
dir = "/home/user/ccache"

# Remote HTTP storage URL (optional)
# Requires WebDAV-compatible HTTP server
remote_storage = "http://ccache-server:8080/ccache"

# Remote-only mode (optional, default: false)
# - false: Use both local and remote cache
# - true: Use only remote, local dir for metadata only
remote_only = false
```

### Environment Variables Set by Celer

When CCache is enabled, Celer automatically sets these environment variables:

- `CCACHE_DIR`: Cache directory path
- `CCACHE_MAXSIZE`: Maximum cache size
- `CCACHE_BASEDIR`: Workspace directory (for relocatable builds)
- `CCACHE_REMOTE_STORAGE`: Remote HTTP server URL (if configured)
- `CCACHE_REMOTE_ONLY`: `1` if remote_only is true

## 📊 Monitoring and Maintenance

### View CCache Statistics

```bash
# Show cache statistics
ccache -s

# Show detailed configuration
ccache --show-config

# Clear cache statistics
ccache -z
```

### Clean Up Cache

```bash
# Clear entire cache
ccache -C

# Set maximum cache size
ccache -M 20G
```

## 🔧 Troubleshooting

### Remote Storage Connection Issues

**Test remote storage availability:**
```bash
# Should return OK
curl -X GET http://SERVER_IP:8080/health

# Should return 201 Created (or 204 No Content)
curl -X PUT -d "test data" http://SERVER_IP:8080/ccache/test.txt

# Should return "test data"
curl http://SERVER_IP:8080/ccache/test.txt
```

### Cache Hit Rate Too Low

**Possible causes:**
1. **Source files frequently changing**: Normal behavior during active development
2. **Build options changing**: Different compiler flags invalidate cache
3. **Headers changing**: CCache invalidates when included headers change
4. **Cache too small**: Increase `maxsize` if cache eviction is frequent

**Check cache statistics:**
```bash
ccache -s | grep "hit rate"
```