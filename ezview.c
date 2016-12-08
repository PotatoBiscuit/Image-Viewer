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

typedef struct{		//This struct holds the coordinates for the object we will attach our texture to
	float position[3];
	float color[4];
	float texcoord[2];
} Vertex;

typedef struct{		//This struct holds texture width, height, and pixel information
	GLubyte* texture_pixels;
	double width;
	double height;
} Triple;

typedef struct{		//This struct holds shader variables for future use
	GLint position_slot;
	GLint color_slot;
	GLint texture_slot;
	GLint mvp_slot;
	GLint textureUniform;
} VariableArray;

const Vertex Vertices[] = {	//This array holds our object coordinates, color, and texture coordinates
  {{-1, 1, 0}, {1, 1, 1, 0}, {0, 0}},
  {{1, 1, 0}, {1, 1, 1, 0}, {1, 0}},
  {{1, -1, 0}, {0, 0, 1, 0}, {1, 1}},
  {{-1, -1, 0}, {0, 0, 0, 0}, {0, 1}}
};


const GLubyte Indices[] = {
  0, 1, 2,
  2, 3, 0
};

char* vertex_shader_src =	//This is our vertex shader info
  "uniform mat4 MVP;\n"
  "attribute vec3 Position;\n"
  "attribute vec4 SourceColor;\n"
  "attribute vec2 TexCoordIn;\n"
  "varying vec2 TexCoordOut;\n"
  "\n"
  "varying vec4 DestinationColor;\n"
  "\n"
  "void main(void) {\n"
  "    DestinationColor = SourceColor;\n"	//Carry color over to next shader
  "    gl_Position = MVP * vec4(Position, 1.0);\n"	//Apply transformations to position coordinates
  "    TexCoordOut = TexCoordIn;\n"					//Retain texture coordinates over shader
  "}\n";

char* fragment_shader_src =		//This is our fragment shader info
  "varying lowp vec4 DestinationColor;\n"
  "varying lowp vec2 TexCoordOut;\n"
  "uniform sampler2D Texture;\n"
  
  "\n"
  "void main(void) {\n"
  "    gl_FragColor = texture2D(Texture, TexCoordOut);\n"	//Cast our texture to the correct coordinates
  "}\n";


GLint simple_shader(GLint shader_type, char* shader_src) {	//Create simple shader, error check

  GLint compile_success = 0;

  int shader_id = glCreateShader(shader_type);	//Create shader

  glShaderSource(shader_id, 1, &shader_src, 0);	//Add information from earlier into shader

  glCompileShader(shader_id);	//Compile shader

  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);	//See if the shader was compiled

  if (compile_success == GL_FALSE) {
    GLchar message[256];
    glGetShaderInfoLog(shader_id, sizeof(message), 0, &message[0]);
    printf("glCompileShader Error: %s\n", message);
    exit(1);
  }

  return shader_id;
}


int simple_program() {	//Create simple program for OpenGL to use

  GLint link_success = 0;

  GLint program_id = glCreateProgram();	//Create program
  //Create shaders
  GLint vertex_shader = simple_shader(GL_VERTEX_SHADER, vertex_shader_src);
  GLint fragment_shader = simple_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
  
  //Attach shaders
  glAttachShader(program_id, vertex_shader);
  glAttachShader(program_id, fragment_shader);

  glLinkProgram(program_id);	//Link program

  glGetProgramiv(program_id, GL_LINK_STATUS, &link_success);	//See if program link was successful

  if (link_success == GL_FALSE) {
    GLchar message[256];
    glGetProgramInfoLog(program_id, sizeof(message), 0, &message[0]);
    printf("glLinkProgram Error: %s\n", message);
    exit(1);
  }

  return program_id;
}

void rotate_matrix(float add_angle){	//Add rotation properties to our transformation matrix (mvp)
	
	mat4x4 m;
	
	//Undo previous rotation
	mat4x4_identity(m);
	mat4x4_rotate_Z(m, m, -angle);
	mat4x4_mul(mvp, mvp, m);	//Multiply tranformation matrix to rotation matrix to apply properties
	
	//Perform new rotation with new calculated angle
	mat4x4_identity(m);
	
	angle -= add_angle;
	mat4x4_rotate_Z(m, m, angle);
	mat4x4_mul(mvp, mvp, m);	//Multiply tranformation matrix to rotation matrix to apply properties
}

