#include "drawHouses.h"

bool drag = false;
int prevX=0, prevY = 0;
int curX = 0, curY = 0;
float rot = 0, ratio=1;
GLuint texSet[40];
GLuint selectBuf[512];
GLuint target = 0;
float lookX=0, lookZ=0, tarXpos=0, tarYpos=80, tarZpos=80;

void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	ratio = (float)w/h;

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, ratio, 75, -75);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(tarXpos, tarYpos, tarZpos, lookX, 0, lookZ, 0, 1, 0);
}

GLuint loadMyTextures(char *filename)
{	
	GLuint myTexture;
	FILE * file;
	unsigned char *tex, *tex2;

	// texture data
	int width = 256;
	int height = 256;

	// allocate buffer
	tex = (unsigned char *)malloc( width * height * 3 );
	tex2 = (unsigned char *)malloc( width * height * 3 );

	// open and read texture data
	file = fopen( filename, "rb" );

	if(file==NULL)
		cout<<"Error! Missing texture!\n";
	else
		cout<<"texture successfully loaded\n";

	fread( tex, width * height * 3, 1, file );
	fclose( file );

	for(int i=0;i<width*height;i++)
	{
		tex2[i*3] = tex[i*3+2];
		tex2[i*3+1] = tex[i*3+1];
		tex2[i*3+2] = tex[i*3];
	}

	 // allocate a texture name
     glGenTextures( 1, &myTexture );
	 glBindTexture( GL_TEXTURE_2D, myTexture );

    // select modulate to mix texture with color for shading
     glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
 
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_DECAL);
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_DECAL);
 
     // when texture area is small, bilinear filter the closest mipmap
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
     // when texture area is large, bilinear filter the first mipmap
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
 
     // texture should tile
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
     glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // build texture mipmaps
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, tex2 );

    // free buffers
    free( tex );
	free(tex2);
	return myTexture;
}


void init(void)
{
	glClearColor (0.73, 0.90, 0.96, 1.0);
	glShadeModel (GL_SMOOTH);
    glEnable(GL_NORMALIZE);

	texSet[0] = loadMyTextures("brick1.bmp");
	texSet[1] = loadMyTextures("brick2.bmp");
	texSet[2] = loadMyTextures("brick3.bmp");
	texSet[3] = loadMyTextures("brick4.bmp");
	texSet[4] = loadMyTextures("door1.bmp");
	texSet[5] = loadMyTextures("door2.bmp");
	texSet[6] = loadMyTextures("flower1.bmp");
	texSet[7] = loadMyTextures("flower2.bmp");
	texSet[8] = loadMyTextures("flower3.bmp");
	texSet[9] = loadMyTextures("grass1.bmp");

	texSet[10] = loadMyTextures("grass2.bmp");
	texSet[11] = loadMyTextures("leaves1.bmp");
	texSet[12] = loadMyTextures("leaves2.bmp");
	texSet[13] = loadMyTextures("roof1.bmp");
	texSet[14] = loadMyTextures("roof2.bmp");
	texSet[15] = loadMyTextures("roof3.bmp");
	texSet[16] = loadMyTextures("roof4.bmp");
	texSet[17] = loadMyTextures("roof5.bmp");
	texSet[18] = loadMyTextures("roof6.bmp");
	texSet[19] = loadMyTextures("stone1.bmp");

	texSet[20] = loadMyTextures("stone2.bmp");
	texSet[21] = loadMyTextures("tile1.bmp");
	texSet[22] = loadMyTextures("tile2.bmp");
	texSet[23] = loadMyTextures("tile3.bmp");
	texSet[24] = loadMyTextures("tile4.bmp");
	texSet[25] = loadMyTextures("tile5.bmp");
	texSet[26] = loadMyTextures("tile6.bmp");
	texSet[27] = loadMyTextures("window1.bmp");
	texSet[28] = loadMyTextures("window2.bmp");
	texSet[29] = loadMyTextures("window3.bmp");

	texSet[30] = loadMyTextures("window4.bmp");
	texSet[31] = loadMyTextures("window5.bmp");
	texSet[32] = loadMyTextures("window6.bmp");
	texSet[33] = loadMyTextures("wood1.bmp");
	texSet[34] = loadMyTextures("wood2.bmp");
	texSet[35] = loadMyTextures("wood3.bmp");
	texSet[36] = loadMyTextures("wood4.bmp");
	texSet[37] = loadMyTextures("wood5.bmp");
	texSet[38] = loadMyTextures("wood6.bmp");
	texSet[39] = loadMyTextures("wood7.bmp");
}


void drawPatch()
{
	//draw road
	glBegin(GL_QUADS);
	glColor3f(0.6, 0.6, 0.6);
	glVertex3f(-5, -0.01, 5);
	glVertex3f(5, -0.01, 5);
	glVertex3f(5, -0.01, -5);
	glVertex3f(-5, -0.01, -5);
	glEnd();

	//one patch
	glBegin(GL_QUADS);
	glColor3f(0.4, 0.83, 0.2);
	glVertex3f(-4, 0, 4);
	glVertex3f(4, 0, 4);
	glVertex3f(4, 0, -4);
	glVertex3f(-4, 0, -4);
	glEnd();
}


