#include <d4est_ghost.h>
#include <d4est_element_data.h>

/* TODO: make sure we can call d4est_exchange multiple times per grid, and check for fields transfered more than once */

// TODO: store strides to data in int* pointers stored in the d4est_ghost dictionary, then store one big double data array for all fields in d4est_ghost. We can keep the int* strides the same for each d4est_exchange_data as long as 1) the grid doesn't change 2) we don't want to transfer more data arrays. To handle 2), we should check that all the arrays exist in the dictionary and that their order hasn't changed. If their order has changed, throw a D4EST_ABORT b.c the user can easily reorder them in the frontend code (assumption for now).

d4est_mesh_data_sizes_t
d4est_ghost_update_element_strides
(
 d4est_ghost_data_t* dgd
)
{
  int stride_nodal = 0;
  /* int stride_trace = 0; */
  
  for (int gid = 0; gid < dgd->ghost->ghosts.elem_count; gid++){
    d4est_element_data_t* ged = &dgd->ghost_elements[gid];
    /* ged->strides.stride_volume = gid; */
    ged->strides.stride_volume_nodal = stride_nodal;
    /* ged->strides.stride_trace = stride_trace; */
    stride_nodal += d4est_element_data_get_size_of_field(ged, VOLUME_NODAL);
    /* stride_trace += d4est_field_get_element_size(TRACE, ged->deg);     */
  }

  d4est_mesh_data_sizes_t sizes;
  sizes.local_nodes = stride_nodal;
  /* sizes.local_trace_trace = stride_trace; */
  /* sizes.local_elements = dgd->ghost->ghost.elem_count; */
  return sizes;
}


d4est_ghost_data_t*
d4est_ghost_init
(
 p4est_t* p4est
)
{
  d4est_ghost_data_t* dgd = D4EST_ALLOC(d4est_ghost_data_t, 1);
  
  dgd->ghost = p4est_ghost_new (p4est, P4EST_CONNECT_FULL);

  /* create space for storing the ghost data */
  dgd->ghost_elements = P4EST_ALLOC (d4est_element_data_t, dgd->ghost->ghosts.elem_count);

  /* only send element_data when ghost region has changed */
  p4est_ghost_exchange_data(p4est, dgd->ghost, dgd->ghost_elements);

  /* update element data strides in ghost_elements order */
  d4est_mesh_data_sizes_t sizes = d4est_ghost_update_element_strides(dgd);
  dgd->ghost_grid_data = d4est_mesh_data_init(p4est->mpirank,&sizes);

  return dgd;
}

void
d4est_ghost_destroy
(
 d4est_ghost_data_t* dgd
)
{
  /* clear the dictionary */
  d4est_mesh_data_destroy(dgd->ghost_grid_data);

  /* free the ghost_elements */
  D4EST_FREE(dgd->ghost_elements);

  /* free the ghost object */
  p4est_ghost_destroy(dgd->ghost);

  D4EST_FREE(dgd);
}

void
d4est_ghost_update
(
 d4est_ghost_data_t* dgd,
 p4est_t* p4est
)
{
  p4est_ghost_destroy(dgd->ghost);
  D4EST_FREE(dgd->ghost_elements);
  d4est_mesh_data_destroy(dgd->ghost_grid_data);
  
  dgd->ghost = p4est_ghost_new(p4est, P4EST_CONNECT_FULL);
  dgd->ghost_elements = P4EST_ALLOC(d4est_element_data_t, dgd->ghost->ghosts.elem_count);  
  
  /* only send element_data when ghost region has changed */
  p4est_ghost_exchange_data(p4est, dgd->ghost, dgd->ghost_elements);

  /* update element data strides in ghost_elements order */
  d4est_mesh_data_sizes_t sizes = d4est_ghost_update_element_strides(dgd);
  dgd->ghost_grid_data = d4est_mesh_data_init(p4est->mpirank,&sizes);
}


