include(CMakeFindDependencyMacro)

find_dependency(Boost 1.70.0)
find_dependency(g3log)
find_dependency(nlohmann_json 3.2.0)

include("${CMAKE_CURRENT_LIST_DIR}/ProcessorNodeTargets.cmake")
