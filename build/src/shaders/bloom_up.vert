#version 460 core

layout (location = 0) in vec2 aPosition;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0);
    texCoord = aPosition*0.5+0.5;
}
