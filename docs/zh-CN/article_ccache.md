# CCache - 编译器缓存

> **通过缓存目标文件加速 C/C++ 编译**

## 🎯 什么是 CCache？

CCache（编译器缓存）是一个编译器缓存工具，通过缓存编译结果来加速重新编译。当检测到相同的编译任务时，直接使用缓存而无需重新编译。与缓存整个已构建库的[二进制缓存](article_binary_cache.md)不同，CCache 在编译期间工作于**目标文件级别**。

**主要优势：**
- ⚡ **更快的增量构建** - 重用缓存的目标文件而不是重新编译
- 🔄 **智能失效** - 自动检测源代码和头文件变更
- 🌐 **远程存储** - 通过 HTTP 在团队间共享编译缓存
- 💾 **节省空间** - 可配置的缓存大小，自动清理

## 💡 CCache vs 二进制缓存

| 特性 | CCache | 二进制缓存 |
|---------|--------|--------------|
| **粒度** | 单个目标文件（`.o`） | 完整的构建库 |
| **应用场景** | 增量构建、小改动 | 完全重建依赖 |
| **速度提升** | 重新编译时高 | 完全重建时最高 |
| **存储** | 本地 + 远程 HTTP | 本地目录 |
| **共享方式** | 通过 HTTP 服务器团队共享 | 通过网络文件夹团队共享 |

**何时同时使用两者：**
- **CCache**：在进行小改动时加速日常开发
- **二进制缓存**：在切换分支或设置新环境时消除完全依赖重建

## 🚀 快速开始

### 步骤 1：启用 CCache

在 `celer.toml` 中添加 `[ccache]` 配置：

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

**会发生什么：**
- ✅ CCache 包装您的编译器调用（`gcc`、`g++`、`clang` 等）
- ✅ 编译的目标文件被缓存到指定目录
- ✅ 后续编译未更改的文件时使用缓存结果

### 步骤 2：验证 CCache 是否工作

构建后，检查 CCache 统计信息：

```bash
ccache -s
```

**示例输出：**
```
cache directory                     /home/user/ccache
primary config                      /home/user/ccache/ccache.conf
secondary config      (readonly)    /etc/ccache.conf
cache hit (direct)                  1234
cache hit (preprocessed)             567
cache miss                           89
cache hit rate                     95.29 %
```

## 🌐 远程存储

### 在团队间共享编译缓存

CCache 支持**基于 HTTP 的远程存储**，允许团队通过 Web 服务器共享编译缓存。这对于 CI/CD 环境和分布式开发团队特别有用。

### 设置 HTTP 远程存储

#### 1. 配置 Nginx WebDAV 服务器

创建一个简单的 Nginx 配置作为远程缓存存储：

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

**创建存储目录：**
```bash
sudo mkdir -p /mnt/data/ccache-storage
sudo chown www-data:www-data /mnt/data/ccache-storage
sudo chmod 775 /mnt/data/ccache-storage
```

**启用并重启 Nginx：**
```bash
sudo ln -s /etc/nginx/sites-available/ccache-server /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl restart nginx
```

#### 2. 配置 Celer 使用远程存储

更新您的 `celer.toml`：

```toml
[ccache]
enabled = true
maxsize = "10G"
dir = "/home/user/ccache"
remote_storage = "http://your-server:8080/ccache"
remote_only = false
```

**配置选项：**

- **`remote_storage`**：CCache 远程存储服务器的 HTTP URL
- **`remote_only`**： 
  - `false`（默认）：同时使用本地和远程存储（推荐）
  - `true`：仅使用远程存储，本地目录仅存储元数据

**工作原理：**
1. CCache 首先检查远程存储中的缓存对象
2. 如果找到，下载并使用缓存的目标文件
3. 如果未找到，编译并上传到远程存储
4. 当 `remote_only = false` 时，同时保留本地缓存副本以加快访问速度

### 仅远程模式

当 `remote_only = true` 时，CCache 以**仅远程模式**运行：

```toml
[ccache]
enabled = true
maxsize = "10G"
dir = "/home/user/ccache-metadata"  # 仅存储统计信息和元数据（约 10MB）
remote_storage = "http://your-server:8080/ccache"
remote_only = true
```

**会发生什么：**
- ✅ 所有编译缓存都存储在远程 HTTP 服务器上
- ✅ 本地目录仅包含元数据文件（`stats`、`inode-cache`）
- ✅ 多个开发者立即共享相同的缓存
- ⚠️ 需要与 HTTP 服务器的稳定网络连接

**何时使用仅远程模式：**
- 具有临时构建代理的 CI/CD 流水线
- 基于 Docker 的构建，其中运行之间会丢失本地缓存
- 具有可靠内部网络的开发团队

## ⚙️ 配置参考

### 所有 CCache 选项

```toml
[ccache]
# 启用/禁用 CCache
enabled = true

# 最大缓存大小（支持 M/G 后缀）
maxsize = "10G"

# 本地缓存目录
# - 当 remote_only=false 时：存储完整缓存
# - 当 remote_only=true 时：仅存储元数据
dir = "/home/user/ccache"

# 远程 HTTP 存储 URL（可选）
# 需要兼容 WebDAV 的 HTTP 服务器
remote_storage = "http://ccache-server:8080/ccache"

# 仅远程模式（可选，默认：false）
# - false：同时使用本地和远程缓存
# - true：仅使用远程，本地目录仅用于元数据
remote_only = false
```

### Celer 设置的环境变量

启用 CCache 时，Celer 会自动设置以下环境变量：

- `CCACHE_DIR`：缓存目录路径
- `CCACHE_MAXSIZE`：最大缓存大小
- `CCACHE_BASEDIR`：工作区目录（用于可重定位构建）
- `CCACHE_REMOTE_STORAGE`：远程 HTTP 服务器 URL（如果已配置）
- `CCACHE_REMOTE_ONLY`：如果 remote_only 为 true，则为 `1`

## 📊 监控和维护

### 查看 CCache 统计信息

```bash
# 显示缓存统计信息
ccache -s

# 显示详细配置
ccache --show-config

# 清除缓存统计信息
ccache -z
```

### 清理缓存

```bash
# 清除整个缓存
ccache -C

# 设置最大缓存大小
ccache -M 20G
```

## 🔧 故障排查

### 远程存储连接问题

**测试远程存储可用性：**
```bash
# 应该返回 OK
curl -X GET http://SERVER_IP:8080/health

# 应该返回 201 Created（或 204 No Content）
curl -X PUT -d "test data" http://SERVER_IP:8080/ccache/test.txt

# 应该返回 "test data"
curl http://SERVER_IP:8080/ccache/test.txt
```

### 缓存命中率太低

**可能的原因：**
1. **源文件频繁更改**：活跃开发期间的正常行为
2. **构建选项更改**：不同的编译器标志会使缓存失效
3. **头文件更改**：CCache 在包含的头文件更改时会使缓存失效
4. **缓存太小**：如果缓存驱逐频繁，增加 `maxsize`

**检查缓存统计信息：**
```bash
ccache -s | grep "hit rate"
```
