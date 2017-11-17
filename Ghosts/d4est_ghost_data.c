#include <d4est_ghost.h>
#include <d4est_ghost_data.h>
#include <d4est_element_data.h>
#include <string.h>



static void
d4est_ghost_data_compute_ids_and_strides
(
 d4est_ghost_t* d4est_ghost,
 d4est_ghost_data_t* d4est_ghost_data
)
{
  int stride_global_receive = 0;
  for (int gid = 0; gid < d4est_ghost->ghost->ghosts.elem_count; gid++){
    d4est_element_data_t* ghost_elem_data = &d4est_ghost->ghost_elements[gid];
    ghost_elem_data->id = gid;

    int stride_local_receive = 0;
    for (int i = 0; i < d4est_ghost_data->num_names; i++){
      d4est_field_type_t type = d4est_ghost_data->transfer_types[i];
      int field_elem_size = d4est_element_data_get_size_of_field(ghost_elem_data, type);
      d4est_ghost_data->receive_strides[gid][i] = stride_global_receive + stride_local_receive;     
      stride_local_receive += field_elem_size;
    }    
    stride_global_receive += stride_local_receive;
  }
}


d4est_ghost_data_t*
d4est_ghost_data_init
(
 p4est_t* p4est,
 d4est_ghost_t* d4est_ghost,
 const char** transfer_names,
 d4est_mesh_data_t* d4est_mesh_data
)
{
  d4est_ghost_data_t* d4est_ghost_data = D4EST_ALLOC(d4est_ghost_data_t, 1); 
  int num_names = 0;
  
  for (int tn = 0; transfer_names[tn] != NULL; tn++){
    /* printf("%s\n", transfer_names[tn]); */
    num_names++;
  }

  d4est_ghost_data->num_ghosts = d4est_ghost->ghost->ghosts.elem_count;
  d4est_ghost_data->num_names = num_names;

  d4est_ghost_data->transfer_names = D4EST_ALLOC(char*, num_names);
  /* d4est_ghost_data->transfer_types = D4EST_ALLOC(d4est_field_type_t, num_names); */

  for (int i = 0; i < num_names; i++){
    d4est_ghost_data->transfer_names[i] = strdup(transfer_names[i]);
  }

  d4est_ghost_data->receive_strides = D4EST_ALLOC(int*, d4est_ghost_data->num_ghosts);
  for (int i = 0; i < d4est_ghost_data->num_ghosts; i++){
    d4est_ghost_data->receive_strides[i] = D4EST_ALLOC(int, d4est_ghost_data->num_names);
  }
  
  d4est_ghost_data->transfer_types = D4EST_ALLOC(d4est_field_type_t, num_names);
  d4est_ghost_data->ghost_data_sizes = D4EST_ALLOC_ZERO(int, p4est->mpisize);
  d4est_ghost_data->receive_size = 0;

  
  for (int tn = 0; transfer_names[tn] != NULL; tn++){
    d4est_mesh_data_get_field_type
      (
       d4est_mesh_data,
       d4est_ghost_data->transfer_names[tn],
       &d4est_ghost_data->transfer_types[tn]
      );

    for (int gid = 0; gid < d4est_ghost->ghost->ghosts.elem_count; gid++){
      d4est_element_data_t* ghost_elem_data = &d4est_ghost->ghost_elements[gid];
      int ghost_rank = d4est_ghost->ghost_elements[gid].mpi_rank;
      int size = d4est_element_data_get_size_of_field
                 (
                  ghost_elem_data,
                  d4est_ghost_data->transfer_types[tn]
                 );
      d4est_ghost_data->ghost_data_sizes[ghost_rank] += size*sizeof(double);
      d4est_ghost_data->receive_size += size;
    }
  }

  d4est_ghost_data->receive_data = D4EST_ALLOC(double, d4est_ghost_data->receive_size);
  

  d4est_ghost_data_compute_ids_and_strides
    (
     d4est_ghost,
     d4est_ghost_data
    );
  
  return d4est_ghost_data;
}

