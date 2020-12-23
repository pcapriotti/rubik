#include "skewb.h"

#include <stdlib.h>

#include "group.h"
#include "puzzle.h"

void skweb_action_init(puzzle_action_t *action)
{
  group_t *group = malloc(sizeof(group_t));
  group_a4_init(group);
  unsigned int num_syms = group->num;

  /* there are two vertex orbits (corresponding to the vertices of two
  tetrahedra) and one face orbit */

  unsigned int num_orbits = 3;
  unsigned int orbit_size[] = { 4, 4, 6 };
  uint8_t stab_gen[] = { 1, 1, 11 };

  uint8_t *orbit[3];
  uint8_t *stab[3];

  for (unsigned int k = 0; k < num_orbits; k++) {
    orbit[k] = malloc(orbit_size[k]);
    stab[k] = malloc(num_syms / orbit_size[k]);
    group_cyclic_subgroup(group, stab[k],
                          num_syms / orbit_size[k],
                          stab_gen[k]);
  }

  /* vertex orbits */
  for (unsigned int i = 0; i < 4; i++) {
    orbit[0][i] = i * 3;
    orbit[1][i] = i * 3;
  }

  /* centre orbit */
  uint8_t found = 0;
  unsigned int face = 0;
  for (unsigned int i = 0; i < 4; i++) {
    for (unsigned int j = i + 1; j < 4; j++) {
      /* symmetry mapping 0 to i and 1 to j */
      unsigned int sym = i * 3 + j - 1;
      orbit[2][face++] = sym;
    }
  }

  puzzle_action_init(action, num_orbits, orbit_size,
                     group, orbit, stab);

  for (unsigned int k = 0; k < num_orbits; k++) {
    free(orbit[k]);
    free(stab[k]);
  }
}
