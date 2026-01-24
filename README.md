# FileVersionManager

A lightweight file version control tool based on Qt and C++, designed for local file version tracking, safe rollback, and change inspection.

---

## English

### Overview

FileVersionManager is a desktop application built with Qt (C++), focusing on local file version management.
It monitors a workspace directory, automatically creates file versions on changes, and allows users to safely rollback with clear confirmation and diff information.

This project is intended as a complete, self-developed C++/Qt project demonstrating architecture design, model-view usage, file I/O safety, and user-oriented UX.

### Implemented Features

- Workspace selection on startup
- Recursive file monitoring
- Automatic version creation on file changes
- Version tree view (Model/View architecture)
- Current version highlighting
- Safe rollback mechanism
- Rollback confirmation with version diff:
  - File size comparison
  - Last modified time
  - Hash equality check
- Clear rollback result dialog:
  - File name
  - Target version short hash
- Explicit rollback failure reasons:
  - Version missing
  - I/O failure
  - Write or rename failure
- Logging system bound to workspace directory
- Automatic creation of initial version for existing files

### Branch Strategy

- dev: active development branch
- release: stable release branch
- main: project entry branch

### Build Environment

- Qt 5.12.x
- C++17
- Windows

### Packaging

```
windeployqt FileVersionManager.exe
```

---

## 中文说明

### 项目简介

FileVersionManager 是一个基于 Qt 和 C++ 的轻量级本地文件版本管理工具。
用于监控工作目录中的文件变化，自动生成版本，并提供安全、可确认的回滚能力。

该项目用于展示 C++ 工程能力、Qt 架构设计、文件系统处理以及完整的用户交互流程。

### 已实现功能

- 启动时选择工作目录
- 文件变化监控
- 自动版本生成
- 版本树展示
- 当前版本高亮
- 安全回滚机制
- 回滚前差异确认
- 回滚成功与失败提示
- 日志系统（随工作目录变化）
- 已有文件自动生成初始版本

### 分支说明

- dev：开发分支
- release：发布分支
- main：入口分支

### 编译与打包

使用 Qt 官方工具：

```
windeployqt FileVersionManager.exe
```
