#pragma vp_name PointDrawable
#pragma vp_entryPoint oe_PointDrawable_VS_VIEW
#pragma vp_location vertex_view
#pragma vp_order last

uniform float oe_GL_PointSize = 1.0;

void oe_PointDrawable_VS_VIEW(inout vec4 vertexView)
{
    gl_PointSize = oe_GL_PointSize;
}

[break]
#pragma vp_name PointDrawable FS
#pragma vp_entryPoint oe_PointDrawable_FS
#pragma vp_location fragment_coloring
#pragma import_defines(OE_POINT_SMOOTH)

void oe_PointDrawable_FS(inout vec4 color)
{
#ifdef OE_POINT_SMOOTH
    vec2 c = 2.0*gl_PointCoord-1.0;
    float r = dot(c, c);
    if (r > 1000)
    {
        // SIM-18501, SIM-16401 do not smooth: mesa 23 issue with gl_PointCoord; testing suggests that r values > 4.5 only occur with mesa driver issues
        if (color.a < 0.1)
            discard;
        return;
    }
    float d = 0.0;
  #ifdef GL_OES_standard_derivatives
    d = fwidth(r);
  #endif
    color.a *= 1.0 - smoothstep(1.0-d, 1.0+d, r);
    if (color.a < 0.1)
        discard;
#endif
}
