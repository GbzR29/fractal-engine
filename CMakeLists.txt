cmake_minimum_required(VERSION 4.2.1)
project(FractalEngine VERSION 0.1.0 LANGUAGES CXX C)

# ─────────────────────────────────────────────────────────────────────────────
#  Configuração Global
# ─────────────────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# ─────────────────────────────────────────────────────────────────────────────
#  Opções de build
# ─────────────────────────────────────────────────────────────────────────────
option(FE_BUILD_TOOLS      "Compilar ferramentas"      ON)
option(FE_BUILD_EDITOR     "Compilar o editor"         ON)
option(FE_ENABLE_VULKAN    "Habilitar backend Vulkan"  OFF)
option(FE_ENABLE_LUA       "Habilitar scripting Lua"   ON)
option(FE_USE_ASSIMP       "Usar Assimp"               ON)

# ─────────────────────────────────────────────────────────────────────────────
#  Caminhos de third_party
# ─────────────────────────────────────────────────────────────────────────────
set(TP "${CMAKE_SOURCE_DIR}/include/third_party")

# ─────────────────────────────────────────────────────────────────────────────
#  GLAD
# ─────────────────────────────────────────────────────────────────────────────
add_library(glad STATIC "${TP}/glad/src/glad.c")
target_include_directories(glad PUBLIC "${TP}" "${TP}/KHR")

# ─────────────────────────────────────────────────────────────────────────────
#  GLFW (Importado como binário estático)
# ─────────────────────────────────────────────────────────────────────────────
add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION             "${TP}/GLFW/lib/libglfw3.a"
    INTERFACE_INCLUDE_DIRECTORIES "${TP}"
)

# ─────────────────────────────────────────────────────────────────────────────
#  Bibliotecas Header-Only (GLM, JSON, STB, ENTT)
# ─────────────────────────────────────────────────────────────────────────────
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE "${TP}")

# nlohmann/json — header-only
# Baixe em: https://github.com/nlohmann/json/releases → json.hpp
# Estrutura esperada: include/third_party/nlohmann/json.hpp
# Uso: #include <nlohmann/json.hpp>  OU  #include <json.hpp>
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
    "${TP}/nlohmann"              # se json.hpp está direto aqui
    "${TP}/nlohmann/include"      # se veio do release completo
)

# stb — header-only, mas TextureLoader.cpp define STB_IMAGE_IMPLEMENTATION
# Estrutura esperada: include/third_party/stb_image/stb_image.h
#                  ou include/third_party/stb/stb_image.h
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE
    "${TP}/stb"            # se stb_image.h está em third_party/stb/
    "${TP}/stb_image"      # se stb_image.h está em third_party/stb_image/
)

# entt — header-only ECS
# Baixe em: https://github.com/skypjack/entt
# Estrutura esperada: include/third_party/entt/single_include/entt/entt.hpp
add_library(entt INTERFACE)
target_include_directories(entt INTERFACE
    "${TP}/entt/single_include"   # release oficial
    "${TP}/entt/src"              # se for clone do repo
    "${TP}/entt"                  # se colocou direto
)

# ─────────────────────────────────────────────────────────────────────────────
#  ImGui (Compilação Direta)
# ─────────────────────────────────────────────────────────────────────────────
file(GLOB IMGUI_SOURCES
    "${TP}/imgui/*.cpp"
    "${TP}/imgui/backends/imgui_impl_glfw.cpp"
    "${TP}/imgui/backends/imgui_impl_opengl3.cpp"
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC "${TP}/imgui" "${TP}/imgui/backends" "${TP}")

# ─────────────────────────────────────────────────────────────────────────────
#  ImGuizmo (Compilação Direta)
#  Baixe em: https://github.com/CedricGuillemet/ImGuizmo
#  Coloque ImGuizmo.h e ImGuizmo.cpp em: include/third_party/imguizmo/
# ─────────────────────────────────────────────────────────────────────────────
if (EXISTS "${TP}/imguizmo/ImGuizmo.cpp")
    add_library(imguizmo STATIC "${TP}/imguizmo/ImGuizmo.cpp")
    target_include_directories(imguizmo PUBLIC
        "${TP}/imguizmo"
        "${TP}/imgui"
    )
    target_link_libraries(imguizmo PUBLIC imgui)
    set(FE_HAS_IMGUIZMO ON)
    message(STATUS " ImGuizmo  : encontrado")
else()
    set(FE_HAS_IMGUIZMO OFF)
    message(WARNING " ImGuizmo  : não encontrado em ${TP}/imguizmo/ — gizmos desabilitados.")
endif()

# ─────────────────────────────────────────────────────────────────────────────
#  Lua (Importado como binário estático)
# ─────────────────────────────────────────────────────────────────────────────
if (FE_ENABLE_LUA)
    file(GLOB LUA_LIB "${TP}/lua/lib/*.a")
    if (LUA_LIB)
        list(GET LUA_LIB 0 LUA_LIB_PATH)
        add_library(lua STATIC IMPORTED)
        set_target_properties(lua PROPERTIES
            IMPORTED_LOCATION             "${LUA_LIB_PATH}"
            INTERFACE_INCLUDE_DIRECTORIES "${TP}/lua"
        )
    else()
        set(FE_ENABLE_LUA OFF)
    endif()
endif()

