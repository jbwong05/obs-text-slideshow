cmake_minimum_required(VERSION 3.16...3.21)

project(obs-text-slideshow VERSION 2.0.0)
add_library(${CMAKE_PROJECT_NAME} MODULE)

set(PLUGIN_AUTHOR "Joshua Wong")

set(MACOS_BUNDLEID "com.jbwong05.${CMAKE_PROJECT_NAME}")

# Replace `me@contoso.com` with the maintainer email address you want to put in Linux packages
set(LINUX_MAINTAINER_EMAIL "jbwong05@gmail.com")

# Add your custom source files here - header files are optional and only required for visibility
# e.g. in Xcode or Visual Studio
target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE src/plugin-main.cpp src/obs-text-freetype2-slideshow.cpp src/obs-text-slideshow.cpp
          src/obs-text-slideshow-dock.cpp src/files.cpp)

# Import libobs as main plugin dependency
find_package(libobs REQUIRED)
include(cmake/ObsPluginHelpers.cmake)

find_package(obs-frontend-api REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)

find_qt(COMPONENTS Widgets Core)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt::Core Qt::Widgets)
set_target_properties(
  ${CMAKE_PROJECT_NAME}
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

configure_file(src/plugin-macros.h.in ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/plugin-macros.generated.h)

# /!\ TAKE NOTE: No need to edit things past this point /!\

# --- Platform-independent build settings ---

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(OS_WINDOWS)
  target_sources(${CMAKE_PROJECT_NAME} PRIVATE src/obs-text-gdiplus-slideshow.cpp)

  configure_file(cmake/bundle/windows/installer-Windows.iss.in
                 ${CMAKE_BINARY_DIR}/installer-Windows.generated.iss)

  configure_file(cmake/bundle/windows/resource.rc.in ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.rc)
  target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.rc)

  if(MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4)
    target_include_directories(
                ${CMAKE_PROJECT_NAME}
                PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/../obs-studio/deps/w32-pthreads)
    target_link_libraries(${CMAKE_PROJECT_NAME}
                PRIVATE 
                ${CMAKE_CURRENT_SOURCE_DIR}/../obs-studio/plugin_build_x64/deps/w32-pthreads/RelWithDebInfo/w32-pthreads.lib)
  endif()
  # --- End of section ---

  # -- macOS specific build settings and tasks --
elseif(OS_MACOS)
  configure_file(cmake/bundle/macos/installer-macos.pkgproj.in
                 ${CMAKE_BINARY_DIR}/installer-macos.generated.pkgproj)

  set(MACOSX_PLUGIN_GUI_IDENTIFIER "${MACOS_BUNDLEID}")
  set(MACOSX_PLUGIN_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
  set(MACOSX_PLUGIN_SHORT_VERSION_STRING "1")

  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall)
  # --- End of section ---

  # --- Linux-specific build settings and tasks ---
else()
  target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall)
endif()
# --- End of section ---

setup_plugin_target(${CMAKE_PROJECT_NAME})
