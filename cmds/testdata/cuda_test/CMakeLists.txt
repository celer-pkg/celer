cmake_minimum_required(VERSION 3.10.0)

project(test_cuda VERSION 0.1.0 LANGUAGES C CXX)

enable_language(CUDA)

add_executable(test_cuda test_cuda.cu)
target_link_libraries(test_cuda PRIVATE ${CUDA_CUDART_LIBRARY})
