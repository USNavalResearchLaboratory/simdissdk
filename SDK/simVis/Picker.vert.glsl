#version $GLSL_VERSION_STR

#pragma vp_location vertex_clip
#pragma vp_entryPoint sdkPickCheckHighlight

// Object ID provided via uniform that should be highlighted
uniform uint sdk_pick_highlight_objectid;
// Uniform for enabling and disabling highlighting
uniform bool sdk_pick_highlight_enabled;

// osgEarth-provided Object ID of the current vertex
uint oe_index_objectid;

// Output to fragment shader to mark an object selected
flat out int sdk_pick_isselected;

// Assigns sdk_pick_isselected based on input values
void sdkPickCheckHighlight(inout vec4 vertex)
{
  sdk_pick_isselected = (sdk_pick_highlight_enabled &&
    sdk_pick_highlight_objectid > 1u &&
    sdk_pick_highlight_objectid == oe_index_objectid)
    ? 1 : 0;
}
