#define GLFW_DLL 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>
#include <stdio.h>


GLFWwindow* window;
mat4x4 mvp;
float angle = 0, width, height;
int line = 1;

typedef struct{
	float position[3];
	float color[4];
	float texcoord[2];
} Vertex;

typedef struct{
	GLubyte* texture_pixels;
	double width;
	double height;
} Triple;

const Vertex Vertices[] = {
  {{-1, 1, 0}, {1, 1, 1, 0}, {0, 0}},
  {{1, 1, 0}, {1, 1, 1, 0}, {1, 0}},
  {{1, -1, 0}, {0, 0, 1, 0}, {1, 1}},
  {{-1, -1, 0}, {0, 0, 0, 0}, {0, 1}}
};


const GLubyte Indices[] = {
  0, 1, 2,
  2, 3, 0
};

char* vertex_shader_src =
  "uniform mat4 MVP;\n"
  "attribute vec3 Position;\n"
  "attribute vec4 SourceColor;\n"
  "attribute vec2 TexCoordIn;\n"
  "varying vec2 TexCoordOut;\n"
  "\n"
  "varying vec4 DestinationColor;\n"
  "\n"
  "void main(void) {\n"
  "    DestinationColor = SourceColor;\n"
  "    gl_Position = MVP * vec4(Position, 1.0);\n"
  "    TexCoordOut = TexCoordIn;\n"
  "}\n";

char* fragment_shader_src =
  "varying lowp vec4 DestinationColor;\n"
  "varying lowp vec2 TexCoordOut;\n"
  "uniform sampler2D Texture;\n"
  
  "\n"
  "void main(void) {\n"
  "    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
  "}\n";


GLint simple_shader(GLint shader_type, char* shader_src) {

  GLint compile_success = 0;

  int shader_id = glCreateShader(shader_type);

  glShaderSource(shader_id, 1, &shader_src, 0);

  glCompileShader(shader_id);

  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);

  if (compile_success == GL_FALSE) {
    GLchar message[256];
    glGetShaderInfoLog(shader_id, sizeof(message), 0, &message[0]);
    printf("glCompileShader Error: %s\n", message);
    exit(1);
  }

  return shader_id;
}


int simple_program() {

  GLint link_success = 0;

  GLint program_id = glCreateProgram();
  GLint vertex_shader = simple_shader(GL_VERTEX_SHADER, vertex_shader_src);
  GLint fragment_shader = simple_shader(GL_FRAGMENT_SHADER, fragment_shader_src);

  glAttachShader(program_id, vertex_shader);
  glAttachShader(program_id, fragment_shader);

  glLinkProgram(program_id);

  glGetProgramiv(program_id, GL_LINK_STATUS, &link_success);

  if (link_success == GL_FALSE) {
    GLchar message[256];
    glGetProgramInfoLog(program_id, sizeof(message), 0, &message[0]);
    printf("glLinkProgram Error: %s\n", message);
    exit(1);
  }

  return program_id;
}

void rotate_matrix(float add_angle){
	float ratio;
	int width, height;
	mat4x4 m, p, temp;
	
	glfwGetFramebufferSize(window, &width, &height);
	ratio = width / (float) height;
	
	mat4x4_identity(m);
	
	mat4x4_rotate_Z(m, m, -angle);
	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(temp, p, m);
	mat4x4_mul(mvp, temp, mvp);
	
	
	mat4x4_identity(m);
	
	angle -= add_angle;
	mat4x4_rotate_Z(m, m, angle);
	mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	mat4x4_mul(temp, p, m);
	mat4x4_mul(mvp, temp, mvp);
}

void scale_matrix(float scale_index){
	mat4x4 scale_matrix = {
		{scale_index, 0.f, 0.f, 0.f},
		{0.f,   scale_index,   0.f, 0.f},
		{0.f,  0.f,   1.f, 0.f},
		{0.f, 0.f, 0.f, 1.f}
	};
	mat4x4_mul(mvp, scale_matrix, mvp);
}

void translate_matrix(float x, float y){
	mat4x4 translate_matrix = {
		{1.f, 0.f, 0.f, 0.f},
		{0.f, 1.f, 0.f, 0.f},
		{0.f, 0.f, 1.f, 0.f},
		{x, y, 0.f, 1.f}
	};
	mat4x4_mul(mvp, translate_matrix, mvp);
}

void shear_matrix(float change_xy, float change_yx){
	mat4x4 shear_matrix = {
		{1.f, change_yx, 0.f, 0.f},
		{change_xy, 1.f, 0.f, 0.f},
		{0.f, 0.f, 1.f, 0.f},
		{0.f, 0.f, 0.f, 1.f}
	};
	mat4x4_mul(mvp, shear_matrix, mvp);
}