void scale_matrix(float scale_index){	//Add scaling property to our tranformation matrix (mvp)
	mat4x4 scale_matrix = {	//Our scaling matrix
		{scale_index, 0.f, 0.f, 0.f},
		{0.f,   scale_index,   0.f, 0.f},
		{0.f,  0.f,   1.f, 0.f},
		{0.f, 0.f, 0.f, 1.f}
	};
	mat4x4_mul(mvp, mvp, scale_matrix);	//Multiply tranformation matrix to apply scaling locally
}

void translate_matrix(float x, float y){	//Add translation property to our tranformation matrix (mvp)
	mat4x4 translate_matrix = {	//Our translation matrix
		{1.f, 0.f, 0.f, 0.f},
		{0.f, 1.f, 0.f, 0.f},
		{0.f, 0.f, 1.f, 0.f},
		{x, y, 0.f, 1.f}
	};
	mat4x4_mul(mvp, translate_matrix, mvp);	//Multiply our translation matrix to transformation matrix to apply
											//properties globally
}

void shear_matrix(float change_xy, float change_yx){	//Add shear property to our transformation matrix (mvp)
	mat4x4 shear_matrix = {	//Our shear matrix
		{1.f, change_yx, 0.f, 0.f},
		{change_xy, 1.f, 0.f, 0.f},
		{0.f, 0.f, 1.f, 0.f},
		{0.f, 0.f, 0.f, 1.f}
	};
	mat4x4_mul(mvp, mvp, shear_matrix);	//Multiply tranform. matrix to shear matrix to apply properties locally
}

static void error_callback(int error, const char* description) {	//Print errors that occur
  fputs(description, stderr);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){	//Listen for keypresses
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)	//Escape to quit functionality
        glfwSetWindowShouldClose(window, GLFW_TRUE);
	
	//Keypress for rotations
	if(key == GLFW_KEY_Q && action == GLFW_PRESS)	//Single keypress
		rotate_matrix(25);
	if(key == GLFW_KEY_W && action == GLFW_PRESS)
		rotate_matrix(-25);
	if(key == GLFW_KEY_Q && action == GLFW_REPEAT)	//Held keypress
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
	
	//Keypress for translation
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
	
	//Keypress for shearing
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

void set_window_hints(){	//Tell compiler how we should be using OpenGL
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2); //Maximum version is version 2
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);	//Minimum version is version 0
}

GLuint new_texture(Triple* texture_struct){	//Put our image into a texture
	//Texture Setup -----------------------------
	GLuint myTexture;
	glGenTextures(1, &myTexture);	//Create new texture
	glBindTexture(GL_TEXTURE_2D, myTexture);	//Bind texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set type of texture filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//GL_LINEAR is used because pretty
	glTexImage2D(GL_TEXTURE_2D,		//Add information to our texture
					0, //No level of detail
					GL_RGB, //FORMAT.. GL_RGB
					texture_struct->width,
					texture_struct->height,
					0, //No border
					GL_RGB,
					GL_UNSIGNED_BYTE, //Whatever your numeric representation is
					texture_struct->texture_pixels);	//Our pixel information

	return myTexture;	//Return texture descriptor
	//-------------------------------------
}

void bind_buffer(){	//Create new buffer, bind, and send it
	GLuint vertex_buffer;
	GLuint index_buffer;
	// Create Buffer
	glGenBuffers(1, &vertex_buffer);

	// Map GL_ARRAY_BUFFER to this buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	// Send the data
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}

VariableArray* get_shader_variables(GLint program_id){	//Retrieve shader variable locations
	VariableArray* our_variables = malloc(sizeof(VariableArray));
	our_variables->mvp_slot = glGetUniformLocation(program_id, "MVP");
	if(our_variables->mvp_slot == -1){	//If variable does not exist in shader, throw error
		fprintf(stderr, "Error: Could not find MVP matrix");
		exit(1);
	}
	our_variables->position_slot = glGetAttribLocation(program_id, "Position");
	if(our_variables->position_slot == -1){
		fprintf(stderr, "Error: Could not find position vector");
		exit(1);
	}
	our_variables->color_slot = glGetAttribLocation(program_id, "SourceColor");
	if(our_variables->color_slot == -1){
		fprintf(stderr, "Error: Could not find color vector");
		exit(1);
	}
	our_variables->texture_slot = glGetAttribLocation(program_id, "TexCoordIn");
	if(our_variables->texture_slot == -1){
		fprintf(stderr, "Error: Could not find texture coordinates");
		exit(1);
	}
	
	glEnableVertexAttribArray(our_variables->position_slot);	//Enable attribute variables
	glEnableVertexAttribArray(our_variables->color_slot);
	glEnableVertexAttribArray(our_variables->texture_slot);
	
	our_variables->textureUniform = glGetUniformLocation(program_id, "Texture");
	if(our_variables->textureUniform == -1){
		fprintf(stderr, "Error: Could not find texture uniform");
		exit(1);
	}
	return our_variables;	//Return struct of all variable locations
}

