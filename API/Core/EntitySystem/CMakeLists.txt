
project(EntitySystem)
set(EntitySystem
CameraComponent.swift
Component.swift
ComponentDispatcher.swift
DispatchHandler.swift
Entity.swift
EntityPool.swift
MeshComponent.swift
MoveComponent.swift
RotateComponent.swift
TransformComponent.swift
)


find_package(Math CONFIG QUIET)
find_package(System CONFIG QUIET)

add_library(EntitySystem SHARED ${EntitySystem})
target_link_libraries(EntitySystem PUBLIC Math.dll)
target_link_libraries(EntitySystem PUBLIC System.dll)
set_target_properties(EntitySystem PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Swift_MODULE_DIRECTORY}
  INTERFACE_LINK_DIRECTORIES $<TARGET_LINKER_FILE_DIR:EntitySystem>)

