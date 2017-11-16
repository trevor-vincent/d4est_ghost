#include <d4est_mesh_data.h>
#include <d4est_element_data.h>
#include <d4est_element.h>
#include <d4est_util.h>

int
d4est_element_data_get_size_of_field
(
 d4est_element_data_t* ed,
 d4est_field_type_t type
)
{
  switch(type){
  case VOLUME_NODAL: return d4est_util_dbl_pow_int(ed->deg+1, (D4EST_DIM));
  default: D4EST_ABORT("Not a supported field type");
  }
}

int d4est_element_data_get_stride_for_field
(
 d4est_element_data_t* ed,
 d4est_field_type_t type
)
{
  switch(type){
  case VOLUME_NODAL: return ed->strides.stride_volume_nodal;
  default: D4EST_ABORT("Not a supported field type");
  }
}


double* d4est_element_data_get_field_on_element
(
 d4est_element_data_t* ed,
 const char* name,
 d4est_field_type_t type,
 d4est_mesh_data_t* lgd
){
  D4EST_ASSERT(ed->mpi_rank == lgd->mpi_rank && lgd != NULL);
  double* field = d4est_mesh_data_get_field(lgd, name);
  D4EST_ASSERT(field != NULL);
  int stride = d4est_element_data_get_stride_for_field(ed, type);
  return &field[stride];
  
}


