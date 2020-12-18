import Swift

func main() {
  print("Test")
}

@_cdecl("start")
public func start() {

}

@_cdecl("start")
public func update() {

}

@_cdecl("testFunc")
public func testFunc(value: Int) -> Int {
  return value / 2
}
