# Mars SDK

Mars SDK 支持 Windows 和 Linux 双平台编译，编译脚本会自动下载所需的工具链，可通过打包平台->工具->MARS烧录进行固件烧录。

**工具链独立于 SDK 目录存放**，重新下载 SDK 后无需重复下载工具链。

---

## 工具链配置

工具链默认安装位置：

| 平台 | 默认路径 | 配置方式 |
|------|----------|----------|
| Windows | `D:\mars-toolchain` | 修改 `build.bat` 中的 `TOOLCHAIN_ROOT` 变量 |
| Linux | `~/mars-toolchain` | 修改 `build.sh` 中的 `TOOLCHAIN_ROOT` 变量 |

如需修改工具链安装位置，请编辑脚本开头的配置：

**Windows (build.bat)**
```batch
:: 【开发者配置】工具链安装目录，可根据需要修改
set TOOLCHAIN_ROOT=D:\mars-toolchain
```

**Linux (build.sh)**
```bash
# 【开发者配置】工具链安装目录，可根据需要修改
TOOLCHAIN_ROOT="$HOME/mars-toolchain"
```

---

## 详细说明

### 1. Windows 编译

#### 环境要求

- Windows 10/11
- PowerShell（用于下载和解压工具链）

#### 使用方法

```batch
:: 清理 build 目录并重新编译
build.bat -r all
```

#### 自动下载的工具

| 工具 | 说明 |
|------|------|
| RISC-V GCC | Nuclei 交叉编译工具链 |
| Python 3.12 | 嵌入式版本，用于脚本执行 |
| CMake 3.28 | 构建系统 |
| Ninja 1.11 | 构建工具 |
| pyelftools | Python 依赖包 |

#### 注意事项

1. **首次编译较慢**：需要下载约 1GB 的工具链文件和其他工具
2. **网络要求**：需要能访问 GitHub 和 nucleisys.com
3. **杀毒软件**：如果下载失败，请检查杀毒软件是否拦截了 PowerShell 网络请求
4. **路径问题**：项目路径中不要包含中文或特殊字符

---

### 2. Linux 编译

#### 环境要求

- Ubuntu 22.04 或其他 Linux 发行版
- 基础工具：curl 或 wget、tar、unzip

#### 使用方法

```bash
# 清理 build 目录并重新编译
./build.sh -r all
```

#### 系统依赖

如果系统已安装以下工具，脚本会优先使用系统版本：

```bash
# Ubuntu/Debian
sudo apt-get install cmake ninja-build python3 python3-pip

# 安装 Python 依赖
pip3 install pyelftools kconfiglib
```

#### 自动下载的工具

| 工具 | 说明 |
|------|------|
| RISC-V GCC | Nuclei 交叉编译工具链（必须下载） |
| CMake | 如果系统未安装则自动下载 |
| Ninja | 如果系统未安装则自动下载 |

#### 注意事项

1. **Python3 必须预装**：脚本不会自动安装 Python3，请确保系统已安装
2. **网络代理**：如果下载失败，请检查网络代理设置

---

## 目录结构

### SDK 目录
```
mars-sdk/
|-- build.bat                # Windows 编译脚本
|-- build.sh                 # Linux 编译脚本
|-- build/                   # 编译输出目录
|   +-- bin/
|       |-- app.elf          # ELF 文件
|       |-- app.bin          # 二进制固件
|       +-- app.img          # 打包后的镜像
|-- source/                  # 源代码目录
+-- ...
```

### 工具链目录（独立于 SDK）

**Windows (D:\mars-toolchain)**
```
D:\mars-toolchain\
|-- gcc/                     # RISC-V GCC 工具链
|-- python/                  # Python 嵌入式版本
|-- cmake-3.28.0-xxx/        # CMake
+-- ninja-win/               # Ninja
```

**Linux (~/mars-toolchain)**
```
~/mars-toolchain/
|-- gcc/                     # RISC-V GCC 工具链
|-- cmake-3.28.0-xxx/        # CMake（可选）
+-- ninja/                   # Ninja（可选）
```

---

## SDK 开发说明

1. 应用逻辑部分重点关注source/listen/app下: ls_app_client.c、ls_app_cfg_mgr.x
2. 命令词和协议重点关注source/listen/app下：ls_app_cfg_def.h、ls_app_cfg_user.h
3. 提示音相关重点关注source/listen/tone下：tone.h、tone_buf.h
4. 算法资源若重新生成，需替换路径为tools/respack/algo下资源，并执行auto_respak.sh重新打包算法资源
5. 平台打包时会自动更新mars sdk以下文件：
   - `source/listen/tone` 下：tone.h、tone_buf.h（提示音文件）
   - `source/listen/app` 下：ls_app_cfg_user.h（命令词和协议配置等）
   - `tools/respack/algo` 下：main.bin、cmds.bin和mlp.bin等（算法资源文件）
   - `libraries` 下：libesr.a（指定芯片的算法库文件）
---

## 手动下载工具链

如果自动下载失败（网络问题、防火墙拦截等），可以手动下载工具链：

### 备用下载地址

| 平台 | 下载链接 |
|------|----------|
| Windows | https://listenai-firmware-delivery.oss-cn-beijing.aliyuncs.com/Mars/toolchain/mars-toolchain.7z |
| Linux | https://listenai-firmware-delivery.oss-cn-beijing.aliyuncs.com/Mars/toolchain/mars-toolchain.tar.gz |

### 手动安装步骤

**Windows:**
1. 下载 `mars-toolchain.7z`
2. 使用 7-Zip 解压到 `D:\mars-toolchain\`（或你配置的 `TOOLCHAIN_ROOT` 路径）
3. 确保解压后目录结构为：`D:\mars-toolchain\gcc\`、`D:\mars-toolchain\python\` 等
4. 运行 `build.bat -r all` 开始编译

**Linux:**
1. 下载 `mars-toolchain.tar.gz`
2. 解压到 `~/mars-toolchain/`（或你配置的 `TOOLCHAIN_ROOT` 路径）
   ```bash
   mkdir -p ~/mars-toolchain
   tar -xzf mars-toolchain.tar.gz -C ~/mars-toolchain
   ```
3. 确保解压后目录结构为：`~/mars-toolchain/gcc/` 等
4. 运行 `./build.sh -r all` 开始编译

---

## 常见问题

### Q: Windows 下载工具链失败？

A: 检查以下几点：
1. 确保网络可以访问 GitHub 和 nucleisys.com
2. 关闭或配置杀毒软件白名单
3. 尝试使用管理员权限运行 CMD
4. **使用备用下载地址手动下载**（见上方"手动下载工具链"章节）

### Q: Linux 下载工具链失败？

A: 检查网络代理设置，或**使用备用下载地址手动下载**（见上方"手动下载工具链"章节）

### Q: Linux 编译报错 "Python3 is not installed"？

A: 安装 Python3：
```bash
sudo apt-get install python3 python3-pip
```

### Q: 编译成功但没有生成 app.img？

A: 检查 tools/bin/ 目录下是否存在打包工具：
- Windows: csk5060-enc-pack.exe
- Linux: csk5060-enc-pack

### Q: 如何更换工具链安装位置？

A: 修改脚本开头的 `TOOLCHAIN_ROOT` 变量即可。
