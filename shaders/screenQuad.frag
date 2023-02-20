#version 410 core

in vec2 fTexCoords;

out vec4 fColor;

uniform sampler2D depthMap;

void main() 
{    
    fColor = vec4(vec3(texture(depthMap, fTexCoords).r), 1.0f);
    //fColor = vec4(fTexCoords, 0.0f, 1.0f);
}
