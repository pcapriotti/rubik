#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

#include "megaminx.h"
#include "cube.h"
#include "polyhedron.h"
#include "piece.h"
#include "square1.h"
#include "scene.h"

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

  scene_t *scene = glfwGetWindowUserPointer(window);
  scene_resize(scene, width, height);
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

static void handle_keypress(void *data, unsigned int c)
{
  scene_t *scene = data;
  if (!scene) return;
  if (!scene->on_keypress) return;
  scene->on_keypress(scene->on_keypress_data, c);
}

static void handle_keys(GLFWwindow *window,
                        int key, int scancode,
                        int action, int mods)
{
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    char c = translate_key(key, mods);
    if (c != 0) handle_keypress(glfwGetWindowUserPointer(window), c);
  }

}

static void handle_char(GLFWwindow* window, unsigned int codepoint)
{
  if (codepoint >= 32 || codepoint < 128) {
    handle_keypress(glfwGetWindowUserPointer(window), codepoint);
  }
}

void normalise_screen_coords(GLFWwindow *window, double *x, double *y)
{
  int width, height;
  glfwGetWindowSize(window, &width, &height);

  *x = *x / width * 2 - 1;
  *y = 1 - *y / height * 2;
}

static void handle_move(GLFWwindow* window, double x, double y)
{
  scene_t *scene = glfwGetWindowUserPointer(window);
  if (!scene) return;
  if (!scene->tb_active) return;

  normalise_screen_coords(window, &x, &y);

  scene_tb_update(scene, x, y);
}

static void handle_click(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    scene_t *scene = glfwGetWindowUserPointer(window);
    if (!scene) return;

    double x, y;
    glfwGetCursorPos(window, &x, &y);
    normalise_screen_coords(window, &x, &y);

    if (action == GLFW_PRESS)
      scene_tb_start(scene, x, y);
    else if (action == GLFW_RELEASE)
      scene_tb_end(scene, x, y);
  }
}

void run(GLFWwindow *window)
{
  double tm0 = glfwGetTime();
  int nframes = 0;

  int width, height;
  glfwGetWindowSize(window, &width, &height);

  scene_t *scene = malloc(sizeof(scene_t));
  scene_init(scene, width, height, glfwGetTime());

  /* cube_scene_new(scene, 4); */
  /* megaminx_scene_new(scene); */
  square1_scene_new(scene);

  glfwSetWindowUserPointer(window, scene);

  glfwSetWindowSizeCallback(window, handle_resize);
  handle_resize(window, width, height);

  int ok = 1;
  while (ok && !glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    double tm = glfwGetTime();

    /* render */
    glfwGetWindowSize(window, &width, &height);
    scene_render(scene, tm);

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

  free(scene);
}

int main(int argc, char **argv)
{
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
  glfwSetMouseButtonCallback(window, handle_click);
  glfwSetCursorPosCallback(window, handle_move);

  run(window);

  return 0;
}
