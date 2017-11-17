#include <d4est_base.h>
#include <d4est_mesh.h>
#include <d4est_field.h>
#include <d4est_element_data.h>

d4est_mesh_data_sizes_t
d4est_mesh_update_element_data
(
 p4est_t* p4est,
 int(*get_deg)(d4est_element_data_t*, void*),
 void* user
)
{
  d4est_mesh_data_sizes_t sizes;
  sizes.local_nodes = 0;
  int id_stride = 0;

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
        if(get_deg != NULL){
          ed->deg = get_deg(ed, user);
        }
        ed->strides.stride_volume_nodal = sizes.local_nodes;
        ed->mpi_rank = p4est->mpirank;
        ed->id = id_stride;
        sizes.local_nodes += d4est_element_data_get_size_of_field(ed, VOLUME_NODAL);
        id_stride++;
      }
    }
  
  return sizes;
}


double* d4est_mesh_get_field_on_element
(
 d4est_element_data_t* ed,
 const char* name,
 d4est_field_type_t type,
 d4est_mesh_data_t* lgd
){

  D4EST_ASSERT(ed->mpi_rank == lgd->mpi_rank);
  D4EST_ASSERT(lgd != NULL);
  double* field = d4est_mesh_data_get_field(lgd, name);
  D4EST_ASSERT(field != NULL);
  int stride = d4est_element_data_get_stride_for_field(ed, type);
  return &field[stride];
}



double* d4est_mesh_get_field_on_ghost
(
 d4est_element_data_t* ed,
 int name_id,
 const char* name,
 d4est_ghost_data_t* dgd
){
  int stride = dgd->receive_strides[ed->id][name_id];
  return &dgd->receive_data[stride];
}
