cmake_minimum_required(VERSION 3.16)

project(sttrader_payment_examples LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(PAYMENT_EXAMPLES_ENABLE_LTO "Enable interprocedural optimization for Release builds" ON)

add_library(payment_praxis MODULE praxis_cashier_payment.cpp)

target_include_directories(payment_praxis PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../include"
)

target_compile_options(payment_praxis PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -fvisibility=hidden
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Release>:-DNDEBUG>
    $<$<CONFIG:RelWithDebInfo>:-O2>
    $<$<CONFIG:RelWithDebInfo>:-g>
    $<$<CONFIG:RelWithDebInfo>:-DNDEBUG>
)

set_target_properties(payment_praxis PROPERTIES
    PREFIX ""
    OUTPUT_NAME "payment_praxis"
)

if(PAYMENT_EXAMPLES_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT PAYMENT_EXAMPLES_IPO_SUPPORTED OUTPUT PAYMENT_EXAMPLES_IPO_ERROR)
    if(PAYMENT_EXAMPLES_IPO_SUPPORTED)
        set_property(TARGET payment_praxis PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        set_property(TARGET payment_praxis PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
    else()
        message(WARNING "Payment example LTO is unavailable: ${PAYMENT_EXAMPLES_IPO_ERROR}")
    endif()
endif()
