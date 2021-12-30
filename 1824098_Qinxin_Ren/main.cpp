#include "stdlib.h"
#include <GL/glut.h>
#include<iostream>
#define _USE_MATH_DEFINES
#include <math.h>
using std::cout;
static float angle = 0.0;


# define  green  0.0 ,  0.502,  0.0
# define  red    0.8 ,  0.0  ,  0.0
# define  gray   0.502, 0.502,0.502
# define  hgray  0.117, 0.180,0.227
# define  blue   0.0  , 0.0  ,  1.0
# define  pi  3.14159265
# define  yellow   1.0,215.0/255.0,0.0
# define  twhite 166/255.0, 142.0/255.0,140/255.0
# define  brown  210.0/255.0, 105.0/255.0,  30.0/255.0
# define  nbrown 128.0/255.0, 64.0/255.0, 64.0/255.0
# define  door   244.0/255.0 ,164.0/255.0 , 96.0/255.0
# define  doorknob   139.0/255.0 ,69.0/255.0,19.0/255.0
# define water 141.0/255.0, 193.0/255.0, 241.0/255.0
# define orange 255.0/255.0, 128.0/255.0, 0.0/255.0
# define sunshine 255.0/255.0, 210.0/255.0, 166.0/255.0
# define BMP_Header_Length 54  //offset of image data in memory block
#define MAX_PARTICLES 1000 //define the number of particles

double rec[8][3]; //define rectangular's 8 vertexes
double tri[8][3];
bool mouseDown = false;

//define the window size
GLfloat w = 1100;
GLfloat h = 1100;

//define the testure
GLuint texGround;
GLuint texPoor;
GLuint texFloor;
GLuint texRoof;

//material parameters
GLfloat matiral_specular[4] = { 0.00, 0.00, 0.00, 1.00 };  //specular
GLfloat matiral_emission[4] = { 0.00, 0.00, 0.00, 1.00 };  //emission
GLfloat diffuse_material[4] = { 0.5, 0.5, 0.5, 1.0 };
GLfloat light_position[] = { 1.0,1.0,0,0 };;//position of light
GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 1.0 };
const GLfloat matiral_shininess = 11.00;

//set parameters
GLfloat sca[3] = { 1.5, 1.5, 1.5 };
GLfloat tra[3] = { -300, 0, -470 };
GLfloat AngleX;
GLfloat AngleY;
GLfloat bx = 0;
GLfloat by = 0;
GLfloat bz = 0;
GLfloat cx = 0;

//define the angle of view
double rotate_x = 0.0;
double rotate_y = 0.0;
double rotate_z = 0.0;


//define the parameter of snow particles
float slowdown = 2.0;
float velocity = 0.0;
float zoom = -10.0;
float pan = 0.0;
float tilt = 0.0;

int loop;
int fall;

//floor colors
float r = 1.0;
float g = 1.0;
float b = 1.0;
float ground_points[21][21][3];
float ground_colors[21][21][4];
float acc = -10.0;

typedef struct {
	// Life
	bool alive;
	float life;
	float fade;
	// color
	float red1;
	float green1;
	float blue1;
	// Position/direction
	float x_pos;
	float y_pos;
	float z_pos;
	// Velocity/Direction, only goes down in y dir
	float vel;
	// Gravity
	float gravity;
}particle;

// Paticle System
particle particles[MAX_PARTICLES];

// Initialize/Reset Particles - give them their attributes
void initParticles(int i) {
	particles[i].alive = true;
	particles[i].life = 1.0;
	particles[i].fade = float(rand() % 100) / 1000.0f + 0.003f;


	particles[i].x_pos = (float)(rand() % 900)-100;
	particles[i].y_pos = 500.0;
	particles[i].z_pos = (float)(rand() % 900);

	particles[i].red1 = 0.5;
	particles[i].green1 = 0.5;
	particles[i].blue1 = 1.0;

	particles[i].vel = velocity;
	particles[i].gravity = -1.8;

}

// For Snow
void drawSnow() {
	float x, y, z;
	for (loop = 0; loop < MAX_PARTICLES; loop = loop + 2) {
		if (particles[loop].alive == true) {
			x = particles[loop].x_pos;
			y = particles[loop].y_pos;
			z = particles[loop].z_pos + zoom;

			// Draw particles
			glColor3f(1.0, 1.0, 1.0);
			glPushMatrix();
			glTranslatef(x, y, z);
			glutSolidSphere(1.8, 16, 16);
			glPopMatrix();

			// Update values
			//Move
			particles[loop].y_pos += particles[loop].vel / (slowdown * 100);
			particles[loop].vel += particles[loop].gravity;
			// Decay
			particles[loop].life -= particles[loop].fade;

			//Revive
			if (particles[loop].life < 0.0) {
				initParticles(loop);
			}
		}
	}
}

//set for the animation
void idle() {
	glutPostRedisplay();
}

