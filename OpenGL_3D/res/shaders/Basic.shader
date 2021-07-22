#shader vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in vec3 a_Normal;

uniform mat4 u_ViewProj;

out vec4 v_Color;
out vec2 v_TexCoord;
out float v_TexIndex;
out vec3 v_FragPos;
out vec3 v_Normal;

void main()
{
	v_Color = a_Color;
	v_TexCoord = a_TexCoord;
	v_TexIndex = a_TexIndex;
	gl_Position = u_ViewProj * vec4(a_Position, 1.0);
	v_FragPos = a_Position;
	v_Normal = a_Normal;
};

#shader fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec4 v_Color;
in vec2 v_TexCoord;
in float v_TexIndex;
in vec3 v_FragPos;
in vec3 v_Normal;

uniform sampler2D u_Textures[32];
uniform vec3 u_ViewPos;

void main()
{
	vec3 lightColor = vec3(1.0, 1.0, 1.0);
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;

	vec3 norm = normalize(v_Normal);
	vec3 lightDir = normalize(vec3(20, 70, 100) - v_FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	float specularStrength = 0.5;
	vec3 viewDir = normalize(u_ViewPos - v_FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * lightColor;

	int index = int(v_TexIndex);
	vec4 result = texture(u_Textures[index], v_TexCoord) * v_Color;
	o_Color = vec4(ambient + diffuse + specular, 1.0) * result;
};