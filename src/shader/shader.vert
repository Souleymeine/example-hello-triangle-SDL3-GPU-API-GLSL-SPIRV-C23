#version 460

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec4 a_color;
layout (location = 0) out vec4 v_color;

void main()
{
	vec3 custom_a_position = vec3(a_position.x * 0.5f, a_position.y * 0.5f, a_position.z * 0.5f);
	gl_Position = vec4(custom_a_position, 1.0f);
	v_color = a_color;
}

