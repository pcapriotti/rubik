#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

#include "polyhedron.h"
#include "piece.h"

static void handle_error(int error, const char* description)
{
  fprintf(stderr, "Error %d: %s\n", error, description);
}

static void handle_opengl_error(GLenum source,
                         GLenum type,
                         GLuint id,
                         GLenum severity,
                         GLsizei length,
                         const GLchar* message,
                         const void* userParam)
{
  printf("opengl: %s\n", message);
}

static void handle_resize(GLFWwindow* window, int width, int height)
{
  glViewport(0.0f, 0.0f, (GLfloat) width, (GLfloat) height);
}

char translate_key(int key, int mods)
{
  if (mods & GLFW_MOD_CONTROL) {
    return key - GLFW_KEY_A + 1;
  }
  if (mods == 0) {
    switch (key) {
    case GLFW_KEY_TAB:
      return '\t';
      break;
    case GLFW_KEY_ENTER:
      return '\r';
      break;
    case GLFW_KEY_ESCAPE:
      return '\x1b';
      break;
    case GLFW_KEY_BACKSPACE:
      return 127;
      break;
    case GLFW_KEY_LEFT:
      return 'h';
      break;
    case GLFW_KEY_DOWN:
      return 'j';
      break;
    case GLFW_KEY_UP:
      return 'k';
      break;
    case GLFW_KEY_RIGHT:
      return 'l';
      break;
    }
  }
  return 0;
}

static void handle_keypress(unsigned int c)
{
}

static void handle_keys(GLFWwindow *window,
                        int key, int scancode,
                        int action, int mods)
{
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    char c = translate_key(key, mods);
    if (c != 0) handle_keypress(c);
  }

}

static void handle_char(GLFWwindow* window, unsigned int codepoint)
{
  if (codepoint >= 32 || codepoint < 128) {
    handle_keypress(codepoint);
  }
}

void run(GLFWwindow *window)
{
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  glfwSetWindowSizeCallback(window, handle_resize);
  handle_resize(window, width, height);

  double tm0 = glfwGetTime();
  int nframes = 0;

  int ok = 1;
  while (ok && !glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double tm = glfwGetTime();

    /* render */
    glfwGetWindowSize(window, &width, &height);
    piece_t *piece = glfwGetWindowUserPointer(window);
    piece_render(piece, width, height);

    if (0) {
      /* display fps */
      nframes++;
      if (tm - tm0 >= 1.0) {
        printf("%f ms/frame\n", 1000.0 / (double) nframes);
        nframes = 0;
        tm0 = tm;
      }
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

int main(int argc, char **argv)
{
  poly_t *poly = malloc(sizeof(poly_t));
  std_dodec(poly);
  poly_debug(poly);

  if (!glfwInit()) return 1;
  glfwSetErrorCallback(handle_error);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  int width = 800;
  int height = 600;
  GLFWwindow* window = glfwCreateWindow(width, height, "rubik", 0, 0);
  if (!window) return 1;

  glfwMakeContextCurrent(window);
  glewExperimental = GL_TRUE;
  glewInit();

  glEnable(GL_BLEND);
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glDebugMessageCallback(handle_opengl_error, NULL);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glfwSetKeyCallback(window, handle_keys);
  glfwSetCharCallback(window, handle_char);

  piece_t *piece = malloc(sizeof(piece_t));
  piece_init(piece, poly);

  glfwSetWindowUserPointer(window, piece);

  run(window);

  free(piece);
  free(poly);
  return 0;
}
