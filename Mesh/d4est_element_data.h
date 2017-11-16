#ifndef D4EST_ELEMENT_H
#define D4EST_ELEMENT_H 

#define TESTS

#include <d4est_base.h>
#include <d4est_field.h>

#define MAX_DEGREE 8
#if (D4EST_DIM) == 3
#define MAX_NODES (MAX_DEGREE + 1) * (MAX_DEGREE + 1) * (MAX_DEGREE + 1)
#else
#define MAX_NODES (MAX_DEGREE + 1) * (MAX_DEGREE + 1)
#endif

typedef struct {

  int stride_volume_nodal;

} d4est_element_data_strides_t;

typedef struct {
  
  int mpi_rank; //process that owns this element
  int id; //process local id for element
  int deg; // nodal (LGL) degree
  d4est_element_data_strides_t strides; //used to get a field on element


#ifdef TESTS
  double test_data [MAX_NODES];
#endif

} d4est_element_data_t;

int d4est_element_data_get_stride_for_field(d4est_element_data_t *ed,d4est_field_type_t type);
int d4est_element_data_get_size_of_field(d4est_element_data_t *ed,d4est_field_type_t type);


#endif