Triple* read_p3_file(FILE* ppm){	//Read p3 file and store in GLubyte array
	double width, height, alpha;
	Triple* texture_struct = malloc(sizeof(Triple));
	int i, j;
	GLubyte* texture_pixels;
	
	skip_comts_ws(ppm);	//Skip comments and whitespace at the beginning of the file
	width = next_number(ppm);	//Grab width value
	
	skip_comts_ws(ppm);	//Skip more comments and whitespace
	height = next_number(ppm);	//Grab height value
	
	texture_struct->width = width;	//Store width and height into our struct
	texture_struct->height = height;
	texture_pixels = malloc(sizeof(GLubyte) * width * height * 3);
	
	skip_comts_ws(ppm);	//You know what this does
	if((alpha = next_number(ppm)) != 255){	//Grab alpha value, make sure it is valid
		fprintf(stderr, "Error: Incorrect alpha value at line %d", line);
		exit(1);
	}
	skip_comts_ws(ppm);
	
	for(i = 0; i < height; i++){	//Iterate through file and store pixel info into GLubyte array
		for(j = 0; j < width; j++){
			texture_pixels[(int)(j + width * i) * 3] = (int) next_number(ppm);
			skip_ws(ppm);
			texture_pixels[(int)(j + width * i) * 3 + 1] = (int) next_number(ppm);
			skip_ws(ppm);
			texture_pixels[(int)(j + width * i) * 3 + 2] = (int) next_number(ppm);
			if(i == width - 1 && j == height - 1) continue;
			skip_ws(ppm);
		}
	}
	texture_struct->texture_pixels = texture_pixels;	//Store GLubyte array into struct
	return texture_struct;	//return struct
}

Triple* read_p6_file(FILE* ppm){	//Read p6 file and store in GLubyte array
	double width, height, alpha;
	Triple* texture_struct = malloc(sizeof(Triple));
	int i, j, c;
	GLubyte* texture_pixels;
	
	skip_comts_ws(ppm);	//Skip comments and whitespace
	width = next_number(ppm);	//Grab width value
	
	skip_comts_ws(ppm);
	height = next_number(ppm);	//Grab height value
	
	texture_struct->width = width;	//Store width and height values into struct
	texture_struct->height = height;
	texture_pixels = malloc(sizeof(GLubyte) * width * height * 3);
	
	skip_comts_ws(ppm);
	if((alpha = next_number(ppm)) != 255){	//Grab alpha value and error check it
		fprintf(stderr, "Error: Incorrect alpha value at line %d", line);
		exit(1);
	}
	if(!isspace(next_c(ppm))){	//There must be exactly one whitespace between header and raw info
		fprintf(stderr, "Error: There must be one whitespace after the alpha field, line %d", line);
		exit(1);
	}
	
	for(i = 0; i < height; i++){	//Iterate through file and store pixel info into GLubyte array
		for(j = 0; j < width; j++){
			texture_pixels[(int)(j + width * i) * 3] = next_c(ppm);
			texture_pixels[(int)(j + width * i) * 3 + 1] = next_c(ppm);
			texture_pixels[(int)(j + width * i) * 3 + 2] = next_c(ppm);
		}
	}
	
	texture_struct->texture_pixels = texture_pixels;	//Store GLubyte array into struct
	return texture_struct;	//return struct
}

