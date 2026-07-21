#!/usr/bin/env python3
"""
Command-line tool to format and output Loh source code in the terminal with syntax highlighting.
"""
import sys
import os
import tokenize
import io
import argparse

# Try to import colorization utilities from the standard library
try:
    from _colorize import get_theme, can_colorize, ANSIColors, Theme
except ImportError:
    # Minimal fallback colors if run outside CPython 3.13+ env
    class ANSIColors:
        RESET = "\x1b[0m"
        BOLD = "\x1b[1m"
        BLUE = "\x1b[34m"
        BOLD_BLUE = "\x1b[1;34m"
        CYAN = "\x1b[36m"
        GREEN = "\x1b[32m"
        RED = "\x1b[31m"
        YELLOW = "\x1b[33m"
        MAGENTA = "\x1b[35m"
        BOLD_MAGENTA = "\x1b[1;35m"

    class ThemeSection:
        def __init__(self, **kwargs):
            for k, v in kwargs.items():
                setattr(self, k, v)
        def __getitem__(self, key):
            return getattr(self, key, "")

    class Theme:
        def __init__(self):
            self.syntax = ThemeSection(
                keyword=ANSIColors.BOLD_BLUE,
                keyword_constant=ANSIColors.BOLD_BLUE,
                builtin=ANSIColors.CYAN,
                comment=ANSIColors.RED,
                string=ANSIColors.GREEN,
                number=ANSIColors.YELLOW,
                op=ANSIColors.RESET,
                definition=ANSIColors.BOLD,
                soft_keyword=ANSIColors.BOLD_BLUE,
                reset=ANSIColors.RESET
            )
            self.reset = ANSIColors.RESET

    def can_colorize(*, file=None):
        if os.environ.get("NO_COLOR"):
            return False
        if os.environ.get("FORCE_COLOR"):
            return True
        if file is None:
            file = sys.stdout
        return hasattr(file, "isatty") and file.isatty()

    def get_theme(*, tty_file=None, force_color=False, force_no_color=False):
        if force_no_color or (not force_color and not can_colorize(file=tty_file)):
            # Empty theme
            t = Theme()
            for k in vars(t.syntax):
                setattr(t.syntax, k, "")
            t.reset = ""
            return t
        return Theme()


def get_token_color(tok, prev_tok, next_tok, is_def, theme):
    """
    Determine the syntax highlighting escape code for a given token.
    """
    if is_def:
        return theme.syntax.definition

    if tok.type == tokenize.COMMENT:
        return theme.syntax.comment
    elif tok.type in (tokenize.STRING, 143, 144, 145, 146, 147, 148):  # FSTRING/TSTRING types
        return theme.syntax.string
    elif tok.type == tokenize.NUMBER:
        return theme.syntax.number
    elif tok.type == tokenize.NAME:
        import keyword
        if keyword.iskeyword(tok.string):
            if tok.string in ("True", "False", "None"):
                return theme.syntax.keyword_constant
            return theme.syntax.keyword
        elif keyword.issoftkeyword(tok.string):
            return theme.syntax.soft_keyword
        # Check builtins
        import builtins
        if hasattr(builtins, tok.string) and not tok.string.startswith("__"):
            return theme.syntax.builtin
        return None
    elif tok.type == tokenize.OP:
        # Check for shorthand self-dot:
        # '.' that is not preceded by NAME, NUMBER, STRING, RPAR, RSQB, RBRACE
        if tok.string == ".":
            is_self_dot = True
            if prev_tok is not None:
                prev_name = tokenize.tok_name.get(prev_tok.type, "")
                if prev_name in ("NAME", "NUMBER", "STRING") or prev_tok.string in (")", "]", "}"):
                    is_self_dot = False
            if is_self_dot:
                return theme.syntax.keyword_constant  # Color self-dot as a constant
            return theme.syntax.op

        # Loh-specific constants
        if tok.string in ("++", "--", "~", "!!", "!!!"):
            return theme.syntax.keyword_constant
        # Loh-specific control flow
        elif tok.string in (
            "?", "??", "?!", "??!", "$", "$?", "$>", "$<", "~^", "?^", "?^*",
            "?*", "?!^", "^^^", "^?!", "^?", "?==", "->", "<-", "<>", "::",
            "%", "&", "/"
        ):
            return theme.syntax.keyword
        # Operators corresponding to Python keywords
        elif tok.string in ("===", "!==", "&&", "||", "!", "<~", "!<~", "=>"):
            return theme.syntax.keyword
        else:
            return theme.syntax.op
    return None


