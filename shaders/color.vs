#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;

// NOTE: Add here your custom variables
const int width = 31;
const int height = 21;
const int cellSize = 20;
const ivec2 borderSize = ivec2(10, 12);
uniform vec4 modulate[(width+1)*height];
uniform vec4 shifting;

ivec2 getPos(){
    ivec2 pos;
    pos.x = (int(vertexPosition.x) - borderSize.x) / cellSize;
    pos.y = (int(vertexPosition.y) - borderSize.y) / cellSize;
    return pos;
}

void main(){
    ivec2 pos = getPos();
    vec4 modColor = modulate[pos.y * (width+1) + pos.x];

    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    // fragColor = vertexColor;
    fragColor = modColor.r >= 0 ? modColor : shifting/255.0;
    // fragColor = vec4(float(pos.x) / float(width), float(pos.y) / float(height), 0.0, 1.0);
    // fragColor = vec4(vertexTexCoord.x / 640.0, vertexTexCoord.y / 480.0, 1.0, 1.0);
    // fragColor = vec4(0.5, 1.0, 1.0, 1.0);

    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}