void
d4est_ghost_exchange_data
(
 p4est_t* p4est,
 d4est_ghost_data_t* dgd,
 d4est_mesh_data_t* dmd,
 const char ** transfer_names
)
{
 
  int num_names = 0;
  for (int tn = 0; transfer_names != NULL; tn++){
    num_names++;
  }

  int* ghost_data_sizes = P4EST_ALLOC_ZERO(int, p4est->mpisize);
  d4est_field_type_t* field_types = D4EST_ALLOC(d4est_field_type_t, num_names);

  int total_receive_datasize = 0;
  for (int tn = 0; transfer_names[tn] != NULL; tn++){
    
    d4est_mesh_data_get_field_type
      (
       dmd,
       transfer_names[tn],
       &field_types[tn]
      );
    
    for (int gid = 0; gid < dgd->ghost->ghosts.elem_count; gid++){
      d4est_element_data_t* ghost_elem_data = &dgd->ghost_elements[gid];
      int ghost_rank = dgd->ghost_elements[gid].mpi_rank;
      int size = d4est_element_data_get_size_of_field
                 (
                  ghost_elem_data,
                  field_types[tn]
                 );      
      ghost_data_sizes[ghost_rank] += size;
      total_receive_datasize += size;
    }
  } 

  double* receive_buffer = D4EST_ALLOC(double, total_receive_datasize);

  
  size_t              zz;
  size_t              data_size;
  p4est_topidx_t      which_tree;
  p4est_locidx_t      which_quad;
  p4est_quadrant_t   *mirror, *q;
  p4est_tree_t       *tree;
  void              **mirror_data;

  mirror_data = D4EST_ALLOC (void *, dgd->ghost->mirrors.elem_count);

  data_size = p4est->data_size == 0 ? sizeof (void *) : p4est->data_size;

  for (zz = 0; zz < dgd->ghost->mirrors.elem_count; ++zz) {
    mirror = p4est_quadrant_array_index (&dgd->ghost->mirrors, zz);
    which_tree = mirror->p.piggy3.which_tree;
    D4EST_ASSERT (p4est->first_local_tree <= which_tree &&
                  which_tree <= p4est->last_local_tree);
    /* D4EST_ASSERT (prev_tree <= which_tree); */

    tree = p4est_tree_array_index (p4est->trees, which_tree);
    which_quad = mirror->p.piggy3.local_num - tree->quadrants_offset;
    D4EST_ASSERT (0 <= which_quad &&
                  which_quad < (p4est_locidx_t) tree->quadrants.elem_count);
    q = p4est_quadrant_array_index (&tree->quadrants, which_quad);
    mirror_data[zz] =
      p4est->data_size == 0 ? &q->p.user_data : q->p.user_data;
  } 
  
  d4est_ghost_exchange_data_send_and_receive_pieces
    (
     p4est,
     ghost,
     transfer_names,
     dmd,
     mirror_data,
     receive_buffer,
     ghost_data_sizes
    );

  /* d4est_ghost_exchange_data_glue_received_pieces */
  /*   ( */
  /*    dgd, */
  /*    dmd, */
  /*    transfer_names, */
  /*    receive_buffer */
  /*   ); */
  
  D4EST_FREE(ghost_data_sizes);
  D4EST_FREE (mirror_data);
  D4EST_FREE (receive_buffer);
  D4EST_FREE (field_types);
}


void
  d4est_ghost_exchange_data_send_and_receive_pieces
