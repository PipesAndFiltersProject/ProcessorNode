cmake_minimum_required(VERSION 3.15)
project(ProcessorNodeProject VERSION 1.0.0 LANGUAGES CXX)

set(LIB_NAME ProcessorNode)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(Boost 1.70.0 REQUIRED COMPONENTS system)
find_package(g3logger REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Add a "doc" target to generate API documentation with Doxygen.
# After "cmake ..", execute "make doc" to create docs. 
# When using Ninja, do "cmake -GNinja .." and "ninja doc" to create the docs.
# There will be a docs directory in the build dir, containing the docs.
find_package(Doxygen)
if (DOXYGEN_FOUND)
   configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doxyfile @ONLY)
   add_custom_target(doc
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM
   )
endif(DOXYGEN_FOUND)

if (Boost_FOUND AND g3logger_FOUND AND nlohmann_json_FOUND)
   add_library(${LIB_NAME} STATIC ConfigurationDataItem.cpp DataItem.cpp Networker.cpp 
       ProcessorNode.cpp ConfigurationFileReader.cpp NodeConfiguration.cpp DataFileReader.cpp
       NetworkReader.cpp Package.cpp DataHandler.cpp NetworkWriter.cpp PingHandler.cpp ConfigurationHandler.cpp
       include/${LIB_NAME}/ConfigurationDataItem.h include/${LIB_NAME}/ConfigurationFileReader.h
       include/${LIB_NAME}/DataFileReader.h include/${LIB_NAME}/DataHandler.h include/${LIB_NAME}/DataItem.h
       include/${LIB_NAME}/DataReaderObserver.h include/${LIB_NAME}/NetworkReader.h
       include/${LIB_NAME}/NetworkReaderObserver.h include/${LIB_NAME}/NetworkWriter.h include/${LIB_NAME}/Networker.h
       include/${LIB_NAME}/NodeConfiguration.h include/${LIB_NAME}/Package.h include/${LIB_NAME}/PingHandler.h
       include/${LIB_NAME}/ProcessorNode.h include/${LIB_NAME}/ProcessorNodeObserver.h include/${LIB_NAME}/ConfigurationHandler.h)

   set_target_properties(${LIB_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
   set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 17)
   target_include_directories(${LIB_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/> $<INSTALL_INTERFACE:include/${LIB_NAME}> ${Boost_INCLUDE_DIRS} ${G3LOG_INCLUDE_DIRS})

   target_link_libraries(${LIB_NAME} PUBLIC Boost::system g3logger nlohmann_json::nlohmann_json)

   set_target_properties(${LIB_NAME} PROPERTIES PUBLIC_HEADER "include/${LIB_NAME}/ConfigurationDataItem.h;include/${LIB_NAME}/DataReaderObserver.h;include/${LIB_NAME}/Networker.h;include/${LIB_NAME}/ConfigurationFileReader.h;include/${LIB_NAME}/NodeConfiguration.h;include/${LIB_NAME}/DataFileReader.h;include/${LIB_NAME}/NetworkReader.h;include/${LIB_NAME}/Package.h;include/${LIB_NAME}/DataHandler.h;include/${LIB_NAME}/NetworkReaderObserver.h;include/${LIB_NAME}/PingHandler.h;include/${LIB_NAME}/DataItem.h;include/${LIB_NAME}/NetworkWriter.h;include/${LIB_NAME}/ProcessorNode.h;include/${LIB_NAME}/ProcessorNodeObserver.h;include/${LIB_NAME}/ConfigurationHandler.h")

   install(TARGETS ${LIB_NAME} EXPORT ${LIB_NAME}Targets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIB_NAME})
   install(EXPORT ${LIB_NAME}Targets FILE ${LIB_NAME}Targets.cmake NAMESPACE ProcessorNode:: DESTINATION lib/cmake/${LIB_NAME})
   install(FILES ${LIB_NAME}Config.cmake DESTINATION lib/cmake/${LIB_NAME})

   export(TARGETS ${LIB_NAME} FILE ${LIB_NAME}Targets.cmake)
endif()
