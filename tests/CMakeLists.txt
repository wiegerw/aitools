project(AITOOLS_TEST)

file(GLOB SOURCES CONFIGURE_DEPENDS "*.cpp")
foreach(SRC ${SOURCES})
  get_filename_component(BASE "${SRC}" NAME_WE)
  add_executable("${BASE}" "${SRC}")
  add_test(NAME "${BASE}" COMMAND "${BASE}")
  target_link_libraries ("${BASE}" LINK_PUBLIC aitoolslib -ltbb)
endforeach(SRC)
