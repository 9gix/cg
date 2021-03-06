//============================================================
// STUDENT NAME: <your name>
// MATRIC NO.  : <matric no.>
// NUS EMAIL   : <your NUS email address>
// COMMENTS TO GRADER:
// <comments to grader, if any>
//
// ============================================================
//
// FILE: radiositysolver.cpp


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GL/glut.h>

#include "common.h"
#include "vector3.h"
#include "quadmodel.h"


/////////////////////////////////////////////////////////////////////////////
// CONSTANTS THAT YOU CHANGE FOR DIFFERENT INPUT MODEL AND
// TO CONTROL HOW GOOD THE SOLUTION YOU WANT
/////////////////////////////////////////////////////////////////////////////

// Input model filename.
static const char inputModelFilename[] = "cornell_box.in";

// Output model filename. This model contains the radiosity solution.
static const char outputModelFilename[] = "cornell_box.out";

// Threshold for subdiving the original quads to get shooter quads.
static const float maxShooterQuadEdgeLength = 70.0f;

// Threshold for subdiving the shooter quads to get gatherer quads.
static const float maxGathererQuadEdgeLength = 30.0f;

// This value tells when to terminate the progressive refinement radiosity computation.
// It sets the maximum number of iterations.
static const int maxIterations = 250;


/**********************************************************
 ****************** WRITE YOUR CODE HERE ******************
 **********************************************************
 * IF NECESSARY, ADD SOME MORE CONSTANTS TO CONTROL WHEN
 * TO TERMINATE THE RADIOSITY COMPUTATION.
 **********************************************************/

// Stop the iteration if the Length of UnshotPower below this threshold
static const float powerThreshold = 1100;



/////////////////////////////////////////////////////////////////////////////
// CONSTANTS
/////////////////////////////////////////////////////////////////////////////

// Window size.
static const int winWidthHeight = 600;     // Window width & height in pixels. Must be even number.

// Use white background, so that it will not conflict
// with the colors of the the gatherer quads.
static const float backgroundColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

// An integer corresponding to the RGB color [255, 255, 255].
static const int backgroundColorInt = (255 * 256 + 255) * 256 + 255;



/////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////

// The 3D model.
static QM_Model model;

// OpenGL display list.
static GLuint gathererQuadsDList = 0;

// Pre-computed delta form factors lookup tables.
static float *topDeltaFormFactors = NULL;
static float *sideDeltaFormFactors = NULL;



/////////////////////////////////////////////////////////////////////////////
// HELPER FUNCTIONS.
/////////////////////////////////////////////////////////////////////////////

static GLuint RGBToUnsignedInt( const GLubyte rgb[3] )
    // Convert RGB 8-bit triplets to an integer.
    // Note that R is the lowest byte of rgb[3].
{
    return ( ( rgb[2] * 256u ) + rgb[1] ) * 256u + rgb[0];
}



static void UnsignedIntToRGB( GLubyte rgb[3], unsigned int i )
    // Convert an integer to RGB 8-bit triplets.
    // The input integer must have value from 0 to (2^24 - 1).
    // Note that R is the lowest byte of rgb[3].
{
    rgb[0] = i % 256u;
    i = i / 256u;
    rgb[1] = i % 256u;
    rgb[2] = i / 256u;
}



static void ReadColorBuffer( GLubyte *buf, bool frontBuffer, int x, int y, int width, int height )
    // Read the RGB color buffer in the window region of size width x height.
    // The bottom-left corner of this window region is at (x, y).
    // If frontBuffer == true, it reads the front buffer, otherwise, the back buffer.
    // The read color buffer region is stored in the 1-D array buf[], which must be 
    // pre-allocated enough memory space.
{
    glPushAttrib( GL_ALL_ATTRIB_BITS );
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    if ( frontBuffer )
        glReadBuffer( GL_FRONT );
    else
        glReadBuffer( GL_BACK );
    glReadPixels( x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, (void *)buf );
    glPopAttrib();
}



