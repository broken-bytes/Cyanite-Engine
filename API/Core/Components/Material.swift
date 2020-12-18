class Material {
    var shader: Shader
    var textures: [Textures]

    init(shader: Shader, textures: [Texture]) {
        self.shader = shader
        self.textures = textures
    }
}