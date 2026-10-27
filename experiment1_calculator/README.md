# 带键盘事件的计算器

Qt Widgets 课程实验一。项目使用 Qt Designer 生成的 `.ui` 文件完成界面布局，
鼠标按钮与键盘事件共用同一套输入处理逻辑。

## 功能

- 加、减、乘、除和连续运算
- 小数输入与重复小数点过滤
- 退格、全部清除
- 除零和结果溢出保护
- 键盘数字、四则运算符、小数点、回车、Esc 和 Backspace 输入

## 构建

```powershell
$env:PATH = "D:\Qt\Tools\mingw1310_64\bin;D:\Qt\6.11.2\mingw_64\bin;" + $env:PATH
cmake -S . -B build -G Ninja `
  -DCMAKE_MAKE_PROGRAM=D:/Qt/Tools/Ninja/ninja.exe `
  -DCMAKE_PREFIX_PATH=D:/Qt/6.11.2/mingw_64 `
  -DCMAKE_CXX_COMPILER=D:/Qt/Tools/mingw1310_64/bin/g++.exe
cmake --build build
```

运行 `build/KeyboardCalculator.exe`。在项目根目录执行 `ctest --test-dir build --output-on-failure`
可运行自动化测试。

## 实验材料

- [Markdown实验报告](docs/实验报告.md)
- [Word实验报告](docs/实验1-带键盘事件的计算器-完成版.docx)
- [界面与运行截图](docs/screenshots)
- [2～3分钟演示视频脚本](docs/演示视频脚本.md)
- [报告生成脚本](tools/build_report.py)
- [截图生成工具](tools/capture_screenshots.cpp)

报告封面中的姓名、学号、班级、实验地点和指导老师因未提供个人信息而保留为空，
提交前请填写。
