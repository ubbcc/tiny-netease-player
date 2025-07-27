cmake_minimum_required(VERSION 3.16)

project(Tiny_netease_music LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Network)

set(TS_FILES Tiny_netease_music_zh_CN.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Tiny_netease_music
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        common.h Imagelabel.cpp Imagelabel.h resources.qrc
        api/Crypto.cpp api/Crypto.h api/Download.cpp api/Download.h api/Lyric.cpp api/Lyric.h api/Playlist.cpp api/Playlist.h api/Request.cpp api/Request.h api/Search.cpp api/Search.h api/SongSearch.cpp api/SongSearch.h api/User.cpp api/User.h
        api/Qt-AES-master/aesni/aesni-enc-cbc.h api/Qt-AES-master/aesni/aesni-enc-ecb.h api/Qt-AES-master/aesni/aesni-key-exp.h api/Qt-AES-master/aesni/aesni-key-init.h
        api/Qt-AES-master/qaesencryption.h
        api/Qt-AES-master/qaesencryption.cpp
        LoginWidget.cpp LoginWidget.h
        QRCodeGenerator/qrcodegen.cpp QRCodeGenerator/qrcodegen.hpp
        api/Comment.h
        api/Comment.cpp

    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Tiny_netease_music APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(Tiny_netease_music SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Tiny_netease_music
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(Tiny_netease_music PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Tiny_netease_music)
endif()
#set_target_properties(Tiny_netease_music PROPERTIES
#    ${BUNDLE_ID_OPTION}
#    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#   MACOSX_BUNDLE TRUE
#   WIN32_EXECUTABLE TRUE
#)

include(GNUInstallDirs)
install(TARGETS Tiny_netease_music
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(APPLE)
    # 设置 App Bundle 图标
    set(MACOSX_BUNDLE_ICON_FILE logo.icns)
    set(APP_ICON_MACOSX ${CMAKE_CURRENT_SOURCE_DIR}/logo.icns)
    set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(Tiny_netease_music PRIVATE ${APP_ICON_MACOSX})
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Tiny_netease_music)
endif()
