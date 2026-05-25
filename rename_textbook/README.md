# book-rename.py

Suggest (and optionally apply) a uniform filename for a book PDF/EPUB.

**Output format:** `Title (Ne) - Author - Year [Publisher].ext`

Examples:

```
Effective C++ (3e) - Meyers - 2005 [AW].pdf
Designing Data-Intensive Applications - Kleppmann - 2017 [OReilly].pdf
Crafting Interpreters - Nystrom - 2021.pdf
```

## Requirements

- Python 3.6+ (uses only stdlib for EPUB)
- [`poppler`](https://poppler.freedesktop.org/) for PDF parsing — provides `pdfinfo` and `pdftotext`:

  ```bash
  brew install poppler
  ```

## Install

The script lives at `~/bin/book-rename.py` and is already executable. To call it without the full path, add `~/bin` to your `PATH`:

```bash
echo 'export PATH="$HOME/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

Then it's available as `book-rename.py`.

## Usage

```
book-rename.py [--rename] [--show]
               [--title T] [--edition N] [--author A] [--year Y] [--publisher P]
               FILE
```

### Dry-run (default)

Prints the suggested filename without touching the file:

```bash
book-rename.py mybook.pdf
# Suggested: Effective C++ (3e) - Meyers - 2005 [AW].pdf
# (dry-run; pass --rename to apply)
```

### Inspect what got extracted

Use `--show` to see the auto-extracted fields. Useful before adding overrides:

```bash
book-rename.py --show mybook.pdf
# title:     Effective C++
# edition:   3
# author:    Meyers
# year:      2005
# publisher: AW
```

### Apply the rename

```bash
book-rename.py --rename mybook.pdf
```

Refuses to overwrite an existing target file.

### Override fields

When the auto-extraction misses something, override any field manually. Overrides combine with whatever the script could detect:

```bash
book-rename.py --title "Effective C++" --edition 3 --author Meyers \
               --year 2005 --publisher "Addison Wesley" --rename mybook.pdf
```

You can override just the fields you need — the rest still come from extraction.

## What it extracts

| Field | Source |
|---|---|
| Title | PDF metadata `Title` / EPUB `<dc:title>`, cleaned of `z-lib`, `PDFDrive`, etc. |
| Author | PDF metadata `Author` / EPUB `<dc:creator>`, then shortened to last names |
| Edition | Regex on first 5 pages: `3rd Edition`, `Third Edition`, `(3e)` |
| Year | Largest 4-digit year near `Copyright`, `©`, `First published`, etc. |
| Publisher | First match in first 5 pages against a known publisher list |

### Author shortening

| Raw author | Shortened |
|---|---|
| `Scott Meyers` | `Meyers` |
| `Jim Blandy, Leonora Tindall` | `Blandy & Tindall` |
| `Cormen, Leiserson, Rivest, Stein` | `Cormen et al.` |

### Publisher abbreviations

Recognized publishers are mapped to short forms:

| Raw | Abbreviated |
|---|---|
| Packt Publishing | `Packt` |
| O'Reilly Media | `OReilly` |
| Addison-Wesley | `AW` |
| Morgan Kaufmann | `MK` |
| Pearson Education | `Pearson` |
| Manning Publications | `Manning` |
| Apress | `Apress` |
| Springer | `Springer` |
| Wiley | `Wiley` |
| MIT Press | `MIT` |
| Cambridge University Press | `Cambridge` |
| Elsevier | `Elsevier` |
| Pragmatic Bookshelf | `Pragmatic` |
| Microsoft Press | `Microsoft Press` |
| Wrox | `Wrox` |
| Hugging Face | `HuggingFace` |
| Sams | `Sams` |
| No Starch | `NoStarch` |
| Artima | `Artima` |
| Databricks | `Databricks` |
| Franklin, Beedle | `Franklin Beedle` |

To add a publisher, edit the `PUBLISHER_ABBR` list at the top of the script.

## Workflow

Auto-extraction is hit-or-miss because many PDFs have weak metadata. Recommended flow when renaming a new download:

1. **Inspect** what got extracted:
   ```bash
   book-rename.py --show ~/Downloads/sketchy_filename.pdf
   ```
2. **Preview** the suggestion, adding overrides for any wrong field:
   ```bash
   book-rename.py --title "Real Title" --edition 2 \
                  ~/Downloads/sketchy_filename.pdf
   ```
3. **Apply** once it looks right:
   ```bash
   book-rename.py --rename --title "Real Title" --edition 2 \
                  ~/Downloads/sketchy_filename.pdf
   ```

## Supported formats

- `.pdf` — uses `pdfinfo` + `pdftotext` from poppler
- `.epub` — reads `META-INF/container.xml` then the OPF Dublin Core metadata

Other extensions are rejected with an error.

## Notes

- Illegal filename characters (`/ \ : * ? " < > |`) are replaced with `-`
- Edition `1e` is dropped (only edition ≥ 2 is included in the name)
- The script never overwrites an existing file — if the target exists it exits with an error
