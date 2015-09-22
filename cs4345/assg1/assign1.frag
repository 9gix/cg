//============================================================
// STUDENT NAME: Eugene
// MATRIC NO.  : A0116631N
// NUS EMAIL   : eugene@comp.nus.edu.sg
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
    vec2 c = TileDensity * gl_TexCoord[0].st; // (0, TileDensity/inf]
    vec2 p = fract( c ) - vec2( 0.5 ); // [-0.5, 0.5]

    // Some useful eye-space vectors.
    vec3 ecNNormal = normalize( ecNormal );
    vec3 ecViewVec = -normalize( ecPosition );


    if (!gl_FrontFacing )
    {
        //======================================================================
        // In here, fragment is backfacing or in the non-bump region.
        //======================================================================
		ecNNormal = -ecNNormal;
		
	
		// For the lighting computation, use the half-vector approach 
		// to compute the specular component.		
		vec3 woods = texture2D(DiffuseTex1, c).rgb;
		vec3 lightVector = normalize(gl_LightSource[0].position.xzy - ecPosition);		
		vec3 halfwayVector = normalize(lightVector + ecViewVec);
		

		vec3 ambient = woods * vec3(gl_FrontLightProduct[0].ambient);
		vec3 diffuse = woods * vec3(gl_FrontLightProduct[0].diffuse) * clamp(dot(ecNNormal, lightVector), 0.0, 1.0);
		vec3 specular = vec3(gl_FrontLightProduct[0].specular) * vec3(pow(clamp(dot(ecNNormal, halfwayVector), 0.0, 1.0), 4 * gl_FrontMaterial.shininess));
		specular = clamp(specular, 0.0, 1.0);

		vec3 phongLighting = vec3(gl_BackLightModelProduct.sceneColor) * woods + ambient + diffuse + specular;
		gl_FragColor = vec4(phongLighting,1);
    }
    else
    {
        //======================================================================
        // In here, fragment is front-facing and in the mirror-like bump region.
        //======================================================================
		vec3 ambient, diffuse, specular;
	
		// For the lighting computation, use the half-vector approach 
		// to compute the specular component.		
		vec3 woods = texture2D(DiffuseTex1, c).rgb;
		vec3 lightVector = normalize(gl_LightSource[0].position.xzy - ecPosition);		
		vec3 halfwayVector = normalize(lightVector + ecViewVec);
		
        vec3 N = ecNNormal;
        vec3 B = normalize( cross( N, ecTangent ) );
		vec3 T = ecTangent;

        vec3 tanPerturbedNormal;  // The perturbed normal vector in tangent space of fragment.
        vec3 ecPerturbedNormal;   // The perturbed normal vector in eye space.
        vec3 ecReflectVec;        // The mirror reflection vector in eye space.

		float tubeStartPos = 0.5 - TubeRadius;
		vec4 fragColor;
		
		if (all(lessThan(abs(p), vec2(tubeStartPos)))) {
			vec2 stickerCentered = p / StickerWidth; 
			vec3 tex;
			if (all(lessThan(abs(stickerCentered), vec2(0.5)))) {
				// Sticker
				tex = texture2D(DiffuseTex2, stickerCentered + 0.5).rgb;
			}  else {
			    // Woods
				tex = woods;
			}

			ambient = tex * vec3(gl_FrontLightProduct[0].ambient);
			diffuse = tex * vec3(gl_FrontLightProduct[0].diffuse) * clamp(dot(ecNNormal, lightVector), 0.0, 1.0);
			specular = vec3(gl_FrontLightProduct[0].specular) * vec3(pow(clamp(dot(ecNNormal, halfwayVector), 0.0, 1.0), 4 * gl_FrontMaterial.shininess));
			fragColor = vec4(vec3(gl_BackLightModelProduct.sceneColor) * tex + ambient + diffuse + specular, 1);
			
		} else if (abs(p.x) > tubeStartPos || abs(p.y) > tubeStartPos){
		    // Mirror

			mat3 M = mat3(
				T.x, B.x, N.x,
				T.y, B.y, N.y,
				T.z, B.z, N.z
			); // Matrix Transformation from Eye Space to Tangent Space

			vec3 tanPosition = normalize(M * ecPosition);

			
			float radius_square = TubeRadius * TubeRadius;
			if (abs(p.x) > abs(p.y))  { 
				float x = (p.x > 0) ? p.x - 0.5 : p.x + 0.5;
				float width_square = (0.5 - abs(p.x))*(0.5 - abs(p.x));
				ecPerturbedNormal = vec3(x, 0, sqrt(radius_square - width_square));
			} else if (abs(p.y) >= abs(p.x)){
				float y = (p.y > 0) ? p.y - 0.5 : p.y + 0.5;
				float width_square = (0.5 - abs(p.y))*(0.5 - abs(p.y));
				ecPerturbedNormal = vec3(0, y, sqrt(radius_square - width_square));
			}
			ecPerturbedNormal = normalize(ecPerturbedNormal);

			ecReflectVec = reflect(tanPosition, ecPerturbedNormal);
			fragColor = textureCube(EnvMap, ecReflectVec);
		}
		
		gl_FragColor = fragColor;
    }

}
