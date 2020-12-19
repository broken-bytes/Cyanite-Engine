import System

class DispatchHandler {

    public let `default`: DispatchHandler = DispatchHandler()

    private var dispatchers: [IComponentDispatcher] = []
    private var cores: UInt

    private init() {
        cores = SystemInfo.cpuCount()
        for x in 0...cores-1 {
           // TODO: Split workers across threads
        }
    }

    // TODO: Load and instantiate all dispatcher classes on meta basis
    private func loadDispatchers() {

    }
    
    private func run(id: UInt) {

    }
}