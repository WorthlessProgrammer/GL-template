#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WIDTH 690
#define HEIGHT 480

#define WIN_NAME "Shader - Fractal"

#define ASSERT(x) if (!(x)) exit(1);
#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall());

void GLClearError()
{
	while (glGetError() != GL_NO_ERROR) printf("1\n");
}

bool GLLogCall()
{
	GLenum error;
	while ((error = glGetError()))
	{
		fprintf(stderr, "[GL ERROR]: (%u) \n", error);
		return false;
	}
	return true;
}

const char* vertex_source = "./shaders/vertex.shader"; 
const char* fragment_source = "./shaders/fragment.shader"; 

size_t file_size(FILE *fs)
{
	unsigned int size;

	if (!fs) {
		fprintf(stderr, "ERROR: No file provided");
		return 0;
	} 

	fseek(fs, 0, SEEK_END);
	size = ftell(fs);

	fseek(fs, 0, SEEK_SET);

	return size;
}

char* parse_code_to_cstr(const char* file_path)
{
	char *buffer;
	FILE *fs;
	size_t fsize;

	if (!file_path) {
		return NULL;
	} 

	fs = fopen(file_path, "r");

	if (!fs) {
		fprintf(stderr, "ERROR: couldn't open file <%s>\n", file_path);
		return NULL;
	} 

	fsize = file_size(fs);

	if (fsize == 0)
		return NULL;

	buffer = malloc(fsize + 1);
	fread(buffer, fsize, 1, fs);
	buffer[fsize] = '\0';

	fclose(fs);
	return buffer;
}

static unsigned int compile_shader(unsigned int type, const char *source)
{
	unsigned int id;
	int result;

	id = glCreateShader(type);
	glShaderSource(id, 1, &source, NULL);
	glCompileShader(id);

	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		char message[512];
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		glGetShaderInfoLog(id, length, &length, message);
		fprintf(stdout, "%s\n", message);
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static unsigned int create_shader(const char *vertexShader, const char *fragmentShader)
{
	unsigned int program, 
				 vs, 
				 fs;

	char *v = parse_code_to_cstr(vertexShader);
	char *f = parse_code_to_cstr(fragmentShader);

	program = glCreateProgram();
	vs = compile_shader(GL_VERTEX_SHADER, v);
	fs = compile_shader(GL_FRAGMENT_SHADER, f);

	free(v);
	free(f);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);
	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit()) {
        fprintf(stderr, "ERROR: Couldn't load glfw3");
		return -1;
	}

	/* Set OpenGL version 3 Core*/
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	
	/* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WIDTH,
							  HEIGHT, 
							  WIN_NAME, 
							  NULL, 
							  NULL);
    if (!window) {
        glfwTerminate();
        fprintf(stderr, "ERROR: Couldn't load glfw3");
		return -1;
    }
	
	int gl_ver_major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
    int gl_ver_minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
    printf("OpenGL %d.%d\n", gl_ver_major, gl_ver_minor);

	/* Allow for input in the window */
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

	/* Allow usage for modern openGL */
	GLenum error = glewInit();

	if (error != GLEW_OK) {
		fprintf(stderr, "ERROR: Couldn't load GLEW");
	}

	/* float a = 0.5f; */
	/* float l = 0.525f/2; */
	/* float c = hypotf(a, l); */ 

	float hex_vertices[] = { 
		/* 0.0f, 0.0f, 0.0f, 0.0f, */
		/* -l,   a,    0.0f, 0.0f, */
		/* l ,   a,    0.0f, 0.0f, */
		/* c ,   0.0f, 0.0f, 0.0f, */
		/* l ,   -a,   0.0f, 0.0f, */
		/* -l,   -a,   0.0f, 0.0f, */
		/* -c,   0.0f, 0.0f, 0.0f, */
		/* -c,   a,    0.0f, 0.0f, */ 
		/*  c,   a,    1.0f, 0.0f, */
		/* -c,   -a,   1.0f, 1.0f, */
		/*  c,   -a,   0.0f, 1.0f */   
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // Top-left 
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Top-right 
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // Bottom-right	
		-0.5f, -0.5f, 1.0f, 1.0f, 1.0f  // Bottom-left
	};

	unsigned int indexes[] = {
		0, 1, 2,
		0, 2, 3 
		/* 0, 1, 2, */
		/* 0, 2, 3, */ 
		/* 0, 3, 4, */
		/* 0, 4, 5, */
		/* 0, 5, 6, */
		/* 0, 6, 1 */
	};

	/* Sync framerate with screen refresh rate 60MHz */
	glfwSwapInterval(1);

	/* glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); */
	/* glEnable(GL_BLEND); */

	/* Generate Vertex Array Object*/
	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Generate vertex buffer data*/
	unsigned int buffer; 
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*5, hex_vertices, GL_STATIC_DRAW);

	/* Necesitas "activar" los pointers para cada atributo*/ 
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	/* Bind VAO and Vertex Buffer */
	glVertexAttribPointer(0, 
						  2, 
						  GL_FLOAT, 
						  GL_FALSE, 
						  sizeof(float)*5, 
						  0);

	glVertexAttribPointer(1, 
						  3, 
						  GL_FLOAT, 
						  GL_FALSE, 
						  sizeof(float)*5, 
						  0);
		
	/* Specify Vertex Buffer Data Layout */
	unsigned int ibo; 
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*3*2, indexes, GL_STATIC_DRAW);

	unsigned int shader;
	shader = create_shader(vertex_source, fragment_source);
	glUseProgram(shader);

	GLint u_col;
	u_col = glGetUniformLocation(shader, "u_color");
	
	glBindVertexArray(0);
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	/* float dt = 0.2f; */ 

	/* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {	 
		/* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
	
		glBindVertexArray(vao);
			
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

		glUseProgram(shader);
		/* dt += 0.02f; */
		/* glUniform4f(u_col, */ 	
		/* 			fabsf(cosf(dt)), */
		/* 			fabsf(sinf(dt)), */
		/* 			fabsf(sinf(1/M_PI - dt)), */ 
		/* 			1.0f); */
		/* glUniform4f(u_col, */ 	
		/* 			1.0f, */
		/* 			1.0f, */
		/* 			1.0f, */ 
		/* 			1.0f); */

		/* glActiveTexture(GL_TEXTURE0); */
		/* glBindTexture(GL_TEXTURE_2D, 0); */

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

		switch(glfwGetKey(window, GLFW_KEY_Q)) { 
			case GLFW_PRESS:
				glfwSetWindowShouldClose(window, GLFW_TRUE);	
				break;
			default:
				break;
		}
 
		/* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

	glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}
