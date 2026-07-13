cmake_minimum_required(VERSION 3.10)
project(SceneGameApp)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

set(GLAD_SOURCE ${CMAKE_CURRENT_LIST_DIR}/../external/glad/glad.c)
set(GLAD_INCLUDE ${CMAKE_CURRENT_LIST_DIR}/../external/glad/include)
set(SCENE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(IMGUI_DIR ${CMAKE_CURRENT_LIST_DIR}/../external/imgui)
set(GLM_DIR ${CMAKE_CURRENT_LIST_DIR}/../external/glm)

set(SCENE_SOURCES
	src/main.cpp
	src/Engine.cpp
	src/ShaderProgram.cpp
	src/SceneNode.cpp
	src/Geometry.cpp
	src/Camera.cpp
	src/MeshNode.cpp
	src/assets.cpp
	src/ui_system.cpp
	src/input_handler.cpp
	src/game_logic.cpp
	src/physics_system.cpp
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_draw.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
	${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
	${GLAD_SOURCE}
)

add_executable(scene_game
	${SCENE_SOURCES}
)
target_include_directories(scene_game PRIVATE ${GLAD_INCLUDE})
target_include_directories(scene_game PRIVATE ${SCENE_INCLUDE})
target_include_directories(scene_game PRIVATE ${IMGUI_DIR})
target_include_directories(scene_game PRIVATE ${IMGUI_DIR}/backends)
target_include_directories(scene_game PRIVATE ${GLM_DIR})
target_include_directories(scene_game PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../external)
target_compile_definitions(scene_game PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)
target_link_libraries(scene_game PRIVATE glfw OpenGL::GL)
