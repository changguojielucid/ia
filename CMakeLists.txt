# Copyright (c) Elucid Bioimaging

# ===================================================
# INSTRUCTIONS FOR WINDOWS
# ===================================================

# open this file (ia/cap/CMakeLists.txt) in CMake GUI
#   * set generator to "Visual Studio 14 2015 Win64"
#   * both source and build directories the same
#   * set ITK_DIR to C:\Program Files\ITK\lib\cmake\ITK-4.9
#   * set Qt5WebKitWidgets_DIR to C:\Qt\Qt-build\qtbase\lib\cmake\Qt5WebKit
#   * configure, then generate

# open ia/cap/cap.sln in Visual Studio
#   * Add Existing Project --> ia/InstallShield/InstallShieldSetup.isl
#   * Add Build Dependencies --> Project Dependencies
#     * check vascuCAP
#   * Right click on ia solution, then Build Solution

# ===================================================
# INSTRUCTIONS FOR MAC
# ===================================================

# build cap (starting from ia/cap)
#   mkdir ../../ia-build/cap
#   cd ../../ia-build/cap
#   cmake ../../ia/cap
#   make

# to install executables on local machine
#   make install

# to create cap client deployments (only on Mac to create dmg)
#   make deploy

# TODO
#   - remove need to explicitly define ITK_DIR and Qt5WebKitWidgets_DIR in Windows build

cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)

project(cap)

# ensure MacOS backward compatibility
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "Minimum OS X deployment version")

# ===================================================
# modules
# ===================================================

find_package(ITK 4.12.2 EXACT REQUIRED)
include(${ITK_USE_FILE})

#find_package(VTK 8.0.1 EXACT REQUIRED)
find_package(VTK 7.1.1 EXACT REQUIRED)
include(${VTK_USE_FILE})

#finding OpenSSL fails on Mac)
#find_package(OpenSSL REQUIRED)

if(APPLE)
  set(DCMTK_DIR /usr/local)
endif(APPLE)
find_package(DCMTK REQUIRED)
find_package(ZLIB REQUIRED)

# /usr/local/Qt-5.5.1 is the normal installed location, which should
# be found automatically.  But if user has not installed but left it
# in ~/Qt, the following lines will pick it up.  IS_DIRECTORY only
# works for absolute directories
get_filename_component(HOMEQT ~/Qt ABSOLUTE)
if(IS_DIRECTORY ${HOMEQT})
  set(CMAKE_PREFIX_PATH ~/Qt/5.5/clang_64/lib/cmake) 
  set(QT_QTGUI_LIBRARY ~/Qt/5.5/clang_64/lib/QtGui.framework)
endif(IS_DIRECTORY ${HOMEQT})

# Widgets finds its own dependencies (QtGui and QtCore).
find_package(Qt5Widgets REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)
find_package(Qt5WebSockets REQUIRED)
find_package(Qt5Network REQUIRED)


# set up the unit test targets
#find_package(Qt5Test REQUIRED)

if(WIN32)
  include_directories("C:/Program\ Files/OpenSSL")
  include_directories("C:/Program Files/OpenSSL")
  include_directories("C:/Program\ Files/OpenSSL/include")
  include_directories("C:/Program Files/OpenSSL/include")
  include_directories("C:/Program\ Files/OpenSSL/include/openssl")
  include_directories("C:/Program Files/OpenSSL/include/openssl")
  include_directories(${OPENSSL_INCLUDE_DIR})
else(WIN32)
  include_directories("/usr/local/ssl/include")
endif(WIN32)

# ===================================================
# options
# ===================================================

set(CMAKE_BUILD_TYPE RelWithDebInfo)

# Tell CMake to run moc when necessary.
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake to always look for includes there.
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ===================================================
# targets
# ===================================================

set(CPP EVWebSocketClient.cpp EVWebSocketServer.cpp EVWorkItem.cpp EVWorkClient.cpp EVWebSocketClient.cpp EVWebSocketServer.cpp EVSocketClient.cpp EVSocketServer.cpp EVTask.cpp cap_main.cpp cap.cpp StatusWatcher.cpp ImageDatabase.cpp capTools.cpp processingParameters.cpp workItem.cpp systemPreferences.cpp dicomQueryRetrieve.cpp sourceConfiguration.cpp SegmentationEditor.cpp seriesSurvey.cpp targetDefine.cpp patientAnalyze.cpp patientReport.cpp Base64.cpp DicomImagesTreeWidget.cpp WorkItemTableView.cpp QITKProgressDialog.cpp)
set(MOC )
set(UI  aboutPage.ui mainWindow.ui systemPreferences.ui workItem.ui seriesSurvey.ui targetDefine.ui patientAnalyze.ui patientReport.ui dicomQueryRetrieve.ui SegmentationEditor.ui)
set(RCC cap.qrc)