//set the material
void setMetiral(GLfloat matiral_diffuse_ambient[4])
{
	//set clolor
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matiral_diffuse_ambient); 
	//set material
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matiral_specular);  
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, matiral_emission);
	glMaterialf(GL_FRONT_AND_BACK, matiral_shininess, 0);

}

////draw the trapezoid
void cons(double x, double y, double z, double x1, double y1, double z1) {  
	tri[0][0] = x;
	tri[0][1] = y;
	tri[0][2] = z;

	tri[1][0] = x;
	tri[1][1] = y;
	tri[1][2] = z + z1;

	tri[4][0] = x;
	tri[4][1] = y + y1;
	tri[4][2] = z;

	tri[5][0] = x;
	tri[5][1] = y + y1;
	tri[5][2] = z + z1 / 2;
	for (int i = 0; i < 3; i++) {
		if (i == 0) {
			tri[3][i] = tri[0][i] + x1;
			tri[2][i] = tri[1][i] + x1;
			tri[6][i] = tri[4][i] + x1;
			tri[7][i] = tri[5][i] + x1;
		}
		else {
			tri[3][i] = tri[0][i];
			tri[2][i] = tri[1][i];
			tri[6][i] = tri[4][i];
			tri[7][i] = tri[5][i];
		}
	}
}

//draw the rectangular
void constract(double x, double y, double z, double x1, double y1, double z1) { 
	rec[0][0] = x;
	rec[0][1] = y;
	rec[0][2] = z;

	rec[1][0] = x;
	rec[1][1] = y;
	rec[1][2] = z + z1;

	rec[2][0] = x + x1;
	rec[2][1] = y;
	rec[2][2] = z + z1;

	rec[3][0] = x + x1;
	rec[3][1] = y;
	rec[3][2] = z;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			if (j == 1)
				rec[i + 4][j] = rec[i][j] + y1;
			else
				rec[i + 4][j] = rec[i][j];
		}
	}
}

//draw wall1
void buildRecTrapzoid() {
	glBegin(GL_POLYGON);
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(tri[0][0], tri[0][1], tri[0][2]);
	glVertex3f(tri[1][0], tri[1][1], tri[1][2]);
	glVertex3f(tri[2][0], tri[2][1], tri[2][2]);
	glVertex3f(tri[3][0], tri[3][1], tri[3][2]);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(tri[1][0], tri[1][1], tri[1][2]);
	glVertex3f(tri[0][0], tri[0][1], tri[0][2]);
	glVertex3f(tri[4][0], tri[4][1], tri[4][2]);
	glVertex3f(tri[5][0], tri[5][1], tri[5][2]);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(tri[7][0], tri[7][1], tri[7][2]);
	glVertex3f(tri[6][0], tri[6][1], tri[6][2]);
	glVertex3f(tri[3][0], tri[3][1], tri[3][2]);
	glVertex3f(tri[2][0], tri[2][1], tri[2][2]);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(tri[5][0], tri[5][1], tri[5][2]);
	glVertex3f(tri[6][0], tri[6][1], tri[6][2]);
	glVertex3f(tri[2][0], tri[2][1], tri[2][2]);
	glVertex3f(tri[1][0], tri[1][1], tri[1][2]);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(tri[0][0], tri[0][1], tri[0][2]);
	glVertex3f(tri[3][0], tri[3][1], tri[3][2]);
	glVertex3f(tri[7][0], tri[7][1], tri[7][2]);
	glVertex3f(tri[4][0], tri[4][1], tri[4][2]);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(tri[4][0], tri[4][1], tri[4][2]);
	glVertex3f(tri[7][0], tri[7][1], tri[7][2]);
	glVertex3f(tri[6][0], tri[6][1], tri[6][2]);
	glVertex3f(tri[5][0], tri[5][1], tri[5][2]);
	glEnd();
}

//draw wall2
void buildRec() {
	glBegin(GL_POLYGON);
	glNormal3f(0.0, -1.0, 0.0);
	glVertex3f(rec[0][0], rec[0][1], rec[0][2]);
	glVertex3f(rec[1][0], rec[1][1], rec[1][2]);
	glVertex3f(rec[2][0], rec[2][1], rec[2][2]);
	glVertex3f(rec[3][0], rec[3][1], rec[3][2]);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(-1.0, 0.0, 0.0);
	glVertex3f(rec[1][0], rec[1][1], rec[1][2]);
	glVertex3f(rec[0][0], rec[0][1], rec[0][2]);
	glVertex3f(rec[4][0], rec[4][1], rec[4][2]);
	glVertex3f(rec[5][0], rec[5][1], rec[5][2]);
	glEnd(); 

	glBegin(GL_POLYGON);
	glNormal3f(1.0, 0.0, 0.0);
	glVertex3f(rec[7][0], rec[7][1], rec[7][2]);
	glVertex3f(rec[6][0], rec[6][1], rec[6][2]);
	glVertex3f(rec[2][0], rec[2][1], rec[2][2]);
	glVertex3f(rec[3][0], rec[3][1], rec[3][2]);
	glEnd(); 
	glBegin(GL_POLYGON);
	glNormal3f(0.0, 0.0, 1.0);
	glVertex3f(rec[5][0], rec[5][1], rec[5][2]);
	glVertex3f(rec[6][0], rec[6][1], rec[6][2]);
	glVertex3f(rec[2][0], rec[2][1], rec[2][2]);
	glVertex3f(rec[1][0], rec[1][1], rec[1][2]);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(0.0, 0.0, -1.0);
	glVertex3f(rec[0][0], rec[0][1], rec[0][2]);
	glVertex3f(rec[3][0], rec[3][1], rec[3][2]);
	glVertex3f(rec[7][0], rec[7][1], rec[7][2]);
	glVertex3f(rec[4][0], rec[4][1], rec[4][2]);
	glEnd();

	glBegin(GL_POLYGON);
	glNormal3f(0.0, 1.0, 0.0);
	glVertex3f(rec[4][0], rec[4][1], rec[4][2]);
	glVertex3f(rec[7][0], rec[7][1], rec[7][2]);
	glVertex3f(rec[6][0], rec[6][1], rec[6][2]);
	glVertex3f(rec[5][0], rec[5][1], rec[5][2]);
	glEnd();
}

