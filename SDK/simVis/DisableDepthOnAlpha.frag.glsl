#version $GLSL_VERSION_STR
$GLSL_DEFAULT_PRECISION_FLOAT

#pragma import_defines(SV_USE_DISABLE_DEPTH_ON_ALPHA)
#pragma import_defines(SV_USE_LOG_DEPTH_BUFFER)

#pragma vp_entryPoint simvis_disable_depth_alpha
#pragma vp_location   fragment_lighting
// Run immediately after LDB
#pragma vp_order      0.995

// Disables depth writes when the color's alpha is less than a threshold
void simvis_disable_depth_alpha(inout vec4 color)
{
#ifndef SV_USE_DISABLE_DEPTH_ON_ALPHA
  // Presumes the use of LDB.  If LDB is not used, then gl_FragDepth must also be
  // written in the else condition.
  if (color.a < 0.05)
  {
    // Fragment depth is all the way in the back; does not matter if LDB is on or not.
    gl_FragDepth = 1.0;
  }

  #ifndef SV_USE_LOG_DEPTH_BUFFER
  else
  {
    // only set this if the LDB didn't already set it
    gl_FragDepth = gl_FragCoord.z;
  }  
  #endif

#endif
}
