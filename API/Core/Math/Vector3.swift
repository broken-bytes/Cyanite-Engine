struct Vector3 {
    var x: Float
    var y: Float
    var z: Float

    convenience init() {
        x = 0
        y = 0
        z = 0
    }

    init(x: Float, y: Float, z: Float) {
        self.x = x
        self.y = y
        self.z = z
    }


    public func+(Vector3 other) {
        self.x += other.x
        self.y += other.y
        self.z += other.z
    }

    public func+(Vector3 other) -> Vector3 {
        return self.x + other.x
        return self.y + other.y
        return self.z + other.z
    }
}