static void error_callback(int error, const char* description) {
  fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
	
	//Keypress for rotations
	if(key == GLFW_KEY_Q && action == GLFW_PRESS)
		rotate_matrix(25);
	if(key == GLFW_KEY_W && action == GLFW_PRESS)
		rotate_matrix(-25);
	if(key == GLFW_KEY_Q && action == GLFW_REPEAT)
		rotate_matrix(-.1);
	if(key == GLFW_KEY_W && action == GLFW_REPEAT)
		rotate_matrix(.1);
	
	//Keypress for scaling
	if(key == GLFW_KEY_A && action == GLFW_PRESS)
		scale_matrix(.9);
	if(key == GLFW_KEY_S && action == GLFW_PRESS)
		scale_matrix(1.1);
	if(key == GLFW_KEY_A && action == GLFW_REPEAT)
		scale_matrix(.97);
	if(key == GLFW_KEY_S && action == GLFW_REPEAT)
		scale_matrix(1.03);
	
	if(key == GLFW_KEY_UP && action == GLFW_PRESS)
		translate_matrix(0, .1);
	if(key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		translate_matrix(0, -.1);
	if(key == GLFW_KEY_UP && action == GLFW_REPEAT)
		translate_matrix(0, .04);
	if(key == GLFW_KEY_DOWN && action == GLFW_REPEAT)
		translate_matrix(0, -.04);
	if(key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		translate_matrix(-.1,0);
	if(key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		translate_matrix(.1, 0);
	if(key == GLFW_KEY_LEFT && action == GLFW_REPEAT)
		translate_matrix(-.04, 0);
	if(key == GLFW_KEY_RIGHT && action == GLFW_REPEAT)
		translate_matrix(.04, 0);
	
	if(key == GLFW_KEY_Z && action == GLFW_PRESS)
		shear_matrix(-.1, 0);
	if(key == GLFW_KEY_X && action == GLFW_PRESS)
		shear_matrix(.1, 0);
	if(key == GLFW_KEY_Z && action == GLFW_REPEAT)
		shear_matrix(-.04, 0);
	if(key == GLFW_KEY_X && action == GLFW_REPEAT)
		shear_matrix(.04, 0);
	if(key == GLFW_KEY_C && action == GLFW_PRESS)
		shear_matrix(0, -.1);
	if(key == GLFW_KEY_V && action == GLFW_PRESS)
		shear_matrix(0, .1);
	if(key == GLFW_KEY_C && action == GLFW_REPEAT)
		shear_matrix(0, -.04);
	if(key == GLFW_KEY_V && action == GLFW_REPEAT)
		shear_matrix(0, .04);
}

// next_c() wraps the getc() function and provides error checking and line
// number maintenance
int next_c(FILE* ppm) {
  int c = fgetc(ppm);
#ifdef DEBUG
  printf("next_c: '%c'\n", c);
#endif
  if (c == '\n') {
    line += 1;
  }
  if (c == EOF) {
    fprintf(stderr, "Error: Unexpected end of file on line number %d.\n", line);
    exit(1);
  }
  return c;
}


// expect_c() checks that the next character is d.  If it is not it emits
// an error.
void expect_c(FILE* ppm, int d) {
  int c = next_c(ppm);
  if (c == d) return;
  fprintf(stderr, "Error: Expected '%c' on line %d.\n", d, line);
  exit(1);    
}


//Skips the whitespace in a file
int skip_ws(FILE* ppm) {
	int c = next_c(ppm);
	if(!isspace(c)){
		ungetc(c, ppm);
		return 0;
	}
	while (isspace(c)) {
		c = next_c(ppm);
	}
	ungetc(c, ppm);
	return 1;
}

//Skips the comments in the file
int skip_comments(FILE* ppm){
	int c = next_c(ppm);
	if(c == '#'){
		while((c = next_c(ppm)) != '\n');
		return 1;
	}
	ungetc(c, ppm);
	return 0;
}

//Skips all comments and whitespace
void skip_comts_ws(FILE* ppm){
	while(skip_ws(ppm) || skip_comments(ppm));
}

double next_number(FILE* json) {	//Parse the next number and return it as a double
	double value;
	int numDigits = 0;
	numDigits = fscanf(json, "%lf", &value);
	if(numDigits == 0){
		fprintf(stderr, "Error: Expected number at line %d\n", line);
		exit(1);
	}
	return value;
}

Triple* read_p3_file(FILE* ppm){
	double width, height, alpha;
	Triple* texture_struct = malloc(sizeof(Triple));
	int i, j;
	GLubyte* texture_pixels;
	
	skip_comts_ws(ppm);
	width = next_number(ppm);
	
	skip_comts_ws(ppm);
	height = next_number(ppm);
	
	texture_struct->width = width;
	texture_struct->height = height;
	texture_pixels = malloc(sizeof(GLubyte) * width * height * 3);
	
	skip_comts_ws(ppm);
	if((alpha = next_number(ppm)) != 255){
		fprintf(stderr, "Error: Incorrect alpha value at line %d", line);
		exit(1);
	}
	skip_comts_ws(ppm);
	
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			texture_pixels[(int)(j + height * i) * 3] = (int) next_number(ppm);
			skip_ws(ppm);
			texture_pixels[(int)(j + height * i) * 3 + 1] = (int) next_number(ppm);
			skip_ws(ppm);
			texture_pixels[(int)(j + height * i) * 3 + 2] = (int) next_number(ppm);
			if(i == width - 1 && j == height - 1) continue;
			skip_ws(ppm);
		}
	}
	texture_struct->texture_pixels = texture_pixels;
	return texture_struct;
}

Triple* read_p6_file(FILE* ppm){
	double width, height, alpha;
	Triple* texture_struct = malloc(sizeof(Triple));
	int i, j, c;
	GLubyte* texture_pixels;
	
	skip_comts_ws(ppm);
	width = next_number(ppm);
	
	skip_comts_ws(ppm);
	height = next_number(ppm);
	
	texture_struct->width = width;
	texture_struct->height = height;
	texture_pixels = malloc(sizeof(GLubyte) * width * height * 3);
	
	skip_comts_ws(ppm);
	if((alpha = next_number(ppm)) != 255){
		fprintf(stderr, "Error: Incorrect alpha value at line %d", line);
		exit(1);
	}
	if(!isspace(next_c(ppm))){
		fprintf(stderr, "Error: There must be one whitespace after the alpha field, line %d", line);
		exit(1);
	}
	
	for(i = 0; i < height; i++){
		for(j = 0; j < width; j++){
			texture_pixels[(int)(j + height * i) * 3] = next_c(ppm);
			texture_pixels[(int)(j + height * i) * 3 + 1] = next_c(ppm);
			texture_pixels[(int)(j + height * i) * 3 + 2] = next_c(ppm);
		}
	}
	texture_struct->texture_pixels = texture_pixels;
	return texture_struct;
}

Triple* read_ppm_file(char* inputName){
	Triple* texture_struct;
	FILE* inputFile = fopen(inputName, "rb");
	skip_comts_ws(inputFile);
	expect_c(inputFile, 'P');
	int c = next_c(inputFile);
	if(c == '3'){
		texture_struct = read_p3_file(inputFile);
	}else if(c == '6'){
		texture_struct = read_p6_file(inputFile);
	}else{
		fprintf(stderr, "Error: Incorrect ppm file number on line %d", line);
		exit(1);
	}
	fclose(inputFile);
	return texture_struct;
}

int main(int argc, char** argv) {
	Triple* texture_struct;
	int i, j;
	int width, height;
	texture_struct = read_ppm_file(argv[1]);

	GLint program_id, position_slot, color_slot, texture_slot, mvp_slot, mvpt_slot;
	GLuint vertex_buffer;
	GLuint index_buffer;
	GLint textureUniform;

	// Initialize GLFW library
	if (!glfwInit())
		return -1;

	glfwSetErrorCallback(error_callback);
	
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create and open a window
	window = glfwCreateWindow(500,
							500,
							"Hello World",
							NULL,
							NULL);

	if (!window) {
		glfwTerminate();
		printf("glfwCreateWindow Error\n");
		exit(1);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	
	//Texture Setup -----------------------------
	GLuint myTexture;
	glGenTextures(1, &myTexture);
	glBindTexture(GL_TEXTURE_2D, myTexture);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,
					0, //No level of detail
					GL_RGB, //FORMAT.. GL_RGB
					texture_struct->width,
					texture_struct->height,
					0, //No border
					GL_RGB,
					GL_UNSIGNED_BYTE, //Whatever your numeric representation is
					texture_struct->texture_pixels);

	//-------------------------------------
	program_id = simple_program();

	glUseProgram(program_id);

	mvp_slot = glGetUniformLocation(program_id, "MVP");
	position_slot = glGetAttribLocation(program_id, "Position");
	color_slot = glGetAttribLocation(program_id, "SourceColor");
	texture_slot = glGetAttribLocation(program_id, "TexCoordIn");
	glEnableVertexAttribArray(position_slot);
	glEnableVertexAttribArray(color_slot);
	glEnableVertexAttribArray(texture_slot);
	
	textureUniform = glGetUniformLocation(program_id, "Texture");

	
	
	// Create Buffer
	glGenBuffers(1, &vertex_buffer);

	// Map GL_ARRAY_BUFFER to this buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	// Send the data
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
	
	// Repeat
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width,  height);
	mat4x4_identity(mvp);
	while (!glfwWindowShouldClose(window)) {
		
		glClearColor(0, 104.0/255.0, 55.0/255.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);


		glVertexAttribPointer(position_slot,
							  3,
							  GL_FLOAT,
							  GL_FALSE,
							  sizeof(Vertex),
							  0);

		glVertexAttribPointer(color_slot,
							  4,
							  GL_FLOAT,
							  GL_FALSE,
							  sizeof(Vertex),
							  (GLvoid*) (sizeof(float) * 3));
		glVertexAttribPointer(texture_slot,
							  2,
							  GL_FLOAT,
							  GL_FALSE,
							  sizeof(Vertex),
							  (GLvoid*) (sizeof(float) * 7));
							  
							
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, myTexture);
		glUniform1i(textureUniform, 0);
		
        glUniformMatrix4fv(mvp_slot, 1, GL_FALSE, (const GLfloat*) mvp);
		
		glDrawElements(GL_TRIANGLES,
					   sizeof(Indices) / sizeof(GLubyte),
					   GL_UNSIGNED_BYTE, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//-------------------------------------
	
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}
