struct Quaternion {
    var x: Float
    var y: Float
    var z: Float
    var w: Float


    var euler: Vector3 { get { toEuler()}}

    private func toEuler() -> Vector3 {
        return Vector3(x: 0, y: 0, z: 0)
    } 
}
