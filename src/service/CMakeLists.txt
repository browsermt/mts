link_directories(${CMAKE_BINARY_DIR}/local/lib) # required to find libnccl.a

# Dependencies. Apparently paths need to be set before targets are
# created, so we ensure dependencies first, then create targets.
# if (COMPILE_REST_SERVER OR COMPILE_AMQP_WORKER OR COMPILE_WEBSOCKET_SERVER)
#   find_package(PCRECPP) # needed for ssplit-cpp
#   add_library(marian-service STATIC
#     api/bergamot/node_translation.cpp
#     api/output_options.cpp
#     api/rapidjson_utils.cpp
#     common/plaintext_translation.cpp
#     common/queued_input.cpp
#     common/translation_job.cpp
#     common/translation_service.cpp
#     common/translation_worker.cpp
#     quality_estimation/qe.cpp
#     )
#   target_compile_options(marian-service PUBLIC ${ALL_WARNINGS})
# endif(COMPILE_REST_SERVER OR COMPILE_AMQP_WORKER OR COMPILE_WEBSOCKET_SERVER)

# FOR DEBUGGING THE CMAKE SETUP
# get_directory_property(IDIRS INCLUDE_DIRECTORIES)
# message("CMAKE INCLUDE DIRS for ${CMAKE_CURRENT_SOURCE_DIR}:")
# foreach(d ${IDIRS})
#   message(" - ${d}")
# endforeach()

if (CUDA_FOUND)
  message("FOUND CUDA")
  set(MARIAN_CUDA_LIB marian_cuda)
endif(CUDA_FOUND)

# BERGAMOT_API
if (COMPILE_BERGAMOT_API)
  add_executable(main bintr/main.cpp)
  add_library(libbergamot STATIC bintr/textops.cpp
      bintr/batch_translator.cpp bintr/request.cpp
      bintr/batcher.cpp
      bintr/service.cpp
      bintr/multifactor_priority.cpp)
  set_target_properties(
    main PROPERTIES OUTPUT bergamot-cli
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
   )

  target_compile_options(main PUBLIC ${ALL_WARNINGS})
  set(EXECUTABLES ${EXECUTABLES} main)
  target_link_libraries(main libbergamot marian ${MARIAN_CUDA_LIB} ${EXT_LIBS} ssplit pcrecpp.a pcre.a)
endif(COMPILE_BERGAMOT_API)


# REST SERVER
# if (COMPILE_REST_SERVER)
#   add_executable(rest-server rest/rest_server.cpp)
#   set_target_properties(rest-server PROPERTIES
#     OUTPUT marian-rest-server
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
#   target_compile_options(rest-server PUBLIC ${ALL_WARNINGS})
#   set(EXECUTABLES ${EXECUTABLES} rest-server)
#   # TODO: use of static vs shared pcre and pcrecpp libraries should
#   # depend on USE_STATIC_LIBS, but I couldn't get that to work.
#   target_link_libraries(rest-server marian-service marian ${MARIAN_CUDA_LIB}
#     ${EXT_LIBS} ssplit pcrecpp.a pcre.a)
# endif(COMPILE_REST_SERVER)

# if (COMPILE_AMQP_WORKER)
#   add_executable(amqp-worker amqp/amqp_worker.cpp)
#   set_target_properties(amqp-worker PROPERTIES OUTPUT marian-amqp-worker)
#   target_compile_options(amqp-worker PUBLIC ${ALL_WARNINGS})
#   set(EXECUTABLES ${EXECUTABLES} amqp-worker)
#   # TODO: use of static vs shared pcre and pcrecpp libraries should
#   # depend on USE_STATIC_LIBS, but I couldn't get that to work.
#   target_link_libraries(amqp-worker marian-service amqpcpp marian ${MARIAN_CUDA_LIB} ${EXT_LIBS} ev ssplit pcrecpp.a pcre.a)
# endif(COMPILE_AMQP_WORKER)
# 
# if (COMPILE_WEBSOCKET_SERVER)
#   add_executable(websocket-server server/websocket-server.cpp)
#   set_target_properties(websocket-server PROPERTIES OUTPUT websocket-server)
#   target_compile_options(websocket-server PUBLIC ${ALL_WARNINGS})
#   # TODO: use of static vs shared pcre and pcrecpp libraries should
#   # depend on USE_STATIC_LIBS, but I couldn't get that to work.
#   target_link_libraries(websocket-server marian-service marian ${MARIAN_CUDA_LIB} ${EXT_LIBS} ssplit pcrecpp.a pcre.a)
#   set(EXECUTABLES ${EXECUTABLES} websocket-server)
# endif(COMPILE_WEBSOCKET_SERVER)
# 
# install(TARGETS rest-server RUNTIME DESTINATION bin)
# install(DIRECTORY rest/ui DESTINATION lib)
