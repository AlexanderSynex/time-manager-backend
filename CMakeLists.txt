cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME worktime-manager)

project(${PROJECT_NAME})

if(NOT CMAKE_BUILD_TYPE)
  set(default_build_type "Debug")
  message(STATUS "Set the build type to `${default_build_type}` as none was specified.")
  set(CMAKE_BUILD_TYPE ${default_build_type} CACHE STRING "Chooce the build type." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
  "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "${PROJECT_NAME} Build Type: ${CMAKE_BUILD_TYPE}")

# Set the version for ${PROJECT_NAME}
set(${PROJECT_NAME}_Version_Major 0)
set(${PROJECT_NAME}_Version_Minor 1)
set(${PROJECT_NAME}_Version_Patch 0)
set(${PROJECT_NAME}_Version_Status "-dev")
set(PROJECT_VERSION
  "${${PROJECT_NAME}_Version_Major}.${${PROJECT_NAME}_Version_Minor}.${${PROJECT_NAME}_Version_Patch}${${PROJECT_NAME}_Version_Status}"
)
message(STATUS "${PROJECT_NAME} version: ${PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(third-party)

find_package(userver COMPONENTS core postgresql REQUIRED)

set(SOURCES src/main.cpp 
            src/info/Worker.cpp
            src/info/Department.cpp
            src/services/AdministrationService.cpp
            src/services/WorktimeService.cpp
            src/services/UserInfoService.cpp
            src/services/AuthService.cpp
            src/services/AuthCheckerFactory.cpp
            src/services/auth/schemas/BearerSchema.cpp
            src/services/details/UserService.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

userver_add_sql_library(
    ${PROJECT_NAME}_postgres_sql
    NAMESPACE
    worktime_postgres_service
    SOURCE_DIR
    db/queries
    OUTPUT_DIR
    ${CMAKE_CURRENT_BINARY_DIR}
    SQL_FILES
    *.sql
)

set(SQL_LIBS ${PROJECT_NAME}_postgres_sql)


target_link_libraries(${PROJECT_NAME} PRIVATE userver::core userver::postgresql ${SQL_LIBS})
target_include_directories(${PROJECT_NAME} PUBLIC include)

if(EXISTS ${CMAKE_BINARY_DIR}/compile_commands.json)
  file(COPY ${CMAKE_BINARY_DIR}/compile_commands.json DESTINATION ${CMAKE_SOURCE_DIR})
endif()
