#version 330

#pragma vp_location vertex_view
#pragma vp_entryPoint simVis_flatten_VS
#pragma import_defines(SIMVIS_WGS_A, SIMVIS_WGS_B)

uniform mat4 osg_ViewMatrixInverse;
uniform mat4 osg_ViewMatrix;
uniform bool simVis_useFlattenShader;

void simVis_flatten_VS(inout vec4 viewVertex)
{
    if (simVis_useFlattenShader)
    {
        // Frame to convert from ECEF to unit sphere
        const vec3 EF = vec3(SIMVIS_WGS_A, SIMVIS_WGS_A, SIMVIS_WGS_B);

        // Latitudinal adjustment heuristic calculated via trial an error.
        // At +/- 45deg latitude, adjust the vertex C meters toward the
        // equator. The effect lessens as the latitude diverges from 45d
        // and is Zero at lat=0 or lat=+/-90. We use this trick because the
        // real equation to clamp an ECEF point to the geodetic surface is
        // not possible in single-precision.
        const float C = 0.003358829;
        
        // Resolve world space vertex (with some precision loss, cannot be helped)
        vec4 worldVertex = osg_ViewMatrixInverse * viewVertex;

        // Radius + altitude of point
        float worldLen = length(worldVertex);

        // Convert into unit-sphere coordinate frame:
        worldVertex.xyz /= EF;

        // Set the vector length to 1.0 to clamp to unit sphere:
        worldVertex.xyz = normalize(worldVertex.xyz);

        // construct the strength [0..1] at which to apply a latitude
        // adjustment. No adjustment at the poles or equator; maximum
        // adjustment at +/- 45 degrees latitude.
        const float D45 = 0.785398; // 45 degrees (in radians)
        float angle = abs(asin(worldVertex.z));
        float adjustment = 1.0-(abs(angle-D45)/D45);

        // Back to ECEF coordinate frame
        worldVertex.xyz *= EF;

        // Calculate the original altitude:
        float alt = worldLen - length(worldVertex);

        // back to view space
        viewVertex = osg_ViewMatrix * worldVertex;
        
        // Adjust the final coordinate based on latitude and altitude.
        // Only need Y dimension relative to the eye since we are rendering
        // in north-up orthographic.
        viewVertex.y -= sign(worldVertex.z) * C * alt * adjustment;
    }
}
