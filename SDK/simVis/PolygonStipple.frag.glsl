#version 330

#pragma vp_entryPoint simvis_polygon_stipple
#pragma vp_location fragment_coloring

#pragma import_defines(SIMVIS_USE_POLYGON_STIPPLE)

// Expected value 0-8 inclusive
uniform uint simvis_stippleindex;

void simvis_polygon_stipple(inout vec4 color)
{
#ifdef SIMVIS_USE_POLYGON_STIPPLE
  bool evenY = (mod(gl_FragCoord.y, 2.0) < 1.0);
  // Modulate the X by 4; corresponds to a nibble in the stipple patterns
  uint modX = uint(floor(mod(gl_FragCoord.x, 4.0)));
  if (evenY)
  {
    if ((simvis_stippleindex / 3u) == 0u) {
      if (modX != 1u) discard;  // 0x4444 pattern
    }
    else if ((simvis_stippleindex / 3u) == 1u) {
      if (modX == 1u || modX == 3u) discard; // 0xAAAA pattern
    }
    else { // divide-by-3 is 2
      if (modX == 2u) discard; // 0xDDDD pattern
    }
  }
  else // odd row
  {
    if ((simvis_stippleindex % 3u) == 0u) {
      if (modX == 1u || modX == 2u) discard;  // 0x9999 pattern
    }
    else if ((simvis_stippleindex % 3u) == 1u) {
      if (modX == 0u || modX == 3u) discard; // 0x6666 pattern
    }
    else { // modulus-by-3 is 2
      if (modX == 0u || modX == 1u) discard; // 0x3333 pattern
    }
  }
#endif
}
