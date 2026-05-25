#!/usr/bin/env python3
"""
Suggest a uniform filename for a book PDF/EPUB and optionally rename it.

Output format:
    Title (Ne) - Author - Year [Publisher].ext

Auto-extracted fields can be overridden with flags. Without --rename the
script just prints the suggestion; with --rename it renames in place.

Requires: poppler (pdfinfo, pdftotext) for PDFs. EPUB uses stdlib only.

Examples:
    book-rename.py mybook.pdf
    book-rename.py --rename mybook.pdf
    book-rename.py --title "Effective C++" --edition 3 --author Meyers \\
                   --year 2005 --publisher AW --rename mybook.pdf
"""

import argparse
import re
import subprocess
import sys
import zipfile
from pathlib import Path

# Map raw publisher strings (lowercase, substring match) to short abbreviations.
PUBLISHER_ABBR = [
    ("packt", "Packt"),
    ("o'reilly", "OReilly"),
    ("oreilly", "OReilly"),
    ("o reilly", "OReilly"),
    ("addison-wesley", "AW"),
    ("addison wesley", "AW"),
    ("pearson", "Pearson"),
    ("manning", "Manning"),
    ("no starch", "NoStarch"),
    ("nostarch", "NoStarch"),
    ("morgan kaufmann", "MK"),
    ("springer", "Springer"),
    ("wiley", "Wiley"),
    ("apress", "Apress"),
    ("mit press", "MIT"),
    ("cambridge university press", "Cambridge"),
    ("cambridge", "Cambridge"),
    ("elsevier", "Elsevier"),
    ("pragmatic", "Pragmatic"),
    ("microsoft press", "Microsoft Press"),
    ("wrox", "Wrox"),
    ("hugging face", "HuggingFace"),
    ("huggingface", "HuggingFace"),
    ("sams", "Sams"),
    ("artima", "Artima"),
    ("databricks", "Databricks"),
    ("franklin, beedle", "Franklin Beedle"),
]

EDITION_WORDS = {
    "first": 1, "second": 2, "third": 3, "fourth": 4,
    "fifth": 5, "sixth": 6, "seventh": 7, "eighth": 8,
}


def run(cmd):
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        return r.stdout
    except FileNotFoundError:
        sys.stderr.write(f"Missing tool: {cmd[0]}. Install with: brew install poppler\n")
        sys.exit(1)
    except Exception:
        return ""


def extract_pdf(path):
    info = {}
    out = run(["pdfinfo", str(path)])
    for line in out.splitlines():
        if ":" in line:
            k, _, v = line.partition(":")
            info[k.strip()] = v.strip()
    text = run(["pdftotext", "-l", "5", str(path), "-"])
    return info.get("Title", ""), info.get("Author", ""), text


def extract_epub(path):
    title = author = ""
    text = ""
    try:
        with zipfile.ZipFile(path) as z:
            container = z.read("META-INF/container.xml").decode("utf-8", "ignore")
            m = re.search(r'full-path="([^"]+)"', container)
            if not m:
                return "", "", ""
            opf = z.read(m.group(1)).decode("utf-8", "ignore")
            t = re.search(r"<dc:title[^>]*>([^<]+)</dc:title>", opf)
            a = re.search(r"<dc:creator[^>]*>([^<]+)</dc:creator>", opf)
            if t:
                title = t.group(1).strip()
            if a:
                author = a.group(1).strip()
            return title, author, opf
    except Exception:
        return title, author, text


def find_edition(text):
    m = re.search(r"\b(\d+)(?:st|nd|rd|th)\s+Edition\b", text, re.IGNORECASE)
    if m:
        return m.group(1)
    for word, num in EDITION_WORDS.items():
        if re.search(rf"\b{word}\s+edition\b", text, re.IGNORECASE):
            return str(num)
    m = re.search(r"\((\d+)e\)", text)
    if m:
        return m.group(1)
    return ""


def find_year(text):
    candidates = []
    for m in re.finditer(
        r"(?:Copyright|©|\(c\)|First published|Published|First Edition)[^\n]{0,80}?(\d{4})",
        text,
        re.IGNORECASE,
    ):
        y = int(m.group(1))
        if 1980 <= y <= 2099:
            candidates.append(y)
    if candidates:
        return str(max(candidates))
    return ""


