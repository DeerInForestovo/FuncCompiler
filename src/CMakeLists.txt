cmake_minimum_required(VERSION 3.5)
project(compiler)

# Find all the required packages
find_package(BISON)
find_package(FLEX)

# Set up the flex and bison targets
bison_target(parser
    ${CMAKE_CURRENT_SOURCE_DIR}/parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp
    COMPILE_FLAGS "-d")
flex_target(scanner
    ${CMAKE_CURRENT_SOURCE_DIR}/scanner.l
    ${CMAKE_CURRENT_BINARY_DIR}/scanner.cpp)
add_flex_bison_dependency(scanner parser)

# Create compiler executable
add_executable(compiler
    definition.cpp definition.hpp
    parsed_type.cpp parsed_type.hpp
    ast.cpp ast.hpp
    type_env.cpp type_env.hpp
    type.cpp type.hpp
    error.cpp error.hpp
    binop.cpp binop.hpp
    uniop.cpp uniop.hpp
    graph.cpp graph.hpp
    ${BISON_parser_OUTPUTS}
    ${FLEX_scanner_OUTPUTS}
    main.cpp
)

# Configure compiler executable
target_include_directories(compiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(compiler PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(compiler PUBLIC ${LLVM_INCLUDE_DIRS})
target_compile_definitions(compiler PUBLIC ${LLVM_DEFINITIONS})
target_link_libraries(compiler ${LLVM_LIBS})
