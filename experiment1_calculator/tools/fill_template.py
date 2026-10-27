#!/usr/bin/env python3
"""Fill the original course DOCX while preserving its existing layout."""

from __future__ import annotations

import shutil
import sys
from pathlib import Path

from docx import Document
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Inches, Pt
from docx.text.paragraph import Paragraph


PROJECT_DIR = Path(__file__).resolve().parents[1]
SOURCE = Path(r"C:\Users\刘\Desktop\实验1-带键盘事件的计算器_2026秋.docx")
OUTPUT = PROJECT_DIR / "docs" / "实验1-带键盘事件的计算器_2026秋_已完成.docx"
COMPAT_OUTPUT = PROJECT_DIR / "docs" / "实验1-带键盘事件的计算器-完成版.docx"
DESKTOP_OUTPUT = Path.home() / "Desktop" / OUTPUT.name


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


def style_body_runs(paragraph: Paragraph, text: str, label: str | None = None) -> None:
    paragraph.paragraph_format.first_line_indent = Pt(24)
    paragraph.paragraph_format.left_indent = Pt(0)
    paragraph.paragraph_format.space_after = Pt(6)
    paragraph.paragraph_format.line_spacing = 1.5
    paragraph.alignment = WD_ALIGN_PARAGRAPH.JUSTIFY

    if label:
        label_run = paragraph.add_run(label + "：")
        label_run.bold = True
        label_run.font.name = "宋体"
        label_run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), "宋体")
        label_run.font.size = Pt(12)

    text_run = paragraph.add_run(text)
    text_run.font.name = "宋体"
    text_run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), "宋体")
    text_run.font.size = Pt(12)


def add_body(
    cell,
    anchor: Paragraph,
    text: str,
    label: str | None = None,
) -> Paragraph:
    paragraph = cell.add_paragraph()
    move_after(paragraph, anchor)
    style_body_runs(paragraph, text, label)
    return paragraph


def add_plain(
    cell,
    anchor: Paragraph,
    text: str,
    *,
    centered: bool = False,
    size: float = 12,
    bold: bool = False,
) -> Paragraph:
    paragraph = cell.add_paragraph()
    move_after(paragraph, anchor)
    paragraph.paragraph_format.first_line_indent = Pt(0)
    paragraph.paragraph_format.space_after = Pt(4)
    if centered:
        paragraph.alignment = WD_ALIGN_PARAGRAPH.CENTER
    run = paragraph.add_run(text)
    run.bold = bold
    run.font.name = "宋体"
    run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), "宋体")
    run.font.size = Pt(size)
    return paragraph


def add_code(cell, anchor: Paragraph, code: str) -> Paragraph:
    paragraph = cell.add_paragraph()
    move_after(paragraph, anchor)
    paragraph.paragraph_format.left_indent = Pt(30)
    paragraph.paragraph_format.right_indent = Pt(18)
    paragraph.paragraph_format.first_line_indent = Pt(0)
    paragraph.paragraph_format.space_before = Pt(3)
    paragraph.paragraph_format.space_after = Pt(7)
    paragraph.paragraph_format.line_spacing = 1.1

    shading = OxmlElement("w:shd")
    shading.set(qn("w:val"), "clear")
    shading.set(qn("w:color"), "auto")
    shading.set(qn("w:fill"), "F2F4F7")
    paragraph._p.get_or_add_pPr().append(shading)

    run = paragraph.add_run(code)
    run.font.name = "Consolas"
    run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), "宋体")
    run.font.size = Pt(9.5)
    return paragraph


def add_image(cell, anchor: Paragraph, image_path: Path, caption: str) -> Paragraph:
    picture_paragraph = cell.add_paragraph()
    move_after(picture_paragraph, anchor)
    picture_paragraph.alignment = WD_ALIGN_PARAGRAPH.CENTER
    picture_paragraph.paragraph_format.first_line_indent = Pt(0)
    picture_paragraph.paragraph_format.space_before = Pt(4)
    picture_paragraph.paragraph_format.space_after = Pt(2)
    picture_paragraph.add_run().add_picture(str(image_path), width=Inches(4.65))

    caption_paragraph = add_plain(
        cell,
        picture_paragraph,
        caption,
        centered=True,
        size=10.5,
    )
    return caption_paragraph