//draw car
void drawCar() {
	glPushMatrix();
	glTranslatef(0 + cx, 30, 650);
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.1, 0.8);
	glVertex3f(-39.0f, 0.0f, 15.0f);
	glVertex3f(39.0f, 0.0f, 15.0f);
	glColor3f(1, 0.5, 0.8);
	glVertex3f(39.0f, -15.0f, 15.0f);
	glVertex3f(-39.0f, -15.0f, 15.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0.3, 0.2, 0.5);
	glVertex3f(39.0f, 0.0f, -15.0f);
	glVertex3f(39.0f, 0.0f, 15.0f);
	glVertex3f(39.0f, -15.0f, 15.0f);
	glVertex3f(39.0f, -15.0f, -15.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0.3, 0.1, 0.3);
	glVertex3f(-39.0f, 0.0f, -15.0f);
	glVertex3f(-39.0f, 0.0f, 15.0f);
	glVertex3f(-39.0f, -15.0f, 15.0f);
	glVertex3f(-39.0f, -15.0f, -15.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0.1, 0.1, 0.8);
	glVertex3f(-39.0f, 0.0f, -15.0f);
	glVertex3f(39.0f, 0.0f, -15.0f);
	glColor3f(1, 0.5, 0.8);
	glVertex3f(39.0f, -15.0f, -15.0f);
	glVertex3f(-39.0f, -15.0f, -15.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0, 0, 1);
	glVertex3f(-39.0f, 0.0f, 15.0f);
	glVertex3f(-39.0f, 0.0f, -15.0f);
	glVertex3f(39.0f, 0.0f, -15.0f);
	glVertex3f(39.0f, 0.0f, 15.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0.8, 0.5, 0.2);
	glVertex3f(-39.0f, -15.0f, 15.0f);
	glVertex3f(-39.0f, -15.0f, -15.0f);
	glVertex3f(39.0f, -15.0f, -15.0f);
	glVertex3f(39.0f, -15.0f, 15.0f);
	glEnd();
	
	glBegin(GL_POLYGON);
	glColor3f(0, 0, 1);
	glVertex3f(-19.0f, 0.0f, 15.0f);
	glVertex3f(-10.0f, 10.0f, 15.0f);
	glVertex3f(19.0f, 10.0f, 15.0f);
	glVertex3f(25.0f, 0.0f, 15.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0, 0, 1);
	glVertex3f(-19.0f, 0.0f, -15.0f);
	glVertex3f(-10.0f, 10.0f, -15.0f);
	glVertex3f(19.0f, 10.0f, -15.0f);
	glVertex3f(25.0f, 0.0f, -15.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0, 1, 1);
	glVertex3f(-10.0f, 10.0f, 15.0f);
	glVertex3f(-10.0f, 10.0f, -15.0f);
	glVertex3f(19.0f, 10.0f, -15.0f);
	glVertex3f(19.0f, 10.0f, 15.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0.5, 0.8, 0.8);
	glVertex3f(-10.0f, 10.0f, 15.0f);
	glVertex3f(-19.0f, 0.0f, 15.0f);
	glVertex3f(-19.0f, 0.0f, -15.0f);
	glVertex3f(-10.0f, 10.0f, -15.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0, 0.5, 0.5);
	glVertex3f(19.0f, 10.0f, 15.0f);
	glVertex3f(19.0f, 10.0f, -15.0f);
	glVertex3f(25.0f, 0.0f, -15.0f);
	glVertex3f(25.0f, 0.0f, 15.0f);
	glEnd();
	glBegin(GL_POLYGON);
	glColor3f(0, 0, 1);
	glVertex3f(-30.0f, -15.0f, 15.0f);
	glVertex3f(-30.0f, -15.0f, -15.0f);
	glVertex3f(30.0f, -15.0f, -15.0f);
	glVertex3f(30.0f, -15.0f, 15.0f);
	glEnd();
	
	glColor3f(1, 0, 0);
	glTranslated(-19.0f, -15.0f, 15.0f);
	glutSolidTorus(2, 5, 5, 100);
	glTranslated(0.0f, 0.0f, -30.0f);
	glutSolidTorus(2, 5, 5, 100);
	glTranslated(45.0f, 0.0f, 0.0f);
	glutSolidTorus(2, 5, 5, 100);
	glTranslated(0.0f, 0.0f, 30.0f);
	glutSolidTorus(2, 5, 5, 100);
	glTranslatef(0 , -30, -650);
	glPopMatrix();
}

