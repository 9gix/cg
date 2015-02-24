// CS3241Lab1.cpp : Defines the entry point for the console application.
#include <cmath>
#include <iostream>
#include <vector>
#include <time.h>
#include "GL/glut.h"

using namespace std;

#define numStars 100
#define numPlanets 8

class Planet
{
public:
	float diameter;
	GLfloat color[3];
	GLfloat alpha;
	GLfloat velocity;
	GLfloat distance;
	GLfloat angle;
	int nSatellite;

	Planet()
	{
		color[0] = 0.4; color[1] = 0.4; color[2] = 0.4;
		velocity = 0.1;
		diameter = 0.5;
		distance = 10;
		angle = 0;
		alpha = 1;
		nSatellite = 0;
	}
};

class Sun
{
public:
	float diameter;
	GLfloat color[3];
	GLfloat alpha;
	
	Sun()
	{
		color[0] = 1; color[1] = 0.7; color[2] = 0.1;
		alpha = 1;
		diameter = 5;
	}
};

struct Star
{
	float x;
	float y;
	float z;
};


GLfloat PI = 3.14;
float alpha = 0.0, k=0.5;
float tx = 0.0, ty=0.0;
Sun sun;

Star stars[numStars];
Planet solar_system[numPlanets];




void drawCircle(float cx, float cy, float r, int num_segments)
{
	// Credit to http://slabode.exofire.net/circle_draw.shtml

	float theta = 2 * PI / float(num_segments);
	float c = cosf(theta);
	float s = sinf(theta);
	float t;

	float x = r;
	float y = 0;
	glBegin(GL_POLYGON);
	for (int i = 0; i < num_segments; i++)
	{
		glVertex2f(x + cx, y + cy);
		t = x;
		x = c * x - s * y;
		y = s * t + c * y;
	}
	glEnd();
}

