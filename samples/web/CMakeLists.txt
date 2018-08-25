cmake_minimum_required(VERSION 3.1)
project(web-samples)

# ==================================================================================================
# Build materials into inc files.
# ==================================================================================================

set(MATERIAL_SRCS
        ../materials/bakedColor.mat)

if (CMAKE_CROSSCOMPILING)
    set(IMPORT_EXECUTABLES ${FILAMENT}/${IMPORT_EXECUTABLES_DIR})
    set(IMPORT_EXECUTABLES ${IMPORT_EXECUTABLES}/ImportExecutables-${CMAKE_BUILD_TYPE}.cmake)
    include(${IMPORT_EXECUTABLES})
endif()

set(MATC_TARGET mobile)
set(MATC_FLAGS -a all)
if (CMAKE_BUILD_TYPE MATCHES Release)
    set(MATC_FLAGS -O ${MATC_FLAGS})
endif()

set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
file(MAKE_DIRECTORY "${GENERATION_ROOT}/generated/material/")

set(MATERIAL_BINS)
foreach (mat_src ${MATERIAL_SRCS})
    get_filename_component(localname "${mat_src}" NAME_WE)
    get_filename_component(fullname "${mat_src}" ABSOLUTE)
    set(output_path "${GENERATION_ROOT}/generated/material/${localname}.inc")
    add_custom_command(
            OUTPUT ${output_path}
            COMMAND matc ${MATC_FLAGS} -p ${MATC_TARGET} -m material -f header
            -o ${output_path} ${fullname}
            MAIN_DEPENDENCY ${mat_src}
            DEPENDS matc
            COMMENT "Compiling material ${mat_src} to ${output_path}")
    list(APPEND MATERIAL_BINS ${output_path})
endforeach()

add_custom_target(sample_materials DEPENDS ${MATERIAL_BINS})

# ==================================================================================================
# Build WebAssembly modules (wasm files).
# ==================================================================================================

set(EXPORTS "'_launch','_main','_render','_resize'")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s EXPORTED_FUNCTIONS=[${EXPORTS}]")

function(add_demo NAME)
    include_directories(${GENERATION_ROOT})
    add_executable(${NAME} ${NAME}.cpp ../app/MeshIO.cpp WebApp.h)
    add_dependencies(${NAME} sample_materials)
    target_link_libraries(${NAME} PRIVATE filament math filamat utils)
endfunction()

add_demo(suzanne)

# ==================================================================================================
# Copy Assets.
#
# This creates a structure like:
#
#    /suzanne.js       built by em++
#    /suzanne.wasm     built by em++
#    /index.html       copied from REPO/samples/web
#    /filament.js      copied from REPO/samples/web
#    /assets/fonts     copied from REPO/assets
#    /assets/models    copied from REPO/assets
#    /textures         copied from REPO/third_party
#    /envs             copied from REPO/samples/envs
#
# ==================================================================================================

function(copy_assets PATH NAME)
    file(COPY ${PATH}/${NAME} DESTINATION ${PROJECT_BINARY_DIR})
    add_custom_target(${NAME} ALL DEPENDS ${NAME})
    add_dependencies(filament ${NAME})
endfunction()

copy_assets("../../third_party" textures)
copy_assets("../.." assets)
copy_assets(".." envs)
copy_assets("." index.html)
copy_assets("." filament.js)
