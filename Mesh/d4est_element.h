#ifndef D4EST_ELEMENT_H
#define D4EST_ELEMENT_H 

/* This file was automatically generated.  Do not edit! */
double *d4est_mesh_get_field_on_element(d4est_element_data_t *ed,const char *name,d4est_field_type_t type,d4est_mesh_data_t *lgd,d4est_ghost_data_t *dgd);
int d4est_element_data_get_stride_for_field(d4est_element_data_t *ed,d4est_field_type_t type);
int d4est_element_data_get_size_of_field(d4est_element_data_t *ed,d4est_field_type_t type);



#endif