static GLuint MakeGathererQuadsDisplayList( const QM_Model *m )
    // Build a OpenGL display list for all the gatherer quads.
    // Each gatherer quad is rendered in a unique color.
    // Used for rendering the quads for the hemicube.
{
    GLubyte rgb[3];
    GLuint dlist = glGenLists( 1 );
    if ( dlist == 0 ) ShowFatalError( __FILE__, __LINE__, "Cannot create display list" );
    glNewList( dlist, GL_COMPILE );
        glBegin( GL_QUADS );
            for ( int q = 0; q < m->totalGatherers; q++ )
            {
                QM_GathererQuad *quad = m->gatherers[q];
                UnsignedIntToRGB( rgb, (unsigned int) q );
                glColor3ubv( rgb );
                glVertex3fv( quad->v[0] );
                glVertex3fv( quad->v[1] );
                glVertex3fv( quad->v[2] );
                glVertex3fv( quad->v[3] );
            }
        glEnd();
    glEndList();
    return dlist;
}



static int FindShooterQuadWithHighestUnshotPower( const QM_Model *m )
{
    int s = 0;
    float maxUnshotPower = 0.0f;

    for ( int q = 0; q < m->totalShooters; q++ )
    {
        float *unshotPower = m->shooters[q]->unshotPower;
        float RGBunshotPower = unshotPower[0] + unshotPower[1] + unshotPower[2]; 
        if ( RGBunshotPower > maxUnshotPower ) { maxUnshotPower = RGBunshotPower;  s = q; }
    }
    return s;
}



static float TriangleArea( const float v1[3], const float v2[3], const float v3[3] )
    // Return the area of the triangle defined by the 3 input vertices.
{
    float normal[3];
    VecTriNormal( normal, v1, v2, v3 );
    return 0.5f * VecLen( normal );
}



static float ComputeHemicubeWidth( const QM_ShooterQuad *shooterQuad )
    // Compute the width of the hemicube such that it is within the boundary of the quad.
{
    const float SQRT_2 = 1.414214f;

    float c01 = TriangleArea( shooterQuad->centroid, shooterQuad->v[0], shooterQuad->v[1] );
    float c12 = TriangleArea( shooterQuad->centroid, shooterQuad->v[1], shooterQuad->v[2] );
    float c23 = TriangleArea( shooterQuad->centroid, shooterQuad->v[2], shooterQuad->v[3] );
    float c30 = TriangleArea( shooterQuad->centroid, shooterQuad->v[3], shooterQuad->v[0] );

    float h01 = 2.0f * c01 / VecDist( shooterQuad->v[0], shooterQuad->v[1] );
    float h12 = 2.0f * c12 / VecDist( shooterQuad->v[1], shooterQuad->v[2] );
    float h23 = 2.0f * c23 / VecDist( shooterQuad->v[2], shooterQuad->v[3] );
    float h30 = 2.0f * c30 / VecDist( shooterQuad->v[3], shooterQuad->v[0] );

    return SQRT_2 * Min3( Min2( h01, h12), h23, h30 );
}



static void PreComputeTopFaceDeltaFormFactors( float deltaFormFactors[], int numPixelsOnWidth )
    // Pre-compute the delta form factors on the top face of the hemicube.
    // The results are stored in the 1-D array deltaFormFactors[] of 
    // size of (numPixelsOnWidth x numPixelsOnWidth) elements.
    // Note that numPixelsOnWidth must be a even number.
{
    /**********************************************************
     ****************** WRITE YOUR CODE HERE ******************
     **********************************************************/
    double patchWidth = 1.0 / numPixelsOnWidth;
    double hemiCubeLength = 2.0;

    // Delta form factor is computed on [-1,1] instead of [0,1]
    // So the patchWidth need to be double scaled
    double scaledPatchWidth = hemiCubeLength / numPixelsOnWidth;

    double areaOfPixel = pow(hemiCubeLength, 2) / pow(numPixelsOnWidth, 2);
    int i = 0;
    for (double y = -1.0 + patchWidth; y < 1; y+= scaledPatchWidth) // y ranges (-1, 1)
    {
        for (double x = -1.0 + patchWidth; x < 1; x+= scaledPatchWidth) // x ranges (-1, 1)
        {
            deltaFormFactors[i] = areaOfPixel / (M_PI * pow(pow(x,2) + pow(y,2) + 1, 2));
            ++i;
        }
    }
}

