# Minimal QT Build
# CMake cross-platform build system recipe
# 2010 Ryan Pavlik <rpavlik at iastate.edu> <abiryan at ryand.net>
# http://academic.cleardefinition.com
# Iowa State University HCI Graduate Program/VRAC

cmake_minimum_required(VERSION 2.6.2 FATAL_ERROR)

# Set package properties
project(DigiDrums)

ADD_SUBDIRECTORY(nestk)
INCLUDE("${nestk_BINARY_DIR}/UseEmbeddedNestk.cmake")

#ADD_DEFINITIONS(-DOSC_SOUND_PLAYER)


###
# Perform build configuration of dependencies

# Set up QT4 and required components
find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL Qt3D REQUIRED)
include(${QT_USE_FILE})

###
# All the project files
set(SOURCES
     app.cpp
     kinect.cpp
     main.cpp
     manipulator.cpp
     tracker.cpp
     utils.cpp
     viewer.cpp
     wiimote.cpp
   )

set(HEADERS
     app.h
     kinect.h
     manipulator.h
     tracker.h
     utils.h
     viewer.h
     wiimote.h
   )

# All header files containing classes with the Q_OBJECT macro must be listed
# here, so that .cpp files will be auto-generated.
set(MOCS
    app.h
    viewer.h
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
add_executable(kiimote
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
target_link_libraries(kiimote ${QT_LIBRARIES} GLU glew wiiuse nestk lo)

