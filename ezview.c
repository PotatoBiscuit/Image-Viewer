#define GLFW_DLL 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>


GLFWwindow* window;
int line = 1;

typedef struct {
  float position[3];
  float color[4];
} Vertex;


const Vertex Vertices[] = {
  {{1, -1, 0}, {1, 0, 0, 1}},
  {{1, 1, 0}, {0, 1, 0, 1}},
  {{-1, 1, 0}, {0, 0, 1, 1}},
  {{-1, -1, 0}, {0, 0, 0, 1}}
};


const GLubyte Indices[] = {
  0, 1, 2,
  2, 3, 0
};


char* vertex_shader_src =
  "attribute vec4 Position;\n"
  "attribute vec4 SourceColor;\n"
  "\n"
  "varying vec4 DestinationColor;\n"
  "\n"
  "void main(void) {\n"
  "    DestinationColor = SourceColor;\n"
  "    gl_Position = Position;\n"
  "}\n";


char* fragment_shader_src =
  "varying lowp vec4 DestinationColor;\n"
  "\n"
  "void main(void) {\n"
  "    gl_FragColor = DestinationColor;\n"
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


static void error_callback(int error, const char* description) {
  fputs(description, stderr);
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

GLubyte*** read_p3_file(FILE* ppm){
	double width, height, alpha;
	int i, j;
	GLubyte*** texture_pixels;
	
	skip_comts_ws(ppm);
	width = next_number(ppm);
	
	texture_pixels = malloc(sizeof(GLubyte**) * width);
	
	skip_comts_ws(ppm);
	height = next_number(ppm);
	
	skip_comts_ws(ppm);
	if((alpha = next_number(ppm)) != 255){
		fprintf(stderr, "Error: Incorrect alpha value at line %d", line);
		exit(1);
	}
	skip_comts_ws(ppm);
	
	for(i = 0; i < width; i++){
		texture_pixels[i] = malloc(sizeof(GLubyte*) * height);
		for(j = 0; j < height; j++){
			texture_pixels[i][j] = malloc(sizeof(GLubyte) * 3);
			texture_pixels[i][j][0] = (int) next_number(ppm);
			skip_ws(ppm);
			texture_pixels[i][j][1] = (int) next_number(ppm);
			skip_ws(ppm);
			texture_pixels[i][j][2] = (int) next_number(ppm);
			if(i == width - 1 && j == height - 1) continue;
			skip_ws(ppm);
		}
	}
	return texture_pixels;
}

GLubyte*** read_p6_file(FILE* ppm){
	double width, height, alpha;
	int i, j, c;
	GLubyte*** texture_pixels;
	
	skip_comts_ws(ppm);
	width = next_number(ppm);
	
	texture_pixels = malloc(sizeof(GLubyte**) * width);
	
	skip_comts_ws(ppm);
	height = next_number(ppm);
	
	skip_comts_ws(ppm);
	if((alpha = next_number(ppm)) != 255){
		fprintf(stderr, "Error: Incorrect alpha value at line %d", line);
		exit(1);
	}
	if(!isspace(next_c(ppm))){
		fprintf(stderr, "Error: There must be one whitespace after the alpha field, line %d", line);
		exit(1);
	}
	
	for(i = 0; i < width; i++){
		texture_pixels[i] = malloc(sizeof(GLubyte*) * height);
		for(j = 0; j < height; j++){
			texture_pixels[i][j] = malloc(sizeof(GLubyte) * 3);
			texture_pixels[i][j][0] = next_c(ppm);
			texture_pixels[i][j][1] = next_c(ppm);
			texture_pixels[i][j][2] = next_c(ppm);
		}
	}
	return texture_pixels;
}

GLubyte*** read_ppm_file(char* inputName){
	GLubyte*** texture_pixels;
	FILE* inputFile = fopen(inputName, "rb");
	skip_comts_ws(inputFile);
	expect_c(inputFile, 'P');
	int c = next_c(inputFile);
	if(c == '3'){
		texture_pixels = read_p3_file(inputFile);
	}else if(c == '6'){
		texture_pixels = read_p6_file(inputFile);
	}else{
		fprintf(stderr, "Error: Incorrect ppm file number on line %d", line);
		exit(1);
	}
	fclose(inputFile);
	return texture_pixels;
}

int main(int argc, char** argv) {
	GLubyte*** texture_pixels;
	texture_pixels = read_ppm_file(argv[1]);

	/*GLint program_id, position_slot, color_slot;
	GLuint vertex_buffer;
	GLuint index_buffer;

	glfwSetErrorCallback(error_callback);

	// Initialize GLFW library
	if (!glfwInit())
	return -1;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create and open a window
	window = glfwCreateWindow(640,
							480,
							"Hello World",
							NULL,
							NULL);

	if (!window) {
	glfwTerminate();
	printf("glfwCreateWindow Error\n");
	exit(1);
	}

	glfwMakeContextCurrent(window);

	program_id = simple_program();

	glUseProgram(program_id);

	position_slot = glGetAttribLocation(program_id, "Position");
	color_slot = glGetAttribLocation(program_id, "SourceColor");
	glEnableVertexAttribArray(position_slot);
	glEnableVertexAttribArray(color_slot);

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
	while (!glfwWindowShouldClose(window)) {

	glClearColor(0, 104.0/255.0, 55.0/255.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, 640, 480);

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

	glDrawElements(GL_TRIANGLES,
				   sizeof(Indices) / sizeof(GLubyte),
				   GL_UNSIGNED_BYTE, 0);

	glfwSwapBuffers(window);
	glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);*/
}
