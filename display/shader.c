#include "utils.h"

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>

GLchar *read_file(const char *filename)
{
  FILE *f = fopen(filename, "r");
  if (!f) {
    return 0;
  }

  fseek(f, 0, SEEK_END);
  int size = ftell(f);
  rewind(f);

  GLchar *buf = malloc((size + 1) * sizeof(GLchar));
  int len = fread(buf, sizeof(GLchar), size, f);
  if (len < size) {
    free(buf);
    return 0;
  }
  buf[size] = '\0';
  fclose(f);

  return buf;
}

int shader_load_from_string(GLint type, const char *filename,
                            const unsigned char *code,
                            unsigned int code_size)
{
  GLuint shader = glCreateShader(type);

  glShaderSource(shader, 1,
                 (const GLchar **)&code,
                 (const GLint *)&code_size);
  glCompileShader(shader);

  {
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
      int logsize;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
      char *buf = malloc(sizeof(char) * logsize);
      glGetShaderInfoLog(shader, logsize, NULL, buf);
      fprintf(stderr, "%s:\n%s", filename, buf);
      free(buf);
      return 0;
    }
  }

  return shader;
}

int shader_load(GLint type, const char *filename)
{
  GLuint shader = glCreateShader(type);

  char *code = read_file(filename);
  if (!code) {
    die("Could not load shader");
  }

  glShaderSource(shader, 1, (const GLchar **)&code, NULL);
  glCompileShader(shader);

  {
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
      int logsize;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
      char *buf = malloc(sizeof(char) * logsize);
      glGetShaderInfoLog(shader, logsize, NULL, buf);
      fprintf(stderr, "%s:\n%s", filename, buf);
      free(buf);
      return 0;
    }
  }

  free(code);
  return shader;
}

int shader_load_program(const unsigned char *vertex,
                        unsigned int vertex_len,
                        const unsigned char *frag,
                        unsigned int frag_len)
{
  GLuint vertex_shader = shader_load_from_string
    (GL_VERTEX_SHADER, "vertex", vertex, vertex_len);
  GLuint frag_shader = shader_load_from_string
    (GL_FRAGMENT_SHADER, "fragment", frag, frag_len);
  GLuint prog = glCreateProgram();

  glAttachShader(prog, vertex_shader);
  glAttachShader(prog, frag_shader);
  glLinkProgram(prog);
  glValidateProgram(prog);
  glUseProgram(prog);

  glDeleteShader(vertex_shader);
  glDeleteShader(frag_shader);

  return prog;
}