static void PreComputeSideFaceDeltaFormFactors(float deltaFormFactors[], int numPixelsOnWidth)
// Pre-compute the delta form factors on a side face of the hemicube.
// The results are stored in the 1-D array deltaFormFactors[] of 
// size of [(numPixelsOnWidth/2) x numPixelsOnWidth] elements.
// Note that numPixelsOnWidth must be a even number.
{
    /**********************************************************
    ****************** WRITE YOUR CODE HERE ******************
    **********************************************************/
    double patchWidth = 1.0 / numPixelsOnWidth;
    double hemiCubeLength = 2.0;

    // Hemicube x:=[-1,1] y:=[-1,1] z:=[0,1]
    double scaledPatchWidth = hemiCubeLength / numPixelsOnWidth;

    double areaOfPixel = pow(hemiCubeLength, 2) / pow(numPixelsOnWidth, 2);
    int i = 0;
    for (double z = 0 + patchWidth; z < 1; z += scaledPatchWidth) // z ranges between (0, 1)
    {
        for (double y = -1.0 + patchWidth; y < 1; y += scaledPatchWidth) // y ranges between (-1, 1)
        {
            deltaFormFactors[i] = (areaOfPixel * z) / (M_PI * pow(pow(y, 2) + pow(z, 2) + 1, 2));
            ++i;
        }
    }
}



static void SetupHemicubeTopView( const QM_ShooterQuad *shooterQuad, float nearPlane, float farPlane )
    // Set up a view for the top face of a hemicube.
    // Need to set up the viewport, projection and view transfromation.
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    /**********************************************************
     ****************** WRITE YOUR CODE HERE ******************
     **********************************************************/

    // ================ //
    // Setup Viewport   //
    // ================ //
    glViewport(0, 0, winWidthHeight, winWidthHeight);

    // ================ //
    // Setup Projection //
    // ================ //
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-nearPlane, nearPlane, -nearPlane, nearPlane, nearPlane, farPlane);

    // ================ //
    // Setup View Model //
    // ================ //
    float tangent[3];
    VecDiff(tangent, shooterQuad->v[0], shooterQuad->v[1]);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        shooterQuad->centroid[0], shooterQuad->centroid[1], shooterQuad->centroid[2],
        shooterQuad->centroid[0] + shooterQuad->normal[0], shooterQuad->centroid[1] + shooterQuad->normal[1], shooterQuad->centroid[2] + shooterQuad->normal[2],
        tangent[0], tangent[1], tangent[2]);
}



static void SetupHemicubeSideView( int face, const QM_ShooterQuad *shooterQuad, float nearPlane, float farPlane )
    // Set up a view for one of the four side faces of a hemicube.
    // The input parameter face is a number from 1 to 4, that indicates which face to set up for.
    // Need to set up the viewport, projection and view transfromation.
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    /**********************************************************
     ****************** WRITE YOUR CODE HERE ******************
     **********************************************************/
    // ================ //
    // Setup Viewport   //
    // ================ //
    glViewport(0, 0, winWidthHeight, winWidthHeight/2);

    // ================ //
    // Setup Projection //
    // ================ //
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-nearPlane, nearPlane, 0, nearPlane, nearPlane, farPlane);

    // ================ //
    // Setup View Model //
    // ================ //
    float tangent[3];
    switch (face) {
    case 1: VecDiff(tangent, shooterQuad->v[0], shooterQuad->v[1]); break;
    case 2: VecDiff(tangent, shooterQuad->v[1], shooterQuad->v[2]); break;
    case 3: VecDiff(tangent, shooterQuad->v[2], shooterQuad->v[3]); break;
    case 4: VecDiff(tangent, shooterQuad->v[3], shooterQuad->v[0]); break;
    default: throw;
    }

    // Reorient the tangent vector into the midpoint of the face
    for (int i = 0; i < 3; ++i)
    {
        tangent[i] /= 2;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(
        shooterQuad->centroid[0], shooterQuad->centroid[1], shooterQuad->centroid[2],
        shooterQuad->centroid[0] + tangent[0], shooterQuad->centroid[1] + tangent[1], shooterQuad->centroid[2] + tangent[2],
        shooterQuad->normal[0], shooterQuad->normal[1], shooterQuad->normal[2]);
}



