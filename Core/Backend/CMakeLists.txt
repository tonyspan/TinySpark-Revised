set(LIBSRC)

file(GLOB SRC_CC "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")
file(GLOB SRC_H "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

foreach(_file ${SRC_CC})
    get_filename_component(CC_FILE ${_file} NAME)
    list(APPEND LIBSRC ${CC_FILE})
endforeach()

foreach(_file ${SRC_H})
    get_filename_component(H_FILE ${_file} NAME)
    list(APPEND LIBSRC ${H_FILE})
endforeach()

add_library(Backend STATIC ${LIBSRC})