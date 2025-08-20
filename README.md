Fatshark's customized branch of LuaJIT.

# Description

This is Fatshark's version of LuaJIT that is used in the Stingray engine.
It is not a fork, as it is regularly synchronized with the upstream
LuaJIT project (https://github.com/LuaJIT/LuaJIT).

# Changes

- Add luaFS_newstate to plug in allocators in 64-bit mode w/o GC64.
- Add LUAFS_GCESTIMATE that returns an estimate of uncollected garbage.

# License

LuaJIT is Copyright (C) 2005-2025 Mike Pall.

Additional patches are copyrighted by Fatshark Studios AB:

Copyright (C) 2017-2019 Fatshark Studios AB. All rights reserved.