#ifndef D4EST_GHOST_H
#define D4EST_GHOST_H 

#include <d4est_base.h>
#include <d4est_element_data.h>
#include <d4est_mesh_data.h>


typedef struct {

  p4est_ghost_t* ghost;
  d4est_element_data_t* ghost_elements;
  d4est_mesh_data_t* ghost_grid_data;
  
} d4est_ghost_data_t;


/* This file was automatically generated.  Do not edit! */
void d4est_ghost_exchange_data(p4est_t *p4est,d4est_ghost_data_t *dgd,d4est_mesh_data_t *dmd,const char **transfer_names);
void d4est_ghost_update(d4est_ghost_data_t *dgd,p4est_t *p4est);
void d4est_ghost_destroy(d4est_ghost_data_t *dgd);
d4est_ghost_data_t *d4est_ghost_init(p4est_t *p4est);
d4est_mesh_data_sizes_t d4est_ghost_update_element_strides(d4est_ghost_data_t *dgd);

  

#endif