qt5_wrap_cpp(MOC_GENERATED ${MOC})
qt5_wrap_ui(UI_GENERATED ${UI})
qt5_add_resources(RCC_RESOURCES ${RCC})

source_group("Form Files" FILES ${UI})
source_group("Resource Files" FILES ${RCC})
source_group("Generated Files" FILES ${MOC_GENERATED} ${UI_GENERATED} ${RCC_GENERATED})

qt5_add_resources(RESOURCE_ADDED ${RCC})
set_source_files_properties(${RESOURCE_ADDED} PROPERTIES GENERATED ON)

#(unclear if the following helps windows, where InstallShield is used so CMake has fewer responsibilities)
#set(APPLICATION_ICON "${CMAKE_CURRENT_SOURCE_DIR}/VascuCap_D1.ico")

# Mac OS X bundle properties
set(MACOSX_BUNDLE_INFO_STRING "See User Guide for product version and indicated use.")
set(MACOSX_BUNDLE_ICON_FILE "VascuCap_D1.icns")
set(MACOSX_BUNDLE_ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${MACOSX_BUNDLE_ICON_FILE}")
set(MACOSX_BUNDLE_GUI_IDENTIFIER "Client")
set(MACOSX_BUNDLE_LONG_VERSION_STRING "Elucid Computer-Aided-Phenotyping® 2016-2021.")
set(MACOSX_BUNDLE_BUNDLE_NAME "vascuCAP")
set(MACOSX_BUNDLE_SHORT_VERSION_STRING "A.3")
set(MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0")
set(MACOSX_BUNDLE_COPYRIGHT "This program owned and all rights reserved by Elucid Bioimaging Inc., no copying, no re-distribution, sale, or transfer of any kind. Elucid, Computer-Aided-phenotyping, CAP, vascuCAP, and lungCAP are registered trademarks of Elucid Bioimaging Inc., and all rights are reserved.")

# Mac OS X icon
set_source_files_properties(${MACOSX_BUNDLE_ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

# Mac OS X TurboActivate
set(TURBOACTIVATE "${CMAKE_CURRENT_SOURCE_DIR}/TurboActivate.dat")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/TurboActivate.dat PROPERTIES MACOSX_PACKAGE_LOCATION "MacOS")

# vascuCAP executable
add_executable(vascuCAP MACOSX_BUNDLE ${MOC_GENERATED} ${UI_GENERATED} ${RCC_GENERATED} main.cpp ${CPP} ${RESOURCE_ADDED} ${MACOSX_BUNDLE_ICON_PATH} ${TURBOACTIVATE})

# lungCAP executable
#add_executable(lungCAP MACOSX_BUNDLE ${MOC_GENERATED} ${UI_GENERATED} ${RCC_GENERATED} main.cpp ${CPP} ${RESOURCE_ADDED})

# test targets
#add_executable(test_workItem EXCLUDE_FROM_ALL MACOSX_BUNDLE ${MOC_GENERATED} ${UI_GENERATED} ${RCC_GENERATED} test_workItem.cpp ${CPP} ${RESOURCE_ADDED})
#add_executable(test_seriesSurvey EXCLUDE_FROM_ALL MACOSX_BUNDLE ${MOC_GENERATED} ${UI_GENERATED} ${RCC_GENERATED} test_seriesSurvey.cpp ${CPP} ${RESOURCE_ADDED})
#add_custom_target(check COMMAND test_workItem.app/Contents/MacOS/test_workItem COMMAND test_seriesSurvey.app/Contents/MacOS/test_seriesSurvey)
#add_dependencies(check test_workItem test_seriesSurvey)

# install target
if(APPLE)
  install(TARGETS vascuCAP BUNDLE DESTINATION ~/bin/)
  #install(TARGETS lungCAP BUNDLE DESTINATION ~/bin/)
else(APPLE)
  install(TARGETS vascuCAP DESTINATION /usr/local/bin)
  #install(TARGETS lungCAP DESTINATION /usr/local/bin)
endif()

# deploy target (Mac only)
if(APPLE)
  add_custom_target(deploy
    macdeployqt vascuCAP.app -dmg
    DEPENDS vascuCAP
    COMMENT "Generating vascuCAP DMG" VERBATIM
    )
endif(APPLE)

# ===================================================
# compiler options
# ===================================================

set_property(TARGET vascuCAP PROPERTY CXX_STANDARD 11)

include_directories($(CMAKE_CURRENT_BINARY_DIR) $(CMAKE_CURRENT_SOURCE_DIR))
include_directories(../libs/caplib ../libs/dicomlib ${DCMTK_INCLUDE_DIRS})

# The Qt5Widgets_INCLUDES also includes the include directories for dependencies QtCore and QtGui.
include_directories(${Qt5Widgets_INCLUDES})
include_directories(${Qt5WebKitWidgets_INCLUDES})
include_directories(${Qt5WebSockets_INCLUDES})
include_directories(${Qt5Network_INCLUDES})

if(APPLE)
  add_definitions(
    -Wno-macro-redefined
    -Wno-inconsistent-missing-override
    -Wno-potentially-evaluated-expression
    -Wno-deprecated-declarations
    -Wno-nonportable-include-path
    -Wno-constant-conversion
    )
elseif(WIN32)
  add_definitions(/MP /EHsc)
endif(APPLE)

# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
add_definitions(${Qt5Widgets_DEFINITIONS} ${Qt5WebKitWidgets_DEFINITIONS})

# this should really just be used for Windows builds but probably causes little harm for non-Windows builds
target_compile_definitions(vascuCAP PUBLIC _USE_MATH_DEFINES)

# Executables fail to build with Qt 5 in the default configuration without -fPIE. We add that here.
set(CMAKE_CXX_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS} ${Qt5WebKitWidgets_EXECUTABLE_COMPILE_FLAGS}")

# ===================================================
# linker options
# ===================================================

add_subdirectory(../libs/dicomlib dicomlib)
add_subdirectory(../libs/caplib caplib)

if(APPLE)
  set(TURBOACTIVATELIB ${CMAKE_CURRENT_SOURCE_DIR}/libTurboActivate.dylib)
elseif(WIN32)
  set(TURBOACTIVATELIB ${CMAKE_CURRENT_SOURCE_DIR}/TurboActivate.lib)
endif(APPLE)

target_link_libraries(vascuCAP caplib ${Qt5WebSockets_LIBRARIES} ${Qt5Network_LIBRARIES} ${TURBOACTIVATELIB} ${Qt5Widgets_LIBRARIES} ${Qt5WebKitWidgets_LIBRARIES} ${ITK_LIBRARIES} ${VTK_LIBRARIES} ${DCMTK_LIBRARIES} ${ZLIB_LIBRARY})
#target_link_libraries(lungCAP caplib ${Qt5Widgets_LIBRARIES} ${Qt5WebKitWidgets_LIBRARIES} ${ITK_LIBRARIES} ${VTK_LIBRARIES})
#target_link_libraries(test_workItem caplib ${Qt5Widgets_LIBRARIES} ${Qt5WebKitWidgets_LIBRARIES} ${ITK_LIBRARIES} ${VTK_LIBRARIES} Qt5::Test)
#target_link_libraries(test_seriesSurvey caplib ${Qt5Widgets_LIBRARIES} ${Qt5WebKitWidgets_LIBRARIES} ${ITK_LIBRARIES} ${VTK_LIBRARIES} Qt5::Test)


#find_package(Qt5WebSockets REQUIRED)  ${Qt5WebSockets_LIBRARIES}
#find_package(Qt5Network REQUIRED)   ${Qt5Network_LIBRARIES}




#CodeSigning
if(WIN32)

	set(SIGNTOOL "\\\\eb-nas1/eng/dev/codesign/signtool.exe")
	set(PFXFILE "\\\\eb-nas1/eng/dev/codesign/eb-sectigo-csc.pfx")
	set(PASSFILE "\\\\eb-nas1/eng/dev/codesign/PassFile.txt")
	set(PASSWORD "")
	
	if (EXISTS ${PASSFILE})
		file(READ ${PASSFILE} PASSWORD [OFFSET 0] [LIMIT 32])
		#message(STATUS "Password is: ${PASSWORD}")
	elseif()
		message(STATUS "PassFile: ${PASSFILE} Not Found")
	endif()
		
	if (EXISTS ${SIGNTOOL} AND EXISTS ${PFXFILE})
		add_custom_command(TARGET vascuCAP
						POST_BUILD
						COMMAND ${SIGNTOOL} sign /v /f ${PFXFILE} /p ${PASSWORD} $(OutDir)$(TargetName)$(TargetExt) 
						VERBATIM
						)
	elseif()
		if(NOT EXISTS ${SIGNTOOL})	
			message(STATUS "SIGNTOOL: ${SIGNTOOL} Not Found")
		elseif(NOT EXISTS ${PFXFILE})
			message(STATUS "PFXFILE: ${PFXFILE} Not Found")
		endif()
	endif()
	
endif(WIN32)

if(APPLE)
    add_custom_command(
        TARGET vascuCAP
        POST_BUILD
        COMMAND security dump-keychain | egrep 'Elucid Bio' && codesign -s 'Elucid Bioimaging' --deep vascuCAP.app || echo 'skipping code signing application, no cert.'
        COMMENT "code signing vascuCAP.app"
        )
endif(APPLE)
