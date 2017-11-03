#include <stdio.h>
#include <stdlib.h>
#include <d4est_base.h>
#include <d4est_mesh.h>
#include <d4est_mesh_data.h>
#include <d4est_ghost.h>

int
get_deg(d4est_element_data_t* ed, void* user){
  return 1;
}


p4est_connectivity_t*
test_build_conn()
{
#if (D4EST_DIM) == 3
  return p8est_connectivity_new_unitcube();
#else
  return p4est_connectivity_new_unitsquare();
#endif
}

p4est_t*
test_build_p4est
(
 sc_MPI_Comm mpicomm,
 p4est_connectivity_t* conn,
 p4est_locidx_t min_quadrants,
 int min_level, 
 int fill_uniform
)
{
  return p4est_new_ext
    (
     mpicomm,
     conn,
     min_quadrants,
     min_level,
     fill_uniform,
     sizeof(d4est_element_data_t),
     NULL,
     NULL
    );
}

int main(int argc, char *argv[])
{
  
  sc_MPI_Comm mpicomm;
  int mpiret = sc_MPI_Init(&argc, &argv);
  SC_CHECK_MPI(mpiret);
  mpicomm = sc_MPI_COMM_WORLD;

  p4est_init(NULL, SC_LP_ERROR);

  p4est_connectivity_t *conn =
    test_build_conn();
  
  p4est_t* p4est =
    test_build_p4est
    (
     mpicomm,
     conn,
     -1,
     1,
     1
    );

  /* p4est_refine_ext */
  /*   ( */
  /*    p4est, */
  /*    0, */
  /*    -1, */
  /*    test_refine, */
  /*    NULL, */
  /*    NULL */
  /*   ); */
  
  d4est_mesh_data_sizes_t sizes = d4est_mesh_update_element_data
                                  (
                                   p4est,
                                   get_deg,
                                   NULL
                                  );
  

  d4est_mesh_data_t* dmd = d4est_mesh_data_init(p4est->mpirank,
                                                &sizes);


  d4est_ghost_data_t* dgd = d4est_ghost_init(p4est);
  

  d4est_mesh_data_destroy(dmd);
  d4est_ghost_destroy(dgd);

  p4est_destroy(p4est);
  p4est_connectivity_destroy(conn);

  sc_finalize ();
  mpiret = sc_MPI_Finalize ();
  SC_CHECK_MPI (mpiret);
  
  return 0;
}
