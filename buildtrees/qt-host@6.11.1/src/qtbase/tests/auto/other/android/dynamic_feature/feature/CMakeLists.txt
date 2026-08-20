cmake_minimum_required(VERSION 3.16)

qt6_add_library(tst_android_dynamic_feature_resources SHARED)
qt6_add_resources(tst_android_dynamic_feature_resources "dynamic_resources"
    PREFIX "/dynamic_resources"
    FILES "qtlogo.png")

target_link_libraries(tst_android_dynamic_feature_resources PRIVATE Qt6::Core)