//draw house
void drawHouse() {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				glBegin(GL_POLYGON);
				constract(-180 + i * 250, 10 + k * 180, 400 - j * 250, 40, 10, 40);
				glColor3f(gray);
				buildRec();

				glBegin(GL_POLYGON);
				constract(-180 + i * 250, 20, 400 - j * 250, 40, 170, 40);
				glColor3f(1.0, 1.0, 1.0);
				buildRec();

				glBegin(GL_POLYGON);
				constract(-172.5 + i * 250, 200, 407.5 - j * 250, 25, 30, 25);
				glColor3f(gray);
				buildRec();
			}
		}
	}

	glBegin(GL_POLYGON);
	constract(-160, 205, 405, 250, 25, 25);
	glColor3f(gray);
	buildRec();

	glBegin(GL_POLYGON);
	constract(-160, 205, 155, 250, 25, 25);
	glColor3f(gray);
	buildRec();


	glPushMatrix();
	glRotatef(20, 0, 0, 1);
	glBegin(GL_POLYGON);
	constract(-120, 270, 130, 184, 15, 300);
	glColor3f(gray);
	buildRec();
	glPopMatrix();

	glPushMatrix();
	glRotatef(180, 0, 1, 0);
	glPushMatrix();
	glRotatef(20, 0, 0, 1);
	glBegin(GL_POLYGON);
	constract(-50, 245, -430, 184, 15, 300);
	glColor3f(gray);
	buildRec();
	glPopMatrix();
	glPopMatrix();

}

//draw ground
void drawGround() {
	glBegin(GL_POLYGON);
	constract(-200, 0, -200, 1100, 10, 800);
	glColor3f(green);
	buildRec();

	//ground
	glBegin(GL_POLYGON);
	constract(-200, -5, -200, 1100, 1, 900);
	glColor3f(nbrown);
	buildRec();

	//road
	glBegin(GL_POLYGON);
	constract(-200, 0, 600, 1100, 10, 100);
	glColor3f(gray);
	buildRec();

	GLfloat matiral_diffused[4] = { 0.20, 0.32, 0.80, 1.00 };
	setMetiral(matiral_diffused);

	//use texture to graw grass
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texGround);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-200, 11, -200);
	glTexCoord2f(0.0f, 5.0f); glVertex3f(-200, 11, 600);
	glTexCoord2f(5.0f, 5.0f); glVertex3f(900, 11, 600);
	glTexCoord2f(5.0f, 0.0f); glVertex3f(900, 11, -200);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//zebra crossing
	for (int i = 0; i < 11; i++) {
		glBegin(GL_POLYGON);
		constract(i * 100 - 200, 10, 600 + 45, 40, 1, 10);
		glColor3f(1.0, 1.0, 1.0);
		buildRec();
	}

	//road
	for (int i = -5; i < 7; i++)
	{
		glColor3f(hgray);
		constract(235 - i * 15, 10, 250 + i * 40, 40, 2, 20);
		buildRec();
	}
	//sun
	glPushMatrix();
	glColor3f(yellow);
	glTranslatef(800, 500, 400);
	glutSolidSphere(35.0, 20, 20);
	glTranslatef(-700, -400, -300);
	glPopMatrix();
}

