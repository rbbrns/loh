# Loh Syntax (VS Code)

This extension adds syntax highlighting for `*.loh` files.

## What it highlights

- Standard Python syntax (via embedded Python grammar)
- Loh control symbols such as `?`, `??`, `?!`, `??!`, `$`, `$?`, `$<<`, `$>>`, `~^`, `?^`, `?^*`, `?*`, `?!^`, `^^^`, `^?!`, `^?`, `?==`, `->`, `~>`, `<>`, `::`
- Loh operators such as `===`, `!==`, `<~`, `!<~`, `=>`, `&&`, `||`, `|>`, and unary `!`
- Loh boolean aliases `++`, `--`, plus `!!` and `!!!`
- Loh import syntax using `/` including relative imports (`/ . / module`, `/ .. / module`)
- Loh async/with line prefixes `%` and `&`
- Loh shorthand dot-self forms (like `.x`, `.method()`, and `.`)
- Loh implicit default assignment forms like `=name` and `=obj.attr`

## Install locally

1. Open VS Code.
2. Run `Extensions: Install from Location...`.
3. Select `Tools/vscode-loh-syntax` in this repository.
