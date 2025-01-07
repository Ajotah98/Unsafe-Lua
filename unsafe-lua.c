#include "unsafe-lua.h"
#define UNSAFE_REGISTRY_KEY "unsafe_registry"

static int store_persistent_reference(lua_State *L, void *ptr) {
    // Create references table
    lua_pushstring(L, UNSAFE_REGISTRY_KEY);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushstring(L, UNSAFE_REGISTRY_KEY);
        lua_pushvalue(L, -2);
        lua_rawset(L, LUA_REGISTRYINDEX);
    }
    
    // Store the userdata
    lua_pushlightuserdata(L, ptr);
    lua_pushvalue(L, -3);  // Copy the userdata
    lua_rawset(L, -3);
    lua_pop(L, 1);  // Erase the references tables
    
    return 1;
}


typedef struct {
    void *base;
    size_t size;
    size_t offset;
} MemoryBlock;


// *** Heap ***
int unsafe_heap_malloc(lua_State *L) {
    size_t size = luaL_checkinteger(L, 1);
    
    void *ptr = malloc(size);
    if (!ptr) {
        return luaL_error(L, "malloc failed");
    }
    
    // Create a userdata to store metamethods
    void **udata = lua_newuserdata(L, sizeof(void *));
    *udata = ptr;
    luaL_getmetatable(L, "Pointer");
    lua_setmetatable(L, -2);
    
    return store_persistent_reference(L, ptr);
}

int unsafe_heap_free(lua_State *L) {
    void **udata = luaL_checkudata(L, 1, "Pointer");
    if (!udata || !*udata) {
        return luaL_error(L, "Attempt to free a NULL pointer");
    }
    
    // Erase the reference
    lua_pushstring(L, UNSAFE_REGISTRY_KEY);
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_pushlightuserdata(L, *udata);
    lua_pushnil(L);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    free(*udata);
    *udata = NULL;

    return 0;
}


// *** Arena ***
typedef struct {
    MemoryBlock memory;
} Arena;


int unsafe_arena_new(lua_State *L) {
    size_t size = luaL_checkinteger(L, 1);
    
    Arena *arena = lua_newuserdata(L, sizeof(Arena));
    arena->memory.base = malloc(size);
    arena->memory.size = size;
    arena->memory.offset = 0;
    
    luaL_getmetatable(L, "Arena");
    lua_setmetatable(L, -2);
    
    return store_persistent_reference(L, arena);
}

int unsafe_arena_alloc(lua_State *L) {
    Arena *arena = lua_touserdata(L, 1);
    size_t size = luaL_checkinteger(L, 2);

    if (arena->memory.offset + size > arena->memory.size) {
        lua_pushnil(L);
        lua_pushstring(L, "Arena out of memory");
        return 2;
    }

    void *ptr = (char *)arena->memory.base + arena->memory.offset;
    arena->memory.offset += size;

    void **ptr_userdata = lua_newuserdata(L, sizeof(void *));
    *ptr_userdata = ptr;

    luaL_getmetatable(L, "Pointer");
    lua_setmetatable(L, -2);

    return 1;
}

int unsafe_arena_reset(lua_State *L) {
    Arena *arena = lua_touserdata(L, 1);
    arena->memory.offset = 0;
    return 0;
}

int unsafe_arena_free(lua_State *L) {
    Arena *arena = lua_touserdata(L, 1);
    if (arena->memory.base) {
        free(arena->memory.base);
        arena->memory.base = NULL;
    }
    return 0;
}

// *** Stack ***
typedef struct {
    MemoryBlock memory;
} Stack;

int unsafe_stack_new(lua_State *L) {
    size_t size = luaL_checkinteger(L, 1);

    Stack *stack = lua_newuserdata(L, sizeof(Stack));

    stack->memory.base = malloc(size);
    stack->memory.size = size;
    stack->memory.offset = 0;

    luaL_getmetatable(L, "stack");
    lua_setmetatable(L, -2);

    return store_persistent_reference(L, stack);
}

int unsafe_stack_push(lua_State *L) {
    Stack *stack = lua_touserdata(L, 1);
    size_t value_size;
    const char *value = lua_tolstring(L, 2, &value_size);

    if (stack->memory.offset + value_size > stack->memory.size) {
        return luaL_error(L, "Stack overflow");
    }

    memcpy((char *)stack->memory.base + stack->memory.offset, value, value_size);
    stack->memory.offset += value_size;

    return 0;
}

int unsafe_stack_pop(lua_State *L) {
    Stack *stack = lua_touserdata(L, 1);
    if (stack->memory.offset == 0) {
        return luaL_error(L, "Stack underflow");
    }

    stack->memory.offset--;
    lua_pushinteger(L, ((char *)stack->memory.base)[stack->memory.offset]);
    return 1;
}

int unsafe_stack_reset(lua_State *L) {
    Stack *stack = lua_touserdata(L, 1);
    stack->memory.offset = 0;
    return 0;
}

int unsafe_stack_free(lua_State *L) {
    Stack *stack = lua_touserdata(L, 1);
    if (stack->memory.base) {
        free(stack->memory.base);
        stack->memory.base = NULL;
    }
    return 0;
}

// *** Pointer ***
int unsafe_pointer_write(lua_State *L) {
    void **address = lua_touserdata(L, 1);
    size_t offset = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);

    char *ptr = (char *)(*address) + offset;
    *ptr = (char)value;

    return 0;
}

int unsafe_pointer_read(lua_State *L) {
    void **address = lua_touserdata(L, 1);
    size_t offset = luaL_checkinteger(L, 2);

    char *ptr = (char *)(*address) + offset;
    lua_pushinteger(L, *ptr);

    return 1;
}

int unsafe_pointer_is_null(lua_State *L) {
    void **ptr = lua_touserdata(L, 1);
    lua_pushboolean(L, ptr == NULL || *ptr == NULL);
    return 1;
}

static const luaL_Reg unsafe_functions[] = {
    {"heap_malloc", unsafe_heap_malloc},
    {"heap_free", unsafe_heap_free},
    {"arena_new", unsafe_arena_new},
    {"arena_alloc", unsafe_arena_alloc},
    {"arena_reset", unsafe_arena_reset},
    {"arena_free", unsafe_arena_free},
    {"stack_new", unsafe_stack_new},
    {"stack_push", unsafe_stack_push},
    {"stack_pop", unsafe_stack_pop},
    {"stack_reset", unsafe_stack_reset},
    {"stack_free", unsafe_stack_free},
    {NULL, NULL}
};

static const luaL_Reg pointer_methods[] = {
    {"write", unsafe_pointer_write},
    {"read", unsafe_pointer_read},
    {"is_null", unsafe_pointer_is_null},
    {NULL, NULL}
};

int luaopen_unsafe(lua_State *L) {
    // Create first the Pointer metatable
    luaL_newmetatable(L, "Pointer");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, pointer_methods, 0);
    lua_pop(L, 1);
    
    lua_newtable(L);
    
    luaL_setfuncs(L, unsafe_functions, 0);
    
    return 1;
}