//draw fense
void drawFense() {
	//base of fense
	glBegin(GL_POLYGON);
	constract(10, 10, 10, 20, 20, 480);
	glColor3f(1.0, 1.0, 1.0);
	buildRec();
	constract(30, 10, 10, 650, 20, 20);
	glColor3f(1.0, 1.0, 1.0);
	buildRec();
	constract(680, 10, 10, 20, 20, 480);
	glColor3f(1.0, 1.0, 1.0);
	buildRec();
	constract(30, 10, 470, 100, 20, 20);
	glColor3f(1.0, 1.0, 1.0);
	buildRec();
	constract(230, 10, 470, 450, 20, 20);
	glColor3f(1.0, 1.0, 1.0);
	buildRec();
	//column
	glBegin(GL_POLYGON);
	constract(10, 10, 10, 20, 50, 20);
	glColor3f(1.0, 1.0, 1.0);
	buildRec();
	glBegin(GL_POLYGON);
	constract(680, 10, 10, 20, 50, 20);
	glColor3f(1.0, 1.0, 1.0);
	buildRec();
	glBegin(GL_POLYGON);
	constract(230, 10, 470, 20, 50, 20);
	glColor3f(1.0, 1.0, 1.0);
	buildRec();
	glBegin(GL_POLYGON);
	constract(10, 10, 470, 20, 50, 20);
	glColor3f(1.0, 1.0, 1.0);
	buildRec();
	glBegin(GL_POLYGON);
	constract(680, 10, 470, 20, 50, 20);
	glColor3f(1.0, 1.0, 1.0);
	buildRec();
	glBegin(GL_POLYGON);
	constract(110, 10, 470, 20, 50, 20);
	glColor3f(1.0, 1.0, 1.0);
	buildRec();
	//fence
	glColor3f(nbrown);
	for (int i = 35; i < 470; i += 25)
	{
		constract(15, 20, i, 6, 30, 6);
		buildRec();
	}
	for (int i = 35; i < 470; i += 25)
	{
		constract(685, 20, i, 6, 30, 6);
		buildRec();
	}
	for (int i = 35; i < 680; i += 25)
	{
		constract(i, 20, 15, 6, 30, 6);
		buildRec();
	}
	for (int i = 35; i < 110; i += 25)
	{
		constract(i, 20, 480, 6, 30, 6);
		buildRec();
	}
	for (int i = 235; i < 680; i += 25)
	{
		constract(i, 20, 480, 6, 30, 6);
		buildRec();
	}
}

//draw pool
void drawPool() {
	glPushMatrix();

	glTranslatef(-35, 0, 120);
	glScalef(0.9, 0.9, 0.8);
	//pool
	glColor3f(water);
	constract(491, 10, 75, 150, 5, 350);
	buildRec();

	glColor3f(1.0, 1.0, 1.0);
	constract(491, 10, 75, 8, 12, 350);
	buildRec();
	glColor3f(1.0, 1.0, 1.0);
	constract(632, 10, 75, 8, 12, 350);
	buildRec();
	glColor3f(1.0, 1.0, 1.0);
	constract(498, 10, 75, 134, 12, 8);
	buildRec();
	glColor3f(1.0, 1.0, 1.0);
	constract(498, 10, 417, 134, 12, 8);
	buildRec();

	//use texture to draw pool
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texPoor);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(491, 16, 75);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(491, 16, 426);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(640, 16, 426);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(640, 16, 75);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//ball
	glColor4f(orange, 0.8);
	glTranslatef(565 - bx, 25 - by, 250 - bz);
	glutSolidSphere(15, 20, 20);
	glTranslatef(-565, -25, -250);
	
	glPopMatrix();
}

//draw garage
void drawGarage() {
	//base of garage
	glBegin(GL_POLYGON);
	constract(330, 20, 250, 140, 1, 250);
	glColor3f(gray);
	buildRec();
	//wall of garage
	glColor3f(1.0, 1.0, 1.0);
	constract(320, 10, 250, 10, 100, 220);
	buildRec();
	constract(459, 10, 250, 10, 100, 220);
	buildRec();
	//roof of garage
	glColor3f(hgray);
	constract(320, 110, 250, 150, 10, 220);
	buildRec();
}

//draw bighouse
void drawBighouse() {
	//wall
	glColor3f(1.0, 1.0, 1.0);
	constract(50, 10, 50, 12, 150, 197);
	buildRec();
	glColor3f(1.0, 1.0, 1.0);
	constract(438, 10, 50, 12, 150, 197);
	buildRec();
	glColor3f(1.0, 1.0, 1.0);
	constract(62, 10, 50, 376, 150, 12);
	buildRec();
	glColor3f(1.0, 1.0, 1.0);
	constract(62, 10, 235, 376, 35, 12);
	buildRec();
	glColor3f(1.0, 1.0, 1.0);
	constract(62, 45, 235, 50, 65, 12);
	buildRec();
	glColor3f(1.0, 1.0, 1.0);
	constract(200, 45, 235, 238, 65, 12);
	buildRec();
	glColor3f(1.0, 1.0, 1.0);
	constract(62, 110, 235, 376, 50, 12);
	buildRec();

	glColor3f(1.0, 1.0, 1.0);
	constract(220, 160, 50, 230, 130, 180);
	buildRec();

	//floor
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texFloor);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(62, 15, 62);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(62, 15, 235);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(438, 15, 235);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(438, 15, 62);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	//glass
	glColor4f(blue, 0.35);
	constract(62 + 50, 45, 235, 88, 65, 12);
	buildRec();

	//roof
	glColor3f(gray);
	cons(50, 160, 148.5, 170, 27, 110);
	buildRecTrapzoid();
	cons(50, 185, 148.5, 170, 27, 85);
	buildRecTrapzoid();
	cons(50, 210, 148.5, 170, 27, 60);
	buildRecTrapzoid();
	cons(50, 235, 148.5, 170, 27, 35);
	buildRecTrapzoid();

	glColor3f(gray);
	cons(50, 160, 148.5, 170, 27, -110);
	buildRecTrapzoid();
	cons(50, 185, 148.5, 170, 27, -85);
	buildRecTrapzoid();
	cons(50, 210, 148.5, 170, 27, -60);
	buildRecTrapzoid();
	cons(50, 235, 148.5, 170, 27, -35);
	buildRecTrapzoid();

	glColor3f(gray);
	cons(220, 290, 148.5, 230, 27, 110);
	buildRecTrapzoid();
	cons(220, 315, 148.5, 230, 27, 85);
	buildRecTrapzoid();
	cons(220, 340, 148.5, 230, 27, 60);
	buildRecTrapzoid();
	cons(220, 365, 148.5, 230, 27, 35);
	buildRecTrapzoid();

	glColor3f(gray);
	cons(220, 290, 148.5, 230, 27, -110);
	buildRecTrapzoid();
	cons(220, 315, 148.5, 230, 27, -85);
	buildRecTrapzoid();
	cons(220, 340, 148.5, 230, 27, -60);
	buildRecTrapzoid();
	cons(220, 365, 148.5, 230, 27, -35);
	buildRecTrapzoid();


	//door
	glColor3f(door);
	constract(230, 10, 250, 50, 100, 5);
	buildRec();
	glColor3f(doorknob);
	constract(235, 60, 255, 5, 5, 5);
	buildRec();
}

