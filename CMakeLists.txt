cmake_minimum_required(VERSION 3.0)
project(libOHARBaseLayer VERSION 1.0.0 LANGUAGES CXX)
set(LIB_NAME OHARBaseLayer)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
find_package(Boost 1.67.0 REQUIRED COMPONENTS system)
if (Boost_FOUND)
   add_library(${LIB_NAME} STATIC ConfigurationDataItem.cpp DataItem.cpp Networker.cpp 
       ProcessorNode.cpp ConfigurationFileReader.cpp Log.cpp NodeConfiguration.cpp DataFileReader.cpp
       NetworkReader.cpp Package.cpp DataHandler.cpp NetworkWriter.cpp PingHandler.cpp 
       ConfigurationDataItem.h DataReaderObserver.h Networker.h ConfigurationFileReader.h
       Log.h NodeConfiguration.h DataFileReader.h NetworkReader.h Package.h DataHandler.h 
       NetworkReaderObserver.h PingHandler.h DataItem.h NetworkWriter.h ProcessorNode.h)
   include_directories(${Boost_INCLUDE_DIR})
   target_link_libraries(${LIB_NAME} ${Boost_LIBRARIES})
   set_target_properties(${LIB_NAME} PROPERTIES PUBLIC_HEADER "ConfigurationDataItem.h;DataReaderObserver.h;Networker.h;ConfigurationFileReader.h;Log.h;NodeConfiguration.h;DataFileReader.h;NetworkReader.h;Package.h;DataHandler.h;NetworkReaderObserver.h;PingHandler.h;DataItem.h;NetworkWriter.h;ProcessorNode.h")
   set_target_properties(${LIB_NAME} PROPERTIES COMPILE_FLAGS -fPIC -g -Wall)
   set_target_properties(${LIB_NAME} PROPERTIES LINK_FLAGS -Wl,-lpthread)
   set_target_properties(${LIB_NAME} PROPERTIES CXX_STANDARD 14) 
   target_include_directories(${LIB_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/> $<INSTALL_INTERFACE:include/${LIB_NAME}>)
   install(TARGETS ${LIB_NAME} EXPORT ${LIB_NAME}Config ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${LIB_NAME})
   install(EXPORT ${LIB_NAME}Config FILE ${LIB_NAME}Config.cmake NAMESPACE OHARBaseLayer:: DESTINATION share/${LIB_NAME}/cmake)
   export(TARGETS ${LIB_NAME} FILE ${LIB_NAME}Config.cmake)
endif()
