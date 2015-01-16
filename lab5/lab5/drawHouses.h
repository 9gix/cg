#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include "GL\glut.h"

using namespace std;

void A0116631House(GLuint texSet[]);
void A0116631Torus(float a, float c, GLuint texture);
void A0116631Rectangle(GLuint texture, float length, float height);
void A0116631House(GLuint roofTexture, GLuint wallTexture, GLuint wall2Texture, GLuint doorTexture, GLuint windowTexture, GLuint window2Texture);
void A0116631Roof(GLuint texture, float width, float length, float height);
void A0116631Building(GLuint roofTexture, GLuint wallTexture, GLuint wall2Texture, GLuint doorTexture, GLuint windowTexture, GLuint window2Texture);