def analyze_logical_line(tokens):
    """
    Analyzes logical line tokens and returns a dictionary mapping
    token local indices to their override categories (e.g. 'definition').
    """
    struct_tokens = [
        (idx, tok) for idx, tok in enumerate(tokens)
        if tok.type not in (tokenize.INDENT, tokenize.DEDENT, tokenize.NEWLINE, tokenize.NL, tokenize.COMMENT)
    ]
    if not struct_tokens:
        return {}

    overrides = {}

    # Skip decorator lines
    if struct_tokens[0][1].string == "@":
        return {}

    # Standard python def/class
    if struct_tokens[0][1].string == "def" and len(struct_tokens) > 1:
        if struct_tokens[1][1].type == tokenize.NAME:
            overrides[struct_tokens[1][0]] = 'definition'
            return overrides

    if struct_tokens[0][1].string == "class" and len(struct_tokens) > 1:
        if struct_tokens[1][1].type == tokenize.NAME:
            overrides[struct_tokens[1][0]] = 'definition'
            return overrides

    # Loh class definition: ClassName:: or ClassName:(Bases):
    if (struct_tokens[0][1].type == tokenize.NAME and 
        len(struct_tokens) >= 2 and 
        struct_tokens[1][1].string == ":" and 
        struct_tokens[-1][1].string == ":"):
        overrides[struct_tokens[0][0]] = 'definition'
        return overrides

    # Loh function definition: func(...) [-> type] :
    idx = 0
    if idx < len(struct_tokens) and struct_tokens[idx][1].string == "%":
        idx += 1
    if idx < len(struct_tokens) and struct_tokens[idx][1].string == ".":
        idx += 1
    
    if (idx + 1 < len(struct_tokens) and 
        struct_tokens[idx][1].type == tokenize.NAME and 
        struct_tokens[idx+1][1].string == "("):
        if struct_tokens[-1][1].string == ":":
            overrides[struct_tokens[idx][0]] = 'definition'
            return overrides

    return overrides


def highlight_loh(source_code: str, theme) -> str:
    """
    Highlights Loh source code and returns the formatted string with ANSI color codes.
    """
    lines = source_code.splitlines(keepends=True)
    line_offsets = [0]
    for line in lines:
        line_offsets.append(line_offsets[-1] + len(line))

    def get_offset(line, col):
        return line_offsets[line - 1] + col

    sio = io.StringIO(source_code)
    try:
        tokens = list(tokenize.generate_tokens(sio.readline))
    except Exception:
        # Fallback to uncolored source code on parse error
        return source_code

    # Group tokens by logical line
    logical_lines = []
    current_line = []
    for tok in tokens:
        current_line.append(tok)
        if tok.type in (tokenize.NEWLINE, tokenize.ENDMARKER):
            if current_line:
                logical_lines.append(current_line)
                current_line = []

    # Map token absolute indices to overrides
    flat_tokens = []
    overrides = {}
    for line in logical_lines:
        line_start_idx = len(flat_tokens)
        flat_tokens.extend(line)
        line_overrides = analyze_logical_line(line)
        for local_idx, category in line_overrides.items():
            overrides[line_start_idx + local_idx] = category

    # Build sliding window window for tokens
    token_window = []
    padded_tokens = [None] + flat_tokens + [None]
    for i in range(1, len(padded_tokens) - 1):
        token_window.append((padded_tokens[i-1], padded_tokens[i], padded_tokens[i+1]))

    out = io.StringIO()
    current_offset = 0
    
    reset_code = getattr(theme, 'reset', ANSIColors.RESET)

    for idx, (prev_tok, tok, next_tok) in enumerate(token_window):
        if tok.type == tokenize.ENDMARKER:
            continue
        start_offset = get_offset(*tok.start)
        end_offset = get_offset(*tok.end)

        # Output any skipped characters (like whitespace)
        if start_offset > current_offset:
            out.write(source_code[current_offset:start_offset])

        # Output the colored token
        tok_str = source_code[start_offset:end_offset]
        is_def = overrides.get(idx) == 'definition'
        color = get_token_color(tok, prev_tok, next_tok, is_def, theme)
        if color:
            out.write(f"{color}{tok_str}{reset_code}")
        else:
            out.write(tok_str)

        current_offset = end_offset

    if current_offset < len(source_code):
        out.write(source_code[current_offset:])

    return out.getvalue()


def main():
    parser = argparse.ArgumentParser(description="Output Loh code in the terminal with syntax highlighting.")
    parser.add_argument("file", nargs="?", default="-", help="The Loh file to highlight (defaults to stdin).")
    group = parser.add_mutually_exclusive_group()
    group.add_argument("--color", action="store_true", default=None, help="Force color output.")
    group.add_argument("--no-color", action="store_true", default=None, help="Disable color output.")

    args = parser.parse_args()

    # Read source code
    if args.file == "-":
        source_code = sys.stdin.read()
    else:
        try:
            with open(args.file, "r", encoding="utf-8") as f:
                source_code = f.read()
        except OSError as e:
            print(f"Error: {e}", file=sys.stderr)
            sys.exit(1)

    # Determine theme
    force_color = args.color is True
    force_no_color = args.no_color is True
    theme = get_theme(
        tty_file=sys.stdout,
        force_color=force_color,
        force_no_color=force_no_color
    )

    highlighted = highlight_loh(source_code, theme)
    sys.stdout.write(highlighted)


if __name__ == "__main__":
    main()
