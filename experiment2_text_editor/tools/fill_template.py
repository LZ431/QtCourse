#!/usr/bin/env python3
"""Fill the official experiment-2 DOCX while preserving the course template."""

from __future__ import annotations

import subprocess
import sys
from pathlib import Path

PROJECT_DIR = Path(__file__).resolve().parents[1]
LOCAL_PACKAGES = PROJECT_DIR / ".tools" / "python"
if LOCAL_PACKAGES.exists():
    sys.path.insert(0, str(LOCAL_PACKAGES))

from docx import Document
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Inches, Pt
from docx.text.paragraph import Paragraph


SOURCE = Path(r"C:\Users\刘\Desktop\QT\实验2-简单文本编辑器_2026秋.docx")
OUTPUT = PROJECT_DIR / "docs" / "实验2-简单文本编辑器_2026秋_已完成.docx"
COMPAT_OUTPUT = PROJECT_DIR / "docs" / "实验2-简单文本编辑器-完成版.docx"
SCREENSHOTS = PROJECT_DIR / "docs" / "screenshots"

DEVELOPER = "刘泽"
STUDENT_ID = "2024414290223"
EMAIL = "2227561800@qq.com"


def set_paragraph_text(paragraph: Paragraph, text: str) -> None:
    if paragraph.runs:
        paragraph.runs[0].text = text
        for extra_run in paragraph.runs[1:]:
            extra_run._r.getparent().remove(extra_run._r)
    else:
        paragraph.add_run(text)


def move_after(paragraph: Paragraph, anchor: Paragraph) -> Paragraph:
    anchor._p.addnext(paragraph._p)
    return paragraph


def set_east_asian_font(run, font_name: str = "宋体", size: float = 12) -> None:
    run.font.name = font_name
    run.font.size = Pt(size)
    run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), font_name)


def add_body(
    cell,
    anchor: Paragraph,
    text: str,
    label: str | None = None,
    *,
    indent: bool = True,
) -> Paragraph:
    paragraph = cell.add_paragraph()
    move_after(paragraph, anchor)
    paragraph.paragraph_format.first_line_indent = Pt(24) if indent else Pt(0)
    paragraph.paragraph_format.space_after = Pt(5)
    paragraph.paragraph_format.line_spacing = 1.4
    paragraph.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY

    if label:
        label_run = paragraph.add_run(label)
        label_run.bold = True
        set_east_asian_font(label_run)

    text_run = paragraph.add_run(text)
    set_east_asian_font(text_run)
    return paragraph


def add_heading(cell, anchor: Paragraph, text: str) -> Paragraph:
    paragraph = cell.add_paragraph()
    move_after(paragraph, anchor)
    paragraph.paragraph_format.first_line_indent = Pt(0)
    paragraph.paragraph_format.space_before = Pt(8)
    paragraph.paragraph_format.space_after = Pt(4)
    run = paragraph.add_run(text)
    run.bold = True
    set_east_asian_font(run, size=13)
    return paragraph


def add_code(cell, anchor: Paragraph, code: str) -> Paragraph:
    paragraph = cell.add_paragraph()
    move_after(paragraph, anchor)
    paragraph.paragraph_format.first_line_indent = Pt(0)
    paragraph.paragraph_format.left_indent = Pt(24)
    paragraph.paragraph_format.right_indent = Pt(12)
    paragraph.paragraph_format.space_before = Pt(2)
    paragraph.paragraph_format.space_after = Pt(6)
    paragraph.paragraph_format.line_spacing = 1.05

    shading = OxmlElement("w:shd")
    shading.set(qn("w:val"), "clear")
    shading.set(qn("w:color"), "auto")
    shading.set(qn("w:fill"), "F2F4F7")
    paragraph._p.get_or_add_pPr().append(shading)

    run = paragraph.add_run(code)
    run.font.name = "Consolas"
    run.font.size = Pt(9)
    run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), "宋体")
    return paragraph