def append_many(
    cell,
    anchor: Paragraph,
    items: list[tuple[str, str | None]],
) -> Paragraph:
    for item_type, value in items:
        if item_type == "text":
            anchor = add_body(cell, anchor, value or "")
        elif item_type == "label":
            label, text = (value or "").split("|", 1)
            anchor = add_body(cell, anchor, text, label)
        elif item_type == "code":
            anchor = add_code(cell, anchor, value or "")
        elif item_type == "plain":
            anchor = add_plain(cell, anchor, value or "")
        else:
            raise ValueError(f"Unknown item type: {item_type}")
    return anchor


def find_content_cell(document: Document):
    for table in document.tables:
        for row in table.rows:
            cell = row.cells[0]
            if any("实验步骤及结果" in paragraph.text for paragraph in cell.paragraphs):
                return cell
    raise RuntimeError("Cannot find the experiment content cell")


def fill_cover(document: Document) -> None:
    for paragraph in document.paragraphs:
        if paragraph.text.strip() == "实验报告模板":
            set_paragraph_text(paragraph, "实验报告")
            break

    for table in document.tables:
        for row in table.rows:
            unique_cells = []
            seen = set()
            for cell in row.cells:
                if id(cell._tc) not in seen:
                    seen.add(id(cell._tc))
                    unique_cells.append(cell)
            for index, cell in enumerate(unique_cells):
                if cell.text.strip() == "实验日期":
                    date_cell = unique_cells[index + 1]
                    if date_cell.paragraphs:
                        set_paragraph_text(
                            date_cell.paragraphs[0], "2026年10月27日"
                        )
                    else:
                        date_cell.add_paragraph("2026年10月27日")
                    return


def by_text(cell, exact_text: str) -> Paragraph:
    for paragraph in cell.paragraphs:
        if paragraph.text.strip() == exact_text:
            return paragraph
    raise RuntimeError(f"Cannot find template paragraph: {exact_text}")


