include(configure_config.cmake)
configure_file(config_functions.h.in ${ConfigIncludePath}/config_functions.h)

add_subdirectory(divide)

include("${ClickHouse_SOURCE_DIR}/cmake/dbms_glob_sources.cmake")
add_headers_and_sources(clickhouse_functions .)

list(REMOVE_ITEM clickhouse_functions_sources IFunction.cpp FunctionFactory.cpp FunctionHelpers.cpp)
list(REMOVE_ITEM clickhouse_functions_headers IFunction.h FunctionFactory.h FunctionHelpers.h)

if (ENABLE_CUDA)
    # list(APPEND clickhouse_functions_sources cuda_func.cu)
    # set (CMAKE_CUDA_STANDARD 14)

    #######################################
    # include(FindCUDA/select_compute_arch)

    # cuda_detect_installed_gpus(INSTALLED_GPU_CCS_1)

    # string(STRIP "${INSTALLED_GPU_CCS_1}" INSTALLED_GPU_CCS_2)
    # string(REPLACE " " ";" INSTALLED_GPU_CCS_3 "${INSTALLED_GPU_CCS_2}")
    # string(REPLACE "." "" CUDA_ARCH_LIST "${INSTALLED_GPU_CCS_3}")
    # set(CMAKE_CUDA_ARCHITECTURES 52)# ${CUDA_ARCH_LIST})
    message("1############ ${CMAKE_CUDA_ARCHITECTURES} ${CMAKE_CXX_COMPILER}")
    message("2############ ${CUDA_INCLUDE_DIRS}")
    message("3############ ${CUDA_HOST_COMPILER} ${CMAKE_CUDA_COMPILER}")
    message(STATUS "4############ CMAKE_CUDA_FLAGS: ${CMAKE_CUDA_FLAGS}")
    # message("???? ==== ???? ${LD_LIBRARY_PATH}")
    ########################################

    # set(CUDA_HOST_COMPILER clang++-14)
    # set(CMAKE_CUDA_COMPILER /usr/bin/clang++-14)

    # set(
    #     CUDA_NVCC_FLAGS
    #     # ${CUDA_NVCC_FLAGS}; 
    #     -O3 
    #     -gencode arch=compute_52,code=sm_52
    # )

    add_library(clickhouse_functions_cuda cuda_func.cu)
    # target_link_directories(clickhouse_functions_cuda PUBLIC /usr/local/cuda-11.6/targets/x86_64-linux/lib)


    # get_target_property(heyheyhey2 clickhouse_functions_cuda INTERFACE_LINK_LIBRARIES)
    # message("### heyhey mortals2: -${heyheyhey2}-")

    # set_target_properties(clickhouse_functions_cuda PROPERTIES INTERFACE_LINK_LIBRARIES "")
    # set_source_files_properties(cuda_func.cu PROPERTIES COMPILE_FLAGS "-std=c++17" INCLUDE_DIRECTORIES "")
    # get_source_file_property(hey_id cuda_func.cu INCLUDE_DIRECTORIES)
    # get_source_file_property(hey_ill cuda_func.cu INTERFACE_LINK_LIBRARIES)
    # message("### heyhey mortals1: *${hey_id}* *${hey_ill}*")
    # get_target_property(heyheyhey2 clickhouse_functions_cuda INTERFACE_LINK_LIBRARIES)
    # message("### heyhey mortals2: ${heyheyhey2}")

    # include(FindCUDA)
    # cuda_add_library(clickhouse_functions_cuda cuda_func.cu OPTIONS -std=c++14)
endif ()

add_library(clickhouse_functions ${clickhouse_functions_sources})

# if (ENABLE_CUDA)
#     list(APPEND clickhouse_functions_sources cuda_func.cu)

#     # set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} 1")
#     # message("## CMAKE_CUDA_STANDARD:  ${CMAKE_CUDA_STANDARD}")
#     # set (CMAKE_CUDA_STANDARD 17)

#     # target_compile_options(clickhouse_functions PRIVATE "$<$<COMPILE_LANGUAGE:CUDA>:SHELL: 2>")
#     # message("## CMAKE_CUDA_FLAGS:  ${CMAKE_CUDA_FLAGS}")
# endif ()

