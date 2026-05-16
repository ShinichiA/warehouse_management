cmake_minimum_required(VERSION 3.28)
project(iot_ware_house VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(APP_SOURCES
        apps/app.cpp
        src/protocols/modbus/modbus_rtu.cpp
        src/protocols/mqtt/mosquitto_client.cpp
        src/protocols/mqtt/hive_mqtt_client.cpp
        src/sensors/SHT35.cpp
)

add_executable(${PROJECT_NAME}
        apps/main.cpp
        ${APP_SOURCES}
)

# Link libraries
find_package(PkgConfig REQUIRED)
pkg_check_modules(MODBUS REQUIRED libmodbus)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)
find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE 
        Threads::Threads
        ${MODBUS_LIBRARIES}
        ${MOSQUITTO_LIBRARIES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${MODBUS_INCLUDE_DIRS}
        ${MOSQUITTO_INCLUDE_DIRS}
)

# Copy config to build directory
configure_file(
        ${CMAKE_SOURCE_DIR}/config/config.json
        ${CMAKE_BINARY_DIR}/config/config.json
        COPYONLY
)
