protocol IComponentDispatcher {
    var entities: [Entity]  { get }

    func execute() -> Void
    func update() -> Void
}

class ComponentDispatcher<T: Component, U: Component, V: Component> : IComponentDispatcher {
    var entities: [Entity] { get { getEntities() }}

    func execute() -> Void {
    }
    func update() -> Void {}


    private func getEntities() -> [Entity] {
        return EntityPool.default.entities(T.self, U.self, V.self)
    }
}