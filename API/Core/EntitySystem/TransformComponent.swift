import Math

struct TransformComponent: Component {
   var position: Vector3
   var rotation: Quaternion
   var euler: Vector3 { get { return rotation.euler}}
}