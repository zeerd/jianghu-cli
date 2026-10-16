# jianghu-cli

「金剑江湖」终端客户端。C++17 + CMake，连接
[jianghu-engine](https://github.com/zeerd/jianghu-engine) 服务端
（默认 `127.0.0.1:6600`）。

## 功能

- 彩色终端界面（服务端颜色码 `$RED` 等自动转 ANSI）
- 游戏状态跟踪：房间、出口、战斗标记、属性、技能、物品、在线
- 菜单 / 对话应答（C=27 QUESTION 消息的多级菜单输入）
- 自动登录（配置了账号密码时）、15 秒心跳保活
- 双层命令：`/xxx` 发送服务端命令，`.xxx` 本地命令

## 构建

依赖：CMake ≥ 3.16、C++17 编译器。
跨平台：同一套源码与 CMake 工程支持 Linux 与 Windows。

Linux（POSIX）：

```sh
./build.sh              # 格式化 + 构建 + 运行测试
./build.sh --coverage   # Clang + llvm-cov 覆盖率报告
./build.sh --clean      # 先清理构建目录
```

Windows（MSVC）：

```bat
build.bat               :: 生成 VS 工程 + 构建 + 运行测试
build.bat --clean       :: 先清理构建目录
```

`build.bat` 生成 Visual Studio 解决方案（`build-win/jianghu-cli.slnx`），
也可用 VS 打开后直接编译。GBK↔UTF-8 转码在 Windows 走 Win32 API，
无需 iconv。

通用方式：

```sh
cmake -B build && cmake --build build -j
```

产物：可执行文件 `jianghu`、静态库 `jianghu_lib`、测试
`test_cases`。Windows 产物在 `build-win/Release/`（或 Debug）。

平台差异集中在 `src/compat.h`（socket 类型、控制台编码、主目录、
建目录等），其余源码不写平台 `#ifdef`。

### GTest

- Windows / MSVC：通过 NuGet 引入（`VS_PACKAGE_REFERENCES`）。生成的
  vcxproj 带 `<PackageReference>`，MSBuild 在编译前自动 restore——
  联网时从 nuget.org 下载，离线时用仓库内 `packages/` 目录兜底。
  包内 `.targets` 自动注入 include 目录与 `.lib`，无需手动链接。
- Linux：使用系统安装的 GTest（如 `libgtest-dev`）。

NuGet 源见 `nuget.config`（nuget.org + 本地 `packages/`）。
注意该 GTest 包为静态 CRT（`rt-static`），故顶层 CMakeLists 将
`CMAKE_MSVC_RUNTIME_LIBRARY` 统一设为 `MultiThreaded`。

## 运行

```
./jianghu [-h] [--dbg] [--config file]
```

配置文件按顺序查找：`--config` 指定 → `$CC_JIANGHU_CONFIG` →
` ~/.local/share/jianghu/config` → `./jianghu.conf`。格式为 `key = value`，
`;` 或 `#` 开头为注释：

```ini
host = 127.0.0.1
port = 6600
account = id
password = secret
logs_dir =  ~/.local/share/jianghu/logs
```

配置项说明：

- `host` / `port` — 服务端地址与端口（默认 `127.0.0.1:6600`）
- `account` / `password` — 登录凭据；非空时连接后自动登录
- `logs_dir` — 会话日志目录（默认 ` ~/.local/share/jianghu/logs`），按天
  追加写入 `YYYY-MM-DD.log`，记录界面上显示的输出，目录不
  存在时自动创建

配置了 `account` + `password` 时连接后自动登录；否则进游戏后
用 `/login <账号> <密码>` 登录。

## 使用

- `.help` — 查看全部命令
- `.quit` / `.exit` — 退出
- `.status` `.info` `.inventory` `.skills` — 角色面板
- `.look` `.objects` `.chars` `.doors` — 场景查看
- `/t <id|name>` — 设定目标，`/t 0` 清除
- `/tune [名称]` — 查看 / 切换频道
- `/look` `/get` `/give` `/say` `/tell` `/kill` `/flee` `/buy`
  `/sell` `/balance` … — 服务端命令（详见 `.help`）
- 直接输入文本 = 说话（say）

## 协议

TCP 文本协议：`\t` 分隔、GBK 编码、XOR 混淆，
`&C=N&<键>=<值>&` 消息格式。规范见引擎仓库的
[docs/protocol.md][proto]（见文末链接）。

抓包解密可用自带的 Wireshark 插件：`tools/wireshark-ccjh.lua`。

## 测试

Google Test；测试源码在 `tests/`（`*_test.cc`，配套
`mock_connection.h`）。`build.sh` / `build.bat` 构建后自动运行，
也可直接执行 `build/tests/test_cases`（Windows 为
`build-win\tests\Release\test_cases.exe`），或 `cd build && ctest`。

所有通讯协议相关函数必须有单元测试覆盖。

## 代码风格

- `.clang-format` / `.editorconfig`：4 空格缩进、LF、单行 ≤ 80
  字符、单文件 ≤ 500 行、单函数 ≤ 50 行
- `game_state.cc` 保持框架化，具体实现放 `game_state_*.cc`

## License

MIT，见 [LICENSE](LICENSE)。

[proto]: https://github.com/zeerd/jianghu-engine/blob/main/docs/protocol.md
