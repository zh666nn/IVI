# Qt 智能车载终端系统

基于 Qt5 Widgets 开发的智能车载终端应用，面向嵌入式 Linux 车载信息娱乐场景。项目实现了首页图标导航、视频播放、音乐播放、车内环境检测、天气定位和倒车影像预留页面，适合在 Linux 桌面环境开发调试，并可移植到 i.MX6U / i.MX6ULL 等嵌入式 Linux 平台运行。

## 技术栈

| 方向 | 技术 |
| --- | --- |
| 开发语言 | C++11 |
| 图形界面 | Qt5 Widgets、Qt Designer、`.ui` |
| 页面管理 | `QStackedWidget`、`QToolButton` |
| 定时刷新 | `QTimer`、`QDateTime` |
| 音视频播放 | `QMediaPlayer`、`QVideoWidget`、`QMediaPlaylist` |
| 网络请求 | `QNetworkAccessManager`、`QNetworkReply` |
| 数据解析 | `QJsonDocument`、`QJsonObject`、`QJsonArray` |
| 第三方接口 | 高德天气 API |
| 资源管理 | Qt Resource System、`.qrc` |
| 构建工具 | qmake、Qt Creator |
| 运行平台 | Linux 桌面、嵌入式 Linux |

## 功能模块

### 首页

- 显示当前系统时间，并每秒自动刷新。
- 使用图标入口展示车载终端主要功能。
- 基于 `QStackedWidget` 实现页面切换，不创建额外窗口，适合嵌入式触摸屏应用。

### 视频播放器

- 支持选择本地视频文件。
- 支持播放、暂停、停止。
- 使用 `QVideoWidget` 显示视频画面。
- 支持播放进度显示、进度条拖动跳转和音量调节。

### 音乐播放器

- 支持批量选择本地音频文件。
- 使用播放列表管理歌曲。
- 支持播放、暂停、停止、上一首、下一首。
- 支持播放进度显示、进度条拖动跳转和音量调节。

### 环境检测

- 使用定时器模拟车内环境数据刷新。
- 显示温度、湿度、光照强度和空气质量。
- 根据温度和空气质量阈值显示正常/异常报警状态。
- 该模块后续可替换为串口传感器或板端环境传感器数据。

### 天气定位

- 调用高德天气 API 获取指定城市实时天气。
- 解析 JSON 数据并显示城市、天气、温度、湿度、风向风力和发布时间。
- 当前版本使用城市 `adcode` 查询，避免中文 URL 编码导致的请求问题。
- 在部分 Linux/Qt 环境中，旧版 Qt 与新版 OpenSSL 可能存在 HTTPS 兼容问题，项目使用 HTTP 接口完成天气数据获取。

### 倒车影像

- 当前保留倒车影像功能页面和首页入口。
- 后续可接入 USB 摄像头或 V4L2 视频采集，将 `/dev/video*` 画面显示到页面中。

## 项目结构

```text
IVI
├── IVI.pro
├── main.cpp
├── mainwindow.h
├── mainwindow.cpp
├── mainwindow.ui
└── icons/
    ├── resources.qrc
    ├── video.png
    ├── music.png
    ├── env.png
    ├── weather.png
    └── camera.png
```

## 核心流程

### 页面切换

```text
首页图标按钮
    ├── videoButton   -> videoPage
    ├── musicButton   -> musicPage
    ├── envButton     -> envPage
    ├── weatherButton -> weatherPage
    └── cameraButton  -> cameraPage
```

### 天气请求流程

```text
点击天气定位
    -> QNetworkAccessManager 发起 HTTP 请求
    -> 高德天气 API 返回 JSON
    -> QJsonDocument 解析 lives 数组
    -> 更新天气页面 QLabel
```

### 音视频播放流程

```text
选择本地文件
    -> QMediaPlayer 加载媒体
    -> durationChanged 更新总时长
    -> positionChanged 更新当前进度
    -> QSlider 控制播放位置
```

## 构建运行

### Qt Creator

1. 打开 Qt Creator。
2. 选择 `Open Project`，打开 `IVI.pro`。
3. 选择 Qt 5.x Kit。
4. 执行 `Run qmake`。
5. 构建并运行。

### Linux 命令行

```bash
cd /home/my/Qt/IVI
mkdir -p build
cd build
qmake ../IVI.pro
make -j$(nproc)
./IVI
```

如果使用指定 Qt 版本：

```bash
/path/to/qt/bin/qmake ../IVI.pro
make -j$(nproc)
./IVI
```

## 运行依赖

Qt 模块：

- `core`
- `gui`
- `widgets`
- `network`
- `multimedia`
- `multimediawidgets`

Linux 桌面环境播放音视频通常需要 GStreamer 插件：

```bash
sudo apt install gstreamer1.0-plugins-base \
                 gstreamer1.0-plugins-good \
                 gstreamer1.0-plugins-bad \
                 gstreamer1.0-plugins-ugly \
                 gstreamer1.0-libav
```

## 板端移植说明

该项目主要使用 Qt 应用层接口，适合移植到 i.MX6U / i.MX6ULL 等嵌入式 Linux 平台。板端部署时需要关注：

- 板端 Qt 版本需要包含 Widgets、Network、Multimedia 模块。
- 音视频播放依赖板端 GStreamer 或对应多媒体后端。
- 天气功能需要板端具备网络连接能力。
- 若使用 HTTPS 接口，需要保证 Qt SSL 插件与 OpenSSL 动态库版本匹配。
- 倒车影像功能后续可通过 V4L2 或 OpenCV 接入摄像头。

## 项目亮点

- 使用 `QStackedWidget` 管理多个车载功能页面，页面结构清晰，适合触摸屏终端。
- 基于 `QMediaPlayer` 实现音视频播放，包含播放控制、进度同步、音量调节和播放列表。
- 使用 `QNetworkAccessManager` 异步请求天气接口，完成 JSON 数据解析和界面更新。
- 使用 `QTimer` 完成首页时间刷新和环境数据模拟，体现 Qt 信号槽和事件驱动编程。
- 项目功能覆盖图形界面、音视频、网络通信、JSON 解析、资源管理和嵌入式 Linux 部署。

## 技术要点

- 为什么使用 `QStackedWidget` 做多页面切换，而不是打开多个窗口。
- `QMediaPlayer` 的媒体加载、播放控制和进度同步流程。
- `durationChanged` 和 `positionChanged` 信号在进度条中的作用。
- `QNetworkAccessManager` 的异步请求机制，以及如何解析高德天气 API 返回的 JSON。
- Linux 下 Qt 多媒体播放为什么依赖 GStreamer。
- Qt 与 OpenSSL 版本不匹配导致 HTTPS 请求失败时的排查思路。
- 项目如何从桌面 Linux 移植到 i.MX6U / i.MX6ULL 嵌入式 Linux 平台。
