#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "ppmrw.h"

typedef struct {
  float Position[2];
  float TexCoord[2];
} Vertex;

// (-1, 1)  (1, 1)
// (-1, -1) (1, -1)

Vertex vertexes[] = {
  {{1, -1}, {0.99999, 0.99999}},
  {{1, 1},  {0.99999, 0}},
  {{-1, 1}, {0, 0}},
  
  {{1, -1}, {0.99999, 0.99999}},
  {{-1, 1}, {0, 0}},
  {{-1, -1}, {0, 0.99999}}
};

static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute vec2 TexCoordIn;\n"
"attribute vec2 vPos;\n"
"varying vec2 TexCoordOut;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    TexCoordOut = TexCoordIn;\n"
"}\n";

static const char* fragment_shader_text =
"varying vec2 TexCoordOut;\n"
"uniform sampler2D Texture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
"}\n";

static mat4x4 m;
static mat4x4 temp;
static double rotate;
static double scale_up = 1;
static double scale_down = 0.5;

static void error_callback(int error, const char* description){
    fprintf(stderr, "Error: %s\n", description);
}

void print_mat4x4(mat4x4 m){
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			printf("%lf ", m[j][i]);
		}
		printf("\n");
	}
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if (action == GLFW_PRESS){
		mat4x4 transform;
		mat4x4_identity(transform);
		switch(key){
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GLFW_TRUE);
			break;
		case GLFW_KEY_UP:
			transform[0][0] = 1.25;
			transform[1][1] = 1.25;
			break;
		case GLFW_KEY_DOWN:
			transform[0][0] = 0.75;
			transform[1][1] = 0.75;
			break;
		case GLFW_KEY_P:
			break;
		case GLFW_KEY_RIGHT:
			mat4x4_rotate_Z(transform, transform, (float) -M_PI/12);
			break;
		case GLFW_KEY_LEFT:
			mat4x4_rotate_Z(transform, transform, (float) M_PI/12);
			break;
		case GLFW_KEY_H:
			transform[3][0] = -0.125;
			break;
		case GLFW_KEY_L:
			transform[3][0] = 0.125;
			break;
		case GLFW_KEY_J:
			transform[3][1] = 0.125;
			break;
		case GLFW_KEY_K:
			transform[3][1] = -0.125;
			break;
		case GLFW_KEY_S:
			transform[0][1] = -0.125;
			break;
		case GLFW_KEY_W:
			transform[0][1] = 0.125;
			break;
		case GLFW_KEY_A:
			transform[1][0] = -0.125;
			break;
		case GLFW_KEY_D:
			transform[1][0] = 0.125;
			break;
		case GLFW_KEY_EQUAL:
			mat4x4_dup(temp, m);
			scale_up = 0;
			break;
		case GLFW_KEY_MINUS:
			mat4x4_dup(temp, m);
			scale_down = 1;
			break;
		case GLFW_KEY_9:
			rotate = 90;
			break;
		case GLFW_KEY_0:
			rotate = -90;
			break;
		case GLFW_KEY_R:
			mat4x4_identity(m);
			break;
		}
		mat4x4_mul(m, transform, m);
	}
}

void glCompileShaderOrDie(GLuint shader) {
  GLint compiled;
  glCompileShader(shader);
  glGetShaderiv(shader,
		GL_COMPILE_STATUS,
		&compiled);
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader,
		  GL_INFO_LOG_LENGTH,
		  &infoLen);
    char* info = malloc(infoLen+1);
    GLint done;
    glGetShaderInfoLog(shader, infoLen, &done, info);
    printf("Unable to compile shader: %s\n", info);
    exit(1);
  }
}

void glLinkProgramOrDie(GLuint program) {
  GLint linked;
  glLinkProgram(program);
  glGetShaderiv(program,
		GL_LINK_STATUS,
		&linked);
  if (!linked) {
    GLint infoLen = 0;
    glGetShaderiv(program,
		  GL_INFO_LOG_LENGTH,
		  &infoLen);
    char* info = malloc(infoLen+1);
    GLint done;
    glGetShaderInfoLog(program, infoLen, &done, info);
    printf("Unable to compile shader: %s\n", info);
    exit(1);
  }
}

int main(int argc, char* argv){
	if (argc != 2){
		fprintf(stderr, "Non proper usage of ezview\n");
		printf("Proper Usage: ezview image.ppm\n");
		exit(1);
	}
	printf("%s\n", argv[1]);
	FILE* fh = fopen(argv[1], "rb");

	if (fh == NULL){
		fprintf(stderr, "Error: Couldn't open file for reading.\n");
		exit(1);
	}
	
	Image img;
	read_file(fh, &img);
	
	unsigned char image[img.height * img.width * 4];
	
	for (int i = 0; i < img.height * img.width; i++){
		Pixel pix = img.buffer[i];
		image[i * 4] = pix.r;
		image[i * 4 + 1] = pix.g;
		image[i * 4 + 2] = pix.b;
		image[i * 4 + 3] = 255;
	}

    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShaderOrDie(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShaderOrDie(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
	glLinkProgramOrDie(program);

    mvp_location = glGetUniformLocation(program, "MVP");
    assert(mvp_location != -1);

    vpos_location = glGetAttribLocation(program, "vPos");
    assert(vpos_location != -1);

    GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
    assert(texcoord_location != -1);

    GLint tex_location = glGetUniformLocation(program, "Texture");
    assert(tex_location != -1);

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location,
			  2,
			  GL_FLOAT,
			  GL_FALSE,
                          sizeof(Vertex),
			  (void*) 0);

    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location,
			  2,
			  GL_FLOAT,
			  GL_FALSE,
                          sizeof(Vertex),
			  (void*) (sizeof(float) * 2));
    

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(tex_location, 0);

	mat4x4_identity(m);
    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

		if (rotate != 0){
			float sign = rotate/ abs(rotate);
			rotate -= 10 * sign;
			mat4x4_rotate_Z(m, m, (float) sign * M_PI/180 * 10);
		}
		
		if (scale_up < 1){
			
			scale_up += 0.0625;
			mat4x4 p;
			mat4x4_identity(p);
			p[0][0] = 1 + scale_up;
			p[1][1] = 1 + scale_up;
			mat4x4_mul(m, temp, p);
		}

		if (scale_down > 0.5){
			
			scale_down -= 0.03125;
			mat4x4 p;
			mat4x4_identity(p);
			p[0][0] = scale_down;
			p[1][1] = scale_down;
			mat4x4_mul(m, temp, p);
		}
		
        mat4x4_dup(mvp, m);
		//mat4x4_transpose(mvp, mvp);
		/*
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				printf("%f ", mvp[i][j]);
			}
			printf("\n");
		}
		*/
		
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
