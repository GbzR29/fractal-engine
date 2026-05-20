# src/scripting/CMakeLists.txt

if (FE_ENABLE_LUA)
    target_sources(FractalEngineLib PRIVATE ScriptEngine.cpp)
endif()
