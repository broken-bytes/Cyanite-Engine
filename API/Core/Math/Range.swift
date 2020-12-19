public struct Range<Element> {
    public var min: Element
    public var max: Element

    public init(from min: Element, to max: Element) {
        self.min = min
        self.max = max
    }
}