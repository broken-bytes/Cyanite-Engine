// swift-tools-version:5.3

import PackageDescription

let EntitySystem = Package(
  name: "EntitySystem",
  products: [
    .library(name: "EntitySystem", type: .dynamic, targets: ["EntitySystem"]),
  ],
  targets: [
    .target(name: "EntitySystem", dependencies: []),
    .testTarget(name: "CassowaryTests", dependencies: ["Cassowary"]),
  ]
)
