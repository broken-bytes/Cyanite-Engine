class DispatchHandler() {

    public let `default`: DispatchHandler = DispatchHandler()

    private var dispatchers: [ComponentDispatcher] = []
    private let cores: Uint

    private init() {
        cores = SystemInfo.cpuCount
        for x in 0...cores-1 {
            backgroundThread(background: {
                for y in 0...dispatchers.count {
                    if y % x == 0 {
                        dispatchers[y].execute()
                    }
                }
                },
                completion: {
                }
            )
        }
    }

    // TODO: Load and instantiate all dispatcher classes on meta basis
    private func loadDispatchers() {

    }
    
    private func run(id: UInt) {

    }
}