def fill_content(document: Document) -> None:
    cell = find_content_cell(document)

    p_ui = by_text(cell, "UI界面布局与设计：说明主要控件和布局方式")
    anchor = append_many(
        cell,
        p_ui,
        [
            (
                "label",
                "实现说明|项目使用 Qt 6.11.2、CMake 和 Qt Widgets，界面在 Qt Designer "
                "可编辑的 calculatorwindow.ui 中完成。窗口根布局采用 QVBoxLayout；"
                "上方 QFrame 内用 QVBoxLayout 放置 expressionLabel（表达式）和 "
                "displayLabel（当前数值或结果）；下方使用 QGridLayout 排列 17 个按钮。"
                "控件统一设置间距、最小高度和 NoFocus 焦点策略，保证点击鼠标后键盘仍由主窗口接收。",
            ),
            (
                "text",
                "主要控件包括 digit0Button～digit9Button、decimalButton、addButton、"
                "subtractButton、multiplyButton、divideButton、equalsButton、"
                "backspaceButton 和 clearButton，命名均能直接反映用途。",
            ),
        ],
    )

    p_code = by_text(cell, "主要代码、设计说明及运行结果截图")
    anchor = append_many(
        cell,
        p_code,
        [
            (
                "label",
                "代码结构|窗口类 CalculatorWindow 负责接收鼠标和键盘事件、刷新标签；"
                "CalculatorLogic 作为独立状态机负责输入、运算和错误恢复，关键状态如下。",
            ),
            (
                "code",
                "QString m_display;           // 当前显示的操作数或结果\n"
                "double m_accumulator;        // 左操作数或中间结果\n"
                "QChar m_pendingOperator;     // +、-、*、/\n"
                "bool m_waitingForOperand;    // 是否等待第二个操作数\n"
                "bool m_justEvaluated;        // 是否刚按过等号\n"
                "bool m_error;                // 是否处于错误状态",
            ),
        ],
    )
    anchor = add_image(
        cell,
        anchor,
        PROJECT_DIR / "docs" / "screenshots" / "calculator-main.png",
        "图1  计算器主界面及加法运行结果（845＋996＝1841）",
    )

    p_input = by_text(
        cell,
        "按键事件处理：完成操作数输入、小数点、退格和清除，并验证异常输入处理",
    )
    anchor = append_many(
        cell,
        p_input,
        [
            (
                "label",
                "实现结果|数字键调用 inputDigit()；小数点调用 inputDecimalPoint()；"
                "Backspace 或退格按钮调用 backspace()；C、Esc、Delete 调用 clear()。"
                "退格时删除当前操作数末位，删空后恢复为 0；清除会同时复位操作符、中间结果和错误状态。",
            )
        ],
    )

    p_decimal = by_text(
        cell,
        "说明如何避免同一操作数重复输入小数点，并给出关键代码及测试结果",
    )
    anchor = append_many(
        cell,
        p_decimal,
        [
            (
                "label",
                "处理方式|追加小数点前检查当前显示文本是否已包含“.”；已经包含时忽略本次输入。"
                "等待新操作数时则先准备一个新数字，因此每个操作数最多只有一个小数点。",
            ),
            (
                "code",
                "if (m_waitingForOperand || m_justEvaluated) {\n"
                "    prepareForNewEntry();\n"
                "    m_display = QStringLiteral(\"0.\");\n"
                "} else if (!m_display.contains(QLatin1Char('.'))) {\n"
                "    m_display.append(QLatin1Char('.'));\n"
                "}",
            ),
            (
                "label",
                "测试结果|输入 12.3.4 时，第二次小数点被忽略，显示 12.34；"
                "输入 0..5 时最终得到 0.5，未出现非法数字格式。",
            ),
        ],
    )

    p_dual = by_text(cell, "双操作数处理：完成四则运算及计算状态管理")
    anchor = append_many(
        cell,
        p_dual,
        [
            (
                "label",
                "实现结果|支持加、减、乘、除和小数运算；运算结果使用 15 位有效数字格式化，"
                "并对非有限结果进行保护。除数为严格 0 时进入错误状态，避免继续使用无穷值计算。",
            )
        ],
    )

    p_state = by_text(
        cell,
        "说明程序如何管理第一个操作数、操作符、第二个操作数和计算结果等状态",
    )
    anchor = append_many(
        cell,
        p_state,
        [
            (
                "label",
                "状态管理|第一次选择运算符时，把当前输入保存到 m_accumulator（第一个操作数），"
                "并保存 m_pendingOperator；输入第二个操作数后，按等号调用 calculate()。"
                "如果连续选择运算符，程序先完成前一项二元运算，再把中间结果作为新的左操作数。",
            ),
            (
                "code",
                "bool CalculatorLogic::selectOperator(QChar operation) {\n"
                "    if (!m_pendingOperator.isNull()) {\n"
                "        if (!m_waitingForOperand && !calculate(currentValue()))\n"
                "            return false;\n"
                "    } else {\n"
                "        m_accumulator = currentValue();\n"
                "    }\n"
                "    m_pendingOperator = operation;\n"
                "    m_waitingForOperand = true;\n"
                "    updatePendingExpression();\n"
                "    return true;\n"
                "}",
            ),
        ],
    )

    p_boundary = by_text(
        cell,
        "连续操作与边界处理：完成连续运算、除零及计算结果后的继续输入",
    )
    anchor = append_many(
        cell,
        p_boundary,
        [
            (
                "label",
                "连续运算|采用基础计算器的即时运算方式。例如按下 6＋3×2＝ 时，"
                "程序在选择乘号时先算出 6＋3＝9，再计算 9×2＝18。",
            ),
            (
                "text",
                "按等号后输入数字会开始新计算；按等号后输入运算符则以当前结果为左操作数继续计算。"
                "输入过程中退格、清除后重算、除零恢复和鼠标键盘混合输入均已验证。",
            ),
        ],
    )

    p_anomaly = by_text(
        cell,
        "至少选择3个异常或边界输入，记录：测试输入→原程序现象→原因分析→修改方法→修改后结果",
    )
    anomaly_texts = [
        (
            "异常1|输入 12.3.4。若直接追加，首轮会得到非法的 12.3.4，转换时无法正确表达输入。"
            "原因是小数输入前未检查当前位置是否已有小数点。修改为 contains('.') 检查后，"
            "第二次小数点被忽略，最终显示 12.34。",
        ),
        (
            "异常2|输入 8＋×2＝。若未区分“已选运算符但尚未输入第二操作数”的状态，"
            "可能把操作符当成数字或丢失左操作数。增加 m_waitingForOperand 后，连续操作符只替换前一个，"
            "最终按 8×2 计算，结果为 16。",
        ),
        (
            "异常3|输入 9÷0＝。直接相除会产生无穷值并污染后续计算。原因是未检查除数是否为 0。"
            "在除法分支中判断 rightOperand == 0.0，显示“错误”并清空挂起运算；"
            "之后输入数字即可重新开始，结果恢复正常。",
        ),
        (
            "异常4|输入 1÷0.0000000000001＝。首轮曾因使用 qFuzzyIsNull 判断零而误报错误。"
            "原因是近似零判断会吞掉极小非零数；改为严格零比较并增加回归测试后，"
            "正确显示 10000000000000。",
        ),
    ]
    anchor = p_anomaly
    for item in anomaly_texts:
        value = item[0] if isinstance(item, tuple) else item
        label, text = value.split("|", 1)
        anchor = add_body(cell, anchor, text, label)

    p_style = by_text(cell, "使用样式表改善界面，并检查控件命名与代码结构")
    anchor = append_many(
        cell,
        p_style,
        [
            (
                "label",
                "样式优化|通过 QSS 统一设置窗口浅蓝背景、显示区白色圆角和按钮卡片样式；"
                "运算键使用淡蓝色，等号键使用高对比度蓝色，并增加悬停和按下反馈。"
                "鼠标与键盘处理分别封装，二者统一调用同一组输入函数，没有复制计算代码。",
            )
        ],
    )

    p_optimized = by_text(cell, "给出优化后的界面截图，并说明主要改进")
    anchor = append_many(
        cell,
        p_optimized,
        [
            (
                "text",
                "优化后的界面将表达式和结果分层显示，使用不同底色区分数字键、运算键和等号键，"
                "同时统一圆角、间距与字号，较初版具有更清晰的视觉层次。",
            )
        ],
    )
    anchor = add_image(
        cell,
        anchor,
        PROJECT_DIR / "docs" / "screenshots" / "calculator-decimal.png",
        "图2  样式优化后的小数乘法运行结果（12.5×2.4＝30）",
    )

    p_keyboard = by_text(
        cell,
        "键盘事件处理：实现键盘输入，并与鼠标按键处理逻辑保持一致",
    )
    anchor = append_many(
        cell,
        p_keyboard,
        [
            (
                "label",
                "实现结果|重写 keyPressEvent()，支持数字、小键盘数字、小数点、+、-、*、/、"
                "Enter/小键盘回车/=、Backspace、Esc/Delete/C。x、X、×、÷ 等字符也会归一化为对应运算符。",
            )
        ],
    )

    p_shared = by_text(
        cell,
        "说明鼠标与键盘输入是否复用同一套处理逻辑，并给出关键代码及测试结果",
    )
    anchor = append_many(
        cell,
        p_shared,
        [
            (
                "label",
                "复用方式|键盘事件只负责把按键映射为输入动作，随后调用 inputDigit()、"
                "inputDecimalPoint()、selectOperator()、evaluate() 等函数；鼠标按钮连接的是同一组函数，"
                "最终都进入 CalculatorLogic，因此两套输入方式结果一致。",
            ),
            (
                "code",
                "void CalculatorWindow::keyPressEvent(QKeyEvent *event) {\n"
                "    const int digit = event->text().front().digitValue();\n"
                "    if (digit >= 0 && digit <= 9) {\n"
                "        inputDigit(digit);       // 与数字按钮调用同一函数\n"
                "        return;\n"
                "    }\n"
                "    // 小数、运算符、等号、退格和清除同样分流到窗口处理函数\n"
                "}",
            ),
            (
                "label",
                "测试结果|键盘输入 12.5＋2＝ 得到 14.5；再点击鼠标乘号并键盘输入 2＝，"
                "得到 29。连续混合输入结果与纯键盘、纯鼠标一致。",
            ),
        ],
    )

    p_github_info = by_text(
        cell,
        "提交GitHub上与实验1相关日志，原则上不少于5次有效提交，应能体现功能实现、调试、问题修复和完善过程",
    )
    anchor = append_many(
        cell,
        p_github_info,
        [
            (
                "label",
                "仓库地址|https://github.com/LZ431/QtCourse（main 分支）",
            ),
            (
                "code",
                "896a1a4  feat: add calculator project skeleton and designed layout\n"
                "2471229  feat: implement arithmetic state machine and mouse input\n"
                "1e7ba3d  feat: route keyboard input through shared calculator handlers\n"
                "4ba6f50  style: apply responsive blue calculator theme\n"
                "0a66d6e  test: cover keyboard, boundary inputs, and recovery paths\n"
                "e62f6bb  fix: preserve tiny nonzero values during division\n"
                "27c7647  docs: add experiment report, screenshots, and demo script",
            ),
        ],
    )

    p_ai = by_text(
        cell,
        "选择1个具有代表性的AI辅助开发过程进行记录：问题→提示词→AI给出的方案→实际运行结果→存在的问题→自己的修改。不要求完整粘贴全部AI对话。",
    )
    anchor = append_many(
        cell,
        p_ai,
        [
            (
                "label",
                "问题|需要让鼠标与键盘复用同一套逻辑，同时正确处理连续运算、重复小数点和除零。",
            ),
            (
                "label",
                "提示词|“使用 Qt Widgets 和 .ui 文件实现基础计算器；鼠标与键盘复用逻辑；"
                "处理连续小数点、连续运算符、除零、按等号后继续输入。”",
            ),
            (
                "label",
                "AI方案|建议把显示值、累加值、挂起运算符、等待操作数、刚求值和错误状态集中到状态机；"
                "键盘事件只负责映射，不直接实现计算。",
            ),
            (
                "label",
                "实际结果|按方案完成项目后编译成功，基础四则运算和混合输入测试通过，"
                "界面可正确显示运算过程与结果。",
            ),
            (
                "label",
                "存在问题|AI初稿使用 qFuzzyIsNull 判断除数，导致极小非零值被当成 0；"
                "部分边界规则也需要明确。",
            ),
            (
                "label",
                "我的修改|将除零判断改为严格比较 rightOperand == 0.0，增加小数值回归测试，"
                "并人工确定连续运算采用基础计算器的即时计算规则，最后逐项运行自动化测试验证。",
            ),
        ],
    )

    p_test = by_text(
        cell,
        "录制2～3分钟测试视频，以附件形式提交到作业系统。视频应连续演示：鼠标完成带小数运算、键盘计算、退格和清除、除零处理、连续运算、至少1个异常输入处理，并展示Git提交历史",
    )
    anchor = append_many(
        cell,
        p_test,
        [
            (
                "label",
                "自动化结果|CTest 1/1 通过，覆盖四则运算、小数、连续运算、重复小数点、"
                "连续运算符、除零、错误恢复、结果后继续输入、退格、清除与鼠标键盘混合输入。",
            ),
            (
                "label",
                "主要结果|2.5＋1.25＝3.75；12.5×2.4＝30；6＋3×2＝18；"
                "2＋3＝×2＝10；8＋×2＝16；9÷0 显示“错误”；"
                "键盘 12.5＋2＝ 后再用鼠标乘 2 得到 29。",
            ),
            (
                "label",
                "视频内容|按实验一演示脚本连续录制，依次展示鼠标小数运算、键盘计算、"
                "退格与清除、除零恢复、连续运算、异常输入和 Git 提交历史，视频作为作业附件提交。",
            ),
        ],
    )
    anchor = add_image(
        cell,
        anchor,
        PROJECT_DIR / "docs" / "screenshots" / "calculator-error.png",
        "图3  除零测试结果（9÷0 显示“错误”）",
    )

    p_summary = by_text(
        cell,
        "出错分析及解决过程，本次实验总结；结合AI辅助开发过程，说明AI提供了哪些帮助、存在什么问题，以及自己如何验证和改进AI生成的代码",
    )
    append_many(
        cell,
        p_summary,
        [
            (
                "label",
                "出错分析|开发中主要处理了重复小数点、连续运算符、除零和小数精度边界。"
                "其中 qFuzzyIsNull 曾把极小非零除数误判为 0，通过严格零比较和回归测试完成修复。",
            ),
            (
                "label",
                "实验总结|本次实验完成了 Qt Designer 界面、信号槽连接、四则运算状态机、"
                "键盘事件、异常处理和样式优化。鼠标与键盘共用同一套输入函数，避免行为分叉；"
                "独立状态机也便于编写自动化测试。",
            ),
            (
                "label",
                "AI辅助总结|AI帮助快速梳理状态机结构和测试清单，但其生成代码不能直接视为正确。"
                "本人通过编译运行、边界输入、QtTest 回归测试和代码复查，修正了浮点判断问题，"
                "并明确了连续运算和错误恢复规则。",
            ),
        ],
    )


def main() -> None:
    if not SOURCE.exists():
        raise FileNotFoundError(SOURCE)

    document = Document(SOURCE)
    fill_cover(document)
    fill_content(document)
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    document.save(OUTPUT)

    shutil.copy2(OUTPUT, COMPAT_OUTPUT)
    shutil.copy2(OUTPUT, DESKTOP_OUTPUT)
    print(f"Filled DOCX: {OUTPUT}")
    print(f"Compatibility copy: {COMPAT_OUTPUT}")
    print(f"Desktop copy: {DESKTOP_OUTPUT}")


if __name__ == "__main__":
    try:
        main()
    except Exception as error:  # pragma: no cover - CLI error report
        print(f"Failed to fill template: {error}", file=sys.stderr)
        raise