(
 p4est_t * p4est,
 p4est_ghost_t * ghost,
 const char** transfer_names,
 d4est_localgrid_data_t* dlgd,
 void **mirror_data,
 void* receive_buffer,
 int* ghost_data_sizes
)
{
  const int           num_procs = p4est->mpisize;
  int                 mpiret;
  int                 q;
  char               *mem, **sbuf;
  size_t              zz;
  sc_array_t          requests, sbuffers;
  p4est_locidx_t      ng_excl, ng_incl, ng, theg;
  p4est_locidx_t      mirr;
  sc_MPI_Request     *r;

  sc_array_init (&requests, sizeof (sc_MPI_Request));
  sc_array_init (&sbuffers, sizeof (char *));

  /* receive data from other processors */
  int data_size_offset = 0;
  ng_excl = 0;
  for (q = 0; q < num_procs; ++q) {
    ng_incl = ghost->proc_offsets[q + 1];
    ng = ng_incl - ng_excl;
    P4EST_ASSERT (ng >= 0);
    if (ng > 0) {
      r = (sc_MPI_Request *) sc_array_push (&requests);
      mpiret = sc_MPI_Irecv ((char *) receive_buffer + data_size_offset,
                             ghost_data_sizes[q], sc_MPI_BYTE, q,
                             P4EST_COMM_GHOST_EXCHANGE, p4est->mpicomm, r);
      SC_CHECK_MPI (mpiret);
      ng_excl = ng_incl;
      data_size_offset += ghost_data_sizes[q];
    }
  }
  P4EST_ASSERT (ng_excl == (p4est_locidx_t) ghost->ghosts.elem_count);

  /* send data to other processors */
  ng_excl = 0;
  for (q = 0; q < num_procs; ++q) {
    ng_incl = ghost->mirror_proc_offsets[q + 1];
    ng = ng_incl - ng_excl;
    P4EST_ASSERT (ng >= 0);
    if (ng > 0) {
      /* calculate total data size to send */
      int total_memsize = 0;
      for (theg = 0; theg < ng; ++theg){
        mirr = ghost->mirror_proc_mirrors[ng_excl + theg];
        P4EST_ASSERT (0 <= mirr && (size_t) mirr < ghost->mirrors.elem_count);

        for (int tn = 0; transfer_names[tn] != NULL; tn++){
          d4est_field_type_t field_type = d4est_localgrid_get_field_type
                                          (
                                           dlgd,
                                           transfer_names[tn]
                                          );
          total_memsize +=  sizeof(double)*d4est_field_get_element_size
                            (
                             field_type,
                             mirror_data[mirr]->deg
                            );              
        }
      }
      /* every peer populates its own send buffer */
      sbuf = (char **) sc_array_push (&sbuffers);
      mem = *sbuf = P4EST_ALLOC (char, total_memsize);

      for (theg = 0; theg < ng; ++theg) {
        mirr = ghost->mirror_proc_mirrors[ng_excl + theg];
        P4EST_ASSERT (0 <= mirr && (size_t) mirr < ghost->mirrors.elem_count);
        for (int tn = 0; transfer_names[tn] != NULL; tn++){
          d4est_field_type_t field_type = d4est_localgrid_get_field_type
                                          (
                                           dlgd,
                                           transfer_names[tn]
                                          );
          int mem_size = sizeof(double)*d4est_field_get_element_size
                            (
                             field_type,
                             mirror_data[mirr]->deg
                            );  

          double* field = d4est_localgrid_data_field_get(dlgd, transfer_names[tn]);
          int type_stride = d4est_field_get_local_stride(mirror_data[mirr]->strides, field_type);
          memcpy (mem, &field[type_stride], mem_size);
          mem += mem_size;
        }
      }
      r = (sc_MPI_Request *) sc_array_push (&requests);
      mpiret = sc_MPI_Isend (*sbuf, total_memsize, sc_MPI_BYTE, q,
                             P4EST_COMM_GHOST_EXCHANGE, p4est->mpicomm, r);
      SC_CHECK_MPI (mpiret);
      ng_excl = ng_incl;
    }
  }

  /* wait and clean up */
  mpiret = sc_MPI_Waitall (requests.elem_count, (sc_MPI_Request *)
                           requests.array, sc_MPI_STATUSES_IGNORE);
  SC_CHECK_MPI (mpiret);
  sc_array_reset (&requests);
  for (zz = 0; zz < sbuffers.elem_count; ++zz) {
    sbuf = (char **) sc_array_index (&sbuffers, zz);
    P4EST_FREE (*sbuf);
  }
  sc_array_reset (&sbuffers);
}