def add_image(
    cell,
    anchor: Paragraph,
    image_name: str,
    caption: str,
    width_inches: float = 5.9,
) -> Paragraph:
    path = SCREENSHOTS / image_name
    if not path.exists():
        raise FileNotFoundError(path)

    picture_paragraph = cell.add_paragraph()
    move_after(picture_paragraph, anchor)
    picture_paragraph.alignment = WD_ALIGN_PARAGRAPH.CENTER
    picture_paragraph.paragraph_format.first_line_indent = Pt(0)
    picture_paragraph.paragraph_format.space_before = Pt(3)
    picture_paragraph.paragraph_format.space_after = Pt(1)
    picture_paragraph.add_run().add_picture(str(path), width=Inches(width_inches))

    caption_paragraph = cell.add_paragraph()
    move_after(caption_paragraph, picture_paragraph)
    caption_paragraph.alignment = WD_ALIGN_PARAGRAPH.CENTER
    caption_paragraph.paragraph_format.first_line_indent = Pt(0)
    caption_paragraph.paragraph_format.space_after = Pt(6)
    caption_run = caption_paragraph.add_run(caption)
    set_east_asian_font(caption_run, size=10)
    return caption_paragraph


def find_by_text(
    cell,
    exact_text: str,
    occurrence: int = 1,
) -> Paragraph:
    seen = 0
    for paragraph in cell.paragraphs:
        if paragraph.text.strip() == exact_text:
            seen += 1
            if seen == occurrence:
                return paragraph
    raise RuntimeError(f"Cannot find paragraph {exact_text!r} occurrence {occurrence}")


def fill_cover(document: Document) -> None:
    for paragraph in document.paragraphs:
        if paragraph.text.strip() == "实验报告模板":
            set_paragraph_text(paragraph, "实验报告")
            break

    table = document.tables[0]
    rows = table.rows
    cover_values = [
        (rows[1].cells[1], DEVELOPER),
        (rows[1].cells[3], STUDENT_ID),
        (rows[1].cells[5], ""),
        (rows[2].cells[1], ""),
        (rows[2].cells[3], "2026年10月29日"),
        (rows[2].cells[5], ""),
    ]
    for cell, value in cover_values:
        if cell.paragraphs:
            set_paragraph_text(cell.paragraphs[0], value)
        elif value:
            cell.add_paragraph(value)


def git_log_for_project() -> str:
    try:
        result = subprocess.run(
            [
                "git",
                "log",
                "--oneline",
                "--",
                "experiment2_text_editor",
            ],
            cwd=PROJECT_DIR.parent,
            capture_output=True,
            text=True,
            encoding="utf-8",
            check=False,
        )
    except OSError:
        return "Git 日志将在提交完成后通过 git log 查看。"
    return result.stdout.strip() or "Git 日志将在提交完成后通过 git log 查看。"


