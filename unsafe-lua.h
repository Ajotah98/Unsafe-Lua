#ifndef UNSAFE_H
#define UNSAFE_H

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdlib.h>
#include <string.h>

// Heap
int unsafe_heap_malloc(lua_State *L);
int unsafe_heap_free(lua_State *L);

// Arena
int unsafe_arena_new(lua_State *L);
int unsafe_arena_alloc(lua_State *L);
int unsafe_arena_reset(lua_State *L);
int unsafe_arena_free(lua_State *L);

// Stack
int unsafe_stack_new(lua_State *L);
int unsafe_stack_push(lua_State *L);
int unsafe_stack_pop(lua_State *L);
int unsafe_stack_reset(lua_State *L);
int unsafe_stack_free(lua_State *L);

// Pointer
int unsafe_pointer_write(lua_State *L);
int unsafe_pointer_read(lua_State *L);
int unsafe_pointer_is_null(lua_State *L);

// Lua lib registers
int luaopen_unsafe(lua_State *L);

#endif // UNSAFE_H

