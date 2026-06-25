# Chipojo Packages

## Create a scoped package

```bash
chpm create @creator/module
```

Or create it with only the module name and enter the author when chpm asks:

```bash
chpm create module
author username: @creator
```

This creates:

```text
@creator/module/
├── package.json
├── main.chp
├── README.md
├── PUBLISHING.md
└── lib/
```

Each created package is its own chpm project.

## MVP architecture

`chpm` is a minimal npm-like package manager for local packages:

- `packets_chpm_create/` is the package repository.
- A package can be a single `name.chp` file or a folder with `package.json`.
- Folder packages use the `package.json` `name`, `main`, and `dependencies` fields.
- `chpm install <package>` copies the package into the consumer project's `chpm_modules/`.
- Dependencies listed by the installed package are installed first.
- The Chipojo runtime resolves imports from `chpm_modules/`, similar to how Node resolves bare module names from `node_modules/`.

`main.chp` can export a default namespace with functions:

```chipojo
export default module {
    func hello(name) {
        return "Hello " + name + " from @creator/module"
    }
}
```

## Install a scoped package

```bash
chpm install @creator/module
```

The package is copied to:

```text
chpm_modules/@creator/module/
├── package.json
├── main.chp
└── lib/
```

If an admin moves a created package folder into `packets_chpm_create/@creator/module/`,
`chpm install @creator/module` detects it from that repository folder. For short names,
`chpm install module` also searches scoped folders like `packets_chpm_create/@creator/module/`.

## Import from a scoped package

Only installed packages can be imported. Use the package name in code. The `@creator` directory is for chpm ownership/registry layout:

```chipojo
import module from "module"

show(module.hello("Chipojo"))
```

Named exports still work when the package declares `export func`:

```chipojo
from "module" import { hello }
```

Unscoped modules continue to work:

```chipojo
import math
from credit import makeCreditPackage
```

## Publishing

Users do not write to the chpm repository or database directly. They create `@username/module` and send that project to a chpm admin. The admin reviews it and uploads it to the official repo/database.

## Full example

Repository package:

```text
packets_chpm_create/@revaydev/random/
├── package.json
└── main.chp
```

`package.json`:

```json
{
  "name": "@revaydev/random",
  "version": "1.0.0",
  "main": "main.chp",
  "dependencies": {}
}
```

Install in a consumer project:

```bash
chpm init
chpm install @revaydev/random
```

Use it:

```chipojo
import random from "random"

show(random.hello("Chipojo"))
```

Resolution flow:

1. `chpm install` finds the folder in `packets_chpm_create/`.
2. It reads `package.json`, installs dependencies, and copies the folder to `chpm_modules/@revaydev/random/`.
3. The runtime sees `import random from "random"`.
4. It searches `chpm_modules/random.chp`, `chpm_modules/random/<main>`, then scoped folders like `chpm_modules/@revaydev/random/<main>`.
5. `<main>` comes from `package.json`; if missing, `main.chp` is used.

Node does the same idea for JavaScript: for `require("pkg")` or `import "pkg"`, it walks up parent directories looking for `node_modules/pkg`, reads `node_modules/pkg/package.json`, uses `main` or `exports`, and finally falls back to files such as `index.js`. chpm keeps only the local project search and the `main` field for the MVP.
