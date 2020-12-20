public struct Rect {
    var left: UInt
    var right: UInt
    var top: UInt
    var bottom: UInt

    public init(left: UInt, right: UInt, top: UInt, bottom: UInt) {
        self.left = left
        self.right = right
        self.top = top
        self.bottom = bottom
    }
}