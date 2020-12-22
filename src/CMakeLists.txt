link_directories(${CMAKE_BINARY_DIR}/local/lib) # required to find libnccl.a

if (CUDA_FOUND)
  message("FOUND CUDA")
  set(MARIAN_CUDA_LIB marian_cuda)
endif(CUDA_FOUND)

if (COMPILE_BERGAMOT_API)
  add_executable(main bergamot/main.cpp)

  add_library(libbergamot STATIC 
      bergamot/textops.cpp
      bergamot/batch_translator.cpp 
      bergamot/multifactor_priority.cpp 
      bergamot/request.cpp 
      bergamot/service.cpp
      bergamot/batcher.cpp
  )

  set_target_properties(
    main PROPERTIES OUTPUT bergamot-cli
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
   )

  target_compile_options(main PUBLIC ${ALL_WARNINGS})
  set(EXECUTABLES ${EXECUTABLES} main)
  target_link_libraries(main libbergamot marian ${MARIAN_CUDA_LIB} ${EXT_LIBS} ssplit pcrecpp.a pcre.a)
endif(COMPILE_BERGAMOT_API)
