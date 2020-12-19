// swift-tools-version:5.3

import PackageDescription

let Math = Package(
  name: "Math",
  products: [
    .library(name: "Math", type: .dynamic, targets: ["Math"]),
  ],
  targets: [
    .target(name: "Math", dependencies: []),
  ]
)
