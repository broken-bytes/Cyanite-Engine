set(System
Input.swift
SystemInfo.swift
Time.swift
)


add_library(System SHARED ${System})
target_link_libraries(System PUBLIC Math)
set_target_properties(System PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_Swift_MODULE_DIRECTORY}
  INTERFACE_LINK_DIRECTORIES $<TARGET_LINKER_FILE_DIR:System>)

