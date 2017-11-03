#ifndef D4EST_ELEMENT_H
#define D4EST_ELEMENT_H 

#include <d4est_base.h>
#include <d4est_field.h>

typedef struct {

  int stride_volume_nodal;

} d4est_element_data_strides_t;

typedef struct {
  
  int mpi_rank; //process that owns this element
  int deg; // nodal (LGL) degree
  d4est_element_data_strides_t strides; //used to get a field on element
    
} d4est_element_data_t;

/* This file was automatically generated.  Do not edit! */
int d4est_element_data_get_stride(d4est_element_data_t *ed,d4est_field_type_t type);
int d4est_element_data_get_size_of_field(d4est_element_data_t *ed,d4est_field_type_t type);

#endif
