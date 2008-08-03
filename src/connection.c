/*
 *  Copyright (C) 2008 Greg Benison
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <glib.h>
#include "connection.h"

/*
 * Calculate a path through 'graph' from vertex 's' to
 * vertex 't'.
 * The adjacent vertices comprising the path will be stored
 * in 'connection'.
 */
int
igraph_connection(const igraph_t *graph,
		  igraph_integer_t s,
		  igraph_integer_t t,
		  igraph_vector_t *connection)
{
  igraph_vector_t cut;
  igraph_vector_init(&cut, igraph_ecount(graph));

  /* FIXME how to calculate the true number of hops needed? */
  igraph_vector_init(connection, 256);

  igraph_integer_t connectivity_overall;
  igraph_st_edge_connectivity(graph,
			      &connectivity_overall,
			      s,
			      t);

  if (connectivity_overall == 0)
    return 1; /* fail */

  igraph_integer_t idx = 0;

  igraph_vs_t vs;
  int done = FALSE;

  while (!done)
    {
      igraph_vector_set(connection, idx, s);
      ++idx;

      igraph_integer_t connectivity;

      igraph_vit_t vit;
      igraph_integer_t next_hop = s;
      igraph_integer_t min_connectivity = connectivity_overall;
      igraph_vs_adj(&vs, s, IGRAPH_ALL);
      igraph_vit_create(graph, vs, &vit);
      
      while (!IGRAPH_VIT_END(vit))
	{
	  igraph_integer_t vertex = IGRAPH_VIT_GET(vit);
	  IGRAPH_VIT_NEXT(vit);

	  /* skip if vertex is in the result already */
	  if (igraph_vector_contains(connection, (igraph_real_t)vertex))
	    continue;

	  /* done if vertex == t */
	  if (vertex == t)
	    {
	      next_hop = vertex;
	      done = TRUE;
	      break;
	    }

	  /* skip if vertex is not connected to t */
	  igraph_st_edge_connectivity(graph,
				      &connectivity,
				      vertex,
				      t);

	  if (connectivity == 0)
	    continue;

	  next_hop = vertex;
	  break;
	}

      g_assert(next_hop != s);
      s = next_hop;

    }

  igraph_vector_set(connection, idx, t);

  return 0;

}
