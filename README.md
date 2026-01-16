# File Version Manager

---

## English

A lightweight file version management tool built with **Qt (C++)**.

### Features
- Automatic file versioning based on SHA-256 content hash
- Hash-based version storage with metadata management
- Tree view (File → Versions) using Qt Model/View
- Current version auto-detection and highlighting
- Safe rollback with atomic file replacement
- Rollback confirmation with diff preview:
  - Hash comparison
  - File size comparison
  - Timestamp comparison
- Rollback is disabled when target version equals current version

### Tech Stack
- Qt 5 / C++17
- QAbstractItemModel
- File system watcher
- JSON metadata storage

---

## 中文说明

基于 **Qt / C++** 实现的轻量级文件版本管理工具。

### 功能特性
- 基于 SHA-256 的自动文件版本生成
- 使用 hash 作为版本标识的内容存储
- Qt Model/View 实现的文件-版本树形结构
- 自动识别并高亮当前文件版本
- 原子操作保证的安全回滚
- 回滚前差异预览：
  - Hash 对比
  - 文件大小对比
  - 修改时间对比
- 当前版本与目标版本一致时禁止回滚

### 技术要点
- Qt 5 / C++17
- QAbstractItemModel
- 文件系统监听
- JSON 元数据管理

---

## License
MIT