def find_publisher(text):
    low = text.lower()
    for needle, abbr in PUBLISHER_ABBR:
        if needle in low:
            return abbr
    return ""


def shorten_author(author):
    """Trim authors to LastName or LastA & LastB or LastA et al."""
    if not author:
        return ""
    # Split on common separators
    parts = re.split(r"\s*(?:,|;|&|\band\b|/)\s*", author)
    parts = [p.strip() for p in parts if p.strip()]
    if not parts:
        return author
    # If full names, keep last word (last name) of each
    last_names = []
    for p in parts:
        toks = p.split()
        last_names.append(toks[-1] if toks else p)
    if len(last_names) == 1:
        return last_names[0]
    if len(last_names) == 2:
        return f"{last_names[0]} & {last_names[1]}"
    return f"{last_names[0]} et al."


def clean_title(title):
    if not title:
        return ""
    # Strip common garbage
    t = re.sub(r"\b(z-?lib(\.org)?|PDFDrive|PDF Room|ebook)\b", "", title, flags=re.IGNORECASE)
    t = re.sub(r"[\[\]{}]", "", t)
    t = re.sub(r"\s+", " ", t).strip(" -_,.")
    return t


def sanitize(s):
    # Replace OS-unsafe chars
    return re.sub(r"[/\\:*?\"<>|]", "-", s).strip()


def build_filename(title, edition, author, year, publisher, ext):
    title = sanitize(clean_title(title)) or "Untitled"
    s = title
    if edition:
        s += f" ({edition}e)"
    tail = []
    if author:
        tail.append(sanitize(author))
    if year:
        tail.append(year)
    if tail:
        s += " - " + " - ".join(tail)
    if publisher:
        s += f" [{sanitize(publisher)}]"
    return s + ext


def main():
    ap = argparse.ArgumentParser(description="Suggest/apply a uniform book filename.")
    ap.add_argument("file", help="PDF or EPUB to rename")
    ap.add_argument("--rename", action="store_true", help="Apply the rename")
    ap.add_argument("--title")
    ap.add_argument("--edition", help="Edition number, e.g. 3 (just the digit)")
    ap.add_argument("--author", help="Author string; will be shortened to last names")
    ap.add_argument("--year")
    ap.add_argument("--publisher", help="Publisher; will be abbreviated if recognized")
    ap.add_argument("--show", action="store_true", help="Show extracted fields and exit")
    args = ap.parse_args()

    path = Path(args.file).expanduser().resolve()
    if not path.exists():
        sys.exit(f"Not found: {path}")
    ext = path.suffix.lower()
    if ext not in (".pdf", ".epub"):
        sys.exit(f"Unsupported extension: {ext}")

    if ext == ".pdf":
        meta_title, meta_author, text = extract_pdf(path)
    else:
        meta_title, meta_author, text = extract_epub(path)

    blob = meta_title + "\n" + meta_author + "\n" + text

    title = args.title or clean_title(meta_title) or path.stem
    edition = args.edition or find_edition(blob)
    author = shorten_author(args.author or meta_author)
    year = args.year or find_year(blob)
    publisher = args.publisher or find_publisher(blob)
    # If user passed --publisher in raw form, try to abbreviate
    if args.publisher:
        pl = args.publisher.lower()
        for needle, abbr in PUBLISHER_ABBR:
            if needle in pl:
                publisher = abbr
                break

    if args.show:
        print(f"title:     {title}")
        print(f"edition:   {edition or '(none)'}")
        print(f"author:    {author or '(none)'}")
        print(f"year:      {year or '(none)'}")
        print(f"publisher: {publisher or '(none)'}")
        return

    new_name = build_filename(title, edition, author, year, publisher, ext)
    new_path = path.with_name(new_name)

    print(f"Suggested: {new_name}")
    if args.rename:
        if new_path == path:
            print("(filename unchanged)")
            return
        if new_path.exists():
            sys.exit(f"Target exists, refusing to overwrite: {new_path}")
        path.rename(new_path)
        print(f"Renamed -> {new_path}")
    else:
        print("(dry-run; pass --rename to apply)")


if __name__ == "__main__":
    main()
