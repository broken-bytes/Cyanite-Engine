struct Quaternion {
    var x: Float
    var y: Float
    var z: Float
    var w: Float


    var euler: Vector3 { get { euler()}}

    convenience init() {
        init(0, 0, 0, 0)
    }

    init(
        x: Float,
        y: Float,
        z: Float,
        w: Float
    ) {
        self.x = x
        self.y = y
        self.z = z
        self.w = w
    }

    private func euler() -> Vector3 {
        return Vector3(0, 0 ,0)
    } 
}