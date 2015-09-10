//============================================================
// STUDENT NAME: <your name>
// MATRIC NO.  : <matric no.>
// NUS EMAIL   : <your NUS email address>
// COMMENTS TO GRADER:
// <comments to grader, if any>
//
//============================================================
//
// FILE: assign1.frag


//============================================================================
// Eye-space position and vectors for setting up a tangent space at the fragment.
//============================================================================

varying vec3 ecPosition;    // Fragment's 3D position in eye space.
varying vec3 ecNormal;      // Fragment's normal vector in eye space.
varying vec3 ecTangent;     // Frgament's tangent vector in eye space.


//============================================================================
// TileDensity specifies the number of tiles to span across each dimension when the
// texture coordinates gl_TexCoord[0].s and gl_TexCoord[0].t range from 0.0 to 1.0.
//============================================================================

uniform float TileDensity;  // (0.0, inf)


//============================================================================
// TubeRadius is the radius of the semi-circular mirror tubes that run along 
// the boundary of each tile. The radius is relative to the tile size, which 
// is considered to be 1.0 x 1.0.
//============================================================================

uniform float TubeRadius;  // (0.0, 0.5]


//============================================================================
// StickerWidth is the width of the square sticker. The entire square sticker 
// must appear at the center of each tile. The width is relative to the 
// tile size, which is considered to be 1.0 x 1.0.
//============================================================================

uniform float StickerWidth;  // (0.0, 1.0]


//============================================================================
// EnvMap references the environment cubemap for reflection mapping.
//============================================================================

uniform samplerCube EnvMap;


//============================================================================
// DiffuseTex1 references the wood texture map whose color is used to 
// modulate the ambient and diffuse lighting components on the non-mirror and
// non-sticker regions.
//============================================================================

uniform sampler2D DiffuseTex1;


//============================================================================
// DiffuseTex2 references the sticker texture map whose color is used to 
// modulate the ambient and diffuse lighting components on the sticker regions.
//============================================================================

uniform sampler2D DiffuseTex2;




void main()
{
    vec2 c = TileDensity * gl_TexCoord[0].st;
    vec2 p = fract( c ) - vec2( 0.5 );

    // Some useful eye-space vectors.
    vec3 ecNNormal = normalize( ecNormal );
    vec3 ecViewVec = -normalize( ecPosition );


    //////////////////////////////////////////////////////////
    // REPLACE THE CONDITION IN THE FOLLOWING IF STATEMENT. //
    //////////////////////////////////////////////////////////

    if ( true )
    {
        //======================================================================
        // In here, fragment is backfacing or in the non-bump region.
        //======================================================================

        // For the lighting computation, use the half-vector approach 
        // to compute the specular component.


        ///////////////////////////
        // WRITE YOUR CODE HERE. //
        ///////////////////////////

    }
    else
    {
        //======================================================================
        // In here, fragment is front-facing and in the mirror-like bump region.
        //======================================================================

        vec3 N = ecNNormal;
        vec3 B = normalize( cross( N, ecTangent ) );
        vec3 T = cross( B, N );

        vec3 tanPerturbedNormal;  // The perturbed normal vector in tangent space of fragment.
        vec3 ecPerturbedNormal;   // The perturbed normal vector in eye space.
        vec3 ecReflectVec;        // The mirror reflection vector in eye space.


        ///////////////////////////
        // WRITE YOUR CODE HERE. //
        ///////////////////////////

    }

}
