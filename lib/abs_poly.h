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

/* incidence data for an abstract polyhedron */
struct poly_data_t
{
  unsigned int num_faces;

  /* for each pair of vertices connected by an edge, the face to the
     left of the vector from the first to the second vertex, -1 if not
     connected */
  int *edges;

  /* for every face, the vertex before its first adjacent face */
  int *first_vertex;

  /* for every face and every vertex, the index of that vertex in the
  face, or -1 if not contained */
  int *adj;

  /* for every face and every vertex, the index of the edge from that
  vertex to the next one */
  unsigned int **edges_by_face;
};
typedef struct poly_data_t poly_data_t;

void poly_data_init(poly_data_t *data, abs_poly_t *poly);
void poly_data_cleanup(poly_data_t *data);

void abs_cube(abs_poly_t *cube);
void abs_dodec(abs_poly_t *dodec);
void abs_prism(abs_poly_t *prism, unsigned int num);

unsigned int abs_poly_num_edges(abs_poly_t *poly);

void abs_poly_debug(abs_poly_t *poly);
void abs_poly_cleanup(abs_poly_t *poly);

int *abs_poly_adj(abs_poly_t *poly);
int *abs_poly_edges(abs_poly_t *poly);
int *abs_poly_first_vertex(abs_poly_t *poly, int *edges);
unsigned int **abs_poly_edges_by_face(abs_poly_t *poly, int *edges, int *adj);

int abs_poly_get_adj_face(abs_poly_t *poly,
                          unsigned int f, int i,
                          int *edges);

#endif /* ABS_POLY_H */
