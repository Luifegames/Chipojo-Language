# Chipojo Language - VS Code Extension

Syntax highlighting, code snippets and language support for the [Chipojo programming language](https://github.com/Luifegames/Chipojo-Language).

## Features

- **Syntax highlighting** for `.chp` files
- **Code snippets** for autocomplete: `if`, `while`, `func`, `show`, `export`, `import`, `export default`, named imports, arrow functions, list/dict methods, and more
- **Language configuration**: bracket matching, auto-closing pairs, comment toggling

## What's New in v0.2.0

- Arrow functions `=>`
- `const`/`var`/`let` declarations
- Type annotations: `float`, `string`, `bool`
- `export` / `export default`
- Package/module system with `chpm`
- `import ... from`, `from ... import`, `import { ... } from`, `from "..." import { ... }`, `import ... as`
- `input`, `number`, `kit` builtins
- Extension icon shown from `icon.svg`

## Install

1. Download the `.vsix` from Releases
2. In VS Code: Extensions view → `...` → Install from VSIX

## Building from source

```bash
npm install -g @vscode/vsce
vsce package
```

This generates `chipojo-language-0.2.0.vsix`.