//draw snowman
void drawSnowman() {
	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(750, 60, 300);
	glutSolidSphere(60, 20, 20);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(750, 150, 300);
	glutSolidSphere(40, 20, 20);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0, 0, 0);
	glTranslatef(730, 148, 325);
	glutSolidSphere(10, 20, 20);
	glPopMatrix();

	glPushMatrix();
	glColor3f(0, 0, 0);
	glTranslatef(770, 148, 325);
	glutSolidSphere(10, 20, 20);
	glPopMatrix();

	glPushMatrix();
	glColor3f(197/255.0, 75/255.0, 56/255.0);
	glTranslatef(750, 180, 300);
	glRotatef(-90, 1, 0, 0);
	glutSolidCone(25, 50, 20, 20);
	glPopMatrix();
}
//draw tree
void drawTree() {
	//trunk
	glColor3f(brown);
	constract(60, 10, 390, 15, 70, 15);
	buildRec();
	//leaves
	glColor3f(twhite);
	glTranslatef(60, 70, 400);
	glutSolidSphere(25.0, 20, 20);

	glTranslatef(20, 0, 0);
	glutSolidSphere(25.0, 20, 20);

	glTranslatef(-10, 0, -10);
	glutSolidSphere(25.0, 20, 20);

	glTranslatef(0, 0, 20);
	glutSolidSphere(25.0, 20, 20);

	glTranslatef(0, 10, -10);
	glutSolidSphere(25.0, 20, 20);
	glTranslatef(-70, -80, -400);
}

//draw table
void drawTable() {
	glPushMatrix();
	glColor3f(hgray);
	constract(-100, 50, 350, 100, 10, 100);
	buildRec();
	glPopMatrix();

	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			glPushMatrix();
			glColor3f(hgray);
			constract(-100 + 90*i, 0, 350 + 90 *j, 10, 50, 10);
			buildRec();
			glPopMatrix();
		}
	}

	glPushMatrix();
	glColor3f(orange);
	glTranslatef(-45, 70, 400);
	glutSolidTeapot(15);
	glPopMatrix();
}
//draw flag
void drawFlag() {
	glPushMatrix();
	constract(660, 10, 420, 5, 360, 5);
	buildRec();
	glPopMatrix();
	GLfloat ctrlpoints[5][5][3] = { {{657,410,419.5},{672,410,439.5},{687,410,419.5},{702,410,399.5},{717,410,419.5}},
	{{657,400,419.5},{672,400,439.5},{687,400,419.5},{702,400,399.5},{717,400,419.5}} ,
	{{657,390,419.5},{672,390,439.5},{687,390,419.5},{702,390,399.5},{717,390,419.5}} ,
	{{657,380,419.5},{672,380,439.5},{687,380,419.5},{702,380,399.5},{717,380,419.5}} ,
	{{657,370,419.5},{672,370,439.5},{687,370,419.5},{702,370,399.5},{717,370,419.5}} };
	glColor3f(0.5, 1, 1);
	glPushMatrix();
	
	glEnable(GL_MAP2_VERTEX_3);
	glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 5, 0, 1, 15, 5, &ctrlpoints[0][0][0]);
	glMapGrid2f(10.0, 0.0, 1.0, 10.0, 0.0, 1.0);
	glEvalMesh2(GL_FILL, 0, 10.0, 0, 10.0);
	glPopMatrix();
}

