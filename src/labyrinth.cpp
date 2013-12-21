#include "include/Angel.h"
#include "cube.h"
#include "ball.h"
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <algorithm>

using namespace std;
using namespace Angel;

const int TIMERMSECS = 100;

//mouse state variables
int last_mx = 0, last_my = 0, cur_mx = 0, cur_my = 0;
int arcball_on = false;

GLfloat screen_width = 600;
GLfloat screen_height = 600;

GLuint program;
GLuint vao;

GLuint P_loc, M_loc, V_loc, W_loc; // locations pointers of P, M, V, W matrices in shader

mat4 I_mat = mat4(1.0); // Identity matrix
mat4 M_mat = mat4(1.0); // Model matrix
mat4 V_mat = mat4(1.0); // View matrix
mat4 W_mat = mat4(1.0); // World matrix (rotation of cube by mouse)
mat4 P_mat = mat4(1.0); // projection matrix

//vec3 eye(2,10,-10);
vec3 eye(1, 10, 0);

// test cube for EPIC RADO :D :D
cube *test_cube;

const int lvl_width = 7;
const int lvl_height = 7;

GLuint cube_width = 1;
GLuint cube_height = 1;
GLuint uniform_tex_sampler;
float xangle = 0;
float zangle = 0;
char map[7][7] = { { '#', '#', '#', '#', '#', '#', '#' }, { '#', '.', '.', '#',
		'.', '.', '#' }, { '#', '.', '.', '#', '.', '.', '#' }, { '#', '.', '#',
		'#', '#', '#', '#' }, { '#', '.', '.', '#', '.', '.', '#' }, { '#', '#',
		'.', '.', '.', '.', '#' }, { '#', '#', '#', '#', '#', '#', '#' } };

//char map[1][1] = {{'#'}};

cube * walls[lvl_height][lvl_width];
ball * _ball;
//==================
//openGL functions
//==================

inline void create_buffer(GLuint* vbo, size_t pts_size, const GLvoid * pts,
		size_t color_size, const GLvoid * color) {
	// example code to be replaced with project customized code
	glGenBuffers(2, vbo); // generate buffers position, color
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, pts_size, pts, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

	//color
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, color_size, color, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
}

void build_lvl() {
	for (int i = 0; i < lvl_height; ++i) {
		for (int j = 0; j < lvl_width; ++j) {
			if (map[i][j] == '#') { //wall
				walls[i][j] = new cube(program, 0);
				walls[i][j]->translation = Translate(
						(-1.0 * lvl_width / 2 + j) * cube_width, 0,
						(-1.0 * lvl_height / 2 + i) * cube_height);
			} else if (map[i][j] == '.') {	// empty
				//nothing to do here :P
			}
		}
	}
	_ball = new ball(program, 0.3, 1, 1);
	_ball->translation = Translate(
			(-1.0 * lvl_width / 2 + _ball->i) * cube_width, 0,
			(-1.0 * lvl_height / 2 + _ball->j) * cube_height);
}

void init_buffers() {
// Initializing  VAOs and VBOs
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	test_cube = new cube(program, 0);
}

void init(void) {

// Load shaders and use the resulting shader program
	program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);
	build_lvl();
	init_buffers();

	V_mat = LookAt(eye, vec3(0, 0, 0), vec3(0, 1, 0));
	P_mat = Perspective(45.0f, 1.0f * screen_width / screen_height, 1.0f,
			100.0f);

// matrices location in shader
	P_loc = glGetUniformLocation(program, "P");
	V_loc = glGetUniformLocation(program, "V");
	M_loc = glGetUniformLocation(program, "M");
	W_loc = glGetUniformLocation(program, "W");

	glUniformMatrix4fv(P_loc, 1, GL_TRUE, P_mat);
	glUniformMatrix4fv(V_loc, 1, GL_TRUE, V_mat);
	glUniformMatrix4fv(M_loc, 1, GL_TRUE, M_mat);
	glUniformMatrix4fv(W_loc, 1, GL_TRUE, W_mat);

	glClearColor(1, 1, 1, 1.0); // grey background :/ not sure about the color

	glEnable(GL_DEPTH_TEST); // Enable depth test
	glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one

	uniform_tex_sampler = glGetUniformLocation(program, "tex_sampler");
	glUniform1i(uniform_tex_sampler, /*GL_TEXTURE*/0);

}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the window//

	glEnableVertexAttribArray(vao);
	glBindVertexArray(vao);

