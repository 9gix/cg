#include "drawHouses.h"


void A0116631Torus(float a, float c, GLuint texture)
{
	const float PI = 3.1416;
	auto x = [a, c, PI](float u, float v) { return (c + a*cos(2 * PI*v))*cos(2 * PI*u); };
	auto y = [a, c, PI](float u, float v) { return (c + a*cos(2 * PI*v))*sin(2 * PI*u); };
	auto z = [a, PI](float v) { return a * sin(2 * PI*v); };

	auto p = [x, y, z](float u, float v){
		float arr[] = { x(u, v), y(u, v), z(v) };
		return arr;
	};


	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	for (float u = 0; u < 1; u += 0.05)
	{
		for (float v = 0; v < 1; v += 0.05)
		{
			glBegin(GL_POLYGON);
			glTexCoord2d(u, v); glVertex3fv(p(u, v));
			glTexCoord2d(u+0.05, v); glVertex3fv(p(u + 0.05, v));
			glTexCoord2d(u + 0.05, v+0.05); glVertex3fv(p(u + 0.05, v + 0.05));
			glTexCoord2d(u, v+0.05); glVertex3fv(p(u, v + 0.05));
			glEnd();
		}
	}

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}


void A0116631Rectangle1(GLuint texture, float length, float height)
{
	/* Texture Mapping Stretched */
	auto p = [length, height](float s, float t){
		float arr[] = { (s - 0.5) * length, t * height, 0 };
		return arr;
	};

	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);

	for (float t = 0; t < 1; t += 0.05)
	{
		for (float s = 0; s < 1; s += 0.05)
		{
			glBegin(GL_QUADS);
			glTexCoord2d(s, t); glVertex3fv(p(s, t));
			glTexCoord2d(s + 0.05, t); glVertex3fv(p(s + 0.05, t));
			glTexCoord2d(s + 0.05, t + 0.05); glVertex3fv(p(s + 0.05, t + 0.05));
			glTexCoord2d(s, t + 0.05); glVertex3fv(p(s, t + 0.05));
			glEnd();
		}
	}

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

}

void A0116631Rectangle2(GLuint texture, float length, float height)
{
	/* Texture Mapping Repeat*/
	auto p = [length, height](float s, float t){
		float arr[] = { (s - 0.5) * length, t * height, 0 };
		return arr;
	};

	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	for (float t = 0; t < 1; t += 0.05)
	{
		for (float s = 0; s < 1; s += 0.05)
		{
			glBegin(GL_QUADS);
			glTexCoord2d(0, 0); glVertex3fv(p(s, t));
			glTexCoord2d(1, 0); glVertex3fv(p(s + 0.05, t));
			glTexCoord2d(1, 1); glVertex3fv(p(s + 0.05, t + 0.05));
			glTexCoord2d(0, 1); glVertex3fv(p(s, t + 0.05));
			glEnd();
		}
	}

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}



void A0116631Roof(GLuint texture, float width, float length, float height){
	const float PI = 3.1416;
	float r = width/2;
	float h = length;

	auto x = [r, PI](float s) { return r * cos(PI * s); };
	auto y = [r, PI](float s) { return r * sin(PI * s); };
	auto z = [h](float t) { return t * h; };

	auto p = [x,y,z](float s, float t){
		float arr[] = { x(s), y(s), z(t) };
		return arr;
	};


	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	for (float t = 0; t < 1; t += 0.05)
	{
		for (float s = 0; s < 1; s += 0.05)
		{
			glBegin(GL_POLYGON);
			glTexCoord2d(s, t); glVertex3fv(p(s, t));
			glTexCoord2d(s + 0.05, t); glVertex3fv(p(s + 0.05, t));
			glTexCoord2d(s + 0.05, t + 0.05); glVertex3fv(p(s + 0.05, t + 0.05));
			glTexCoord2d(s, t + 0.05); glVertex3fv(p(s, t + 0.05));
			glEnd();
		}
	}


	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	
	A0116631Torus(1,1, texture);
	glTranslatef(0, 0, length);
	A0116631Torus(1,1, texture);
	
}

