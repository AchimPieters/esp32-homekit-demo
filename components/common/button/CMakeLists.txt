idf_component_register(
    SRCS toggle.c button.c port.c
    INCLUDE_DIRS .
)
target_compile_options(${COMPONENT_LIB} PRIVATE -DESP_IDF)
