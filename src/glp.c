
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <GLFW/glfw3.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

static GLFWwindow *window;

static lua_State *lua_create(void);
static void       lua_destroy(lua_State *L);

static int luaopen_glp(lua_State *L);

static void luaX_loadbase(lua_State *L);
static void luaX_preload(lua_State *L, luaL_Reg *M);

static int l_register(lua_State *L);
static int l_unregister(lua_State *L);
static int l_keyname(lua_State *L);
static int l_modmask(lua_State *L);

static void glfw_keycb(GLFWwindow *, int, int, int, int);
static void glfw_closecb(GLFWwindow *);

static luaL_Reg libs[] = {
  { LUA_COLIBNAME, luaopen_coroutine },
  { LUA_TABLIBNAME, luaopen_table },
  { LUA_IOLIBNAME, luaopen_io },
  { LUA_OSLIBNAME, luaopen_os },
  { LUA_STRLIBNAME, luaopen_string },
  { LUA_MATHLIBNAME, luaopen_math },
  { LUA_UTF8LIBNAME, luaopen_utf8 },
  { LUA_DBLIBNAME, luaopen_debug },

  { "glp", luaopen_glp },

  { NULL, NULL },
};

static luaL_Reg mods[] = {
  { "register", l_register },
  { "unregister", l_unregister },
  { "modmask", l_modmask },
  { "keyname", l_keyname },
  { NULL, NULL },
};

int
main(int argc,char **argv){
  lua_State *L;
  if(!glfwInit())
    return EXIT_FAILURE;

  L = lua_create();

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  window = glfwCreateWindow(640,480,"GLP", NULL, NULL);

  if(NULL == window) goto fail;

  glfwSetWindowUserPointer(window, L);
  glfwSetKeyCallback(window, glfw_keycb);
  glfwSetWindowCloseCallback(window, glfw_closecb);
  glfwMakeContextCurrent(window);

  while(!glfwWindowShouldClose(window)){
    glfwSwapBuffers(window);
    glfwWaitEvents();
  }

  glfwDestroyWindow(window);

fail:
  glfwTerminate();
  lua_destroy(L);
  return 0;
}

static lua_State *
lua_create(void){
  lua_State *L = luaL_newstate();
  if(NULL == L)
    exit(EXIT_FAILURE);

  luaX_loadbase(L);
  luaX_preload(L, libs);

  luaL_loadfile(L, NULL);

  if(lua_pcall(L, 0, 0, 0) && !lua_isnil(L, -1))
    fprintf(stderr, "%s\n", lua_tostring(L, -1));

  return L;
}

static void
lua_destroy(lua_State *L){
  if(NULL != L)
    lua_close(L);
}

static int
luaopen_glp(lua_State *L){
  luaL_newlib(L, mods);
  return 1;
}

static int
l_register(lua_State *L){
  if(!lua_isfunction(L, 3) || !lua_isnumber(L, 2) || !lua_isnumber(L, 1))
    fputs("Could not register string, not in the form .register(key, function)", stderr);

  int key = lua_tointeger(L, 1);
  keys[key].mod = lua_tointeger(L, 2);
  keys[key].ref = luaL_ref(L, LUA_REGISTRYINDEX);
  return 0;
}

static int
l_unregister(lua_State *L){
  if(!lua_isnumber(L, 1))
    fputs("Could not unregister function, not in the form .unregister(key)", stderr);

  int key = lua_tointeger(L, 1);
  luaL_unref(L, LUA_REGISTRYINDEX, keys[key].ref);
  keys[key].ref = LUA_REFNIL;
  keys[key].mod = 0;
  return 0;
}

static int
l_keyname(lua_State *L){
  const char *str;
  if(!lua_isstring(L, 1))
    fputs("Could not find keyname, not in the form .keyname(string)", stderr);

  str = lua_tostring(L, 1);

  lua_pushinteger(L, 0);
  return 1;
}

static int
l_modmask(lua_State *L){
  const char *str;
  if(!lua_isstring(L, 1))
    fputs("Could not create modmask, not in the form .modmask(string)", stderr);

  str = loa_tostring(L, 1);

  lua_pushinteger(L, 0);
  return 1;
}

static void
glfw_keycb(GLFWwindow *win, int key, int scancode, int act, int mod){
  lua_State *L;
  if(GLFW_KEY_UNKNOWN == key) return;
  if(LUA_REFNIL == keys[key].ref) return;
  if(mod != keys[key].mod) return;
  
  L = glfwGetWindowUserPointer(win);
  lua_rawgeti(L, LUA_REGISTRYINDEX, keys[key].ref);
  lua_pushinteger(L, act);
  lua_pcall(L, 1, 0, 0);
}

static void
glfw_closecb(GLFWwindow *win){
  lua_State *L = glfwGetWindowUserPointer(win);
}

static void
luaX_loadbase(lua_State *L){
  luaL_requiref(L, LUA_GNAME, luaopen_base, 1);
  luaL_requiref(L, LUA_LOADLIBNAME, luaopen_package, 1);
  lua_pop(L, 2);
}

static void
luaX_preload(lua_State *L, luaL_Reg *M){
  const luaL_Reg *lib;
  luaL_getsubtable(L, LUA_REGISTRYINDEX, LUA_PRELOAD_TABLE);
  for(lib = M; lib->func; lib++){
    lua_pushcfunction(L, lib->func);
    lua_setfield(L, -2, lib->name);
  }
  lua_pop(L, 1);
}
