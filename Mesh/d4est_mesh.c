#include <d4est_base.h>
#include <d4est_mesh.h>
#include <d4est_field.h>
#include <d4est_util.h>
#include <d4est_element_data.h>

d4est_mesh_data_sizes_t
d4est_mesh_update_element_data
(
 p4est_t* p4est,
 int(*get_deg)(d4est_element_data_t*, void*),
 int field_sizes [D4EST_FIELD_TYPES],
 void* user
)
{
  
  for (int i = 0; i < D4EST_FIELD_TYPES; i++){
    field_sizes[i] = 0;
  }
  
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
        
        d4est_element_data_initialize_sizes(ed);
        ed->mpi_rank = p4est->mpirank;
        ed->id = id_stride;       

        for (int i = 0; i < D4EST_FIELD_TYPES; i++){
          ed->strides[i] = field_sizes[i];
          field_sizes[i] += ed->sizes[i];
        }
        id_stride++;
      }
    }
  
}


double* d4est_mesh_get_field_on_element
(
 d4est_element_data_t* ed,
 const char* name,
 d4est_field_type_t type,
 double* field_on_mesh_if_available, /* can be NULL if not available */
 d4est_mesh_data_t* dmd /* cannot be NULL if field_on_mesh_if_available is NULL */
){
  if (field_on_mesh_if_available == NULL){
    D4EST_ASSERT(dmd != NULL);
    field_on_mesh_if_available = d4est_mesh_data_get_field(dmd, name);
  }  
  D4EST_ASSERT(field_on_mesh_if_available != NULL);
  int stride = d4est_element_data_get_stride_for_field(ed, type);
  return &field_on_mesh_if_available[stride];
}



double* d4est_mesh_get_field_on_ghost
(
 d4est_element_data_t* ed,
 int ghost_name_id,
 const char* name,
 d4est_ghost_data_t* dgd
){
  D4EST_ASSERT(dgd != NULL);
  if (ghost_name_id == -1){
    D4EST_ASSERT(name != NULL);
    for (int i = 0; i < dgd->num_names; i++){
      if(d4est_util_compare_strings(name, dgd->transfer_names[i])){
        ghost_name_id = i;
        break;
      }
    }
  }
  D4EST_ASSERT(ghost_name_id >= 0 && ghost_name_id < dgd->num_names);
  if (name != NULL){
    D4EST_ASSERT(d4est_util_compare_strings(dgd->transfer_names[ghost_name_id], name));
  }
  int stride = dgd->receive_strides[ed->id][ghost_name_id];
  return &dgd->receive_data[stride];
}


double* d4est_mesh_get_field_on_element_or_ghost
(
 d4est_element_data_t* ed,
 const char* name,
 d4est_field_type_t type,
 double* field_on_local_mesh_if_available,
 d4est_mesh_data_t* dmd,
 d4est_ghost_data_t* dgd,
 int ghost_name_id //for faster lookup
)
{
  D4EST_ASSERT(dmd != NULL && dgd != NULL);
  if (ed->mpi_rank == dmd->mpi_rank){
    d4est_mesh_get_field_on_element(ed, name, type, field_on_local_mesh_if_available, dmd);
  }
  else {
    d4est_mesh_get_field_on_ghost(ed, ghost_name_id, name, dgd);
  }

}