void
d4est_ghost_data_destroy
(
 d4est_ghost_data_t* d4est_ghost_data
){
  for (int i = 0; i < d4est_ghost_data->num_names; i++){
    free(d4est_ghost_data->transfer_names[i]);
  }
  D4EST_FREE(d4est_ghost_data->transfer_names);
  D4EST_FREE(d4est_ghost_data->transfer_types);
  D4EST_FREE(d4est_ghost_data->ghost_data_sizes);
  
  for (int i = 0; i < d4est_ghost_data->num_ghosts; i++){
    D4EST_FREE(d4est_ghost_data->receive_strides[i]);
  }
  D4EST_FREE(d4est_ghost_data->receive_strides);
  D4EST_FREE(d4est_ghost_data->receive_data);

  d4est_ghost_data->receive_size = -1;
  d4est_ghost_data->num_ghosts = -1;
  d4est_ghost_data->num_names = -1;
  
  D4EST_FREE(d4est_ghost_data);
}



void
d4est_ghost_data_exchange
(
 p4est_t * p4est,
 d4est_ghost_t* d4est_ghost,
 d4est_ghost_data_t* d4est_ghost_data,
 d4est_mesh_data_t* d4est_mesh_data
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
    ng_incl = d4est_ghost->ghost->proc_offsets[q + 1];
    ng = ng_incl - ng_excl;
    D4EST_ASSERT (ng >= 0);
    if (ng > 0) {
      r = (sc_MPI_Request *) sc_array_push (&requests);

      /* this is wrong, check if ghost_data_sizes is sizeof(double) in it, otherwise its wrong */
      mpiret = sc_MPI_Irecv ((char *) (d4est_ghost_data->receive_data) + data_size_offset,
                             d4est_ghost_data->ghost_data_sizes[q], sc_MPI_BYTE, q,
                             P4EST_COMM_GHOST_EXCHANGE, p4est->mpicomm, r);
      SC_CHECK_MPI (mpiret);
      ng_excl = ng_incl;
      data_size_offset += d4est_ghost_data->ghost_data_sizes[q];
    }
  }
  D4EST_ASSERT (ng_excl == (p4est_locidx_t) d4est_ghost->ghost->ghosts.elem_count);

  /* send data to other processors */
  ng_excl = 0;
  for (q = 0; q < num_procs; ++q) {
    ng_incl = d4est_ghost->ghost->mirror_proc_offsets[q + 1];
    ng = ng_incl - ng_excl;
    D4EST_ASSERT (ng >= 0);
    if (ng > 0) {
      /* calculate total data size to send */
      int total_memsize = 0;
      for (theg = 0; theg < ng; ++theg){
        mirr = d4est_ghost->ghost->mirror_proc_mirrors[ng_excl + theg];
        D4EST_ASSERT (0 <= mirr && (size_t) mirr < d4est_ghost->ghost->mirrors.elem_count);
        for (int tn = 0; tn < d4est_ghost_data->num_names; tn++){
          total_memsize +=  sizeof(double)*
                            d4est_element_data_get_size_of_field
                            (
                             d4est_ghost->mirror_elements[mirr],
                             d4est_ghost_data->transfer_types[tn]
                            );

          /* printf("total_memsize = %d on proc %d\n", total_memsize, p4est->mpirank); */
        }
      }
      /* every peer populates its own send buffer */
      sbuf = (char **) sc_array_push (&sbuffers);
      mem = *sbuf = D4EST_ALLOC (char, total_memsize);

      for (theg = 0; theg < ng; ++theg) {
        mirr = d4est_ghost->ghost->mirror_proc_mirrors[ng_excl + theg];
        D4EST_ASSERT (0 <= mirr && (size_t) mirr < d4est_ghost->ghost->mirrors.elem_count);
        for (int tn = 0; tn < d4est_ghost_data->num_names; tn++){
          int mem_size = sizeof(double)*
                         d4est_element_data_get_size_of_field
                         (
                          d4est_ghost->mirror_elements[mirr],
                          d4est_ghost_data->transfer_types[tn]
                         );
          
          double* field = d4est_mesh_data_get_field(d4est_mesh_data, d4est_ghost_data->transfer_names[tn]);
          D4EST_ASSERT(field != NULL);
          
          int stride = d4est_element_data_get_stride_for_field(d4est_ghost->mirror_elements[mirr], d4est_ghost_data->transfer_types[tn]);
          memcpy (mem, &field[stride], mem_size);
          mem += mem_size;

          /* printf("mem_size = %d on proc %d\n", mem_size, p4est->mpirank); */
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
    D4EST_FREE (*sbuf);
  }
  sc_array_reset (&sbuffers);
  
}
