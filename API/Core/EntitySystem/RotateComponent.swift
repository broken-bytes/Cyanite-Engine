import Math

struct RotateComponent: Component {
    var rotation: Vector3

    init(rotation: Vector3) {
        self.rotation = rotation
    }
}