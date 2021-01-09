project('glp-project','c',
  default_options:[
    'c_std=c11',
    ]
  )

glfw_dep = dependency('glfw3', required:true)
lua_dep = dependency('lua', required:true)
glp_dep = [
  glfw_dep,
  lua_dep,
  ]
glp_exe = executable('glp','src/glp.c',
  dependencies:glp_dep)

