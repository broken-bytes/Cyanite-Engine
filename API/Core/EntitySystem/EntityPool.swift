class EntityPool {
    public static `default`: EntityPool = EntityPool()
    private var pool: [Entity] = []


    public func entities<T>(c0 : T.Type) -> [Entity] {
        var matching: [Entity] = []

        for item in pool {
            var match = false
            for comp in item.components {
                if type(of: comp) == c0 {
                      match = true
                      break
                }
            }
            if match {
                matching.append(item)
            }
        }
        return matching
    }

    public func entities<T, U>(c0 : T.Type, c1: U.Type) -> [Entity] {
        var matching: [Entity] = []

        for item in pool {
            var matches = 0
            for comp in item.components {
                if type(of: comp) == c0 ||
                    type(of: comp) == c1 {
                      matches+=1  
                }
            }
            if matches >= 2 {
                matching.append(item)
            }
        }
        return matching
    }

    public func entities<T, U, V>(c0 : T.Type, c1: U.Type, c2: V.Type) -> [Entity] {
        var matching: [Entity] = []

        for item in pool {
            var matches = 0
            for comp in item.components {
                if type(of: comp) == c0 ||
                    type(of: comp) == c1 ||
                    type(of: comp) == c2 {
                      matches+=1  
                }
            }
            if matches >= 3 {
                matching.append(item)
            }
        }
        return matching
    }
}