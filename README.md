# Fatshark's customized branch of LuaJIT.

## Description

This is Fatshark's version of LuaJIT that is used in the Stingray engine.
It is not a fork, as it is regularly synchronized with the upstream
LuaJIT project (https://github.com/LuaJIT/LuaJIT).

## Building

Compile all platforms and package a SPM zip:

```
cmake --workflow default
```

Install locally in SR_LIB_DIR:

```
cmake --install build
```

## Changes

- Add `luaFS_newstate` to plug in allocators in 64-bit mode w/o GC64.
- Add `LUAFS_GCESTIMATE` that returns an estimate of memory actually in use.
- Change the bytecode header to `\x1bFS` as a step towards private modifications.
- Change VM security values to suit performance needs.
- Add .pdb to .gitignore.
- Set ERRORLEVEL on msvcbuild.bat failure.
- Add line numbers and constants to bc.lua output.
- Add a CMake wrapper around existing buildscripts.
- Add `luaFS_sizeof` to get the size in bytes of objects.

## Functions

Functions tagged with *[Debug]* are exclusively for developer tooling. They must **not** be used in production code.

| API | Description |
| --- | --- |
| `local copy = table.fatshark.dup(t [, maxdepth])` | Returns a deep clone of the given table. Throws an error if `maxdepth` is exceeded. |
| `local bool = table.fatshark.empty(tab)` | Returns true if a table is empty and false otherwise. |
| `local tkeys, nkeys = table.fatshark.keys(t [, out])` | Returns an array of the given table's keys. Order is unspecified. |
| `local asize, hmask = table.fatshark.size(tab)` | *[Debug]* Returns the internal array & hash sizes of a table. |
| `local n = table.fatshark.count(tab) ` | Returns the total number of entries in a table. |
| `local dst = table.fatshark.merge(t, dst)` | Copies entries from a source to a destination table. |
| `string.fatshark.fixstrid()` |  *[Debug]* Ease porting legacy code that depends on [`pairs()` order](https://luajit.org/faq.html#order). |
| `local hash, sid = string.fatshark.hash(str)` | *[Debug]* List all new or modified files |
| `local bytes = jit.fatshark.sizeof(value)` | *[Debug]* Returns the size of a value in bytes. Returns nil if not known. |

# License

LuaJIT is Copyright (C) 2005-2025 Mike Pall.

Additional patches are copyrighted by Fatshark Studios AB:

Copyright (C) 2017-2019 Fatshark Studios AB. All rights reserved.