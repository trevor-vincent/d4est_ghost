#ifndef D4EST_MESH_H
#define D4EST_MESH_H 

#include <d4est_mesh_data.h>
#include <d4est_element_data.h>
#include <d4est_ghost_data.h>

/* This file was automatically generated.  Do not edit! */
double *d4est_mesh_get_field_on_element(d4est_element_data_t *ed,const char *name,d4est_field_type_t type,d4est_mesh_data_t *lgd, d4est_ghost_data_t *dgd);
d4est_mesh_data_sizes_t d4est_mesh_update_element_data(p4est_t *p4est,int(*get_deg)(d4est_element_data_t *,void *),void *user);

#endif
