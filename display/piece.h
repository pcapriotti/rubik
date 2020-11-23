#ifndef PIECE_H
#define PIECE_H

struct poly_t;
typedef struct poly_t poly_t;

typedef struct
{
  unsigned int vao;
  unsigned int num_elements;
  unsigned int shader;
} piece_t;

void piece_render(piece_t *piece, unsigned int width, unsigned int height);
void piece_init(piece_t *piece, poly_t *poly);

#endif /* PIECE_H */
