// swift-tools-version:5.3

import PackageDescription

let System = Package(
  name: "System",
  products: [
    .library(name: "System", type: .dynamic, targets: ["System"]),
  ],
  targets: [
    .target(name: "System", dependencies: []),
  ]
)