//draw lamp
void drawLamp() {
	glPushMatrix();
	glColor3f(hgray);
	constract(160, 10, 320, 10, 100, 10);
	buildRec();
	glPopMatrix();

	glPushMatrix();
	glColor3f(nbrown);
	constract(150, 80, 310, 30, 40, 30);
	buildRec();
	glPopMatrix();

	glPushMatrix();
	glColor3f(hgray);
	constract(200, 10, 270, 10, 60, 10);
	buildRec();
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(205, 80, 275);
	glutSolidSphere(15, 25, 25);
	glPopMatrix();


	glPushMatrix();
	glColor3f(hgray);
	constract(200, 10, 200, 10, 100, 10);
	buildRec();
	glPopMatrix();

	glPushMatrix();
	glColor3f(nbrown);
	constract(190, 80, 190, 30, 40, 30);
	buildRec();
	glPopMatrix();

	glPushMatrix();
	glColor3f(hgray);
	constract(220, 10, 150, 10, 60, 10);
	buildRec();
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(225, 80, 155);
	glutSolidSphere(15, 25, 25);
	glPopMatrix();

	glPushMatrix();
	glColor3f(hgray);
	constract(240, 10, 100, 10, 100, 10);
	buildRec();
	glPopMatrix();

	glPushMatrix();
	glColor3f(nbrown);
	constract(230, 80, 90, 30, 40, 30);
	buildRec();
	glPopMatrix();

	glPushMatrix();
	glColor3f(hgray);
	constract(260, 10, 50, 10, 60, 10);
	buildRec();
	glPopMatrix();

	glPushMatrix();
	glColor3f(1, 1, 1);
	glTranslatef(265, 80, 55);
	glutSolidSphere(15, 25, 25);
	glPopMatrix();
}
//draw houses
void drawHouses() {
	glPushMatrix();
	glTranslatef(0, 0, 100);
	drawHouse();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(300, 0, -200);
	drawBighouse();
	glPopMatrix();


	glPushMatrix();
	glTranslatef(425, 0, -450);
	drawGarage();
	glPopMatrix();

	glPushMatrix();
	glScalef(0.7, 0.7, 0.7);
	glTranslatef(550, 0, 250);
	drawFense();
	glPopMatrix();
}

//draw trees
void drawTrees() {
	glPushMatrix();
	glTranslatef(-350, -20, -1200);
	glScalef(3, 3, 3);
	drawTree();
	glPopMatrix();

	glPushMatrix();
	glScalef(2, 2, 2);
	glTranslatef(-80, 0, -400);
	drawTree();
	glPopMatrix();

	glPushMatrix();
	glScalef(2.5, 2.5, 2.5);
	glTranslatef(0, 0, -400);
	drawTree();
	glPopMatrix();
}

//display function
void display(void)
{
	int i, j;
	float x, y, z;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(rotate_x, 1.0, 0.0, 0.0);
	glRotatef(rotate_y, 0.0, 1.0, 0.0);
	glRotatef(rotate_z, 0.0, 0.0, 1.0);

	glScalef(sca[0], sca[1], sca[2]);
	glTranslatef(tra[0], tra[1], tra[2]);

	drawCar();
	drawSnow();
	drawGround();
	drawHouses();
	drawSnowman();
	drawPool();
	drawTrees();
	drawFlag();
	drawLamp();
	drawTable();
	
	

	glFlush();
	glutSwapBuffers();
}

//init function to initialize some parameters
void init(void)
{
	GLfloat sun_direction[] = { 700.0, 400.0, 100.0, 1.0 };
	GLfloat sun_intensity[] = { sunshine, 1.0 };
	GLfloat ambient_intensity[] = { 0.5, 0.5, 0.5, 1.0 };

	glEnable(GL_LIGHTING);              // Set up ambient light.
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_intensity);

	glEnable(GL_LIGHT0);                // Set up sunlight.
	glLightfv(GL_LIGHT0, GL_POSITION, sun_direction);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, sun_intensity);

	glEnable(GL_COLOR_MATERIAL);        // Configure glColor().
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	cout << "The OpenGL version is: " << glGetString(GL_VERSION) << "\n";
	cout << glGetString(GL_VENDOR) << "\n";
	glLineWidth(5);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_BLEND); 
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
	glEnable(GL_DEPTH_TEST); //enable depth test

	glMatrixMode(GL_PROJECTION);     //places the camera at (0,0,0) and faces it along(0,0,-1).
	glOrtho(-w, w, -h, h, -w, w); // specify clipping volume

	int x, z;

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);

	// Ground Verticies
	  // Ground Colors
	for (z = 0; z < 21; z++) {
		for (x = 0; x < 21; x++) {
			ground_points[x][z][0] = x - 10.0;
			ground_points[x][z][1] = acc;
			ground_points[x][z][2] = z - 10.0;

			ground_colors[z][x][0] = r; // red value
			ground_colors[z][x][1] = g; // green value
			ground_colors[z][x][2] = b; // blue value
			ground_colors[z][x][3] = 0.0; // acummulation factor
		}
	}

	// Initialize particles
	for (loop = 0; loop < MAX_PARTICLES; loop++) {
		initParticles(loop);
	}
}

int flip = 1;
//set timer to let ball flow automatically
void timerFunction(int value)
{
	if (flip == 1 ) {
		by = by + 2;
		flip++;
	}
	else if (flip == 2) {
		by = by - 2;
		flip--;
	}
	if (cx <= 600) {
		cx += 20;
	}
	else if (cx > 600) {
		cx -= 600;
	}
	else {
		cout << "Wrong!\n";
	}

	glutPostRedisplay();
	glutTimerFunc(200, timerFunction, 1);
}


