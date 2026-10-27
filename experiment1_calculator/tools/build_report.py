#!/usr/bin/env python3
"""Build the experiment report DOCX from the reviewed Markdown source."""

from __future__ import annotations

import re
import struct
import sys
import zipfile
from datetime import datetime, timezone
from pathlib import Path
from xml.sax.saxutils import escape


PROJECT_DIR = Path(__file__).resolve().parents[1]
SOURCE = PROJECT_DIR / "docs" / "实验报告.md"
OUTPUT = PROJECT_DIR / "docs" / "实验1-带键盘事件的计算器-完成版.docx"

WORD_NS = "http://schemas.openxmlformats.org/wordprocessingml/2006/main"
REL_NS = "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
DRAWING_NS = "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
MAIN_DRAWING_NS = "http://schemas.openxmlformats.org/drawingml/2006/main"
PICTURE_NS = "http://schemas.openxmlformats.org/drawingml/2006/picture"


def run(
    text: str,
    *,
    bold: bool = False,
    italic: bool = False,
    code: bool = False,
    color: str | None = None,
    size: int | None = None,
) -> str:
    properties: list[str] = []
    if code:
        properties.append(
            '<w:rFonts w:ascii="Consolas" w:hAnsi="Consolas" w:eastAsia="等线"/>'
        )
    if bold:
        properties.append("<w:b/>")
    if italic:
        properties.append("<w:i/>")
    if color:
        properties.append(f'<w:color w:val="{color}"/>')
    if size:
        properties.append(f'<w:sz w:val="{size}"/><w:szCs w:val="{size}"/>')
    props = f"<w:rPr>{''.join(properties)}</w:rPr>" if properties else ""
    return f'<w:r>{props}<w:t xml:space="preserve">{escape(text)}</w:t></w:r>'


def inline_runs(text: str) -> str:
    text = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", r"\1 (\2)", text)
    token_pattern = re.compile(r"(\*\*.*?\*\*|`.*?`)")
    result: list[str] = []
    for token in token_pattern.split(text):
        if not token:
            continue
        if token.startswith("**") and token.endswith("**"):
            result.append(run(token[2:-2], bold=True))
        elif token.startswith("`") and token.endswith("`"):
            result.append(run(token[1:-1], code=True))
        else:
            result.append(run(token))
    return "".join(result)


def paragraph(
    text: str = "",
    *,
    style: str = "Normal",
    align: str | None = None,
    inline: bool = True,
    spacing_after: int = 100,
    keep_next: bool = False,
) -> str:
    properties: list[str] = [f'<w:pStyle w:val="{style}"/>']
    if align:
        properties.append(f'<w:jc w:val="{align}"/>')
    if spacing_after >= 0:
        properties.append(
            '<w:spacing w:after="{}" w:line="340" w:lineRule="auto"/>'.format(
                spacing_after
            )
        )
    if keep_next:
        properties.append("<w:keepNext/>")
    content = inline_runs(text) if inline else run(text)
    return f'<w:p><w:pPr>{"".join(properties)}</w:pPr>{content}</w:p>'


def code_paragraph(text: str) -> str:
    return paragraph(
        text if text else " ",
        style="Code",
        inline=False,
        spacing_after=0,
    )


