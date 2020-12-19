public struct Quaternion {
    public var x: Float
    public var y: Float
    public var z: Float
    public var w: Float


    public init(
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

    public var euler: Vector3 { get { toEuler()}}

    private func toEuler() -> Vector3 {
        return Vector3(x: 0, y: 0, z: 0)
    } 
}
