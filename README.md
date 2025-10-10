Fatshark's customized branch of LuaJIT.

# Description

This is Fatshark's version of LuaJIT that is used in the Stingray engine.
It is not a fork, as it is regularly synchronized with the upstream
LuaJIT project (https://github.com/LuaJIT/LuaJIT).

# Changes

- Add luaFS_newstate to plug in allocators in 64-bit mode w/o GC64.
- Add LUAFS_GCESTIMATE that returns an estimate of uncollected garbage.
- Change the bytecode header to \x1bFS as a step towards private modifications.
- Add string.fatshark.fixstrid to ease porting legacy code that depends on [`pairs()` order](https://luajit.org/faq.html#order).
- Change VM security values to suit performance needs.
- Add .pdb to .gitignore.
- Set ERRORLEVEL on msvcbuild.bat failure.
- Add table.fatshark.size to get raw array/hash sizes.
- Add line numbers and constants to bc.lua output.
- Add table.fatshark.dup.
- Add table.fatshark.keys.
- Add table.fatshark.any.
- Add table.fatshark.count.
- Add string.fatshark.hash.

# License

LuaJIT is Copyright (C) 2005-2025 Mike Pall.

Additional patches are copyrighted by Fatshark Studios AB:

Copyright (C) 2017-2019 Fatshark Studios AB. All rights reserved.