target_link_libraries(clickhouse_functions
    PUBLIC
        ch_contrib::cityhash
        ch_contrib::farmhash
        clickhouse_dictionaries
        clickhouse_dictionaries_embedded
        clickhouse_parsers
        ch_contrib::consistent_hashing
        dbms
        ch_contrib::metrohash
        ch_contrib::murmurhash

    PRIVATE
        ch_contrib::zlib
        boost::filesystem
        divide_impl
)

if (TARGET OpenSSL::Crypto)
    target_link_libraries(clickhouse_functions PUBLIC OpenSSL::Crypto)
endif()

if (CMAKE_BUILD_TYPE_UC STREQUAL "RELEASE"
    OR CMAKE_BUILD_TYPE_UC STREQUAL "RELWITHDEBINFO"
    OR CMAKE_BUILD_TYPE_UC STREQUAL "MINSIZEREL")
    set (STRIP_DSF_DEFAULT ON)
else()
    set (STRIP_DSF_DEFAULT OFF)
endif()


#  Provides faster linking and lower binary size.
#  Tradeoff is the inability to debug some source files with e.g. gdb
#  (empty stack frames and no local variables)."
option(STRIP_DEBUG_SYMBOLS_FUNCTIONS "Do not generate debugger info for ClickHouse functions" ${STRIP_DSF_DEFAULT})

if (STRIP_DEBUG_SYMBOLS_FUNCTIONS)
    message(INFO "Not generating debugger info for ClickHouse functions")
    target_compile_options(clickhouse_functions PRIVATE "$<$<COMPILE_LANGUAGE:ASM,C,CXX>:-g0>")
else()
    message(STATUS "Generating debugger info for ClickHouse functions")
endif()

if (TARGET ch_contrib::icu)
    target_link_libraries (clickhouse_functions PRIVATE ch_contrib::icu)
endif ()

if (TARGET ch_contrib::fastops)
    target_link_libraries (clickhouse_functions PRIVATE ch_contrib::fastops)
endif ()

if (ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (TARGET ch_contrib::llvm)
    target_link_libraries(clickhouse_functions PRIVATE ch_contrib::llvm)
endif ()

if (TARGET ch_contrib::base64)
    target_link_libraries(clickhouse_functions PRIVATE ch_contrib::base64)
endif()

target_link_libraries(clickhouse_functions PRIVATE ch_contrib::lz4)

if (ENABLE_NLP)
    target_link_libraries(clickhouse_functions PRIVATE ch_contrib::cld2)
endif()

if (ENABLE_CUDA)
    target_link_libraries(clickhouse_functions PRIVATE clickhouse_functions_cuda)
endif()

if (TARGET ch_contrib::h3)
    target_link_libraries (clickhouse_functions PRIVATE ch_contrib::h3)
endif()

if (TARGET ch_contrib::hyperscan)
    target_link_libraries(clickhouse_functions PRIVATE ch_contrib::hyperscan)
endif()

if (TARGET ch_contrib::simdjson)
    target_link_libraries(clickhouse_functions PRIVATE ch_contrib::simdjson)
endif()

if (TARGET ch_contrib::rapidjson)
    target_link_libraries(clickhouse_functions PRIVATE ch_contrib::rapidjson)
endif()

add_subdirectory(GatherUtils)
target_link_libraries(clickhouse_functions PRIVATE clickhouse_functions_gatherutils)

add_subdirectory(URL)
target_link_libraries(clickhouse_functions PRIVATE clickhouse_functions_url)

add_subdirectory(array)
target_link_libraries(clickhouse_functions PRIVATE clickhouse_functions_array)

add_subdirectory(JSONPath)

# Signed integer overflow on user-provided data inside boost::geometry - ignore.
set_source_files_properties("pointInPolygon.cpp" PROPERTIES COMPILE_FLAGS -fno-sanitize=signed-integer-overflow)

if (ENABLE_FUZZING)
    add_compile_definitions(FUZZING_MODE=1)
endif ()

# get_target_property(heyheyhey2 clickhouse_functions_cuda INTERFACE_LINK_LIBRARIES)
# message("### heyhey mortals2: ${heyheyhey2}")
