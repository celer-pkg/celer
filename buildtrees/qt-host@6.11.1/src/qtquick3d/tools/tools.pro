TEMPLATE = subdirs

# Only build on platforms that would run tooling
!integrity:!android|android_app:!wasm:!cross_compile {
    SUBDIRS = \
        balsam \
        meshdebug \
        shadergen \
        instancer \
        shapegen
}
