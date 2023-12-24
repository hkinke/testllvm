cmake_minimum_required(VERSION 3.24)

project(node_jit)

include(GNUInstallDirs)

# https://llvm.org/docs/CMake.html#id19
find_package(LLVM REQUIRED CONFIG)

# include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(llvm_libs core native orcjit)



message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "${LLVM_INCLUDE_DIRS}")

add_executable(main main.cpp)
target_link_libraries(main "${llvm_libs}")
target_include_directories(main PUBLIC "${LLVM_INCLUDE_DIRS}")
