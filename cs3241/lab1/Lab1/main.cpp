// CS3241Lab1.cpp : Defines the entry point for the console application.
#include <cmath>
#include <iostream>
#include "GL\glut.h"

using namespace std;

GLfloat PI = 3.14;
float alpha = 0.0, k = 0.15;
float tx = 0.0, ty = 0.0;

float to_radian(float degree)
{
	return degree * (PI / 180);
}

float to_angle(float radian)
{
	return radian * (180 / PI);
}

void drawLine(float length)
{
	glBegin(GL_LINES);

	glVertex2f(0, 0);
	glVertex2f(0, length);

	glEnd();
	glTranslatef(0, length, 0);
}

void AwesomeSpiral(float distance, float angle, float increment, int n_segment)
{
	// Feel Free to change the angle to produce a variety of the result.
	glPushMatrix();

	for (int i = 0; i < n_segment; i++)
	{
		float red = (1.0 * i / n_segment);
		float green = 1 - (1.0 * i / n_segment);
		float blue = (1.0 * i / n_segment);

		glColor3f(red, green, blue);
		drawLine(distance);
		glRotatef(angle-0.8, 0, 0, 1);
		distance += increment;

		glPushMatrix();
			glColor3f(1, green, blue);
			glRotatef(-angle-90, 0, 0, 1);
			drawLine(distance);
			glPushMatrix();
				glColor3f(1, green, 0);
				glRotatef(angle+90, 0, 0, 1);
				drawLine(distance);
			glPopMatrix();
		glPopMatrix();
	}
	glPopMatrix();
}

void drawMySpiral()
{
	glTranslatef(-40, -40, 0);
	AwesomeSpiral(0.1, 71.5, 0.05, 200);
	glTranslatef(40, 0, 0);
	AwesomeSpiral(0.1, 120, 0.05, 200);
	glTranslatef(40, 0, 0);
	AwesomeSpiral(0.1, 68, 0.05, 200);
	glTranslatef(0, 40, 0);
	AwesomeSpiral(0.1, 179, 0.05, 200);
	glTranslatef(-40, 0, 0);
	AwesomeSpiral(0.1, 45.5, 0.05, 200);
	glTranslatef(-40, 0, 0); 
	AwesomeSpiral(0.1, 150, 0.05, 200);
	glTranslatef(0, 40, 0);
	AwesomeSpiral(0.1, 60, 0.05, 200);
	glTranslatef(40, 0, 0);
	AwesomeSpiral(0.1, 80, 0.05, 200);
	glTranslatef(40, 0, 0);
	AwesomeSpiral(0.1, 55, 0.05, 200);

}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glPushMatrix();

	//controls transformation
	glScalef(k, k, k);
	glTranslatef(tx, ty, 0);
	glRotatef(alpha, 0, 0, 1);
	glColor3f(0, 0, 0);
	
	drawMySpiral();
	

	glPopMatrix();
	glFlush();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(-10, 10, -10, 10, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void init(void)
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glShadeModel(GL_SMOOTH);
}



void keyboard(unsigned char key, int x, int y)
{
	//keys to control scaling - k
	//keys to control rotation - alpha
	//keys to control translation - tx, ty
	switch (key) {

	case 'a':
		alpha += 10;
		glutPostRedisplay();
		break;

	case 'd':
		alpha -= 10;
		glutPostRedisplay();
		break;

	case 'q':
		k += 0.1;
		glutPostRedisplay();
		break;

	case 'e':
		if (k>0.1)
			k -= 0.1;
		glutPostRedisplay();
		break;

	case 'z':
		tx -= 0.1;
		glutPostRedisplay();
		break;

	case 'c':
		tx += 0.1;
		glutPostRedisplay();
		break;

	case 's':
		ty -= 0.1;
		glutPostRedisplay();
		break;

	case 'w':
		ty += 0.1;
		glutPostRedisplay();
		break;

	default:
		break;
	}
}

int main(int argc, char **argv)
{
	cout << "CS3241 Lab 1\n\n";
	cout << "+++++CONTROL BUTTONS+++++++\n\n";
	cout << "Scale Up/Down: Q/E\n";
	cout << "Rotate Clockwise/Counter-clockwise: A/D\n";
	cout << "Move Up/Down: W/S\n";
	cout << "Move Left/Right: Z/C\n";

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(50, 50);
	glutCreateWindow(argv[0]);
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	//glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutMainLoop();

	return 0;
}
