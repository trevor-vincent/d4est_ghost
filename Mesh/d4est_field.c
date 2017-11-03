#include <d4est_field.h>


/* static inline */
/* int d4est_field_get_local_stride */
/* ( */
/*  d4est_field_local_strides_t* strides, */
/*  d4est_field_type_t type */
/* ) */
/* { */
/*   switch(type){ */
/*   case NODAL: return strides->stride_nodal; */
/*   case TRACE: return strides->stride_trace;  */
/*   case VOLUME: return strides->stride_volume; */
/*   case FACE: return (D4EST_FACES)*strides->stride_volume; */
/*   default: D4EST_ABORT("Not a supported field type"); */
/*   }  */
/* } */

/* static inline */
/* int d4est_field_get_element_size */
/* ( */
/*  d4est_field_type_t type, */
/*  int deg */
/* ) */
/* { */
/*   switch(type){ */
/*   case NODAL: return d4est_lgl_get_nodes((D4EST_DIM), deg); */
/*   case TRACE: return (D4EST_FACES)*d4est_lgl_get_nodes((D4EST_DIM) - 1, deg); */
/*   case VOLUME: return 1; */
/*   case FACE: return (D4EST_FACES);   */
/*   default: D4EST_ABORT("Not a supported field type"); */
/*   } */
/* } */