# ─────────────────────────────────────────────────────────────────────────────
#  Assimp — subdirectory (fonte completa) ou find_package (vcpkg/sistema)
# ─────────────────────────────────────────────────────────────────────────────
if (FE_USE_ASSIMP)
    if (EXISTS "${TP}/assimp/CMakeLists.txt")
        set(ASSIMP_BUILD_TESTS          OFF CACHE BOOL "" FORCE)
        set(ASSIMP_BUILD_ASSIMP_TOOLS   OFF CACHE BOOL "" FORCE)
        set(ASSIMP_INSTALL              OFF CACHE BOOL "" FORCE)
        set(BUILD_SHARED_LIBS           OFF CACHE BOOL "" FORCE)
        add_subdirectory("${TP}/assimp")
        # Assimp define CMAKE_DEBUG_POSTFIX "d" globalmente — impede que nossos targets ganhem o sufixo
        set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "" FORCE)
        set(FE_ASSIMP_TARGET assimp)
        message(STATUS " Assimp   : compilando do source em ${TP}/assimp")
    else()
        find_package(assimp CONFIG QUIET)
        if (assimp_FOUND)
            set(FE_ASSIMP_TARGET assimp::assimp)
            message(STATUS " Assimp   : encontrado via find_package (vcpkg/sistema)")
        else()
            set(FE_USE_ASSIMP OFF)
            message(WARNING " Assimp   : nao encontrado — coloque o source em ${TP}/assimp/ ou instale via vcpkg.")
        endif()
    endif()
endif()

# ─────────────────────────────────────────────────────────────────────────────
#  Engine (FractalEngineLib)
# ─────────────────────────────────────────────────────────────────────────────
add_library(FractalEngineLib STATIC)

file(GLOB_RECURSE FRACTAL_ENGINE_DIRS CONFIGURE_DEPENDS LIST_DIRECTORIES true "${CMAKE_SOURCE_DIR}/include/fractal_engine/*")
set(ENGINE_INCLUDE_PATHS "${CMAKE_SOURCE_DIR}/include")
foreach(dir ${FRACTAL_ENGINE_DIRS})
    if(IS_DIRECTORY ${dir})
        list(APPEND ENGINE_INCLUDE_PATHS ${dir})
    endif()
endforeach()

target_include_directories(FractalEngineLib PUBLIC
    ${ENGINE_INCLUDE_PATHS}
    "${TP}"
    "${TP}/imguizmo"
    # stb
    "${TP}/stb"
    "${TP}/stb_image"
    # entt
    "${TP}/entt/single_include"
    "${TP}/entt/src"
    "${TP}/entt"
    # nlohmann
    "${TP}/nlohmann"
    "${TP}/nlohmann/include"
)

add_subdirectory(src)

target_link_libraries(FractalEngineLib PUBLIC
    glad glfw glm imgui nlohmann_json stb entt
    opengl32 gdi32 user32 kernel32 winmm dwmapi
)

if (FE_HAS_IMGUIZMO)
    target_link_libraries(FractalEngineLib PUBLIC imguizmo)
    target_compile_definitions(FractalEngineLib PUBLIC FE_HAS_IMGUIZMO)
endif()

if (FE_ENABLE_LUA)
    target_link_libraries(FractalEngineLib PUBLIC lua)
    target_compile_definitions(FractalEngineLib PUBLIC FE_ENABLE_LUA)
endif()

if (FE_USE_ASSIMP)
    target_link_libraries(FractalEngineLib PUBLIC ${FE_ASSIMP_TARGET})
    target_compile_definitions(FractalEngineLib PUBLIC FE_USE_ASSIMP)
endif()

# Caminho absoluto para assets em tempo de compilação (resolve o problema de CWD ≠ source root)
# Usa configure_file para evitar problemas de escaping com espaços no caminho
set(FE_ASSETS_ROOT_PATH "${CMAKE_SOURCE_DIR}/assets")
configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/AssetsRoot.hpp.in"
    "${CMAKE_BINARY_DIR}/generated/AssetsRoot.hpp"
    @ONLY
)
target_include_directories(FractalEngineLib PUBLIC "${CMAKE_BINARY_DIR}/generated")

# ─────────────────────────────────────────────────────────────────────────────
#  Executável Principal
# ─────────────────────────────────────────────────────────────────────────────
add_executable(FractalEngine src/main.cpp)
target_link_libraries(FractalEngine PRIVATE FractalEngineLib)

if (CMAKE_BUILD_TYPE STREQUAL "Release" AND WIN32)
    target_link_options(FractalEngine PRIVATE -mwindows)
endif()

# MinGW + Assimp (static): evita conflito de símbolos typeinfo/RTTI entre libstdc++ e assimp
if (MINGW)
    target_link_options(FractalEngine PRIVATE -Wl,--allow-multiple-definition)
endif()

if (FE_BUILD_TOOLS)
    # add_subdirectory(tools)
endif()

# ─────────────────────────────────────────────────────────────────────────────
#  Flags e Sumário
# ─────────────────────────────────────────────────────────────────────────────
target_compile_options(FractalEngineLib PRIVATE -Wall -Wextra)

message(STATUS "========================================")
message(STATUS " FractalEngine: Pronta para build")
message(STATUS " Lua      : ${FE_ENABLE_LUA}")
message(STATUS " Assimp   : ${FE_USE_ASSIMP}")
message(STATUS " ImGuizmo : ${FE_HAS_IMGUIZMO}")
message(STATUS "========================================")