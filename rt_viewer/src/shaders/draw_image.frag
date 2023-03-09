// Fragment shader
#version 330
#extension GL_ARB_explicit_attrib_location : require

uniform sampler2D u_texture;
uniform bool u_gamma_correction = true;

in vec2 v_texcoord;
out vec4 frag_color;

void main()
{
    if (u_gamma_correction)
    {
        frag_color = texture(u_texture, v_texcoord);
        frag_color.rgb /= frag_color.a;
        frag_color.rgb = pow(frag_color.rgb, vec3(1.0 / 2.2));
    }
    else
    {
        frag_color = texture(u_texture, v_texcoord);
        frag_color.rgb /= frag_color.a;
    }
}
