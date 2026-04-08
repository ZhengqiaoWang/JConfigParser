cmake_minimum_required(VERSION 3.10)
project(JConfigParser CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 编译选项
option(JCONFIGPARSER_BUILD_EXAMPLES "Build example programs" ON)
option(JCONFIGPARSER_BUILD_TESTS "Build unit tests" ON)
option(JCONFIGPARSER_BUILD_BENCHMARKS "Build benchmark programs" ON)
option(JCONFIGPARSER_ENABLE_COVERAGE "Enable code coverage" OFF)

add_compile_definitions(RAPIDJSON_HAS_STDSTRING)

# 头文件路径
add_library(JConfigParser INTERFACE)
target_include_directories(JConfigParser INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)


# 查找 RapidJSON
find_package(RapidJSON QUIET)
if(RapidJSON_FOUND AND TARGET RapidJSON::rapidjson)
    target_link_libraries(JConfigParser INTERFACE RapidJSON::rapidjson)
else()
    # 如果未找到 RapidJSON 目标，使用系统默认路径
    target_include_directories(JConfigParser SYSTEM INTERFACE
        /usr/include
    )
endif()

# 覆盖率选项
if(JCONFIGPARSER_ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage -O0")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# ==================== 示例程序 ====================
if(JCONFIGPARSER_BUILD_EXAMPLES)
    add_executable(example example/example.cpp)
    if(WIN32)
        set_target_properties(example PROPERTIES WIN32_EXECUTABLE FALSE)
    endif()
    target_link_libraries(example PRIVATE JConfigParser)
    target_include_directories(example PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/rapidjson/include
    )
    target_include_directories(example PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/JConfigParser
    )
endif()

# ==================== 单元测试 ====================
if(JCONFIGPARSER_BUILD_TESTS)
    enable_testing()

    add_executable(test_jconfigparser ut/unittest.cpp)
    if(WIN32)
        set_target_properties(test_jconfigparser PROPERTIES WIN32_EXECUTABLE FALSE)
    endif()
    target_include_directories(test_jconfigparser PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/doctest
    )
    target_include_directories(test_jconfigparser PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/rapidjson/include
    )
    target_include_directories(test_jconfigparser PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/JConfigParser
    )
    target_link_libraries(test_jconfigparser PRIVATE JConfigParser)

    add_test(NAME JConfigParserTest COMMAND test_jconfigparser)

    # 覆盖率报告目标
    if(JCONFIGPARSER_ENABLE_COVERAGE)
        find_program(LCOV_PATH lcov)
        find_program(GENHTML_PATH genhtml)

        if(LCOV_PATH AND GENHTML_PATH)
            add_custom_target(coverage
                COMMAND ${LCOV_PATH} --capture --directory ${CMAKE_BINARY_DIR}
                    --base-directory ${CMAKE_SOURCE_DIR}
                    --output-file ${CMAKE_BINARY_DIR}/coverage.info --no-external --quiet
                COMMAND ${LCOV_PATH} --extract ${CMAKE_BINARY_DIR}/coverage.info
                    '*/Node.h'
                    --output-file ${CMAKE_BINARY_DIR}/jconfigparser_coverage.info --quiet
                COMMAND ${GENHTML_PATH} ${CMAKE_BINARY_DIR}/jconfigparser_coverage.info
                    --output-directory ${CMAKE_BINARY_DIR}/coverage_report
                    --title "Node.h Coverage" --quiet
                COMMENT "Generating coverage report..."
            )
        endif()
    endif()
endif()

# # ==================== 性能测试 ====================
# if(FASTJSON_BUILD_BENCHMARKS)
#     # 数据生成器
#     add_executable(gen_data benchmark/gen_data.cpp)
#     target_link_libraries(gen_data PRIVATE JConfigParser)

#     # 各库独立 benchmark
#     add_executable(bench_fastjson benchmark/bench_fastjson.cpp)
#     target_link_libraries(bench_fastjson PRIVATE JConfigParser)

#     add_executable(bench_rapidjson benchmark/bench_rapidjson.cpp)

#     add_executable(bench_nlohmann benchmark/bench_nlohmann.cpp)
# endif()
