# src/physics/CMakeLists.txt

target_sources(FractalEngineLib PRIVATE
    # collision
    collision/AABB.cpp
    collision/BVH.cpp
    collision/CollisionDetection.cpp

    # dynamics
    dynamics/Collider.cpp
    dynamics/Constraint.cpp
    dynamics/Rigidbody.cpp

    # engine
    PhysicsEngine.cpp
)