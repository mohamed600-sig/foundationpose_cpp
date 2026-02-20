cmake_minimum_required(VERSION 3.8)
project(foundationpose_cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)



add_subdirectory(easy_deploy_tool)
add_subdirectory(detection_6d_foundationpose)

add_subdirectory(simple_tests)

set(CONFIG_INSTALL_DIR lib/cmake/${PROJECT_NAME})

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
"## Auto-generated ${PROJECT_NAME}Config.cmake\n
include(\"\${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Targets.cmake\")\n
")

install(TARGETS deploy_core trt_core detection_6d_foundationpose

    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin

)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/detection_6d_foundationpose/include/
        DESTINATION include)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/easy_deploy_tool/deploy_core/include/
        DESTINATION include)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/easy_deploy_tool/deploy_utils/image_processing_utils/include/
        DESTINATION include)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/easy_deploy_tool/inference_core/trt_core/include/
        DESTINATION include)

install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CONFIG_INSTALL_DIR})
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    DESTINATION ${CONFIG_INSTALL_DIR}
)