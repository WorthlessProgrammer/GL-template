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

const char* vertex_source = "./shaders/vertex.shader"; 
const char* fragment_source = "./shaders/fragment.shader";

#define PROGRAM_COUNT 1

typedef struct {
	GLFWwindow *window;
	unsigned int vao;
	unsigned int vbo;
	unsigned int ibo;
	unsigned int texture;
	unsigned int programs[PROGRAM_COUNT];
} Context;

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

void init_program(Context *render, size_t program_slot)
{
	if (program_slot >= PROGRAM_COUNT)
		return;
	render->programs[program_slot] = glCreateProgram();
}

void attach_shader_to_prog(Context *render, size_t program_slot, 
			   const char *shder_src_file, unsigned int stype)
{	
	if (program_slot >= PROGRAM_COUNT)
		return;

	char *source = parse_code_to_cstr(shder_src_file);
	if (!source)
		return;

	unsigned int shader_id;
	shader_id = compile_shader(stype, source);

	free(source);

	glAttachShader(render->programs[program_slot], shader_id);
	glLinkProgram(render->programs[program_slot]); //Obscure maybe read about this function
	glValidateProgram(render->programs[program_slot]);
	glDeleteShader(shader_id);
}

void init_renderer(Context *render)
{
	if (!render->window)
		return;
	
	/* float a = 0.5f; */
	/* float l = 0.525f/2; */
	/* float c = hypotf(a, l); */ 

	float hex_vertices[] = { 
		/* 0.0f, 0.0f,  */
		/* -l,   a,     */
		/* l ,   a,     */
		/* c ,   0.0f,  */
		/* l ,   -a,    */
		/* -l,   -a,    */
		/* -c,   0.0f,  */
		-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Top-left 
		 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, // Top-right 
		 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // Bottom-right	
		-0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f   // Bottom-left
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
	glGenVertexArrays(1, &render->vao);
	glBindVertexArray(render->vao);

	/* Generate vertex buffer data*/ 
	glGenBuffers(1, &render->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, render->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*7, hex_vertices, GL_STATIC_DRAW);

	/* Necesitas "activar" los pointers para cada atributo*/ 
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	/* Bind VAO and Vertex Buffer */
	glVertexAttribPointer(0, 
			      2, 
			      GL_FLOAT, 
			      GL_FALSE, 
			      sizeof(float)*7, 
			      0);

	glVertexAttribPointer(1, 
			      3, 
			      GL_FLOAT, 
			      GL_FALSE, 
			      sizeof(float)*7, 
			      (void *) (2 * sizeof(float)));

	glVertexAttribPointer(2,
				  2,
				  GL_FLOAT,
				  GL_FALSE,
				  sizeof(float)*7,
				  (void *) (5 * sizeof(float)));

	/* Specify Vertex Buffer Data Layout */
	glGenBuffers(1, &render->ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render->ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*3*2, indexes, GL_STATIC_DRAW);
}

void get_texture(const char* file_path, unsigned int* tex_id)
{

	int x, y, n;
	unsigned char *img = stbi_load(file_path, &x, &y, &n, 4);

	if (!img) {
		fprintf(stderr, "ERROR: Couldn't read file <%s>", file_path);
		return;
	} else {
		printf("img read => %d x %d, %d\n", x, y, n);
	}
	
	glGenTextures(1, tex_id);	
	glBindTexture(GL_TEXTURE_2D, *tex_id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
	/* glGenerateMipmap(GL_TEXTURE_2D); */

	STBI_FREE(img);
}

void delete_programs(Context *render) 
{
	for (size_t i = 0; i < PROGRAM_COUNT; i++)
		glDeleteProgram(render->programs[i]);
}

void unbind_all()
{
	glBindVertexArray(0);
	glUseProgram(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

int main(void)
{
	Context render;
	
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
    	render.window = glfwCreateWindow(WIDTH,
					 HEIGHT, 
					 WIN_NAME, 
					 NULL, 
					 NULL);
    	if (!render.window) {
        	glfwTerminate();
        	fprintf(stderr, "ERROR: Couldn't load glfw3");
		return -1;
    	}
	
	int gl_ver_major = glfwGetWindowAttrib(render.window, GLFW_CONTEXT_VERSION_MAJOR);
    	int gl_ver_minor = glfwGetWindowAttrib(render.window, GLFW_CONTEXT_VERSION_MINOR);
    	printf("OpenGL %d.%d\n", gl_ver_major, gl_ver_minor);

	/* Allow for input in the render.window */
	glfwSetInputMode(render.window, GLFW_STICKY_KEYS, GLFW_TRUE);

    	/* Make the render.window's context current */
    	glfwMakeContextCurrent(render.window);

	/* Allow usage for modern openGL */
	GLenum error = glewInit();

	if (error != GLEW_OK) {
		fprintf(stderr, "ERROR: Couldn't load GLEW");
	}

	init_renderer(&render);
	get_texture("C_Logo.jpg", &render.texture);

	init_program(&render, 0);
	attach_shader_to_prog(&render, 0, vertex_source, GL_VERTEX_SHADER);
	attach_shader_to_prog(&render, 0, fragment_source, GL_FRAGMENT_SHADER);

	/* GLint u_col; */
	/* u_col = glGetUniformLocation(shader, "u_color"); */
	
	unbind_all();
	
	glBindTexture(GL_TEXTURE_2D, render.texture);

	/* Loop until the user closes the render.window */
 	while (!glfwWindowShouldClose(render.window)) {	 

		glClear(GL_COLOR_BUFFER_BIT);
	
		glBindVertexArray(render.vao);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render.ibo);

		glUseProgram(render.programs[0]);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

		switch(glfwGetKey(render.window, GLFW_KEY_Q)) { 
			case GLFW_PRESS:
				glfwSetWindowShouldClose(render.window, GLFW_TRUE);	
				break;
			default:
				break;
		}
 
		/* Swap front and back buffers */
        glfwSwapBuffers(render.window);

        /* Poll for and process events */
        glfwPollEvents();
    }

	delete_programs(&render);

 	glfwTerminate();
	return 0;
}
