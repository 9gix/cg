//============================================================
// STUDENT NAME: Eugene
// MATRIC NO.  : A0116631N
// NUS EMAIL   : eugene@comp.nus.edu.sg
// COMMENTS TO GRADER:
// <comments to grader, if any>
//
//============================================================
//
// FILE: assign1.vert


varying vec3 ecPosition; // Vertex's position in eye space.
varying vec3 ecNormal;   // Vertex's normal vector in eye space.
varying vec3 ecTangent;  // Vertex's tangent vector in eye space.

attribute vec3 Tangent;  // Input vertex's tangent vector in model space.


void main( void )
{

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();

	ecPosition = vec3(gl_ModelViewMatrix * gl_Vertex);
	ecNormal = gl_NormalMatrix * gl_Normal;
	ecTangent = gl_NormalMatrix * Tangent;

}
