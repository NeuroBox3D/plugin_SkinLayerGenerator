################################################################################
# SKIN_LAYER_GENERATOR
# Note: Activate by -DSkinLayerGenerator=ON
#
# author: Stephan Grein
################################################################################
# minimum CMAKE version
cmake_minimum_required(VERSION 2.6)

# set project
set(pluginName SkinLayerGenerator)
project(UG_PLUGIN_${pluginName})

# default values
set(SLGC++0x OFF)
set(SLGTestsuite ON)

# include the definitions and dependencies for ug-plugins
include(${UG_ROOT_CMAKE_PATH}/ug_plugin_includes.cmake)

# set the sources and unit test sources
set(SOURCES plugin_main.cpp skin_layer_generator.cpp)
set(SOURCES_TEST unit_tests/src/tests.cpp)

# options for building cleft_generator
message(STATUS "Info: Options for SkinLayerGenerator (SLG) plugin:")
option(SLGTestsuite "Build Testsuite" ${SLGTestsuite})
message(STATUS "Info: Testsuite:       " ${SLGTestsuite} " (options are: ON, OFF)")
option(SLGC++0x "Build C++0x " ${SLGC++0x})
message(STATUS "Info: C++0x:           " ${SLGC++0x} " (options are: ON, OFF)")

# decide if you want to build the boost testsuite executable (SLGTestsuite)
if(${SLGTestsuite} STREQUAL "ON")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${UG_ROOT_PATH}/bin/)
  add_executable(SLGTestsuite ${SOURCES_TEST})
endif(${SLGTestsuite} STREQUAL "ON")

# build project above with C++0x extensions (only .cpp files are affected)
IF(${SLGC++0x} STREQUAL "ON")                                                          
  SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++0x")
ENDIF(${SLGC++0x} STREQUAL "ON")

# create a shared library from the sources and link it against ug
if(buildEmbeddedPlugins)
	EXPORTSOURCES(${CMAKE_CURRENT_SOURCE_DIR} ${SOURCES} ${SOURCES_TEST})
	if(${SLGTestsuite} STREQUAL "ON")
		target_link_libraries (SLGTestsuite ug4)
	endif(${SLGTestsuite} STREQUAL "ON")
else(buildEmbeddedPlugins)
    add_library(SkinLayerGenerator SHARED ${SOURCES})
    target_link_libraries(SkinLayerGenerator ug4 ProMesh)
	if(${SLGTestsuite} STREQUAL "ON")
		target_link_libraries (SLGTestsuite SkinLayerGenerator ug4 ProMesh)
	endif(${SLGTestsuite} STREQUAL "ON")
endif(buildEmbeddedPlugins)