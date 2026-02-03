# 项目编译与运行指北
注：git-auto-commit.sh是自动提交脚本，会将COMMITLOG中的信息自动添加到CHANGELOG并commit或push到远程（可选）。项目的修改记录可以在CHANGELOG中看见。
## 一键编译（Task1 ~ Task3）
```bash
./build_all.sh
```
可选：指定并行编译线程数（默认自动探测）
```bash
JOBS=6 ./build_all.sh
```

## 依赖准备（简要）
- OpenCV：Task1 / Task2 / Task3 需要
- OpenVINO：Task3 需要（含 Runtime + TBB）

常见 OpenVINO 环境加载：
```bash
source /opt/intel/openvino_2025/setupvars.sh
# 或
source /opt/intel/openvino_2025.4.0/setupvars.sh
```

## Task1 运行
编译产物：`task1/build/task1`

示例（视频输入）：
```bash
./task1/build/task1 task1/origin.avi
```
或：
```bash
./task1/build/task1 task1/装甲板.avi
```

## Task2 运行
Task2 有两个子工程：`CMake_I` 与 `CMake_II`。

```bash
./task2/CMake_I/build/test
```

`CMake_II` 包含服务端与客户端（建议两个终端）：
```bash
./task2/CMake_II/build/server
```
```bash
./task2/CMake_II/build/client
```

## Task3 运行（OpenVINO 推理）
编译产物：`task3/build/Robot_Vison_Open_Vino_demo`

通用用法：
```bash
./task3/build/Robot_Vison_Open_Vino_demo <video_path|0> [model.onnx] [device] [detect_color] [conf] [nms]
```

示例（ONNX + CPU + 全颜色）：
```bash
./task3/build/Robot_Vison_Open_Vino_demo /home/harekasa/下载/装甲板.avi task3/Model/0526.onnx CPU -1 0.008 0.25
```

参数说明：
- `video_path|0`：视频路径或 `0`（摄像头）
- `model.onnx`：模型路径（默认 `task3/Model/0526.onnx`）
- `device`：`CPU/GPU/NPU...`
- `detect_color`：`-1` 全部，`0` 蓝，`1` 红
- `conf`：置信度阈值
- `nms`：NMS 阈值