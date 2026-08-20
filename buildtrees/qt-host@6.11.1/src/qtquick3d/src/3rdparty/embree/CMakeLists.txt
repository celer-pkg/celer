qt_internal_add_3rdparty_library(BundledEmbree
    STATIC
    INSTALL
    SKIP_AUTOMOC
    EXCEPTIONS
    SOURCES
        include/embree3/rtcore_builder.h
        include/embree3/rtcore.h
        include/embree3/rtcore.isph
        include/embree3/rtcore_buffer.h
        include/embree3/rtcore_quaternion.isph
        include/embree3/rtcore_scene.isph
        include/embree3/rtcore_common.h
        include/embree3/rtcore_scene.h
        include/embree3/rtcore_geometry.h
        include/embree3/rtcore_ray.h
        include/embree3/rtcore_buffer.isph
        include/embree3/rtcore_device.h
        include/embree3/rtcore_common.isph
        include/embree3/rtcore_quaternion.h
        include/embree3/rtcore_ray.isph
        include/embree3/rtcore_config.h
        include/embree3/rtcore_geometry.isph
        include/embree3/rtcore_device.isph
        common/sys/sysinfo.cpp
        common/sys/alloc.cpp
        common/sys/filename.cpp
        common/sys/library.cpp
        common/sys/thread.cpp
        common/sys/string.cpp
        common/sys/regression.cpp
        common/sys/mutex.cpp
        common/sys/condition.cpp
        common/sys/barrier.cpp
        common/math/constants.cpp
        common/simd/sse.cpp
        common/lexers/stringstream.cpp
        common/lexers/tokenstream.cpp
        common/tasking/taskschedulerinternal.cpp
        kernels/common/device.cpp
        kernels/common/stat.cpp
        kernels/common/acceln.cpp
        kernels/common/accelset.cpp
        kernels/common/state.cpp
        kernels/common/rtcore.cpp
        kernels/common/rtcore_builder.cpp
        kernels/common/scene.cpp
        kernels/common/alloc.cpp
        kernels/common/geometry.cpp
        kernels/common/scene_triangle_mesh.cpp
        kernels/geometry/primitive4.cpp
        kernels/builders/primrefgen.cpp
        kernels/bvh/bvh.cpp
        kernels/bvh/bvh_statistics.cpp
        kernels/bvh/bvh4_factory.cpp
        kernels/bvh/bvh8_factory.cpp
        kernels/bvh/bvh_collider.cpp
        kernels/bvh/bvh_rotate.cpp
        kernels/bvh/bvh_refit.cpp
        kernels/bvh/bvh_builder.cpp
        kernels/bvh/bvh_builder_morton.cpp
        kernels/bvh/bvh_builder_sah.cpp
        kernels/bvh/bvh_builder_sah_spatial.cpp
        kernels/bvh/bvh_builder_sah_mb.cpp
        kernels/bvh/bvh_builder_twolevel.cpp
        kernels/bvh/bvh_intersector1_bvh4.cpp
    PUBLIC_INCLUDE_DIRECTORIES
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

qt_disable_warnings(BundledEmbree)
qt_set_symbol_visibility_hidden(BundledEmbree)

if (WIN32)
    qt_disable_unicode_defines(BundledEmbree)
endif()

if (IOS)
    qt_internal_extend_target(BundledEmbree COMPILE_OPTIONS -fembed-bitcode)
endif()

# Use SSE2 only, ignore AVX/SSE4.2 for now
qt_internal_extend_target(BundledEmbree DEFINES
    EMBREE_TARGET_SSE2
    __SSE2__
    __SSE__
    EMBREE_LOWEST_ISA
    TASKING_INTERNAL
    NDEBUG
)
