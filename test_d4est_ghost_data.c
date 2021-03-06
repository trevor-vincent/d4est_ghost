#include <stdio.h>
#include <stdlib.h>
#include <d4est_base.h>
#include <d4est_mesh.h>
#include <d4est_util.h>
#include <d4est_mesh_data.h>
#include <d4est_ghost_data.h>
#include <d4est_ghost.h>


static int
test_refine
(
 p4est_t * p4est,
 p4est_topidx_t which_tree,
 p4est_quadrant_t * quadrant
)
{
  return 1;
}


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

  int num_refines = 2;
  for (int i = 0; i < num_refines; i++) {
    p4est_refine_ext
      (
       p4est,
       0,
       -1,
       test_refine,
       NULL,
       NULL
      );  
 }


  printf("%d elements on proc %d\n", p4est->local_num_quadrants, p4est->mpirank);
  
  int sizes [D4EST_FIELD_TYPES];
  d4est_mesh_update_element_data
    (
     p4est,
     get_deg,
     sizes,
     NULL
    );
  

  d4est_mesh_data_t* dmd = d4est_mesh_data_init(p4est->mpirank,
                                                sizes);






  d4est_mesh_data_add_field(dmd, "u", VOLUME_NODAL);
  d4est_mesh_data_add_field(dmd, "u_face", FACE_POINT);
  
  for (p4est_topidx_t tt = p4est->first_local_tree;
       tt <= p4est->last_local_tree;
       ++tt)
    {
      p4est_tree_t* tree = p4est_tree_array_index (p4est->trees, tt);
      sc_array_t* tquadrants = &tree->quadrants;
      int Q = (p4est_locidx_t) tquadrants->elem_count;
      
      for (int q = 0; q < Q; ++q) {
        p4est_quadrant_t* quad = p4est_quadrant_array_index (tquadrants, q);
        d4est_element_data_t* ed = (d4est_element_data_t*)(quad->p.user_data);

        int elem_size = d4est_element_data_get_size_of_field
                        (
                         ed,
                         VOLUME_NODAL
                        );
        
        double* u = d4est_mesh_get_field_on_element
                    (
                     ed,
                     "u",
                     VOLUME_NODAL,
                     NULL,
                     dmd
                    );


        double* u_face = d4est_mesh_get_field_on_element
                    (
                     ed,
                     "u_face",
                     FACE_POINT,
                     NULL,
                     dmd
                    );
        

        for (int i = 0; i < D4EST_FACES; i++){
          u_face[i] = i*ed->id + 2.3;
          ed->test_data_face[i] = i*ed->id + 2.3;
        }
        
        for (int i = 0; i < elem_size; i++){
          ed->test_data[i] = i*ed->id + 1;
          u[i] = i*ed->id + 1;
          /* printf("ed->id, i, %d, %d\n", ed->id, i); */
          /* printf("u[i] = %f\n", u[i]); */
          /* printf("test_data[i] = %f\n", ed->test_data[i]); */
        }
        
      }
    }

  d4est_ghost_t* d4est_ghost = d4est_ghost_init(p4est);

  const char* transfer_names [] = {"u", "u_face", NULL};
  
  d4est_ghost_data_t* d4est_ghost_data
    = d4est_ghost_data_init(
                            p4est,
                            d4est_ghost,
                            &transfer_names[0],
                            dmd
                           );

  /* double* u = d4est_mesh_data_get_field */
  /*             ( */
  /*              dmd, */
  /*              "u" */
  /*             );   */

  /* for (int i = 0; i < dmd->field_sizes.local_nodes; i++){ */
  /*   printf(" u = %f on proc %d\n", u[i], p4est->mpirank);  */
  /* } */
  
  /* d4est_mpi_gdb_stall(); */
  d4est_ghost_data_exchange(p4est, d4est_ghost, d4est_ghost_data, dmd);


  for (int gid = 0; gid < d4est_ghost->ghost->ghosts.elem_count; gid++){
    d4est_element_data_t* ged = &d4est_ghost->ghost_elements[gid];

    double* ud = d4est_mesh_get_field_on_ghost(ged,0,"u",d4est_ghost_data);
    double* up = &ged->test_data[0];


    double* ud_face = d4est_mesh_get_field_on_ghost(ged,1,"u_face",d4est_ghost_data);
    double* up_face = &ged->test_data_face[0];
    
    d4est_field_type_t type = d4est_ghost_data->transfer_types[0];
    int size = d4est_element_data_get_size_of_field(ged,type);

    int compare = d4est_util_compare_vecs(ud, up, size, 1e-15);
    if (!compare){
      DEBUG_PRINT_2ARR_DBL(ud, up, size);
    }

    int compare1 = d4est_util_compare_vecs(ud_face, up_face, (D4EST_FACES), 1e-15);
    if (!compare1){
      DEBUG_PRINT_2ARR_DBL(ud_face, up_face, (D4EST_FACES));
    }

    
  }


  d4est_ghost_data_exchange(p4est, d4est_ghost, d4est_ghost_data, dmd);


  for (int gid = 0; gid < d4est_ghost->ghost->ghosts.elem_count; gid++){
    d4est_element_data_t* ged = &d4est_ghost->ghost_elements[gid];

    double* ud = d4est_mesh_get_field_on_ghost(ged,0,NULL,d4est_ghost_data);
    double* up = &ged->test_data[0];


    double* ud_face = d4est_mesh_get_field_on_ghost(ged,1,"u_face",d4est_ghost_data);
    double* up_face = &ged->test_data_face[0];
    
    d4est_field_type_t type = d4est_ghost_data->transfer_types[0];
    int size = d4est_element_data_get_size_of_field(ged,type);

    int compare = d4est_util_compare_vecs(ud, up, size, 1e-15);
    if (!compare){
      DEBUG_PRINT_2ARR_DBL(ud, up, size);
    }

    int compare1 = d4est_util_compare_vecs(ud_face, up_face, (D4EST_FACES), 1e-15);
    if (!compare1){
      DEBUG_PRINT_2ARR_DBL(ud_face, up_face, (D4EST_FACES));
    }

    
  }

  
  d4est_ghost_data_destroy(d4est_ghost_data);
  d4est_mesh_data_destroy(dmd);
  d4est_ghost_destroy(d4est_ghost);

  p4est_destroy(p4est);
  p4est_connectivity_destroy(conn);

  sc_finalize ();
  mpiret = sc_MPI_Finalize ();
  SC_CHECK_MPI (mpiret);
  
  return 0;
}
