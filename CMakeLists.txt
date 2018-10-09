cmake_minimum_required(VERSION 3.0)
project(libOHARBaseLayer VERSION 1.0.0 LANGUAGES CXX)
set(LIB_NAME OHARBaseLayer)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(BUILD_SHARED_LIBS FALSE)
find_package(Boost 1.67.0 REQUIRED COMPONENTS system)
if (Boost_FOUND)
   add_library(${LIB_NAME} STATIC ConfigurationDataItem.cpp DataItem.cpp Networker.cpp 
       ProcessorNode.cpp ConfigurationFileReader.cpp Log.cpp NodeConfiguration.cpp DataFileReader.cpp
       NetworkReader.cpp Package.cpp DataHandler.cpp NetworkWriter.cpp PingHandler.cpp
       include/${LIB_NAME}/ConfigurationDataItem.h include/${LIB_NAME}/ConfigurationFileReader.h
       include/${LIB_NAME}/DataFileReader.h include/${LIB_NAME}/DataHandler.h include/${LIB_NAME}/DataItem.h
       include/${LIB_NAME}/DataReaderObserver.h include/${LIB_NAME}/Log.h include/${LIB_NAME}/NetworkReader.h
       include/${LIB_NAME}/NetworkReaderObserver.h include/${LIB_NAME}/NetworkWriter.h include/${LIB_NAME}/Networker.h
       include/${LIB_NAME}/NodeConfiguration.h include/${LIB_NAME}/Package.h include/${LIB_NAME}/PingHandler.h
       include/${LIB_NAME}/ProcessorNode.h)
   set_target_properties(${LIB_NAME} PROPERTIES COMPILE_FLAGS -fPIC -g -Wall)
   set_target_properties(${LIB_NAME} PROPERTIES LINK_FLAGS -Wl)
   set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 14)
   include_directories(${Boost_INCLUDE_DIR})
   target_include_directories(${LIB_NAME} SYSTEM PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/> $<INSTALL_INTERFACE:include/${LIB_NAME}>)
   target_link_libraries(${LIB_NAME} ${Boost_LIBRARIES})
   set_target_properties(${LIB_NAME} PROPERTIES PUBLIC_HEADER "include/${LIB_NAME}/ConfigurationDataItem.h;include/${LIB_NAME}/DataReaderObserver.h;include/${LIB_NAME}/Networker.h;include/${LIB_NAME}/ConfigurationFileReader.h;include/${LIB_NAME}/Log.h;include/${LIB_NAME}/NodeConfiguration.h;include/${LIB_NAME}/DataFileReader.h;include/${LIB_NAME}/NetworkReader.h;include/${LIB_NAME}/Package.h;include/${LIB_NAME}/DataHandler.h;include/${LIB_NAME}/NetworkReaderObserver.h;include/${LIB_NAME}/PingHandler.h;include/${LIB_NAME}/DataItem.h;include/${LIB_NAME}/NetworkWriter.h;include/${LIB_NAME}/ProcessorNode.h")
   install(TARGETS ${LIB_NAME} EXPORT ${LIB_NAME}Targets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIB_NAME})
   install(EXPORT ${LIB_NAME}Targets FILE ${LIB_NAME}Config.cmake NAMESPACE OHARBaseLayer:: DESTINATION lib/cmake/${LIB_NAME})
   export(TARGETS ${LIB_NAME} FILE ${LIB_NAME}Targets.cmake)
endif()