void A0116631Building(GLuint roofTexture, GLuint wallTexture, GLuint wall2Texture, GLuint doorTexture, GLuint windowTexture, GLuint window2Texture)
{
	float width = 4; float length = 6; float height = 6;

	// Gate
	glPushMatrix();
	glTranslatef(0, height *0.2, width*0.7);
	A0116631Torus(1, 3, wall2Texture);
	glPopMatrix();

	// wall
	for (int i = 0; i < 2; i++)
	{
		glRotatef(90, 0, 1, 0);
		glPushMatrix();
		glTranslatef(0, 0, width / 2);
		A0116631Rectangle1(wallTexture, length, height);
		glPopMatrix();

		glRotatef(90, 0, 1, 0);
		glPushMatrix();
		glTranslatef(0, 0, length / 2);
		A0116631Rectangle1(wallTexture, width, height);
		glPopMatrix();

	}

	

	// door
	glPushMatrix();
	glTranslatef(0, 0, (length / 2) + 0.01);
	A0116631Rectangle1(doorTexture, 0.2 * width, 0.4 * height);
	glPopMatrix();

	// window
	glPushMatrix();
	glTranslatef(0, height/2, (length / 2) + 0.01);
	A0116631Rectangle1(window2Texture, 0.2 * width, 0.4 * height);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(width / 2 + 0.01, 0.3*height, -length*0.4);
	glRotatef(-90, 0, 1, 0);
	for (int i = 0; i < 5; i++){
		A0116631Rectangle1(windowTexture, length*0.2, 0.4 * height);
		glTranslatef(length*0.2, 0, 0);
	}
	glPopMatrix();
	
	glPushMatrix();
	glTranslatef(-width/2-0.01, 0.3*height, -length*0.4);
	glRotatef(90, 0, 1, 0);
	for (int i = 0; i < 5; i++){
		A0116631Rectangle1(windowTexture, length*0.2, 0.4 * height);
		glTranslatef(-length*0.2, 0, 0);
	}
	glPopMatrix();


	// Roof
	glPushMatrix();
	glTranslatef(0, height, -3);
	A0116631Roof(roofTexture, width, length, 0.2 * height);
	glPopMatrix();
	
}

void A0116631House(GLuint texSet[])
{
	//plot dimension
	//x: -4:4
	//y: 0:12
	//z: -4:4
	
	//bounding volume
	glPushMatrix();
	glTranslatef(0, 6, 0);
	glScalef(8, 12, 8);
	glColor3f(1.0, 1.0, 1.0);
	glutWireCube(1);
	glPopMatrix();

	// Lawn
	GLuint lawnTexture = texSet[9];
	glPushMatrix();
	glTranslatef(0, 0.01, 0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, lawnTexture);
	glBegin(GL_QUADS);
	glTexCoord2d(0.0, 0.0); glVertex3f(-4, 0, -4);
	glTexCoord2d(1.0, 0.0); glVertex3f(4, 0, -4);
	glTexCoord2d(1.0, 1.0); glVertex3f(4, 0, 4);
	glTexCoord2d(0.0, 1.0); glVertex3f(-4, 0, 4);
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

	// Wall
	GLuint fenceTexture = texSet[0];
	glPushMatrix();
	float length = 8; float height = 4.5;
	for (int i = 0; i < 3; i++)
	{
		glRotatef(90, 0, 1, 0);

		glPushMatrix();
		glTranslatef(0, 0, length / 2);
		A0116631Rectangle1(fenceTexture, length, height);
		glPopMatrix();
	}


	// House
	GLuint roofTexture = texSet[36];
	GLuint wallTexture = texSet[20];
	GLuint wall2Texture = texSet[2];
	GLuint doorTexture = texSet[4];
	GLuint windowTexture = texSet[28];
	GLuint window2Texture = texSet[29];
	glRotatef(90, 0, 1, 0);
	glPushMatrix();
	A0116631Building(roofTexture, wallTexture, wall2Texture, doorTexture, windowTexture, window2Texture);
	glPopMatrix();
}