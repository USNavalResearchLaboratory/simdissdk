#version $GLSL_VERSION_STR

#pragma vp_location fragment_coloring
#pragma vp_entryPoint sdkPickHighlightFragment
#pragma vp_order 3.0

// Input from the vertex shader
flat in int sdk_pick_isselected;

// OSG built-in for frame time
uniform float osg_FrameTime;

void sdkPickHighlightFragment(inout vec4 color)
{
#ifndef OE_IS_PICK_CAMERA
  if (sdk_pick_isselected == 1) {
    // Borrow code fromGlowHighlight.frag.glsl and modified to fit mouse operation a bit better
    float glowPct = sin(osg_FrameTime * 12.0);
    color.rgb += 0.2 + 0.2 * glowPct;
    // Make the glow slightly blue-ish
    color *= vec4(0.7, 0.7, 1.0, 1.0);
  }
#endif
}
