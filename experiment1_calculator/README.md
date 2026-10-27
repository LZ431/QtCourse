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
cmake -S . -B build -G Ninja `
  -DCMAKE_PREFIX_PATH=D:/Qt/6.11.2/mingw_64 `
  -DCMAKE_CXX_COMPILER=D:/Qt/Tools/mingw1310_64/bin/g++.exe
cmake --build build
```

运行 `build/KeyboardCalculator.exe`。在项目根目录执行 `ctest --test-dir build --output-on-failure`
可运行自动化测试。
