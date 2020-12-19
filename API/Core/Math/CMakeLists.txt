project(Math
  LANGUAGES C CXX Swift)
set(Math
Quaternion.swift
Range.swift
Vector2.swift
Vector3.swift
)

add_library(Math SHARED ${Math})
set_target_properties(Math PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Swift_MODULE_DIRECTORY}
  INTERFACE_LINK_DIRECTORIES $<TARGET_LINKER_FILE_DIR:Math>)


