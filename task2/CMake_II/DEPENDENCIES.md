# CMake_II 依赖关系与模块说明

本文基于 `task2/CMake_II` 现有源码梳理构成、依赖关系与各部分作用，便于编写/核对 CMake 逻辑。

## 目录结构概览

- `server.cpp` / `client.cpp`：顶层可执行程序入口。
- `CMakeLists.txt`：顶层配置（包含 BUILD_A / BUILD_B / BUILD_TESTS 选项与子目录引入）。
- `common/`
  - `rmath/`：数学与几何辅助函数库（依赖 OpenCV Core）。
  - `singleton/`：头文件版全局单例模板。
- `modules/`
  - `assembly1/`：简单类库 + 小测试程序。
  - `assembly2/`：持有 `cv::Point2f` 的类库（依赖 OpenCV Core）。
  - `module1/`：两套实现（A/B），由宏 `WITH_A` / `WITH_B` 切换。
  - `module2/`：OPC UA client/server 封装，内含 `open62541.c`。

## 产物与入口

- 可执行文件：
  - `server`（`server.cpp`）
  - `client`（`client.cpp`）
- 库目标（按目录习惯命名）：
  - `rmath`、`singleton`（头文件为主）、`assembly1`、`assembly2`、`module1`、`module2`
- 测试：
  - `assembly1_test`（`modules/assembly1/test/assembly1_test.cpp`）

## 关键编译选项与宏

- `BUILD_A` / `BUILD_B`：用于选择 `module1` 的 A/B 实现。
  - 需要在 CMake 中分别定义 `WITH_A` 或 `WITH_B` 供 `module1.hpp` 与 `server.cpp` 使用。
- `BUILD_TESTS`：开启后调用 `enable_testing()`，需额外生成 `assembly1_test` 并注册到 `ctest`。

## 模块说明与依赖

### common/rmath

- 作用：数学/几何工具函数与常量（`getDistances`、PnP 相关计算等）。
- 依赖：OpenCV Core（使用 `cv::Point2f`, `cv::Matx` 等）。
- 形式：头文件 + `rmath.cpp`。

### common/singleton

- 作用：`GlobalSingleton<T>` 单例模板。
- 依赖：仅标准库；头文件实现，无需编译单独源文件。

### modules/assembly1

- 作用：最小类库示例 `assembly1`，用于测试。
- 依赖：无外部库。
- 测试：`assembly1_test` 仅构造并打印结果。

### modules/assembly2

- 作用：维护 `cv::Point2f` 的简单组件。
- 依赖：OpenCV Core。

### modules/module1

- 作用：演示 A/B 两种实现，通过编译宏选择。
  - A 版（`module1a`）：四个打印函数，无额外依赖。
  - B 版（`module1b`）：内部持有 `assembly2` 指针，并使用 OpenCV 类型输出。
- 依赖：
  - A 版：无外部依赖。
  - B 版：`assembly2` + OpenCV Core。

### modules/module2 (opcua_cs)

- 作用：OPC UA 客户端/服务器封装（`ua::Client` / `ua::Server`）。
- 组成：
  - C++ 封装源码（`client.cpp` / `server.cpp` / `object.cpp` / `variable.cpp` / `argument.cpp`）。
  - C 语言实现（`open62541.c`）+ 头文件。
- 依赖：
  - `pthread`（顶层 CMake 注释提示必须链接）。
  - 系统网络/线程相关头（由 `open62541.h` 引入）。

## 顶层可执行程序依赖关系

### server

- 直接包含：
  - `robotlab/rmath.h`（`common/rmath`）
  - `robotlab/module1.hpp`（`modules/module1`）
  - `robotlab/opcua_cs.hpp`（`modules/module2`）
  - `robotlab/singleton.hpp`（`common/singleton`）
  - OpenCV Core（`cv::Point2f`）
- 运行行为：
  - 先打印模块输出（A 或 B）。
  - 计算 `getDistances`，将结果写入 OPC UA 变量并启动服务器。

### client

- 直接包含：
  - `robotlab/opcua_cs.hpp`（`modules/module2`）
- 运行行为：
  - 连接 server，读取 `distance` 变量并打印。

## 依赖关系图（简化）

```
server
├─ common/rmath ── OpenCV
├─ common/singleton
├─ module1
│  ├─ (A) module1a
│  └─ (B) module1b ── assembly2 ── OpenCV
└─ module2 (opcua_cs) ── open62541.c ── pthread

client
└─ module2 (opcua_cs) ── open62541.c ── pthread

assembly1_test
└─ assembly1
```

## CMake 编写要点（从依赖推导）

- `OpenCV`：
  - 至少 `rmath`、`assembly2`、`module1b`、`server` 需要 include 与 link。
- `pthread`：
  - `module2` 需要链接（顶层 CMake 注释已提示）。
- `WITH_A` / `WITH_B`：
  - 建议在 CMake 中设置 `target_compile_definitions`。
- `BUILD_TESTS`：
  - `assembly1_test` 需要 `add_test`，运行 `ctest` 输出通过。

如需我补充 CMakeLists 的具体建议或草案，请告诉我你希望采用的目标命名与目录组织方式。
