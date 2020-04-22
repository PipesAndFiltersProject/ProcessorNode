cmake_minimum_required(VERSION 3.15)
project(ProcessorNodeProject VERSION 1.0.0 LANGUAGES CXX)

set(LIB_NAME ProcessorNode)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

find_package(Boost 1.70.0 REQUIRED COMPONENTS system)
find_package(g3logger REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

# Add a "doc" target to generate API documentation with Doxygen.
# Doxygen is _not_ a component that ProcessorNode uses, but a _tool_ 
# used in library document generation as part of the build process.
# After "cmake ..", execute "make doc" to create docs. 
# When using Ninja, do "cmake -GNinja .." and "ninja doc" to create the docs.
# There will be a docs directory in the build dir, containing the docs.
find_package(Doxygen)
if (DOXYGEN_FOUND)
   configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/doxyfile @ONLY)
   add_custom_target(doc
      ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM
   )
endif(DOXYGEN_FOUND)

if (WIN32)
    macro(get_WIN32_WINNT version)
        if (CMAKE_SYSTEM_VERSION)
            set(ver ${CMAKE_SYSTEM_VERSION})
            string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
            string(REGEX MATCH "^([0-9]+)" verMajor ${ver})
            # Check for Windows 10, b/c we'll need to convert to hex 'A'.
            if ("${verMajor}" MATCHES "10")
                set(verMajor "A")
                string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
            endif ("${verMajor}" MATCHES "10")
            # Remove all remaining '.' characters.
            string(REPLACE "." "" ver ${ver})
            # Prepend each digit with a zero.
            string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
            set(${version} "0x${ver}")
        endif(CMAKE_SYSTEM_VERSION)
    endmacro(get_WIN32_WINNT)

    get_WIN32_WINNT(ver)
    add_definitions(-D_WIN32_WINNT=${ver})
    add_definitions(-DBOOST_UUID_RANDOM_PROVIDER_FORCE_WINCRYPT)
endif(WIN32)

if (Boost_FOUND AND g3logger_FOUND AND nlohmann_json_FOUND)
   add_library(${LIB_NAME} STATIC ConfigurationDataItem.cpp DataItem.cpp Networker.cpp 
       ProcessorNode.cpp ConfigurationFileReader.cpp NodeConfiguration.cpp DataFileReader.cpp
       NetworkReader.cpp Package.cpp DataHandler.cpp NetworkWriter.cpp PingHandler.cpp ConfigurationHandler.cpp  EncryptHandler.cpp
       include/${LIB_NAME}/ConfigurationDataItem.h include/${LIB_NAME}/ConfigurationFileReader.h
       include/${LIB_NAME}/DataFileReader.h include/${LIB_NAME}/DataHandler.h include/${LIB_NAME}/DataItem.h
       include/${LIB_NAME}/DataReaderObserver.h include/${LIB_NAME}/NetworkReader.h
       include/${LIB_NAME}/NetworkReaderObserver.h include/${LIB_NAME}/NetworkWriter.h include/${LIB_NAME}/Networker.h
       include/${LIB_NAME}/NodeConfiguration.h include/${LIB_NAME}/Package.h include/${LIB_NAME}/PingHandler.h
       include/${LIB_NAME}/ProcessorNode.h include/${LIB_NAME}/ProcessorNodeObserver.h include/${LIB_NAME}/ConfigurationHandler.h  include/${LIB_NAME}/EncryptHandler.h)

   set_target_properties(${LIB_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
   set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 17)
   target_include_directories(${LIB_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/> $<INSTALL_INTERFACE:include/${LIB_NAME}> ${Boost_INCLUDE_DIRS} ${G3LOGGER_INCLUDE_DIRS})

   target_link_libraries(${LIB_NAME} PUBLIC Boost::system g3logger nlohmann_json::nlohmann_json)

   set_target_properties(${LIB_NAME} PROPERTIES PUBLIC_HEADER "include/${LIB_NAME}/ConfigurationDataItem.h;include/${LIB_NAME}/DataReaderObserver.h;include/${LIB_NAME}/Networker.h;include/${LIB_NAME}/ConfigurationFileReader.h;include/${LIB_NAME}/NodeConfiguration.h;include/${LIB_NAME}/DataFileReader.h;include/${LIB_NAME}/NetworkReader.h;include/${LIB_NAME}/Package.h;include/${LIB_NAME}/DataHandler.h;include/${LIB_NAME}/NetworkReaderObserver.h;include/${LIB_NAME}/PingHandler.h;include/${LIB_NAME}/DataItem.h;include/${LIB_NAME}/NetworkWriter.h;include/${LIB_NAME}/ProcessorNode.h;include/${LIB_NAME}/ProcessorNodeObserver.h;include/${LIB_NAME}/ConfigurationHandler.h;include/${LIB_NAME}/EncryptHandler.h")

   install(TARGETS ${LIB_NAME} EXPORT ${LIB_NAME}Targets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIB_NAME})
   install(EXPORT ${LIB_NAME}Targets FILE ${LIB_NAME}Targets.cmake NAMESPACE ProcessorNode:: DESTINATION lib/cmake/${LIB_NAME})
   install(FILES ${LIB_NAME}Config.cmake DESTINATION lib/cmake/${LIB_NAME})

   export(TARGETS ${LIB_NAME} FILE ${LIB_NAME}Targets.cmake)
endif()
