# 简单文本编辑器

Qt Widgets 课程实验二。主窗口使用 Qt Designer 的 `.ui` 文件完成混合式界面设计，编辑区通过控件提升接入派生自 `QPlainTextEdit` 的 `CodeEditor`，实现行号、当前行高亮和扩展编辑功能。

## 功能

- 新建、打开、保存、另存为及未保存关闭提示
- UTF-8 文本文件读写、异常文件和不存在文件提示
- 撤销、重做、剪切、复制、粘贴、全选及 Action 状态联动
- 字体、字体颜色、字体背景色和编辑器背景色设置
- 自动换行、行号显示、工具栏和状态栏显隐
- 查找、连续查找、区分大小写、方向切换、环绕查找
- 替换当前项、全部替换、“未找到”提示和单次可撤销的全部替换
- 定时自动保存（60 秒，仅处理已经指定路径的已修改文件）
- 修改状态自动反映在窗口标题中

## 构建

在 PowerShell 中执行：

```powershell
$env:PATH = "D:\Qt\Tools\mingw1310_64\bin;D:\Qt\6.11.2\mingw_64\bin;" + $env:PATH
& "D:\Qt\Tools\CMake_64\bin\cmake.exe" -S . -B build -G Ninja `
  -DCMAKE_MAKE_PROGRAM=D:/Qt/Tools/Ninja/ninja.exe `
  -DCMAKE_PREFIX_PATH=D:/Qt/6.11.2/mingw_64 `
  -DCMAKE_CXX_COMPILER=D:/Qt/Tools/mingw1310_64/bin/g++.exe `
  -DBUILD_TESTING=ON
& "D:\Qt\Tools\CMake_64\bin\cmake.exe" --build build
```

运行 `build/SimpleTextEditor.exe`。

## 测试

```powershell
& "D:\Qt\Tools\CMake_64\bin\ctest.exe" --test-dir build --output-on-failure
```

也可通过 `BUILD_TESTING` 构建后用 Qt Creator 打开工程，运行 `TextEditorTests` 目标。

## 截图与报告

- `docs/screenshots/`：主界面、菜单、查找、替换和关于对话框截图
- `docs/实验报告.md`：完整实验报告源稿
- `docs/实验2-简单文本编辑器_2026秋_已完成.docx`：按课程模板填写的 Word 报告
- `docs/演示视频脚本.md`：测试视频录制顺序