def fill_content(document: Document) -> None:
    content_cell = None
    for table in document.tables:
        for row in table.rows:
            for cell in row.cells:
                if any(
                    paragraph.text.strip() == "实验步骤及结果："
                    for paragraph in cell.paragraphs
                ):
                    content_cell = cell
                    break
            if content_cell:
                break
        if content_cell:
            break
    if content_cell is None:
        raise RuntimeError("Cannot find experiment content cell")

    anchor = find_by_text(content_cell, "实验步骤及结果：")
    anchor = add_body(
        content_cell,
        anchor,
        "本实验使用 Qt 6.11.2、CMake 和 Qt Widgets 完成。主窗口采用 Qt Designer "
        "设计，编辑区通过控件提升接入派生自 QPlainTextEdit 的 CodeEditor；关于、"
        "查找和替换使用独立对话框。所有菜单项均由 QAction 驱动，并与工具栏共用"
        "快捷键、图标和使能状态。",
        "总体实现：",
        indent=False,
    )

    anchor = find_by_text(
        content_cell,
        "UI界面布局与设计，设计菜单和工具栏，QPlainTextEdit控件，并调整layoutMargins",
    )
    anchor = add_body(
        content_cell,
        anchor,
        "主窗口采用无边距 QVBoxLayout，中央只放置 CodeEditor，行号栏因此紧贴编辑区。"
        "菜单按“文件、编辑、格式、查看、帮助”分组，工具栏按“文件、历史、剪贴板、"
        "查找替换、格式”顺序加入分隔线。Action 同时用于菜单和工具栏，避免两套逻辑。",
        "实现结果：",
    )
    anchor = add_image(
        content_cell,
        anchor,
        "editor-main.png",
        "图 1  主界面、行号栏、工具栏与状态栏",
    )

    anchor = find_by_text(content_cell, "主要代码及运行结果截图", 1)
    anchor = add_body(
        content_cell,
        anchor,
        "编辑区由 mainwindow.ui 中的自定义 CodeEditor 控件创建，并在 MainWindow "
        "中统一连接文档、光标和 Action 信号。行号宽度随最大行号位数变化，滚动时"
        "同步更新。",
        "代码与结构：",
    )
    anchor = add_code(
        content_cell,
        anchor,
        "connect(editor, &QPlainTextEdit::blockCountChanged,\n"
        "        this, &CodeEditor::updateLineNumberAreaWidth);\n"
        "connect(editor, &QPlainTextEdit::updateRequest,\n"
        "        this, &CodeEditor::updateLineNumberArea);\n"
        "connect(editor, &QPlainTextEdit::cursorPositionChanged,\n"
        "        this, &CodeEditor::highlightCurrentLine);",
    )
    anchor = add_image(
        content_cell,
        anchor,
        "menu-edit.png",
        "图 2  编辑菜单中的 Action、分组和快捷键",
        width_inches=2.8,
    )

    anchor = find_by_text(content_cell, "设计关于对话框")
    anchor = add_body(
        content_cell,
        anchor,
        "AboutDialog 使用独立 .ui 文件完成标题、记事本图标、版本号和按钮布局。"
        f"开发者显示为{DEVELOPER}，学号为{STUDENT_ID}，email 为{EMAIL}。",
        "实现结果：",
    )
    anchor = add_image(
        content_cell,
        anchor,
        "dialog-about.png",
        "图 3  关于对话框",
        width_inches=4.4,
    )

    anchor = find_by_text(content_cell, "设计查找对话框")
    anchor = add_body(
        content_cell,
        anchor,
        "查找对话框包括查找目标、区分大小写、向上/向下、查找下一个和取消。"
        "查找从当前选区末端开始，到达边界后从另一端环绕；全篇未命中时提示“未找到”。",
        "实现结果：",
    )
    anchor = add_image(
        content_cell,
        anchor,
        "dialog-find.png",
        "图 4  查找对话框",
        width_inches=5.2,
    )

    anchor = find_by_text(content_cell, "设计替换对话框")
    anchor = add_body(
        content_cell,
        anchor,
        "替换对话框增加“替换为”“替换”和“全部替换”。全部替换使用 "
        "beginEditBlock/endEditBlock 包裹，整批替换可一次撤销，并显示替换数量；"
        "没有匹配项时显示“未找到”。",
        "实现结果：",
    )
    anchor = add_image(
        content_cell,
        anchor,
        "dialog-replace.png",
        "图 5  替换对话框",
        width_inches=5.3,
    )

    anchor = find_by_text(content_cell, "处理新建文件、另存文件、保存文件、打开文件操作")
    anchor = add_body(
        content_cell,
        anchor,
        "新建、打开和退出前调用 maybeSave 检查修改标记；保存到无路径文档时自动进入"
        "另存为。打开前验证文件存在、类型、32 MB 上限和 NUL 字节，失败时不覆盖"
        "当前编辑内容。保存采用 QSaveFile 原子写入。",
        "实现结果：",
    )
    anchor = add_code(
        content_cell,
        anchor,
        "QSaveFile file(path);\n"
        "if (!file.open(QIODevice::WriteOnly)) return false;\n"
        "const QByteArray data = editor->toPlainText().toUtf8();\n"
        "if (file.write(data) != data.size()) {\n"
        "    file.cancelWriting();\n"
        "    return false;\n"
        "}\n"
        "return file.commit();",
    )

    anchor = find_by_text(content_cell, "处理撤销，恢复，剪切，复制，粘贴，全选操作")
    anchor = add_body(
        content_cell,
        anchor,
        "编辑动作直接复用 QPlainTextEdit 的撤销栈。copyAvailable、undoAvailable、"
        "redoAvailable 与剪贴板 dataChanged 信号驱动 QAction 使能状态：存在选区时"
        "启用剪切和复制，剪贴板有文本时启用粘贴，历史栈可用时启用撤销或恢复。",
        "实现结果：",
    )
    anchor = add_image(
        content_cell,
        anchor,
        "selection-actions.png",
        "图 6  选区存在时剪切和复制 Action 自动启用",
    )

    anchor = find_by_text(content_cell, "设置字体，字体颜色，背景色、自动换行操作")
    anchor = add_body(
        content_cell,
        anchor,
        "字体使用 QFontDialog，字体颜色、字符背景色与编辑器背景色使用 QColorDialog。"
        "存在选区时合并 QTextCharFormat 到选区，否则应用到当前输入格式。自动换行在"
        "WidgetWidth 和 NoWrap 间切换，行号仍按逻辑行编号。",
        "实现结果：",
    )

    anchor = find_by_text(content_cell, "设置显示、影藏工具栏、状态栏操作，更新状态栏信息")
    anchor = add_body(
        content_cell,
        anchor,
        "查看菜单中的工具栏、状态栏 Action 为可勾选项，直接控制对应控件可见性。"
        "状态栏在文本变化或光标移动时更新 length、lines、Ln、Col 和当前用户名。",
        "实现结果：",
    )

    anchor = find_by_text(
        content_cell,
        "处理cut，copy，paste按钮的使能问题",
    )
    anchor = add_body(
        content_cell,
        anchor,
        "执行选中文字、复制、删除或撤销后，动作状态均通过 Qt 信号刷新，不依赖用户"
        "手动重新打开菜单，因此工具栏和菜单中的按钮状态保持一致。",
        "实现结果：",
    )

    anchor = find_by_text(content_cell, "处理查找窗口操作")
    anchor = add_body(
        content_cell,
        anchor,
        "连续点击“查找下一个”时，下一轮从当前选区末端开始，避免反复选中首项；"
        "向上查找使用选区起点和 FindBackward。大小写选项映射为 "
        "QTextDocument::FindCaseSensitively。",
        "实现结果：",
    )

    anchor = find_by_text(content_cell, "处理替换窗口操作")
    anchor = add_body(
        content_cell,
        anchor,
        "替换当前项先判断选区是否匹配，匹配时插入替换文本并继续查找下一项；"
        "不匹配时先查找。全部替换循环搜索并把所有修改放入一个编辑块，显示替换数量。",
        "实现结果：",
    )

    anchor = find_by_text(
        content_cell,
        "参考资料3，从QPlainTextEdit派生出一个类，支持行号的显示，通过promote操作，铁环QPlainTextEdit",
    )
    anchor = add_body(
        content_cell,
        anchor,
        "CodeEditor 继承 QPlainTextEdit，内部 LineNumberArea 负责绘制行号。"
        "updateLineNumberAreaWidth 根据总行数计算数字位数，updateLineNumberArea "
        "处理滚动和局部重绘，当前行使用 ExtraSelection 绘制浅黄色背景。",
        "实现结果：",
    )
    anchor = add_code(
        content_cell,
        anchor,
        "int CodeEditor::lineNumberAreaWidth() const {\n"
        "    int digits = 1;\n"
        "    int maxBlocks = qMax(1, blockCount());\n"
        "    while (maxBlocks >= 10) { maxBlocks /= 10; ++digits; }\n"
        "    return 8 + fontMetrics().horizontalAdvance('9') * digits;\n"
        "}",
    )

    anchor = find_by_text(content_cell, "Github日志：")
    anchor = add_body(
        content_cell,
        anchor,
        "实验二相关提交通过 main 分支管理，代码、自动测试、运行截图和实验报告"
        "分别提交。",
        "提交说明：",
    )
    anchor = add_code(content_cell, anchor, git_log_for_project())

    anchor = find_by_text(content_cell, "测试结果：")
    anchor = add_body(
        content_cell,
        anchor,
        "执行 ctest --test-dir build --output-on-failure，构建目录 1/1 测试通过，"
        "退出码为 0。QtTest 中共有 8 个功能测试，覆盖 UTF-8 保存/重开、缺失文件、"
        "二进制文件、连续与环绕查找、大小写不敏感全部替换、行号显隐、动作状态"
        "、未保存关闭提示以及关于菜单触发。",
        "自动化结果：",
    )
    anchor = add_code(
        content_cell,
        anchor,
        "1/1 Test #1: text_editor_tests ................ Passed\n"
        "100% tests passed, 0 tests failed out of 1",
    )

    anchor = find_by_text(
        content_cell,
        "除功能截图外，至少选择3个边界或异常场景进行测试，并说明测试输入、程序现象、原因分析及修改后的结果。",
    )
    edge_cases = [
        (
            "边界 1（不存在文件）：",
            "输入不存在的 missing.txt。程序返回失败并显示“文件不存在”，编辑器"
            "原内容保持不变。原因是在读取前检查 exists/isFile，且验证成功后才调用"
            "setPlainText。测试结果通过。",
        ),
        (
            "边界 2（二进制文件）：",
            "输入含 NUL 字节的 text\\0binary 数据。程序识别为二进制内容并拒绝打开，"
            "不会把不可读字符写入编辑器。测试结果通过。",
        ),
        (
            "边界 3（连续查找与环绕）：",
            "光标位于文末时向下查找首个 Alpha，再从第二项向上查找。程序先环绕到"
            "文首，反向查找回到前一项，没有停在原地重复命中。测试结果通过。",
        ),
        (
            "边界 4（替换未命中）：",
            "全部替换一个不存在的词。替换数量为 0，文档内容不变，界面提示“未找到”。"
            "测试结果通过。",
        ),
        (
            "边界 5（未保存关闭）：",
            "修改文档后关闭窗口，先选择取消，窗口和内容继续保留；再次关闭并选择"
            "放弃，窗口正常退出。测试结果通过。",
        ),
    ]
    for label, text in edge_cases:
        anchor = add_body(content_cell, anchor, text, label)

    anchor = find_by_text(
        content_cell,
        "实验报告选择1个有代表性的AI辅助过程，简要记录“问题→提示词→AI方案→运行验证→发现的问题→自己的修改”，不要求粘贴完整对话。",
    )
    ai_items = [
        (
            "问题：",
            "需要统一菜单和工具栏状态，并正确处理连续查找、全部替换和未保存保护。",
        ),
        (
            "提示词摘要：",
            "使用 Qt Widgets 和 Designer 完成实验二文本编辑器，要求主窗口加独立"
            "查找、替换、关于对话框，包含文件操作、格式、行号、自动换行、动作"
            "使能和边界测试。",
        ),
        (
            "AI 方案：",
            "用 QAction 统一菜单和工具栏；用 CodeEditor 派生 QPlainTextEdit 绘制"
            "行号；查找失败后才环绕；全部替换使用 beginEditBlock/endEditBlock。",
        ),
        (
            "运行验证：",
            "Qt 6.11.2 编译成功，CTest 1/1 通过；生成并检查主界面、菜单、对话框"
            "和选区截图。",
        ),
        (
            "发现的问题：",
            "AI 初稿若每轮都从文档起点查找，会不断命中首项；直接调用 "
            "setWindowModified 而未定义 [*] 占位符会产生 Qt 警告；逐项替换造成"
            "撤销粒度过细。",
        ),
        (
            "自己的修改：",
            "改为从当前选区末端开始并仅在失败后环绕；手工组合 * 与标题；把全部"
            "替换合并为一个撤销块；补充文件异常和动作状态测试。",
        ),
    ]
    for label, text in ai_items:
        anchor = add_body(content_cell, anchor, text, label)

    anchor = find_by_text(content_cell, "出错分析，如何解决的，本次实验总结。结合AI辅助开发过程，说明AI在哪些环节提供了帮助、生成结果存在哪些问题，以及自己如何通过运行、调试和测试进行验证与改进。")
    anchor = add_body(
        content_cell,
        anchor,
        "本实验完成了基于 Qt Widgets 的简单文本编辑器。主窗口使用 Designer 组织"
        "菜单与 Action，编辑区通过派生控件实现行号和当前行高亮，关于、查找与替换"
        "采用独立对话框。文件状态集中在 MainWindow 管理，打开失败不会破坏现有内容。",
        "实验总结：",
    )
    anchor = add_body(
        content_cell,
        anchor,
        "实现中最重要的是区分文档修改状态与当前保存路径：标题只反映修改状态，"
        "无路径文档保存时进入另存为，自动保存只处理已有路径的修改文档。查找、"
        "替换和文件读写均抽取为可测试成员函数，既能在界面中运行，也能通过 QtTest"
        "覆盖异常与边界情况。",
        "设计收获：",
    )
    anchor = add_body(
        content_cell,
        anchor,
        "AI 可以快速整理界面 XML、Qt API 和测试清单，但边界行为仍需要本地编译、"
        "CTest、真实窗口截图和人工交互逐项确认。最终程序经过修正后才用于实验提交。",
        "AI 辅助反思：",
    )


def main() -> None:
    if not SOURCE.exists():
        raise FileNotFoundError(SOURCE)
    document = Document(SOURCE)
    fill_cover(document)
    fill_content(document)
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    document.save(OUTPUT)
    COMPAT_OUTPUT.write_bytes(OUTPUT.read_bytes())
    print(f"Filled DOCX: {OUTPUT}")
    print(f"Compatibility copy: {COMPAT_OUTPUT}")


if __name__ == "__main__":
    main()

