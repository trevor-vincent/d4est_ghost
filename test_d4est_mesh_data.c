#include <stdio.h>
#include <stdlib.h>
#include <d4est_base.h>
#include <d4est_mesh_data.h>

static double
sqr_fcn(
       double x,
       double y,
       double z,
       void* arg
      )
{
  return x*x + y*y + z*z;
}

int main(int argc, char *argv[])
{
  
  int local_nodes = 8*8*(3*3*3);
  int local_sizes [D4EST_FIELD_TYPES];
  local_sizes[0] = local_nodes;
  
  d4est_mesh_data_t* lgd = d4est_mesh_data_init(0,local_sizes);

  d4est_mesh_data_add_field(lgd, "_foo_1", VOLUME_NODAL);

  d4est_mesh_data_add_field(lgd, "_bar_1", VOLUME_NODAL);

  d4est_mesh_data_add_field(lgd, "_geom_x", VOLUME_NODAL);
  d4est_mesh_data_add_field(lgd, "_geom_y", VOLUME_NODAL);
  d4est_mesh_data_add_field(lgd, "_geom_z", VOLUME_NODAL);  

  double* restrict x = d4est_mesh_data_get_field(lgd, "_geom_x");
  double* restrict y = d4est_mesh_data_get_field(lgd, "_geom_y");
#if (D4EST_DIM==3)
  double* restrict z = d4est_mesh_data_get_field(lgd, "_geom_z");
#endif
  
  for (int i = 0; i < local_nodes; i++){
    x[i] = (double)i;
    y[i] = (double)i;
    z[i] = (double)i;
  }
  
  d4est_mesh_data_init_field
    (
     lgd,
     "_foo_1",
     "_geom_x",
     "_geom_y",
     "_geom_z",
     sqr_fcn,
     NULL
    );

  double* restrict foo1 = d4est_mesh_data_get_field(lgd, "_foo_1");
  int test_check = 1.;

  for (int i = 0; i < local_nodes; i++){
    double di = (double)i;
    /* printf("foo1, ans = %f,%f\n", foo1[i], 3.*di*di); */
    test_check *= fabs(foo1[i] - 3.*di*di) < DBL_EPSILON;
  }

  double* grid [(D4EST_DIM)];
  d4est_mesh_data_get_vector_field(lgd, "_geom_", grid);

  for (int i = 0; i < local_nodes; i++){
    double di = (double)i;
    for (int d = 0; d < (D4EST_DIM); d++)
      test_check *= fabs(grid[d][i] - di) < DBL_EPSILON;
  }


  /* local_sizes[0] = 8*8*(3*3*3)*8;   */
  /* d4est_mesh_data_update(lgd, local_sizes); */


    x = d4est_mesh_data_get_field(lgd, "_geom_x");
    y = d4est_mesh_data_get_field(lgd, "_geom_y");
#if (D4EST_DIM==3)
    z = d4est_mesh_data_get_field(lgd, "_geom_z");
#endif
  
  for (int i = 0; i < local_nodes; i++){
    x[i] = (double)i;
    y[i] = (double)i;
#if (D4EST_DIM==3)
    z[i] = (double)i;
#endif
  }
  
  d4est_mesh_data_init_field
    (
     lgd,
     "_foo_1",
     "_geom_x",
     "_geom_y",
     "_geom_z",
     sqr_fcn,
     NULL
    );

  foo1 = d4est_mesh_data_get_field(lgd, "_foo_1");

  for (int i = 0; i < local_nodes; i++){
    double di = (double)i;
    /* printf("foo1, ans = %f,%f\n", foo1[i], 3.*di*di); */
    test_check *= fabs(foo1[i] - 3.*di*di) < DBL_EPSILON;
  }
  
  /* D4EST_FREE(d4est_mesh_data_get_field(lgd, "_geom_x")); */

  /* printf("d4est_mesh_data_add_field(lgd, _geom_x, VOLUME_NODAL) = %d",d4est_mesh_data_add_field(lgd, "_geom_x", VOLUME_NODAL)); */
  /* if(d4est_dictionary_delete(&lgd->field_ptrs, "_geom_x") == 0){ */
  /*   printf("field = _geom_x\n"); */
  /*   D4EST_ABORT("Could not delete field from dictionary\n"); */
  /* } */

  /* d4est_mesh_data_get_vector_field(lgd, "_geom_", grid); */
  /* for (int d = 0; d < (D4EST_DIM); d++) */
  /*   test_check *= (grid[d] == NULL); */
     
  d4est_mesh_data_delete_field
    (
     lgd,
     "_geom_x"
    );
  d4est_mesh_data_destroy(lgd);

  /* d4est_mesh_data_add_field(lgd, "_geom_x", VOLUME_NODAL); */
  D4EST_ASSERT(test_check == 1);
  return 0;
}
