#version 450

layout(set=0, binding = 0) uniform globalUniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 lightColor;
    vec3 lightPos;
    vec3 ambColor;
    vec4 coneInOutDecayExp;
    vec3 spotPosition1;
    vec3 spotPosition2;
    vec3 spotDirection;
    vec3 eyePos;
    vec2 selector;
} gubo;

layout(set=1, binding = 0) uniform UniformBufferObject {
    mat4 model;
} ubo;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec2 fragTexCoord;

void main() {
    gl_Position = gubo.proj * gubo.view * ubo.model * vec4(pos, 1.0);
    fragPos = (ubo.model * vec4(pos, 1.0)).xyz;
    fragNorm = (ubo.model * vec4(norm, 0.0)).xyz;
    fragTexCoord = texCoord;
}
