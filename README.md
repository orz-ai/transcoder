# 视频转码器 (Transcoder)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Qt Version](https://img.shields.io/badge/Qt-5.15.2+-blue.svg)](https://qt.io/)
[![FFmpeg](https://img.shields.io/badge/FFmpeg-required-red.svg)](https://ffmpeg.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)](https://github.com/)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)](https://github.com/)

一个基于 Qt 和 FFmpeg 构建的强大且用户友好的视频转码应用程序。

[English Documentation](README_EN.md)

## 功能特性

- **批量转码**: 支持批量处理多个视频文件
- **智能文件检测**: 自动检测并跳过已转码的文件
- **进度跟踪**: 实时进度监控，带有可视化状态指示器
- **视频分析**: 内置基于 FFmpeg 的视频信息分析器
- **灵活组织**: 按源目录结构组织输出文件
- **现代界面**: 简洁直观的界面，支持多种主题
- **筛选搜索**: 按转码状态筛选文件
- **多线程处理**: 并发处理以获得更好的性能

## 界面截图

![主界面](screenshots/main_interface.png)
*主界面显示转码队列和状态指示器*

![批量重命名](screenshots/rename.png)
*批量重命名对话框*

![视频分析](screenshots/video_analysis.png)
*视频信息分析对话框*

![设置界面](screenshots/settings.png)
*转码设置配置界面*

## 系统要求

- Windows 10/11
- Qt 5.15.2 或更高版本
- FFmpeg 已安装并可在 PATH 中访问

## 安装说明

1. 从发布页面下载最新版本
2. 将压缩包解压到所需位置
3. 确保 FFmpeg 已安装且可访问
4. 运行 `transcoder.exe`

## 使用方法

1. **选择源目录**: 点击"选择转码目录"选择包含视频文件的文件夹
2. **选择目标目录**: 点击"选择保存目录"设置输出位置
3. **查看文件**: 应用程序将显示所有检测到的视频文件及其状态
4. **开始转码**: 点击"开始转码"开始处理过程
5. **监控进度**: 使用内置进度指示器实时跟踪进度

## 从源码构建

```bash
# 克隆仓库
git clone <repository-url>
cd transcoder

# 使用 qmake 构建
qmake transcoder.pro
make

# 或者使用 Qt Creator 打开 transcoder.pro
```

## 打包分发

```bash
# Windows 部署
windeployqt transcoder.exe

# 或者使用提供的脚本
& 'D:\Qt\5.15.2\msvc2019_64\bin\windeployqt.exe' --release --compiler-runtime --dir 'dist' 'transcoder.exe'
```

## 技术栈

- **前端**: Qt 5.15.2+ (C++)
- **后端**: FFmpeg for video processing
- **UI框架**: Qt Widgets with custom styling
- **多线程**: QThread for concurrent processing
- **配置管理**: JSON-based configuration system

## 贡献指南

欢迎提交问题报告和功能请求！如果您想为项目贡献代码，请：

1. Fork 本仓库
2. 创建您的功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交您的更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开一个 Pull Request

## 许可证

此项目基于 MIT 许可证 - 详情请查看 LICENSE 文件。
