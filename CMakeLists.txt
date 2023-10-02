# CMakeList.txt: проект CMake для TransportCatalogue; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.

cmake_minimum_required(VERSION 3.10)

# Включение горячей перезагрузки для компиляторов MSVC, если поддерживается.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("TransportCatalogue")


set(CMAKE_CXX_STANDARD 17)


# Эта команда найдёт собранный нами пакет Protobuf.
# REQUIRED означает, что библиотека обязательна.
# Путь для поиска укажем в параметрах команды cmake.
find_package(Protobuf REQUIRED)
# Помимо Protobuf, понадобится библиотека Threads
find_package(Threads REQUIRED)

set (proto
    "proto/transport_catalogue.proto"
    "proto/map_renderer.proto"
    "proto/svg.proto"
    "proto/graph.proto"
    "proto/transport_router.proto")

set (sources
    "main.cpp"
    "src/domain.cpp"
    "src/geo.cpp"
    "src/json.cpp"
    "src/json_builder.cpp"
    "src/json_reader.cpp"
    "src/map_renderer.cpp"
    "src/request_handler.cpp"
    "src/serialization.cpp"
    "src/svg.cpp"
    "src/transport_catalogue.cpp"
    "src/transport_router.cpp")

set (headers
    "include/domain.h"
    "include/geo.h"
    "include/graph.h"
    "include/json.h"
    "include/json_builder.h"
    "include/json_reader.h"
    "include/map_renderer.h"
    "include/ranges.h"
    "include/request_handler.h"
    "include/router.h"
    "include/serialization.h"
    "include/svg.h"
    "include/transport_catalogue.h"
    "include/transport_router.h")

# Команда вызова protoc. 
# Ей переданы названия переменных, в которые будут сохранены 
# списки сгенерированных файлов, а также сам proto-файл.
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${proto})

# добавляем цель - transport_catalogue
add_executable(transport_catalogue ${sources} ${headers} ${proto} ${PROTO_SRCS} ${PROTO_HDRS})

# find_package определила переменную Protobuf_INCLUDE_DIRS,
# которую нужно использовать как include-путь.
# Также нужно добавить как include-путь директорию, куда
# protoc положит сгенерированные файлы.
target_include_directories(transport_catalogue PRIVATE "include")

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# Также find_package определила Protobuf_LIBRARY.
# Protobuf зависит от библиотеки Threads. Добавим и её при компоновке.
string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)
