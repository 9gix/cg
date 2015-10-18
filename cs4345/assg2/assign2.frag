//============================================================
// STUDENT NAME: Eugene
// MATRIC NO.  : A0116631N
// NUS EMAIL   : eugene@comp.nus.edu.sg
// COMMENTS TO GRADER:
// Does not run on my computer :( Unsupported framebuffer object ):
// Lucky that Lab PC is working on Weekend
// ============================================================
//
// FILE: assign2.frag


// The GL_EXT_gpu_shader4 extension extends GLSL 1.10 with 
// 32-bit integer (int) representation, integer bitwise operators, 
// and the modulus operator (%).

#extension GL_EXT_gpu_shader4 : require

#extension GL_ARB_texture_rectangle : require


uniform sampler2DRect InputTex;  // The input texture.

uniform int TexWidth;   // Always an even number.
uniform int TexHeight;

uniform int PassCount;  // For the very first pass, PassCount == 0.


void main()
{
    float P1 = texture2DRect( InputTex, gl_FragCoord.xy ).a;
    float P2;

    if ( PassCount % 2 == 0 )  // PassCount is Even.
    {

        //***********************************************
        //*********** WRITE YOUR CODE HERE **************
        //***********************************************
		int row = int( gl_FragCoord.y );
        int column = int( gl_FragCoord.x );

		int partner_column;
		bool is_even_column = column % 2 == 0;
		bool is_odd_column = !is_even_column;
		
		if (is_even_column){ // EVEN Column Indexed
			partner_column = column + 1;
		} else { // ODD Column Indexed
			partner_column = column - 1;
		}
		
		P2 = texture2DRect( InputTex, vec2(partner_column, row)).a;
		
		if ((is_even_column && P1 > P2) || 
			(is_odd_column && P1 < P2)){
			// SWAP
			gl_FragColor = vec4(P2);
		} else { 
			// NO SWAP
			gl_FragColor = vec4(P1);
		}
    }

    else  // PassCount is Odd.
    {
        int row = int( gl_FragCoord.y );
        int column = int( gl_FragCoord.x );
        int index1D = row * TexWidth + column;

        //***********************************************
        //*********** WRITE YOUR CODE HERE **************
        //***********************************************
		
		bool is_even_index = index1D % 2 == 0;
		bool is_odd_index = !is_even_index;
		
		int last_index = (TexWidth * TexHeight) - 1;
		
		if (index1D == 0 || index1D == last_index){
			// NO SWAP FOR FIRST AND LAST
			gl_FragColor = vec4(P1);
			return;
		} 
	
		int partner_index; // Partner index is the index to be compared with
		// Decide Partner Index
		if (is_even_index){ 
			partner_index = index1D - 1;
		} else {
			partner_index = index1D + 1;
		}		
		int partner_column = partner_index % TexWidth;
		int partner_row = partner_index / TexWidth;
		vec2 partner_coord = vec2(partner_column, partner_row);
		
		P2 = texture2DRect( InputTex, partner_coord).a;
		
		if ((is_even_index && P1 < P2) || 
			(is_odd_index && P1 > P2)) {
			// SWAP
			gl_FragColor = vec4(P2);
		} else {
			// NO SWAP
			gl_FragColor = vec4(P1);
		}
    }
}
