class EntityPool {
    public static var `default`: EntityPool = EntityPool()
    private var pool: [Entity] = []

    public func entities(_ c: Component.Type...) -> [Entity] {
        var matching: [Entity] = []

        for item in pool {
            var matches = 0
            for comp in item.components {
              for req in c {
                  if type(of: comp) == req {
                      matches+=1
                  }
              }
            }
            if matches == c.count {
                matching.append(item)
            }
        }
        return matching
    }
}