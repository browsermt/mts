add_subdirectory(marian)
add_subdirectory(ssplit-cpp)

# include_directories(crow/include)
include_directories(ssplit-cpp/src)
# include_directories(rapidjson/include)

################################################################################
# add_subdirectory does not propagate certain variables to the parent.
# We need to do that specifically.

# INCLUDE_DIRECTORIES from marian
get_property(INCDIRS DIRECTORY marian/src PROPERTY INCLUDE_DIRECTORIES)
get_directory_property(CTI DIRECTORY marian DEFINITION CUDA_TOOLKIT_INCLUDE)
include_directories(${INCDIRS})
include_directories(${CTI})

get_property(INCDIRS DIRECTORY marian/src/3rd_party/sentencepiece/src PROPERTY INCLUDE_DIRECTORIES)
include_directories(${INCDIRS})

get_property(INCLUDE_DIRECTORIES DIRECTORY . PROPERTY INCLUDE_DIRECTORIES)
set(INCLUDE_DIRECTORIES ${INCLUDE_DIRECTORIES} PARENT_SCOPE)

# EXT_LIBS
get_directory_property(EXT_LIBS DIRECTORY marian DEFINITION EXT_LIBS)
set(EXT_LIBS ${EXT_LIBS} PARENT_SCOPE)

# Compilation flags
get_directory_property(CMAKE_C_FLAGS DIRECTORY marian DEFINITION CMAKE_C_FLAGS)
get_directory_property(CMAKE_CXX_FLAGS DIRECTORY marian DEFINITION CMAKE_CXX_FLAGS)
set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} PARENT_SCOPE)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} PARENT_SCOPE)
