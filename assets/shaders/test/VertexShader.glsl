#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec4 in_color;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec2 in_normal;

out vec4 v_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    v_color = in_color;
    gl_Position = projection * view * model * vec4(in_position, 1.0f);
}
