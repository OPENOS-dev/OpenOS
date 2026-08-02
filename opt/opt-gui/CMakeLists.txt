cmake_minimum_required(VERSION 3.20)
project(opt-gui VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Find Qt6 (fallback to Qt5) ──
find_package(Qt6 COMPONENTS Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

# ── Find libopt (static library from Rust) ──
find_library(LIBOPT_LIB
    NAMES libopt liblibopt
    PATHS "${CMAKE_SOURCE_DIR}/../libopt/target/release"
          "${CMAKE_SOURCE_DIR}/../libopt/target/debug"
    NO_DEFAULT_PATH
)

if(LIBOPT_LIB-NOTFOUND)
    message(WARNING "libopt not found — build it first: cd ../libopt && cargo build --release")
    set(LIBOPT_LIB "")
endif()

# ── Sources ──
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/optbackend.cpp
    src/packagelistmodel.cpp
)

set(HEADERS
    src/mainwindow.h
    src/optbackend.h
    src/packagelistmodel.h
)

# ── Executable ──
add_executable(opt-gui ${SOURCES} ${HEADERS})

target_link_libraries(opt-gui PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    ${LIBOPT_LIB}
    dl          # for dlopen/dlsym (cdylib path)
    pthread     # Rust runtime needs pthread
)

target_include_directories(opt-gui PRIVATE src)

# ── Install ──
install(TARGETS opt-gui DESTINATION /usr/local/bin)
