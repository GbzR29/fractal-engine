# src/scripting/CMakeLists.txt

if (FE_ENABLE_LUA)
    target_sources(FractalEngineLib PRIVATE
        LuaBinding.cpp
        ScriptContext.cpp
        ScriptEngine.cpp
    )
endif()