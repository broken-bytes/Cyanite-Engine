protocol IComponentDispatcher {
    var entities: [Entity]  { get }

    func execute() -> Void
    func update() -> Void
}

class ComponentDispatcher<T, U, V> : IComponentDispatcher {
    var entities: [Entity] { get { entities() }}


    private func entities() -> [Entity] {
        return EntityPool.entities(c0: T, c1: U, c2: V)
    }
}