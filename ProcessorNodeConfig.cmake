include(CMakeFindDependencyMacro)

find_dependency(Boost 1.70.0)

include("${CMAKE_CURRENT_LIST_DIR}/ProcessorNodeTargets.cmake")
