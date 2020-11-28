#ifndef ABS_POLY_H
#define ABS_POLY_H

typedef struct
{
  unsigned int num_vertices;
  unsigned int *vertices;
} face_t;

/* abstract polyhedron */
struct abs_poly_t
{
  unsigned int num_vertices;
  unsigned int num_faces;
  face_t *faces;
  unsigned int *vertices;
  unsigned int len; /* size of the vertices array */
};
typedef struct abs_poly_t abs_poly_t;

void abs_cube(abs_poly_t *cube);
void abs_dodec(abs_poly_t *dodec);
void abs_prism(abs_poly_t *prism, unsigned int num);

void abs_poly_debug(abs_poly_t *poly);

int *abs_poly_adj(abs_poly_t *poly);
int *abs_poly_edges(abs_poly_t *poly);
int abs_poly_get_adj_face(abs_poly_t *poly,
                          unsigned int f, int i,
                          int *edges);
int *abs_poly_first_vertex(abs_poly_t *poly, int *edges);


#endif /* ABS_POLY_H */
