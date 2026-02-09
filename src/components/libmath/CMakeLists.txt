idf_component_register(
    SRCS "gcc_math.c"
    INCLUDE_DIRS "."
    REQUIRES driver
)

target_compile_options(${COMPONENT_LIB} PRIVATE -fPIC)