Triple* read_ppm_file(char* inputName){	//Figure out type of file, and calle read_p3_file or read_p6_file
	Triple* texture_struct;
	FILE* inputFile = fopen(inputName, "rb");	//Open input file
	if(inputFile == NULL){	//If file does not exist, throw error
		fprintf(stderr, "Error: File does not exist\n");
		exit(1);
	}
	skip_comts_ws(inputFile);	//Skip comments and whitespace
	expect_c(inputFile, 'P');	//Expect a P
	int c = next_c(inputFile);	//Get next magic number
	if(c == '3'){				//If three, call our p3 function
		texture_struct = read_p3_file(inputFile);
	}else if(c == '6'){			//If six, call our p6 function
		texture_struct = read_p6_file(inputFile);
	}else{						//Else, invalid file
		fprintf(stderr, "Error: Incorrect ppm file number on line %d", line);
		exit(1);
	}
	fclose(inputFile);	//Close file
	return texture_struct;	//Return struct containing image information
}

int main(int argc, char** argv) {	//Execute our program
	Triple* texture_struct;
	VariableArray* our_variables;
	int i, j;
	int width, height, new_width, new_height;
	GLint program_id, mvp_slot, position_slot, color_slot, texture_slot, textureUniform;
	GLuint myTexture;
	texture_struct = read_ppm_file(argv[1]);	//Read and retrieve pixel information

	// Initialize GLFW library
	if (!glfwInit())
		return -1;

	glfwSetErrorCallback(error_callback);	//Initialize error callback function
	
	set_window_hints();	//Set OpenGL settings

	width = texture_struct->width;
	height = texture_struct->height;
	while(width < 1400 && height < 750){
		width += texture_struct->width/2;
		height += texture_struct->height/2;
	}
	width -= texture_struct->width/2;
	height -= texture_struct->height/2;
	// Create and open a window
	window = glfwCreateWindow(width,
							height,
							"Hello World",
							NULL,
							NULL);

	if (!window) {	//If window was not opened, close program
		glfwTerminate();
		printf("glfwCreateWindow Error\n");
		exit(1);
	}

	glfwSetKeyCallback(window, key_callback);	//Initialize key listener
	glfwMakeContextCurrent(window);				//Make window current
	
	//Texture Setup -----------------------------
	myTexture = new_texture(texture_struct);

	program_id = simple_program();	//Set up program

	glUseProgram(program_id);	//Use program
	
	our_variables = get_shader_variables(program_id);	//Get shader variable locations
	
	bind_buffer();	//Create, bind, and send buffer
	
	mat4x4_identity(mvp);	//Create new transformation array, that starts as an identity matrix
	
	glVertexAttribPointer(our_variables->position_slot,	//Send position information to vertex shader
							  3,
							  GL_FLOAT,
							  GL_FALSE,
							  sizeof(Vertex),
							  0);
							  
	glVertexAttribPointer(our_variables->color_slot,	//Send color information to vertex shader
							  4,
							  GL_FLOAT,
							  GL_FALSE,
							  sizeof(Vertex),
							  (GLvoid*) (sizeof(float) * 3));
	glVertexAttribPointer(our_variables->texture_slot,	//Send texture information to vertex shader
							  2,
							  GL_FLOAT,
							  GL_FALSE,
							  sizeof(Vertex),
							  (GLvoid*) (sizeof(float) * 7));
	
	glActiveTexture(GL_TEXTURE0);	//Make texture active
	glBindTexture(GL_TEXTURE_2D, myTexture);	//Bind texture to window
	glUniform1i(our_variables->textureUniform, 0);	//Get ready to use texture information retrieved from fragment shader
	
	glfwGetFramebufferSize(window, &width, &height);	//Get size of window
	glViewport(0, 0, width,  height);	//Set Viewport size, and set it to size of window
	while (!glfwWindowShouldClose(window)) {
		
		glClearColor(0, 104.0/255.0, 55.0/255.0, 1.0);	//Clear window color
		glClear(GL_COLOR_BUFFER_BIT);
							  					
        glUniformMatrix4fv(our_variables->mvp_slot, 1, GL_FALSE, (const GLfloat*) mvp);	//Send transform. matrix to vertex shader
		
		glDrawElements(GL_TRIANGLES,	//Draw everything
					   sizeof(Indices) / sizeof(GLubyte),
					   GL_UNSIGNED_BYTE, 0);

		glfwSwapBuffers(window);	//Display buffer of stuff drawn
		glfwPollEvents();			//Listen for keypress or error events
	}
	//-------------------------------------
	
	glfwDestroyWindow(window);	//Destroy window
	glfwTerminate();			//Terminate program
	exit(EXIT_SUCCESS);			//Exit with a cheerful heart
}