def table(headers: list[str], rows: list[list[str]], column_count: int) -> str:
    if column_count == 5:
        widths = [1050, 1350, 1950, 2200, 1500]
    elif column_count == 6:
        widths = [1050, 1150, 1550, 1900, 1900, 1500]
    elif column_count == 4:
        widths = [1500, 2500, 1300, 2720]
    elif column_count == 3:
        widths = [1800, 2500, 4720]
    else:
        widths = [9000 // max(column_count, 1)] * column_count
    table_width = sum(widths)
    grid = "".join(f'<w:gridCol w:w="{width}"/>' for width in widths)

    def cell(text: str, width: int, *, header: bool = False) -> str:
        fill = '<w:shd w:val="clear" w:fill="D9EAF7"/>' if header else ""
        content = inline_runs(text)
        if header:
            content = run(re.sub(r"[`*]", "", text), bold=True, size=17)
        return (
            "<w:tc>"
            f'<w:tcPr><w:tcW w:w="{width}" w:type="dxa"/>{fill}'
            '<w:vAlign w:val="center"/></w:tcPr>'
            f'<w:p><w:pPr><w:pStyle w:val="TableText"/></w:pPr>{content}</w:p>'
            "</w:tc>"
        )

    header_row = (
        '<w:tr><w:trPr><w:tblHeader/></w:trPr>'
        + "".join(cell(value, widths[index], header=True) for index, value in enumerate(headers))
        + "</w:tr>"
    )
    body_rows: list[str] = []
    for values in rows:
        padded = values + [""] * (column_count - len(values))
        body_rows.append(
            "<w:tr>"
            + "".join(cell(value, widths[index]) for index, value in enumerate(padded))
            + "</w:tr>"
        )

    return (
        "<w:tbl>"
        "<w:tblPr>"
        f'<w:tblW w:w="{table_width}" w:type="dxa"/>'
        '<w:tblLayout w:type="fixed"/>'
        '<w:tblBorders>'
        '<w:top w:val="single" w:sz="4" w:color="AFC6DA"/>'
        '<w:left w:val="single" w:sz="4" w:color="AFC6DA"/>'
        '<w:bottom w:val="single" w:sz="4" w:color="AFC6DA"/>'
        '<w:right w:val="single" w:sz="4" w:color="AFC6DA"/>'
        '<w:insideH w:val="single" w:sz="4" w:color="D7E3ED"/>'
        '<w:insideV w:val="single" w:sz="4" w:color="D7E3ED"/>'
        "</w:tblBorders>"
        '<w:tblCellMar><w:top w:w="70" w:type="dxa"/>'
        '<w:left w:w="80" w:type="dxa"/><w:bottom w:w="70" w:type="dxa"/>'
        '<w:right w:w="80" w:type="dxa"/></w:tblCellMar>'
        "</w:tblPr>"
        f"<w:tblGrid>{grid}</w:tblGrid>"
        f"{header_row}{''.join(body_rows)}"
        "</w:tbl>"
        + paragraph("", spacing_after=60)
    )


def png_size(path: Path) -> tuple[int, int]:
    with path.open("rb") as image_file:
        header = image_file.read(24)
    if header[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"Not a PNG image: {path}")
    return struct.unpack(">II", header[16:24])


def image_paragraph(
    path: Path,
    relationship_id: str,
    document_id: int,
    alt_text: str,
    width_inches: float = 4.55,
) -> str:
    pixel_width, pixel_height = png_size(path)
    cx = int(width_inches * 914400)
    cy = int(cx * pixel_height / pixel_width)
    return (
        "<w:p><w:pPr><w:jc w:val=\"center\"/>"
        '<w:spacing w:before="80" w:after="120"/></w:pPr>'
        "<w:r><w:drawing>"
        f'<wp:inline distT="0" distB="0" distL="0" distR="0">'
        f'<wp:extent cx="{cx}" cy="{cy}"/>'
        f'<wp:docPr id="{document_id}" name="Picture {document_id}" descr="{escape(alt_text)}"/>'
        '<wp:cNvGraphicFramePr><a:graphicFrameLocks noChangeAspect="1"/></wp:cNvGraphicFramePr>'
        "<a:graphic>"
        f'<a:graphicData uri="{PICTURE_NS}">'
        "<pic:pic>"
        '<pic:nvPicPr>'
        f'<pic:cNvPr id="{document_id}" name="Picture {document_id}"/>'
        "<pic:cNvPicPr/>"
        "</pic:nvPicPr>"
        f'<pic:blipFill><a:blip r:embed="{relationship_id}"/>'
        "<a:stretch><a:fillRect/></a:stretch></pic:blipFill>"
        '<pic:spPr><a:xfrm><a:off x="0" y="0"/>'
        f'<a:ext cx="{cx}" cy="{cy}"/></a:xfrm>'
        '<a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr>'
        "</pic:pic></a:graphicData></a:graphic></wp:inline>"
        "</w:drawing></w:r></w:p>"
    )


def page_break() -> str:
    return '<w:p><w:r><w:br w:type="page"/></w:r></w:p>'


def split_table_row(line: str) -> list[str]:
    line = line.strip()
    if line.startswith("|"):
        line = line[1:]
    if line.endswith("|"):
        line = line[:-1]
    return [cell.strip() for cell in line.split("|")]


def is_table_separator(line: str) -> bool:
    cells = split_table_row(line)
    return bool(cells) and all(re.fullmatch(r":?-{3,}:?", cell) for cell in cells)


def markdown_to_body(markdown: str) -> tuple[str, list[tuple[str, bytes]]]:
    lines = markdown.splitlines()
    body: list[str] = []
    image_files: list[tuple[str, bytes]] = []
    image_index = 0
    heading_seen = 0
    index = 0

    while index < len(lines):
        line = lines[index]
        stripped = line.strip()

        if not stripped:
            index += 1
            continue

        if stripped.startswith("```"):
            index += 1
            while index < len(lines) and not lines[index].strip().startswith("```"):
                body.append(code_paragraph(lines[index]))
                index += 1
            index += 1
            body.append(paragraph("", spacing_after=80))
            continue

        image_match = re.fullmatch(r"!\[([^\]]*)\]\(([^)]+)\)", stripped)
        if image_match:
            image_index += 1
            relative_path = image_match.group(2)
            image_path = (SOURCE.parent / relative_path).resolve()
            media_name = f"media/image{image_index}.png"
            relationship_id = f"rIdImage{image_index}"
            image_files.append((media_name, image_path.read_bytes()))
            body.append(
                image_paragraph(
                    image_path,
                    relationship_id,
                    image_index,
                    image_match.group(1),
                )
            )
            index += 1
            continue

        if stripped.startswith("|") and index + 1 < len(lines):
            if is_table_separator(lines[index + 1]):
                headers = split_table_row(lines[index])
                index += 2
                rows: list[list[str]] = []
                while index < len(lines) and lines[index].strip().startswith("|"):
                    rows.append(split_table_row(lines[index]))
                    index += 1
                body.append(table(headers, rows, len(headers)))
                continue

        if stripped.startswith("#"):
            level = len(stripped) - len(stripped.lstrip("#"))
            heading = stripped[level:].strip()
            heading_seen += 1
            if level == 1:
                body.append(
                    paragraph(heading, style="Title", align="center", keep_next=True)
                )
            elif level == 2 and heading_seen == 2:
                body.append(
                    paragraph(heading, style="Subtitle", align="center", keep_next=True)
                )
            else:
                if heading.startswith(("三、", "四、")):
                    body.append(page_break())
                style = "Heading1" if level == 2 else "Heading2"
                body.append(
                    paragraph(heading, style=style, keep_next=True, spacing_after=120)
                )
            index += 1
            continue

        bullet_match = re.match(r"^(\s*)([-*])\s+(.*)$", line)
        if bullet_match:
            indent = len(bullet_match.group(1))
            text = bullet_match.group(3)
            body.append(
                paragraph(
                    "\u2022 " + text,
                    style="ListText",
                    inline=True,
                    spacing_after=55,
                ).replace(
                    "<w:pStyle w:val=\"ListText\"/>",
                    f'<w:pStyle w:val="ListText"/><w:ind w:left="{360 + indent * 180}"/>',
                )
            )
            index += 1
            continue

        numbered_match = re.match(r"^\s*(\d+)\.\s+(.*)$", line)
        if numbered_match:
            body.append(
                paragraph(
                    numbered_match.group(1) + ". " + numbered_match.group(2),
                    style="ListText",
                    spacing_after=55,
                )
            )
            index += 1
            continue

        paragraph_lines = [stripped]
        index += 1
        while index < len(lines):
            candidate = lines[index].strip()
            if (
                not candidate
                or candidate.startswith(("#", "```", "|", "!["))
                or re.match(r"^\s*(?:[-*]|\d+\.)\s+", candidate)
            ):
                break
            paragraph_lines.append(candidate)
            index += 1
        body.append(paragraph(" ".join(paragraph_lines)))

    return "".join(body), image_files


def document_xml(body: str) -> str:
    section_properties = (
        "<w:sectPr>"
        '<w:pgSz w:w="11906" w:h="16838"/>'
        '<w:pgMar w:top="1050" w:right="1134" w:bottom="1050" w:left="1134" '
        'w:header="600" w:footer="600" w:gutter="0"/>'
        '<w:cols w:space="708"/>'
        '<w:docGrid w:linePitch="312"/>'
        "</w:sectPr>"
    )
    return (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        f'<w:document xmlns:w="{WORD_NS}" xmlns:r="{REL_NS}" '
        f'xmlns:wp="{DRAWING_NS}" xmlns:a="{MAIN_DRAWING_NS}" '
        f'xmlns:pic="{PICTURE_NS}">'
        f"<w:body>{body}{section_properties}</w:body></w:document>"
    )


def styles_xml() -> str:
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:styles xmlns:w="{WORD_NS}">
  <w:docDefaults>
    <w:rPrDefault><w:rPr>
      <w:rFonts w:ascii="Aptos" w:hAnsi="Aptos" w:eastAsia="宋体" w:cs="Aptos"/>
      <w:sz w:val="22"/><w:szCs w:val="22"/>
      <w:lang w:val="zh-CN" w:eastAsia="zh-CN" w:bidi="ar-SA"/>
    </w:rPr></w:rPrDefault>
    <w:pPrDefault><w:pPr><w:spacing w:after="100" w:line="340" w:lineRule="auto"/></w:pPr></w:pPrDefault>
  </w:docDefaults>
  <w:style w:type="paragraph" w:default="1" w:styleId="Normal">
    <w:name w:val="Normal"/><w:qFormat/>
    <w:rPr><w:sz w:val="22"/><w:szCs w:val="22"/></w:rPr>
  </w:style>
  <w:style w:type="paragraph" w:styleId="Title">
    <w:name w:val="Title"/><w:basedOn w:val="Normal"/><w:next w:val="Normal"/><w:qFormat/>
    <w:pPr><w:spacing w:before="500" w:after="160"/><w:jc w:val="center"/></w:pPr>
    <w:rPr><w:b/><w:color w:val="17365D"/><w:sz w:val="44"/><w:szCs w:val="44"/></w:rPr>
  </w:style>
  <w:style w:type="paragraph" w:styleId="Subtitle">
    <w:name w:val="Subtitle"/><w:basedOn w:val="Normal"/><w:next w:val="Normal"/><w:qFormat/>
    <w:pPr><w:spacing w:before="40" w:after="360"/><w:jc w:val="center"/></w:pPr>
    <w:rPr><w:color w:val="49728F"/><w:sz w:val="30"/><w:szCs w:val="30"/></w:rPr>
  </w:style>
  <w:style w:type="paragraph" w:styleId="Heading1">
    <w:name w:val="heading 1"/><w:basedOn w:val="Normal"/><w:next w:val="Normal"/><w:qFormat/>
    <w:pPr><w:keepNext/><w:spacing w:before="280" w:after="140"/>
      <w:outlineLvl w:val="0"/></w:pPr>
    <w:rPr><w:b/><w:color w:val="1F4E79"/><w:sz w:val="32"/><w:szCs w:val="32"/></w:rPr>
  </w:style>
  <w:style w:type="paragraph" w:styleId="Heading2">
    <w:name w:val="heading 2"/><w:basedOn w:val="Normal"/><w:next w:val="Normal"/><w:qFormat/>
    <w:pPr><w:keepNext/><w:spacing w:before="220" w:after="100"/>
      <w:outlineLvl w:val="1"/></w:pPr>
    <w:rPr><w:b/><w:color w:val="2E74B5"/><w:sz w:val="26"/><w:szCs w:val="26"/></w:rPr>
  </w:style>
  <w:style w:type="paragraph" w:styleId="Code">
    <w:name w:val="Code"/><w:basedOn w:val="Normal"/>
    <w:pPr><w:shd w:val="clear" w:fill="F3F6F9"/><w:spacing w:after="0" w:line="260" w:lineRule="auto"/>
      <w:ind w:left="180" w:right="120"/></w:pPr>
    <w:rPr><w:rFonts w:ascii="Consolas" w:hAnsi="Consolas" w:eastAsia="等线"/>
      <w:sz w:val="18"/><w:szCs w:val="18"/></w:rPr>
  </w:style>
  <w:style w:type="paragraph" w:styleId="TableText">
    <w:name w:val="Table Text"/><w:basedOn w:val="Normal"/>
    <w:pPr><w:spacing w:after="35" w:line="250" w:lineRule="auto"/></w:pPr>
    <w:rPr><w:sz w:val="17"/><w:szCs w:val="17"/></w:rPr>
  </w:style>
  <w:style w:type="paragraph" w:styleId="ListText">
    <w:name w:val="List Text"/><w:basedOn w:val="Normal"/>
    <w:pPr><w:spacing w:after="55" w:line="320" w:lineRule="auto"/><w:ind w:left="360"/></w:pPr>
  </w:style>
</w:styles>"""


def settings_xml() -> str:
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:settings xmlns:w="{WORD_NS}">
  <w:zoom w:percent="100"/>
  <w:defaultTabStop w:val="420"/>
  <w:characterSpacingControl w:val="doNotCompress"/>
  <w:compat><w:compatSetting w:name="compatibilityMode" w:uri="http://schemas.microsoft.com/office/word" w:val="15"/></w:compat>
</w:settings>"""


def content_types_xml(image_count: int) -> str:
    return f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="xml" ContentType="application/xml"/>
  <Default Extension="png" ContentType="image/png"/>
  <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
  <Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
  <Override PartName="/word/settings.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.settings+xml"/>
  <Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>
  <Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>
</Types>"""


def build_document() -> None:
    markdown = SOURCE.read_text(encoding="utf-8")
    body, images = markdown_to_body(markdown)

    relationships = [
        '<Relationship Id="rIdStyles" '
        'Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" '
        'Target="styles.xml"/>',
        '<Relationship Id="rIdSettings" '
        'Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/settings" '
        'Target="settings.xml"/>',
    ]
    for index, (_, _) in enumerate(images, start=1):
        relationships.append(
            f'<Relationship Id="rIdImage{index}" '
            'Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" '
            f'Target="media/image{index}.png"/>'
        )

    document_relationships = (
        '<?xml version="1.0" encoding="UTF-8" standalone="yes"?>'
        '<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">'
        + "".join(relationships)
        + "</Relationships>"
    )
    root_relationships = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
  <Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/>
  <Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" Target="docProps/app.xml"/>
</Relationships>"""

    now = datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")
    core = f"""<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties"
 xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/"
 xmlns:dcmitype="http://purl.org/dc/dcmitype/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <dc:title>实验1-带键盘事件的计算器</dc:title>
  <dc:subject>Qt应用程序开发实验报告</dc:subject>
  <dc:creator>LZ431</dc:creator>
  <cp:lastModifiedBy>LZ431</cp:lastModifiedBy>
  <dcterms:created xsi:type="dcterms:W3CDTF">{now}</dcterms:created>
  <dcterms:modified xsi:type="dcterms:W3CDTF">{now}</dcterms:modified>
</cp:coreProperties>"""
    app = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties"
 xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes">
  <Application>Qt Course Report Generator</Application>
  <Company>DGUT</Company>
</Properties>"""

    with zipfile.ZipFile(OUTPUT, "w", compression=zipfile.ZIP_DEFLATED) as archive:
        archive.writestr("[Content_Types].xml", content_types_xml(len(images)))
        archive.writestr("_rels/.rels", root_relationships)
        archive.writestr("word/document.xml", document_xml(body))
        archive.writestr("word/styles.xml", styles_xml())
        archive.writestr("word/settings.xml", settings_xml())
        archive.writestr("word/_rels/document.xml.rels", document_relationships)
        archive.writestr("docProps/core.xml", core)
        archive.writestr("docProps/app.xml", app)
        for media_name, media_data in images:
            archive.writestr(f"word/{media_name}", media_data)

    print(f"Created {OUTPUT}")


if __name__ == "__main__":
    try:
        build_document()
    except Exception as error:  # pragma: no cover - command-line reporting
        print(f"Report generation failed: {error}", file=sys.stderr)
        raise