static void UpdateRadiosities( const QM_Model *m, const float shotPower[3], const GLubyte colorBuf[], 
                               const float deltaFormFactors[], int width, int height )
    // Use the color buffer (item buffer) to update the radiosities of the gatherer quads,
    // and update the unshot power of their parent shooter quads.
{
    for ( int i = 0; i < width * height; i++ )
    {
        int g = (int) RGBToUnsignedInt( &colorBuf[3 * i] );	// Which gatherer quad.
        if ( g < 0 || g >= m->totalGatherers || g == backgroundColorInt ) continue;

        /**********************************************************
         ****************** WRITE YOUR CODE HERE ******************
         **********************************************************/
        float rgb[3];

        // Increment the Gatherer's radiosity given by the current shooter
        for (int j = 0; j < 3; ++j) {
            rgb[j] = m->gatherers[g]->surface->reflectivity[j] * shotPower[j] * deltaFormFactors[i];
            m->gatherers[g]->radiosity[j] += rgb[j] / m->gatherers[g]->area;
        }

        //  Increment the Gatherer's Unshot Power given by the current shooter
        for (int j = 0; j < 3; ++j) {
            m->gatherers[g]->shooter->unshotPower[j] += rgb[j];
        }
    }
}



/////////////////////////////////////////////////////////////////////////////
// The display callback function.
// This is where the progressive refinement radiosity computation is performed.
/////////////////////////////////////////////////////////////////////////////

static void ComputeRadiosity( void )
{
    // Allocate temporary memory for reading in the colorbuffer.
    GLubyte *colorBuf = (GLubyte *) CheckedMalloc( sizeof(GLubyte) * 3 * winWidthHeight * winWidthHeight );

    for( int iterationCount = 0; iterationCount < maxIterations; iterationCount++ )
    {
    // Find a shooter quad to shoot power.
        printf( "Iteration %d\n", iterationCount );

        int s = FindShooterQuadWithHighestUnshotPower( &model );
        QM_ShooterQuad *shooterQuad = model.shooters[s];
        float unshotPower[3] = { shooterQuad->unshotPower[0], shooterQuad->unshotPower[1], shooterQuad->unshotPower[2] };

        /**********************************************************
         ****************** WRITE YOUR CODE HERE ******************
         **********************************************************
         * ADD ADDITIONAL TERMINATING CONDITION TO TERMINATE
         * RADIOSITY COMPUTATION.
         **********************************************************/

        // Terminate if the length of unshot power below the given threshold
        if (VecLen(unshotPower) < powerThreshold)
        {
            printf("unshot length %f is below threshold.\n", VecLen(unshotPower));
            break;
        }




        // After shooting power, the shooter quad's unshot power becomes zero.
        shooterQuad->unshotPower[0] = shooterQuad->unshotPower[1] = shooterQuad->unshotPower[2] = 0.0f;

    // Set up a hemicube at the centroid of the shooter.

        float hemicubeWidth = ComputeHemicubeWidth( shooterQuad );

        // Top face.
        SetupHemicubeTopView( shooterQuad, hemicubeWidth/2.0f, 2.0f * model.radius );
        glCallList( gathererQuadsDList );
        glFinish();
        ReadColorBuffer( colorBuf, true, 0, 0, winWidthHeight, winWidthHeight );
        UpdateRadiosities( &model, unshotPower, colorBuf, topDeltaFormFactors, winWidthHeight, winWidthHeight );

        // Side faces.
        for ( int face = 1; face <= 4; face++ )
        {
            SetupHemicubeSideView( face, shooterQuad, hemicubeWidth/2.0f, 2.0f * model.radius );
            glCallList( gathererQuadsDList );
            glFinish();
            ReadColorBuffer( colorBuf, true, 0, 0, winWidthHeight, winWidthHeight/2 );
            UpdateRadiosities( &model, unshotPower, colorBuf, sideDeltaFormFactors, winWidthHeight, winWidthHeight/2 );
        }
    }
    
    free( colorBuf );
    printf( "Radiosity computation completed.\n" );

    printf( "Computing vertex radiosities...\n" );
    QM_ComputeVertexRadiosities( &model );

    printf( "Writing output model file...\n" );
    QM_WriteGatherersToFile( outputModelFilename, &model );

    printf( "DONE.\nPress ENTER to exit program.\n" );
    char ch;
    scanf( "%c", &ch );
    exit( 0 );
}




/////////////////////////////////////////////////////////////////////////////
// The reshape callback function.
/////////////////////////////////////////////////////////////////////////////

static void MyReshape( int w, int h )
{
    if ( w != winWidthHeight || h != winWidthHeight )
        ShowFatalError( __FILE__, __LINE__, "Window size has been changed" );
}



