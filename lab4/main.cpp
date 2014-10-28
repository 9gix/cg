// CS3241Lab1.cpp : Defines the ent7ry point for the console application.
//#include <cmath>
#include "math.h"
#include <iostream>
#include <fstream>
#include "GL\glut.h"

#define MAXPTNO 1000
#define NLINESEGMENT 32
#define NOBJECTONCURVE 8

const int NDEGREE = 3;
const int CTRL_POINT = NDEGREE + 1;


using namespace std;

// Global variables that you can use
struct Point {
	int x,y;
};

// Storage of control points
int nPt = 0;
Point ptList[MAXPTNO];

// Display options
bool displayControlPoints = true;
bool displayControlLines = true;
bool displayTangentVectors = false;
bool displayObjects = false;
bool C1Continuity = false;
	


int triangle[10][10];


int C(int n, int r) {
	return triangle[n][r];
}

float bezier_basis(int i, float t)
{
	return C(NDEGREE, i) * pow(1 - t, NDEGREE - i) * pow(t, i);
}

Point bezier_curve(Point pts[], float t)
{
	Point tmp;
	tmp.x = 0;
	tmp.y = 0;
	for (int j = 0; j < CTRL_POINT; j++)
	{
		float basis = bezier_basis(j, t);
		tmp.x += basis * pts[j].x;
		tmp.y += basis * pts[j].y;
	}
	return tmp;
}



void makePascalTriangle() {
	int i, j;
	triangle[0][0] = 1;

	for (i = 1; i < 9; i++) {
		triangle[i][0] = 1;
		for (j = 1; j <= i; j++) {
			triangle[i][j] = triangle[i - 1][j - 1] + triangle[i - 1][j];
		}
	}
}


void drawRightArrow()
{
	glColor3f(0,1,0);
	glBegin(GL_LINE_STRIP);
		glVertex2f(0,0);
		glVertex2f(100,0);
		glVertex2f(95,5);
		glVertex2f(100,0);
		glVertex2f(95,-5);
	glEnd();
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	if(displayControlPoints)
	{

		glPointSize(5);
		glBegin(GL_POINTS);
		for(int i=0;i<nPt; i++)
		{
				glColor3f(0,0,0);
				glVertex2d(ptList[i].x,ptList[i].y);
		}
		glEnd();
		glPointSize(1);

	}

	if(displayControlLines)
	{
		glColor3f(0,1,0);
		
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i < nPt; i++)
		{
			glVertex2d(ptList[i].x, ptList[i].y);
		}
		glEnd();
	}



	if (C1Continuity)
	{

		glBegin(GL_LINE_STRIP);
		glColor3f(0, 0, 1);
		for (int i = NDEGREE; i < nPt; i += NDEGREE)
		{

			
			if (i % 3 == 1)
			{
				// let pts start from index 0
				// p[4] = c1(p[4]-2, p[4]-1)
				// p[7] = c1(p[7]-2, p[7]-1)
				// p[10] = c1(p[10] - 2, p[10] - 1)

			}

			Point pts[CTRL_POINT];
			for (int j = 0; j < CTRL_POINT; j++)
			{
				pts[j] = ptList[i - NDEGREE + j];
			}


			for (int j = 0; j < NLINESEGMENT; j++)
			{
				Point pt = bezier_curve(pts, (float)j / NLINESEGMENT);
				glVertex2d(pt.x, pt.y);
			}
		}
		glEnd();
	}
	else
	{
		glBegin(GL_LINE_STRIP);
		glColor3f(0, 0, 1);
		for (int i = NDEGREE; i < nPt; i += NDEGREE)
		{
			Point pts[CTRL_POINT];
			for (int j = 0; j < CTRL_POINT; j++)
			{
				pts[j] = ptList[i - NDEGREE + j];
			}


			for (int j = 0; j < NLINESEGMENT; j++)
			{
				Point pt = bezier_curve(pts, (float)j / NLINESEGMENT);
				glVertex2d(pt.x, pt.y);
			}
		}
		glEnd();
	}

	glPopMatrix();
	glutSwapBuffers ();
}


void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0,w,h,0);  
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
}

void init(void)
{
	glClearColor (1.0,1.0,1.0, 1.0);
	makePascalTriangle();
}

void readFile()
{

	std::ifstream file;
	file.open("savefile.txt");
	file >> nPt;

	if(nPt>MAXPTNO)
	{
		cout << "Error: File contains more than the maximum number of points." << endl;
		nPt = MAXPTNO;
	}

	for(int i=0;i<nPt;i++)
	{
		file >> ptList[i].x;
		file >> ptList[i].y;
	}
	file.close();// is not necessary because the destructor closes the open file by default
}

void writeFile()
{
	std::ofstream file;
	file.open("savefile.txt");
	file << nPt << endl;

	for(int i=0;i<nPt;i++)
	{
		file << ptList[i].x << " ";
		file << ptList[i].y << endl;
	}
	file.close();// is not necessary because the destructor closes the open file by default
}

void keyboard (unsigned char key, int x, int y)
{
	switch (key) {
		case 'r':
		case 'R':
			readFile();
		break;

		case 'w':
		case 'W':
			writeFile();
		break;

		case 'T':
		case 't':
			displayTangentVectors = !displayTangentVectors;
		break;

		case 'o':
		case 'O':
			displayObjects = !displayObjects;
		break;

		case 'p':
		case 'P':
			displayControlPoints = !displayControlPoints;
		break;

		case 'L':
		case 'l':
			displayControlLines = !displayControlLines;
		break;

		case 'C':
		case 'c':
			C1Continuity = !C1Continuity;
		break;

		case 'e':
		case 'E':
			// erase all the control points added
			nPt = 0;
		break;

		case 'Q':
		case 'q':
			exit(0);
		break;

		default:
		break;
	}

	glutPostRedisplay();
}



void mouse(int button, int state, int x, int y)
{
	/*button: GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, or GLUT_RIGHT_BUTTON */
	/*state: GLUT_UP or GLUT_DOWN */
	enum
	{
		MOUSE_LEFT_BUTTON = 0,
		MOUSE_MIDDLE_BUTTON = 1,
		MOUSE_RIGHT_BUTTON = 2,
		MOUSE_SCROLL_UP = 3,
		MOUSE_SCROLL_DOWN = 4
	};
	if((button == MOUSE_LEFT_BUTTON)&&(state == GLUT_UP))
	{
		if(nPt==MAXPTNO)
		{
			cout << "Error: Exceeded the maximum number of points." << endl;
			return;
		}
		ptList[nPt].x=x;
		ptList[nPt].y=y;
		nPt++;
	}
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	cout<<"CS3241 Lab 4"<< endl<< endl;
	cout << "Left mouse click: Add a control point"<<endl;
	cout << "Q: Quit" <<endl;
	cout << "P: Toggle displaying control points" <<endl;
	cout << "L: Toggle displaying control lines" <<endl;
	cout << "E: Erase all points (Clear)" << endl;
	cout << "C: Toggle C1 continuity" <<endl;	
	cout << "T: Toggle displaying tangent vectors" <<endl;
	cout << "O: Toggle displaying objects" <<endl;
	cout << "R: Read in control points from \"savefile.txt\"" <<endl;
	cout << "W: Write control points to \"savefile.txt\"" <<endl;
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize (600, 600);
	glutInitWindowPosition (50, 50);
	glutCreateWindow ("CS3241 Assignment 4");
	init ();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}