//set keys to move the scene, ball.
void specialKeys(int key, int x, int y) {

	//move the ball
	if (key == GLUT_KEY_F1)
		bx -= 1;
	else if (key == GLUT_KEY_F2)
		bx += 1;
	else if (key == GLUT_KEY_F3)
		bz += 1;
	else if (key == GLUT_KEY_F4)
		bz -= 1;

	//roate
	if (key == GLUT_KEY_RIGHT)
		rotate_y -= 1;
	else if (key == GLUT_KEY_LEFT)
		rotate_y += 1;
	else if (key == GLUT_KEY_DOWN)
		rotate_x -= 1;
	else if (key == GLUT_KEY_UP)
		rotate_x += 1;

	//translate
	if (key == GLUT_KEY_F7)
		tra[0] -= 1;
	else if (key == GLUT_KEY_F8)
		tra[0] += 1;
	else if (key == GLUT_KEY_F9)
		tra[1] += 1;
	else if (key == GLUT_KEY_F10)
		tra[1] -= 1;

	//zoom
	if (key == GLUT_KEY_F5) {
		for (int i = 0; i < 3; i++)
			sca[i] = sca[i] + 0.1;
	}
	else if (key == GLUT_KEY_F6) {
		for (int i = 0; i < 3; i++)
			sca[i] = sca[i] - 0.1;
	}

	//
	glutPostRedisplay();
}

//set mouse to move the scene
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		mouseDown = true;
		AngleX = x - rotate_y;
		AngleY = -y + rotate_x;
	}
	else
		mouseDown = false;
}

//rotate
void mouseMotion(int x, int y) {
	if (mouseDown) {
		rotate_y = x - AngleX;
		rotate_x = y + AngleY;

		glutPostRedisplay();
	}
}

//justify the size of pictures
int power_of_two(int n)
{
	if (n <= 0)
		return 0;
	return (n & (n - 1)) == 0;
}

//load the texture
GLuint loadTexture(const char* file_name)
{
	GLint width, height, total_bytes;
	GLubyte* pixels = 0;
	GLuint last_texture_ID = 0, texture_ID = 0;
	FILE* pFile;
	errno_t err;

	if ((err = fopen_s(&pFile, file_name, "rb")) != 0)
	{
		printf("can not open this file\n");
		exit(0);
	}

	fseek(pFile, 0x0012, SEEK_SET);
	fread(&width, 4, 1, pFile);
	fread(&height, 4, 1, pFile);
	fseek(pFile, BMP_Header_Length, SEEK_SET);

	{
		GLint line_bytes = width * 3;
		while (line_bytes % 4 != 0)
			++line_bytes;
		total_bytes = line_bytes * height;
	}

	pixels = (GLubyte*)malloc(total_bytes);
	if (pixels == 0)
	{
		fclose(pFile);
		return 0;
	}

	if (fread(pixels, total_bytes, 1, pFile) <= 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	{
		GLint max;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max);
		if (!power_of_two(width)
			|| !power_of_two(height)
			|| width > max
			|| height > max)
		{
			const GLint new_width = 256;
			const GLint new_height = 256;
			GLint new_line_bytes, new_total_bytes;
			GLubyte* new_pixels = 0;

			new_line_bytes = new_width * 3;
			while (new_line_bytes % 4 != 0)
				++new_line_bytes;
			new_total_bytes = new_line_bytes * new_height;

			new_pixels = (GLubyte*)malloc(new_total_bytes);
			if (new_pixels == 0)
			{
				free(pixels);
				fclose(pFile);
				return 0;
			}

			gluScaleImage(GL_RGB,
				width, height, GL_UNSIGNED_BYTE, pixels,
				new_width, new_height, GL_UNSIGNED_BYTE, new_pixels);

			free(pixels);
			pixels = new_pixels;
			width = new_width;
			height = new_height;
		}
	}

	glGenTextures(1, &texture_ID);
	if (texture_ID == 0)
	{
		free(pixels);
		fclose(pFile);
		return 0;
	}

	// bind the texture
	GLint lastTextureID = last_texture_ID;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureID);
	glBindTexture(GL_TEXTURE_2D, texture_ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
		GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
	glBindTexture(GL_TEXTURE_2D, lastTextureID);
	free(pixels);
	return texture_ID;
}

//main function
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(w, h);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("House in a snow day");
	init();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_TEXTURE_2D);    // enable texture
	texGround = loadTexture("snow.bmp");  //load texture
	texPoor = loadTexture("pool.bmp");
	texFloor = loadTexture("floor.bmp");

	glutDisplayFunc(display);
	glutSpecialFunc(specialKeys);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutTimerFunc(200, timerFunction, 1);//timer for ball and car
	glutIdleFunc(idle);

	glutMainLoop();
	system("pause");
	return 0;
}