// update rotation and translation matrices of each object before uploading to shader
//	glUniformMatrix4fv(M_loc, 1, GL_TRUE, M_mat);
//	test_cube->draw();

	for (int i = 0; i < lvl_height; ++i) {
		for (int j = 0; j < lvl_width; ++j) {
			if (map[i][j] == '#') { //wall
				M_mat = walls[i][j]->translation * walls[i][j]->rotation;
				glUniformMatrix4fv(M_loc, 1, GL_TRUE, M_mat);
				walls[i][j]->draw();
			} else if (map[i][j] == '.') { // empty
				//nothing to do here :P
			}
		}
	}
	M_mat = _ball->translation;
	glUniformMatrix4fv(M_loc, 1, GL_TRUE, M_mat);
	_ball->draw();
	glDisableVertexAttribArray(vao);
	glutSwapBuffers(); // Double buffering
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 033: //esc
		cout << "exit" << endl;
		exit(EXIT_SUCCESS);
		break;
	}
}

void keyboard_special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_RIGHT:
		break;
	case GLUT_KEY_LEFT:
		break;
	case GLUT_KEY_DOWN:
		break;
	case GLUT_KEY_UP:
		break;
	}
}

void onPassiveMotion(int x, int y) {
}

void onReshape(int width, int height) {
	screen_width = width;
	screen_height = height;
	glViewport(0, 0, screen_width, screen_height);
	P_mat = Perspective(45.0f, 1.0f * screen_width / screen_height, 1.0f,
			100.0f);
	glUniformMatrix4fv(P_loc, 1, GL_FALSE, P_mat);
}

vec3 get_arcball_vector(int x, int y) { // what is the purpose of this method?
//	convert the x,y screen coordinates to [-1,1] coordinates (and reverse y coordinates)
	vec3 P = vec3(1.0 * x / screen_width * 2 - 1.0,
			1.0 * y / screen_height * 2 - 1.0, 0);
	P.y = -P.y;

	float OP_squared = P.x * P.x + P.y * P.y;
// use Pythagorean theorem to get P.z
	if (OP_squared <= 1 * 1)
		P.z = sqrt(1 * 1 - OP_squared);  // Pythagore
	else
		P = normalize(P);  // nearest point

	return P;
}

void onMouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		arcball_on = true;
		last_mx = cur_mx = x;
		last_my = cur_my = y;
	} else {
		arcball_on = false;
	}
}

void onMotion(int x, int y) {
	if (arcball_on) {  // if left button is pressed
		cur_mx = x;
		cur_my = y;

		if (cur_mx != last_mx || cur_my != last_my) {
			xangle += (last_mx - cur_mx) / 5.0;
			zangle += (last_my - cur_my) / 5.0;
			if (abs(xangle) > 30)
				xangle -= (last_mx - cur_mx) / 5.0;
			if (abs(zangle) > 30)
				zangle -= (last_my - cur_my) / 5.0;

			W_mat = RotateX(xangle) * RotateZ(zangle);
//			cout << last_mx - cur_mx << "  " << last_my - cur_my << endl;
			last_mx = cur_mx;
			last_my = cur_my;

			glUniformMatrix4fv(W_loc, 1, GL_TRUE, W_mat);
			glutPostRedisplay();
		}
	}
}
void animate(int n) {
	mat4 tmp = _ball->translation;
	tmp *= Translate(-zangle / 100.0, 0, 0);
	vec4 pos = tmp * vec3(0, 0, 0);
	pos[0] += lvl_width / 2.0;
	pos[2] += lvl_height / 2.0;
	cout << pos[2] << " " << pos[0] << endl;
	if (map[(int) (pos[2] + 0.3)][(int) (pos[0] + 0.3)] == '#'
			|| map[(int) ceil(pos[2] - 0.3)][(int) ceil(pos[0] - 0.3)] == '#') {
		tmp = _ball->translation;
		cout << "xx " << (int) pos[2] << " " << (int) pos[0] << endl;
	}
	mat4 tt = tmp;
	tmp *= Translate(0, 0, xangle / 100.0);
	pos = tmp * vec3(0, 0, 0);
	pos[0] += lvl_height / 2.0;
	pos[2] += lvl_width / 2.0;
//	cout << pos[2] << " " << pos[0] << endl;
	if (map[(int) (pos[2] + 0.3)][(int) (pos[0] + 0.3)] == '#'
			|| map[(int) ceil(pos[2] - 0.3)][(int) ceil(pos[0] - 0.3)] == '#') {
		tmp = tt;
	}
	_ball->translation = tmp;
	display();
	glutTimerFunc(TIMERMSECS, animate, 0);
}
//=========
//main loop
//=========

int main(int argc, char **argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize(screen_width, screen_height);

//----------------------------------------
// If you are using freeglut, the next two lines will check if
// the code is truly 3.2. Otherwise, comment them out

//glutInitContextVersion( 3, 2 );
//glutInitContextProfile( GLUT_CORE_PROFILE );
//----------------------------------------

	glutCreateWindow("Labyrinth :D");
	glewInit();
	init();
	glutTimerFunc(TIMERMSECS, animate, 0);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard_special);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);
	glutReshapeFunc(onReshape);

	glutMainLoop();
	return 0;
}

