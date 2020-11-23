#ifndef SHADER_H
#define SHADER_H

int shader_load_program(const unsigned char *vertex,
                        unsigned int vertex_len,
                        const unsigned char *frag,
                        unsigned int frag_len);

#endif /* SHADER_H */
