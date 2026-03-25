# tests/CMakeLists.txt

# ── GoogleTest (ou catch2) ────────────────────────────────────────────────────
# Se quiser usar GoogleTest coloque em third_party/googletest
# add_subdirectory("${CMAKE_SOURCE_DIR}/third_party/googletest")

# ── Helper para criar testes unitários facilmente ────────────────────────────
function(fe_add_test TEST_NAME SOURCE)
    add_executable(${TEST_NAME} ${SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE FractalEngineLib)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# ── Unit tests ────────────────────────────────────────────────────────────────
fe_add_test(TestEngine          unit/core/TestEngine.cpp)
fe_add_test(TestEventSystem     unit/core/TestEventSystem.cpp)
fe_add_test(TestWindow          unit/core/TestWindow.cpp)

fe_add_test(TestMathUtils       unit/math/TestMathUtils.cpp)
fe_add_test(TestNoise           unit/math/TestNoise.cpp)
fe_add_test(TestTransform       unit/math/TestTransform.cpp)

fe_add_test(TestCollision       unit/physics/TestCollision.cpp)
fe_add_test(TestRigidbody       unit/physics/TestRigidbody.cpp)

# ── Integration tests ─────────────────────────────────────────────────────────
fe_add_test(TestSceneLoading    integration/TestSceneLoading.cpp)

# ── Benchmarks (sem CTest, só executáveis) ────────────────────────────────────
add_executable(BenchmarkRenderer benchmarks/BenchmarkRenderer.cpp)
target_link_libraries(BenchmarkRenderer PRIVATE FractalEngineLib)