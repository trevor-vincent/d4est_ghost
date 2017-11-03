#include <d4est_base.h>
#include <d4est_field.h>
#include <d4est_util.h>
#include <d4est_element_data.h>

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

int d4est_element_data_get_stride
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
