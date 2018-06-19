#version $GLSL_VERSION_STR
$GLSL_DEFAULT_PRECISION_FLOAT

#pragma import_defines(SV_USE_DISABLE_DEPTH_ON_ALPHA)

#pragma vp_entryPoint simvis_disable_depth_alpha
#pragma vp_location   fragment_lighting
// Run immediately after LDB
#pragma vp_order      0.995

#ifdef SV_USE_DISABLE_DEPTH_ON_ALPHA
uniform float oe_logDepth_FC = 0.0;

// Disables depth writes when the color's alpha is less than a threshold
void simvis_disable_depth_alpha(inout vec4 color)
{
  // Presumes the use of LDB.  If LDB is not used, then gl_FragDepth must also be
  // written in the else condition.
  if (color.a < 0.05)
  {
    // Fragment depth is all the way in the back; does not matter if LDB is on or not.
    gl_FragDepth = 1.0;
  }
  else if (oe_logDepth_FC == 0.0)
  {
    // LDB is disabled.  We can't be sure gl_FragDepth was written, so we must write here.
    gl_FragDepth = gl_FragCoord.z;
  }
}
#else

void simvis_disable_depth_alpha(inout vec4 color)
{
  // noop
}
#endif
