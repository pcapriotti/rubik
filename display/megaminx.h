#ifndef MEGAMINX_H
#define MEGAMINX_H

/* realisation of a polyhedron */
struct poly_t;
typedef struct poly_t poly_t;

void megaminx_corner(poly_t *mm, poly_t *dodec, float edge);
void megaminx_edge(poly_t *mm, poly_t *dodec, float edge);

#endif /* MEGAMINX_H */