/////////////////////////////////////////////////////////////////////////////
// Initialize some OpenGL states.
/////////////////////////////////////////////////////////////////////////////

static void InitOpenGL( void )
{
    // Set background color.
    glClearColor( backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3] ); 

    glShadeModel( GL_FLAT );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glEnable( GL_DEPTH_TEST );
    glDisable( GL_DITHER );
    glDisable( GL_BLEND );
    glDisable( GL_LIGHTING );
    glDisable( GL_CULL_FACE );
}



/////////////////////////////////////////////////////////////////////////////
// Initialize for the progressive refinement radiosity computation.
/////////////////////////////////////////////////////////////////////////////

static void InitRadiosityComputation( void )
{
// Check that we have 24-bit RGB colorbuffer for item buffering.
    GLint Rbits, Gbits, Bbits;
    glGetIntegerv( GL_RED_BITS, &Rbits );
    glGetIntegerv( GL_GREEN_BITS, &Gbits );
    glGetIntegerv( GL_BLUE_BITS, &Bbits );
    printf( "R = %d bits, G = %d bits, B = %d bits\n", Rbits, Gbits, Bbits );

    if ( Rbits != 8 || Gbits != 8 || Bbits != 8 )
        ShowFatalError( __FILE__, __LINE__, "Colorbuffer is not 24-bit RGB" );

// Read input model file.
    printf( "Reading input model file...\n" );
    model = QM_ReadFile( inputModelFilename );

// Subdivide the original quads to shooter quads and gatherer quads.
    printf( "Subdividing original quads...\n" );
    QM_Subdivide( &model, maxShooterQuadEdgeLength, maxGathererQuadEdgeLength );

// Make OpenGL display list for the gatherer quads.
    gathererQuadsDList = MakeGathererQuadsDisplayList( &model );

// Pre-compute the delta form factors for the fixed window resolution.
    topDeltaFormFactors = (float *) CheckedMalloc( sizeof(float) * winWidthHeight * winWidthHeight );
    sideDeltaFormFactors = (float *) CheckedMalloc( sizeof(float) * winWidthHeight * winWidthHeight / 2 );
    PreComputeTopFaceDeltaFormFactors( topDeltaFormFactors, winWidthHeight );
    PreComputeSideFaceDeltaFormFactors( sideDeltaFormFactors, winWidthHeight );

// Initialize the unshot power of the shooter quads.
    for ( int s = 0; s < model.totalShooters; s++ )
    {
        QM_ShooterQuad *shooterQuad = model.shooters[s];
        shooterQuad->unshotPower[0] = shooterQuad->area * shooterQuad->surface->emission[0];
        shooterQuad->unshotPower[1] = shooterQuad->area * shooterQuad->surface->emission[1];
        shooterQuad->unshotPower[2] = shooterQuad->area * shooterQuad->surface->emission[2];
    }

// Initialize the radiosity of the gatherer quads.
    for ( int g = 0; g < model.totalGatherers; g++ )
    {
        QM_GathererQuad *gathererQuad = model.gatherers[g];
        gathererQuad->radiosity[0] = gathererQuad->surface->emission[0];
        gathererQuad->radiosity[1] = gathererQuad->surface->emission[1];
        gathererQuad->radiosity[2] = gathererQuad->surface->emission[2];
    }
}




/////////////////////////////////////////////////////////////////////////////
// The main function.
/////////////////////////////////////////////////////////////////////////////

int main( int argc, char** argv )
{   
    printf( "Do not minimize, move, resize, or cover the drawing window.\n" );
    printf( "Press ENTER to start the radiosity computation.\n" );
    char ch;
    scanf( "%c", &ch );

// Initialize GLUT and create the drawing window.
    glutInit( &argc, argv );
    glutInitDisplayMode ( GLUT_RGB | GLUT_SINGLE | GLUT_DEPTH );
    glutInitWindowSize( winWidthHeight, winWidthHeight ); // Window must be square and size is fixed.
    glutCreateWindow( "Radiosity Solver" );
    InitOpenGL();

// Initialize for the progressive refinement radiosity computation.
    InitRadiosityComputation();

// Register the callback functions.
    glutDisplayFunc( ComputeRadiosity ); 
    glutReshapeFunc( MyReshape );

// Enter GLUT event loop.
    glutMainLoop();
    return 0;
}
