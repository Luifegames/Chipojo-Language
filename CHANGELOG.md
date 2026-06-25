# Chipojo Changes

## Branch plan

### master

- Kept master changes limited to basic hardening and metadata fixes.
- Hardened value ownership for lists, dictionaries and modules with deep copies when values are stored or exported.
- Replaced shell-based directory creation in `chpm` with native recursive directory creation.
- Added package-name validation in `chpm` to reject absolute paths, `..`, empty segments and invalid characters.
- Declared the VS Code extension icon through `chipojo-vscode/package.json`.

### dev

- Added dev language features: `class`, `void`, `switch`, and the `mientras` alias for `while`.
- Added `def` as an alias for `func`.
- Added scoped package layout support: `chpm_modules/@creator/module/main.chp`.
- Added short-name imports for scoped chpm packages:
  - `import module from "module"`
  - `import { hello } from "module"`
- Package examples now use `export default hello` instead of default metadata objects.
- Added named imports with braces:
  - `import { hello } from "module"`
  - `from "module" import { hello }`
- Updated `chpm create` to ask for an author when only a module name is provided and generate `@author/module` as a package project.
- Documented admin-only publishing: users create `@username/module`, then a chpm admin uploads it to the official repository/database.
- Updated VS Code grammar/snippets for dev keywords, named imports and scoped package examples.

## Notes

- Exported functions do not capture private module variables yet. Generated package functions use literal package metadata until closures/module environments are implemented.
- This workspace does not currently expose a valid Git repository, so the branch split above is documented for applying these changes to real `master` and `dev` branches.
