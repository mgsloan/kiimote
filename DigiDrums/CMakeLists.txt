# Minimal QT Build
# CMake cross-platform build system recipe
# 2010 Ryan Pavlik <rpavlik at iastate.edu> <abiryan at ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC

cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)

# Set package properties
project(DigiDrums)

ADD_SUBDIRECTORY(nestk)
INCLUDE("${nestk_BINARY_DIR}/UseNestk.cmake")

ADD_DEFINITIONS(-DOPENAL_SOUND_PLAYER)
#ADD_DEFINITIONS(-DOSC_SOUND_PLAYER)


###
# Perform build configuration of dependencies

# Set up QT4 and required components
find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL Qt3D REQUIRED)
include(${QT_USE_FILE})

###
# All the project files
set(SOURCES
     drum_viewer.cpp
     kinect_controller.cpp
     wii_controller.cpp
     chronos_controller.cpp
     kw_controller.cpp
     utils.cpp
     mesh_transforms.cpp
     tracker.cpp
     idrumsticks.cpp
     sound_player.cpp
     drumset.cpp
     drum_controller.cpp
     app_viewer.cpp
     app_controller.cpp
     main.cpp
     clone_record.cpp
   )

set(HEADERS
      drum_viewer.h
      kinect_controller.h
      wii_controller.h
      chronos_controller.h
      kw_controller.h
      utils.h
      tracker.h
      idrumsticks.h
      sound_player.h
      drumset.h
      drum_controller.h
      app_viewer.h
      app_controller.h
      clone_record.h
   )

# All header files containing classes with the Q_OBJECT macro must be listed
# here, so that .cpp files will be auto-generated.
set(MOCS
    app_viewer.h
    wii_controller.h
    chronos_controller.h
    app_controller.h
    clone_record.h
  )

set(UIS)

set(RESOURCES)

###
# Build and link the project

# Let QT pre-process the files and add the generated files to the source 
#list
#qt4_wrap_ui(GENERATED_SOURCES ${UIS})
#qt4_add_resources(GENERATED_SOURCES ${RESOURCES})
qt4_wrap_cpp(GENERATED_SOURCES ${MOCS})

source_group("Generated Sources - Do Not Edit" FILES ${GENERATED_SOURCES})

# Make sure the compiler can find the pre-processed files from qt4_wrap_ui
include_directories("${CMAKE_BINARY_DIR}")

# Build the app!
add_executable(DigiDrums
     WIN32
     # source files that are actually built direclty
     ${SOURCES}
     ${GENERATED_SOURCES}

     # items included so they show up in your IDE
     ${HEADERS}
     ${UIS}
     ${MOCS}
     ${RESOURCES})

# Link the app!
target_link_libraries(DigiDrums ${QT_LIBRARIES} GLU glew wiiuse nestk openal alut lo)
