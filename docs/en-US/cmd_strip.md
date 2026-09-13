# Strip Command

`strip` builds a **runtime-oriented** tree from the current platform's installed directory into `workspace/stripped/<platform>/<project>/<buildType>/`.

It handles **ELF** (Linux and similar) and **PE** (Windows `.exe` / `.dll`) binaries: symbols are removed with `toolchain.strip`; shell scripts, configs, and other runtime data are copied. Headers, static libraries, CMake / pkg-config metadata, PDBs, and similar build-only artifacts are omitted. The `installed/` tree itself is left unchanged.

## Command Syntax

```shell
celer strip
```

No flags or arguments. Uses the current workspace context (`platform`, `project`, `build_type`).

## Prerequisites

- The workspace is configured and the target `installed` tree already has packages.
- `toolchain.strip` **must** be set and the tool must exist under the toolchain directory:
  - Linux: e.g. `x86_64-linux-gnu-strip`
  - Windows: e.g. `llvm-strip.exe` (must support PE)

If `strip` is not configured, the command fails; it does not fall back to copy-only.

See [Platform configuration](./article_platform.md).

## What Is Produced

| Action | Content |
|--------|---------|
| **strip** | ELF / PE executables and shared libraries |
| **copy** | `.so` version symlinks, shell/config and other runtime data |
| **skip** | Static libs (`.a` / `.lib`), headers, `.cmake` / `.pc`, `.pdb`, `.exp` / `.ilk`, object files, etc. |
| **skip dirs** | `include`, `cmake`, `pkgconfig`, `man`, `doc`, `info`, `aclocal` |

Example log:

```text
[✔] [copy ] lib/libffi.so
[✔] [copy ] lib/libffi.so.8
[✔] [strip] lib/libffi.so.8.1.4
```

Windows example:

```text
[✔] [strip] bin/app.exe
[✔] [strip] bin/foo.dll
[✔] [copy ] etc/app.conf
```

## Common Examples

```shell
# After install or deploy, build the runtime tree
celer install glog@0.6.0
celer strip

# Same logic after a successful deploy
celer deploy --strip
```

## Notes

- Output path: `stripped/<platform>/<project>/<buildType>/` (same library folder layout as `installed`).
- `deploy --strip` shares the same implementation as `celer strip`. Run `strip` alone when you want to regenerate the runtime tree from an existing install.
- Missing `toolchain.strip`, or a configured tool that cannot be found, both cause an error.
