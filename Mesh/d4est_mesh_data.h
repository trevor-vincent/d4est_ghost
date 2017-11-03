#ifndef D4EST_MESH_DATA_H
#define D4EST_MESH_DATA_H

#include <d4est_dictionary.h>
#include <d4est_field.h>


typedef double
(*d4est_mesh_data_init_fcn_t)
(
 double,
 double,
#if (D4EST_DIM)==3
 double,
#endif 
 void*
);


typedef struct {
  
  int local_nodes;       /**< size of local nodal space */
  
} d4est_mesh_data_sizes_t;


typedef struct {
  int mpi_rank; /**< mpirank or id of subdomain */

  d4est_dictionary_t fields; /**< dictionary for fields defined on local grid */
  d4est_mesh_data_sizes_t field_sizes;
 
} d4est_mesh_data_t;

/* This file was automatically generated.  Do not edit! */
void d4est_mesh_data_destroy(d4est_mesh_data_t *s);
d4est_mesh_data_t *d4est_mesh_data_init(int mpi_rank,d4est_mesh_data_sizes_t *loc_sizes);
void d4est_mesh_data_update(d4est_mesh_data_t *s,d4est_mesh_data_sizes_t *loc_sizes);
void d4est_mesh_data_get_vector_field(d4est_mesh_data_t *s,const char *prefix,double *vfield[(D4EST_DIM)]);
void d4est_mesh_data_delete_field(d4est_mesh_data_t *s,const char *name);
int d4est_mesh_data_add_field(d4est_mesh_data_t *s,const char *name,d4est_field_type_t field_type);
int d4est_mesh_data_get_field_type(d4est_mesh_data_t *s,const char *name,d4est_field_type_t *type);
double *d4est_mesh_data_get_field(d4est_mesh_data_t *s,const char *name);
void d4est_mesh_data_init_field(d4est_mesh_data_t *s,const char *name,const char *x,const char *y,const char *z,d4est_mesh_data_init_fcn_t init_fcn,void *arg);

#endif
