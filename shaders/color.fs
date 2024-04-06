#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Output fragment color
out vec4 finalColor;

// NOTE: Add here your custom variables


void main(){
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    // NOTE: Implement here your fragment shader code
    finalColor = texelColor*colDiffuse*fragColor;
    // finalColor = fragColor;
    // finalColor = modulate[int((fragTexCoord.y*weight*weight+fragTexCoord.x*height)/textureSize(texture0,0).x)];
    // finalColor = vec4(fragTexCoord.x,fragTexCoord.y,0.0,1.0);
    // finalColor = vec4(1.0,1.0,1.0,1.0);
}