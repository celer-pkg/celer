#! [0]
cmake_minimum_required(VERSION 3.16...3.22)
project(MyProject)

find_package(Qt6 REQUIRED COMPONENTS Protobuf Grpc)
qt_standard_project_setup()

qt_add_executable(MyApp main.cpp)

qt_add_protobuf(MyApp
    PROTO_FILES
        path/to/messages.proto
)

qt_add_grpc(MyApp CLIENT
    PROTO_FILES
        path/to/service.proto
)

target_link_libraries(MyApp PRIVATE Qt6::Protobuf Qt6::Grpc)
#! [0]
