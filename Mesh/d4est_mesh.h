#ifndef D4EST_MESH_H
#define D4EST_MESH_H 

#include <d4est_mesh_data.h>
#include <d4est_element_data.h>
#include <d4est_ghost_data.h>

/* This file was automatically generated.  Do not edit! */
double *d4est_mesh_get_field_on_element_or_ghost(d4est_element_data_t *ed,const char *name,d4est_field_type_t type,double *field_on_local_mesh_if_available,d4est_mesh_data_t *dmd,d4est_ghost_data_t *dgd,int ghost_name_id);
double *d4est_mesh_get_field_on_ghost(d4est_element_data_t *ed,int ghost_name_id,const char *name,d4est_ghost_data_t *dgd);
double *d4est_mesh_get_field_on_element(d4est_element_data_t *ed,const char *name,d4est_field_type_t type,double *field_on_mesh_if_available,d4est_mesh_data_t *dmd);
d4est_mesh_data_sizes_t d4est_mesh_update_element_data(p4est_t *p4est,int(*get_deg)(d4est_element_data_t *,void *),int field_sizes[D4EST_FIELD_TYPES],void *user);

#endif
