class PlayerMoveDispatcher: ComponentDispatcher<MoveComponent, MeshComponent, TransformComponent> {
    override func execute() {
        for item in entities {
            item.components[2].position += item.components[2].speed
        }
    }
}