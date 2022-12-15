ifdef component_compile_rules

    INC_DIRS += $(button_ROOT)

    button_INC_DIR = $(button_ROOT)
    button_SRC_DIR = $(button_ROOT)

    $(eval $(call component_compile_rules,button))
else
    COMPONENT_ADD_INCLUDEDIRS = .
endif
