#shader vertex
#version 430
in vec3 pos;
void main() {
    gl_Position = vec4(pos, 1);
}

#shader fragment
#version 430
out vec4 FragColor;
void main() {
    FragColor = vec4(1, 1, 1, 1);
}
