// swift-tools-version:5.3

import PackageDescription

let CyaniteScripting = Package(
  name: "CyaniteScripting",
  products: [
    .library(name: "CyaniteScripting", type: .dynamic, targets: ["Math", "System", "EntitySystem"]),
  ],
  dependencies: [
  ],
  targets: [
    .target(
      name: "Math",
      dependencies: [
      ],
      path: "Core/Math",
      exclude: [
        "CMakeLists.txt",
      ],
      swiftSettings: [
      ],
      linkerSettings: [
      ]
    ),
    .target(
      name: "System",
      dependencies: [
        "Math",
      ],
      path: "Core/System",
      exclude: [
        "CMakeLists.txt",
      ]
    ),
    .target(
      name: "EntitySystem",
      dependencies: [
        "Math",
        "System"
      ],
      path: "Core/EntitySystem",
      exclude: [
        "CMakeLists.txt",
      ],
      swiftSettings: [
      ]
    ),
  ]
)
