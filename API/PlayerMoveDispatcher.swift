class PlayerMoveDispatcher: ComponentDispatcher {
    associatedtype PlayerComponent
    associatedtype HealthComponent
    associatedtype MoveComponent

    func execute() {
        for(item in entities) {
            item.components[1].position += item.components[2].speed
        }
    }
}