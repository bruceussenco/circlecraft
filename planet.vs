#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;
uniform float worldWidthPx;
uniform float worldHeight;
uniform float angleBetweenTiles;
uniform float textureSize;

out vec2 fragTexCoord;
out vec4 fragColor;

const float PI = 3.14159265359;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    float angle = (vertexPosition.x / worldWidthPx) * 2.0 * PI;
    float radius = (textureSize/angleBetweenTiles)*pow(1.0+angleBetweenTiles, worldHeight-vertexPosition.y/textureSize);

    vec4 bentPosition = vec4(
        radius * sin(angle),
        -radius * cos(angle),
        vertexPosition.z,
        1.0
    );

    gl_Position = mvp * bentPosition;
}