void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h);

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-10, 10, -10, 10, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void init(void)
{
	glClearColor (0.0, 0.0, 0.0, 1.0);
	glShadeModel (GL_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	Planet mercury;
	mercury.diameter = 0.1;
	mercury.velocity = 0.7;
	mercury.distance = 4;

	Planet venus;
	venus.diameter = 0.2;
	venus.velocity = 0.5;
	venus.distance = 6;
	venus.angle = 10;
	venus.color[0] = 0.4;
	venus.color[1] = 0.2;
	venus.color[2] = 0.1;

	Planet earth;
	earth.diameter = 0.4;
	earth.velocity = 0.4;
	earth.distance = 7;
	earth.color[0] = 0.1;
	earth.color[1] = 0.3;
	earth.color[2] = 0.4;
	earth.nSatellite = 1;
	
	Planet mars;
	mars.diameter = 0.2;
	mars.velocity = 0.3;
	mars.distance = 8;
	mars.color[0] = 1;
	mars.color[1] = 0.1;
	mars.color[2] = 0.1;
	mars.nSatellite = 2;

	Planet jupiter;
	jupiter.diameter = 1.5;
	jupiter.velocity = 0.25;
	jupiter.distance = 12;
	jupiter.color[0] = 0.5;
	jupiter.color[1] = 0.3;
	jupiter.color[2] = 0.1;
	jupiter.nSatellite = 4;

	Planet saturn;
	saturn.diameter = 1.3;
	saturn.velocity = 0.2;
	saturn.distance = 16;
	saturn.color[0] = 0.3;
	saturn.color[1] = 0.3;
	saturn.color[2] = 0.1;
	saturn.nSatellite = 9;

	Planet uranus;
	uranus.diameter = 0.9;
	uranus.velocity = 0.15;
	uranus.distance = 20;
	uranus.color[0] = 0.1;
	uranus.color[1] = 0.1;
	uranus.color[2] = 1;
	uranus.nSatellite = 6;

	Planet neptune;
	neptune.diameter = 0.9;
	neptune.velocity = 0.1;
	neptune.distance = 25;
	neptune.color[0] = 0.1;
	neptune.color[1] = 0.1;
	neptune.color[2] = 1;
	neptune.nSatellite = 3;

	solar_system[0] = mercury;
	solar_system[1] = venus;
	solar_system[2] = earth;
	solar_system[3] = mars;
	solar_system[4] = jupiter;
	solar_system[5] = saturn;
	solar_system[6] = uranus;
	solar_system[7] = neptune;

	for (int i = 0; i < numStars; i++)
	{
		stars[i].x = rand() % 50 - 25;
		stars[i].y = rand() % 50 - 25;
		stars[i].z = rand() % 50 - 25;

	}
}

void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glPushMatrix();

	//controls transformation
	glScalef(k, k, k);	
	glTranslatef(tx, ty, 0);	

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// Drawing Stars
	for (int i = 0; i < numStars; i++)
	{
		glPushMatrix();
		glTranslatef(stars[i].x, stars[i].y, stars[i].z);
		// Blinking Stars
		glColor3f((double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX), (double)rand() / (RAND_MAX));
		glutSolidSphere(0.1, 10, 10);
		glPopMatrix();
	}

	// Drawing Sun
	glPushMatrix();
		glColor4f(sun.color[0], sun.color[1], sun.color[2], sun.alpha);
		glutSolidSphere(3, 50, 50);
	glPopMatrix();


	//Drawing Planets
	for (int i = 0; i < numPlanets; i++)
	{
		glPushMatrix();
		glRotatef(solar_system[i].angle, 0, 0, 1);
		glTranslatef(solar_system[i].distance, 0, 0);
		glColor4f(solar_system[i].color[0], solar_system[i].color[1], solar_system[i].color[2], solar_system[i].alpha);
		glutSolidSphere(solar_system[i].diameter, 50, 50);

		// Drawing Satellite (use the planets variable to determine the speed etc.)
		for (int j = 0; j < solar_system[i].nSatellite; j++)
		{
		glPushMatrix();
			glRotatef(solar_system[i].angle * (j + 1), 0, 0, 1);
			glTranslatef(solar_system[i].diameter + (solar_system[i].diameter * j / 10) + 0.3, 0, 0);
			glColor3f(0.5, 0.5, 0.5);
			glutSolidSphere(solar_system[i].diameter / 10, 50, 50);
		glPopMatrix();
		}


		glPopMatrix();
	}

	glPopMatrix();
	glFlush ();

	glutSwapBuffers();
}

void idle()
{	
	for (int i = 0; i < numPlanets; i++)
	{

		solar_system[i].angle += solar_system[i].velocity;
		if (solar_system[i].angle > 360.0)
		{
			solar_system[i].angle = solar_system[i].angle - 360.0;
		}
	}
	glutPostRedisplay();	//after updating, draw the screen again
}

void keyboard (unsigned char key, int x, int y)
{
	//keys to control scaling - k
	//keys to control rotation - alpha
	//keys to control translation - tx, ty
	switch (key) {

		case 'a':
			alpha+=10;
			glutPostRedisplay();
		break;

		case 'd':
			alpha-=10;
			glutPostRedisplay();
		break;

		case 'q':
			k+=0.1;
			glutPostRedisplay();
		break;

		case 'e':
			if(k>0.1)
				k-=0.1;
			glutPostRedisplay();
		break;

		case 'z':
			tx-=0.1;
			glutPostRedisplay();
		break;

		case 'c':
			tx+=0.1;
			glutPostRedisplay();
		break;

		case 's':
			ty-=0.1;
			glutPostRedisplay();
		break;

		case 'w':
			ty+=0.1;
			glutPostRedisplay();
		break;

		default:
		break;
	}
}

int main(int argc, char **argv)
{
	cout<<"CS3241 Lab 2\n\n";
	cout<<"+++++CONTROL BUTTONS+++++++\n\n";
	cout<<"Scale Up/Down: Q/E\n";
	cout<<"Rotate Clockwise/Counter-clockwise: A/D\n";
	cout<<"Move Up/Down: W/S\n";
	cout<<"Move Left/Right: Z/C\n";

	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize (600, 600);
	glutInitWindowPosition (50, 50);
	glutCreateWindow (argv[0]);
	init ();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);	
	//glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}
