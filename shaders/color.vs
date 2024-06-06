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
const int width = 35;
const int height = 25;
const int cellSize = 20;
uniform ivec2 offset;
uniform vec4 modulate[(width+1)*height];
uniform vec4 shifting;

ivec2 getPos(){
    ivec2 pos;
    pos.x = (int(vertexPosition.x) - offset.x) / cellSize;
    pos.y = (int(vertexPosition.y) - offset.y) / cellSize;
    return pos;
}

void main(){
    ivec2 pos = getPos();
    vec4 modColor = modulate[pos.y * (width+1) + pos.x];

    // Send vertex attributes to fragment shader
    fragTexCoord = vertexTexCoord;
    fragColor = modColor.r >= 0 ? modColor : shifting/255.0;

    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}