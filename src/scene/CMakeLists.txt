# src/scene/CMakeLists.txt

target_sources(FractalEngineLib PRIVATE
    Scene.cpp
    SceneManager.cpp
    SceneSerializer.cpp
    SceneFrameBuffer.cpp

    # ECS
    ecs/ComponentPool.cpp
    ecs/Entity.cpp
    ecs/Registry.cpp
    ecs/System.cpp
    
)