void display(void)
{
	int startTime=glutGet(GLUT_ELAPSED_TIME);
	int counter = 1;
	
	glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//floor plane
	glBegin(GL_POLYGON);
	glColor3f(0.3, 0.73, 0.1);
	glVertex3f(-75, -0.02, 75);
	glVertex3f(75, -0.02, 75);
	glVertex3f(75, -0.02, -75);
	glVertex3f(-75, -0.02, -75);
	glEnd();

	//draw patches
	for(int i=0;i<9;i++)
	{
		for(int j=0;j<9;j++)
		{
			glPushMatrix();
			glTranslatef((i-4.5)*10+5, 0, (j-4.5)*10+5);
			glLoadName(counter);
			counter++;
			drawPatch();
			glPopMatrix();
		}
	}

	
	A0116631House(texSet);

	/*
	glPushMatrix();
	drawMyHouse(texSet);

	glTranslatef(10, 0, 0);
	drawMyHouse(texSet);

	glTranslatef(10, 0, 0);
	drawMyHouse(texSet);

	glTranslatef(10, 0, 0);
	drawMyHouse(texSet);

	glTranslatef(10, 0, 0);
	drawMyHouse(texSet);

	glPopMatrix();
	*/

	
	int endTime=glutGet(GLUT_ELAPSED_TIME);
	cout<<"Frame Rate: "<<(float)1000/(endTime-startTime)<<"\n";
	glFlush();
	glutSwapBuffers();

}

void startPicking(int x, int y) 
{
 	GLint hits, view[4];
 	int id;
 
 	glSelectBuffer(512, selectBuf);
 	glGetIntegerv(GL_VIEWPORT, view);
 	
 	glRenderMode(GL_SELECT);
 	glInitNames();
 	glPushName(0); 

 	//modify the viewing volume, restricting selection area around the cursor
 	glMatrixMode(GL_PROJECTION);
 	glPushMatrix();
	glLoadIdentity();
 
	gluPickMatrix(x, y, 5.0, 5.0, view);
	gluPerspective(60, (float)view[2]/(float)view[3], 0.0001, 1000.0);
 
	glMatrixMode(GL_MODELVIEW);
	display();

	glMatrixMode(GL_PROJECTION);
 	glPopMatrix();
 
 	hits = glRenderMode(GL_RENDER);
	target = selectBuf[4+3];
	glMatrixMode(GL_MODELVIEW);

	tarXpos = ((int)((target-1)/9)-4.5)*10+5;
	tarYpos = 20;
	tarZpos = ((target-1)%9-4.5)*10+5+20;
	lookX = ((int)((target-1)/9)-4.5)*10+5;;
	lookZ = ((target-1)%9-4.5)*10+5;

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, ratio, 0.01, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(tarXpos, tarYpos, tarZpos, lookX, 0, lookZ, 0, 1, 0);
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{

	if(!drag && button == GLUT_LEFT_BUTTON && state==GLUT_DOWN)
	{
		drag = true;
		prevX = x;
		prevY = y;
	}

	else if(button == GLUT_LEFT_BUTTON && state==GLUT_UP)
	{		
		drag = false;
		prevX = 0;
		prevY = 0;
	}

	else if(button==GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		startPicking(x, 600-y);

	else if(button==GLUT_MIDDLE_BUTTON && state==GLUT_DOWN)
	{
		glMatrixMode (GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(60, ratio, 75, -75);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		lookX=0;
		lookZ=0; 
		tarXpos=0;
		tarYpos=80;
		tarZpos=80;

		gluLookAt(tarXpos, tarYpos, tarZpos, lookX, 0, lookZ, 0, 1, 0);
		glutPostRedisplay();
	}
}

void motion(int x, int y)
{
	if(drag)
	{
		curX = x;
		curY = y;		

		float xVal = curX-prevX;
		float yVal = curY-prevY;
		
		float theta = sqrt(xVal*xVal + yVal*yVal)/180 * 3.14;

		if(curX<prevX)
		{
			theta = -1 * theta;
		}

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		float dist = sqrt((float)(tarXpos-lookX)*(float)(tarXpos-lookX)+(float)(tarZpos-lookZ)*(float)(tarZpos-lookZ));
		
		float xMove = (tarXpos-lookX)*cos(theta)-(tarZpos-lookZ)*sin(theta);
		float zMove = (tarXpos-lookX)*sin(theta)+(tarZpos-lookZ)*cos(theta);

		dist = sqrt(xMove*xMove+zMove*zMove);

		tarXpos=lookX+xMove;
		tarZpos=lookZ+zMove;
		gluLookAt(tarXpos, tarYpos, tarZpos, lookX, 0, lookZ, 0, 1, 0);

		prevX = curX;
		prevY = curY;

		glutPostRedisplay();
	}	
}


int main(int argc, char **argv)
{
	cout<<"CS3241 Lab 5\n\n";
	cout<<"+++++CONTROLS+++++++\n\n";
	cout<<"Left Click: Rotate\n";
	cout<<"Right Click: Select patch\n";
	cout<<"Middle Click: Overview\n";

	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize (600, 600);
	glutInitWindowPosition (50, 50);
	glutCreateWindow (argv[0]);

	init ();
	glutDisplayFunc(display);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);	
	glutMouseFunc(mouse);
	glutMainLoop();

	return 0;
}

