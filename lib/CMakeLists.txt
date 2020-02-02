cmake_minimum_required (VERSION 3.5.0 FATAL_ERROR)
project (simple-svg-writer)

add_library (${PROJECT_NAME} INTERFACE)
target_include_directories (${PROJECT_NAME} INTERFACE include/)
                     
if(MSVC)
   target_compile_definitions (${PROJECT_NAME} 
    PUBLIC
            _CRT_SECURE_NO_WARNINGS
            _SCL_SECURE_NO_WARNINGS
            NOMINMAX
   )
endif(MSVC)

