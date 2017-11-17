#include <d4est_util.h>
#include <d4est_element_data.h>


int
d4est_element_data_initialize_sizes
(
 d4est_element_data_t* ed
)
{
  ed->sizes[VOLUME_NODAL] = d4est_util_dbl_pow_int(ed->deg+1, (D4EST_DIM));
  ed->sizes[FACE_POINT] = D4EST_FACES;
}

int
d4est_element_data_get_size_of_field
(
 d4est_element_data_t* ed,
 d4est_field_type_t type
)
{
  D4EST_FIELD_CHECK_TYPE(type);
  return ed->sizes[type];
}

int d4est_element_data_get_stride_for_field
(
 d4est_element_data_t* ed,
 d4est_field_type_t type
)
{
  D4EST_FIELD_CHECK_TYPE(type);
  return ed->strides[type];
}
