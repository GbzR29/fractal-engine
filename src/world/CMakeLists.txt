# src/world/CMakeLists.txt

target_sources(FractalEngineLib PRIVATE
    terrain/Chunk.cpp
    terrain/ChunkMesh.cpp
    streaming/ChunkStreamer.cpp
    streaming/AsyncLoader.cpp
    terrain/HeightMap.cpp
    terrain/TerrainGenerator.cpp
    World.cpp
    WorldManager.cpp
)