#include <d4est_base.h>
#include <d4est_mesh_data.h>
#include <d4est_field.h>

static inline
int d4est_mesh_data_get_size_of_field
(
 d4est_mesh_data_sizes_t* sizes,
 d4est_field_type_t type
)
{
  /* in the future there will be multiple types */
  switch(type){
  case VOLUME_NODAL: return sizes->local_nodes;
  default: D4EST_ABORT("Not a supported field type");
  }
}

static
int d4est_mesh_data_free_fields_callback(const char *key, void *val, void *arg)
{
  d4est_field_t* field = (d4est_field_t*)val;
  field->field_type = NO_TYPE;
  D4EST_FREE(field->field_data);
  D4EST_FREE(field);
  return 1;
}

static
int d4est_mesh_data_realloc_fields_callback(const char *key, void *val, void *arg)
{
  d4est_field_t* field = val;
  d4est_mesh_data_sizes_t* sizes = arg;
  int size = d4est_mesh_data_get_size_of_field(sizes, field->field_type);
  field->field_data = D4EST_REALLOC(field->field_data, double, size);
  return 1;
}

void
d4est_mesh_data_init_field
(
 d4est_mesh_data_t *s,
 const char *name,
 const char* x,
 const char* y,
 const char* z,
 d4est_mesh_data_init_fcn_t init_fcn,
 void *arg
)
{
  double* restrict field = d4est_mesh_data_get_field(s, name); 
  double* restrict xf = d4est_mesh_data_get_field(s, x);
  double* restrict yf = d4est_mesh_data_get_field(s, y);
#if (D4EST_DIM)==3
  double* restrict zf = d4est_mesh_data_get_field(s, z);
#endif

  if (field == NULL ||
#if (D4EST_DIM)==3
      zf == NULL ||
#endif
      yf == NULL ||
      xf == NULL
     ){D4EST_ABORT("Init field error, can't find field, x, y, or z arrays");}

  d4est_field_type_t typef, typex, typey, typez;
  int rval = d4est_mesh_data_get_field_type(s, name, &typef); D4EST_ASSERT(rval == 1);
  rval = d4est_mesh_data_get_field_type(s, x, &typex); D4EST_ASSERT(rval == 1);
  rval = d4est_mesh_data_get_field_type(s, y, &typey); D4EST_ASSERT(rval == 1);
  
#if (D4EST_DIM)==3
  rval = d4est_mesh_data_get_field_type(s, z, &typez); D4EST_ASSERT(rval == 1);
  D4EST_ASSERT(typey == typez);
#endif
  D4EST_ASSERT(typef == typex && typex == typey);
  
  int size = d4est_mesh_data_get_size_of_field(&s->field_sizes, typef);
  for (int i = 0; i < size; i++){
    field[i] = init_fcn
               (
                xf[i],
                yf[i],
#if (D4EST_DIM)==3
                zf[i],
#endif
                arg
               );
  }
}

int
d4est_mesh_data_add_field
(
 d4est_mesh_data_t *s,
 const char *name,
 d4est_field_type_t field_type
)
{
  int size = d4est_mesh_data_get_size_of_field(&s->field_sizes, field_type);
  if(d4est_dictionary_get_value_ptr(&s->fields,name))
    return 1;

  d4est_field_t* field = D4EST_ALLOC(d4est_field_t, 1);
  double* field_data = D4EST_ALLOC(double, size);

  field->field_data = field_data;
  field->field_type = field_type;
  
  int rval = d4est_dictionary_insert_ptr(&s->fields, name, field);

  D4EST_ASSERT(rval != 1);
  if(rval == 0){
    D4EST_FREE(field_data);
    D4EST_FREE(field);
    D4EST_ABORT("Out of memory, can't store field in dictionary\n");
  }  
  return rval;
}

void
d4est_mesh_data_delete_field
(
 d4est_mesh_data_t *s,
 const char *name
)
{
  d4est_field_t* field = d4est_dictionary_get_value_ptr(&s->fields, name);
  if (field == NULL){
    D4EST_ABORT("Could not find field to delete");
    return;
  }
  field->field_type = NO_TYPE;
  D4EST_FREE(field->field_data);
  D4EST_FREE(field);
  if(d4est_dictionary_delete(&s->fields, name) == 0){
    D4EST_ABORT("Could not delete field from dictionary\n");
  }
}

double*
d4est_mesh_data_get_field
(
 d4est_mesh_data_t* s,
 const char *name
)
{
  d4est_field_t* field = d4est_dictionary_get_value_ptr(&s->fields, name);
  if (field == NULL)
    return NULL;
  else
    return field->field_data;
}

void
d4est_mesh_data_get_vector_field
(
 d4est_mesh_data_t* s,
 const char *prefix,
 double* vfield [(D4EST_DIM)]
)
{
  char* vx = NULL;
  char* vy = NULL;
  d4est_asprintf(&vx, "%sx", prefix);
  d4est_asprintf(&vy, "%sy", prefix);
  vfield[0] = d4est_mesh_data_get_field(s, vx);
  vfield[1] = d4est_mesh_data_get_field(s, vy);
  free(vx);
  free(vy);
  if ((D4EST_DIM) == 3){
    char* vz = NULL;
    d4est_asprintf(&vz, "%sz", prefix);
    vfield[2] = d4est_mesh_data_get_field(s, vz);
    free(vz);
  }
}


int
d4est_mesh_data_get_field_type
(
 d4est_mesh_data_t* s,
 const char *name,
 d4est_field_type_t* type
)
{
  d4est_field_t* field = d4est_dictionary_get_value_ptr(&s->fields, name);
  if (field == NULL){
    type = NULL;
    return 0;
  }
  else {
    *type = field->field_type;
    return 1;
  }        
}

void
d4est_mesh_data_update
(
 d4est_mesh_data_t* s,
 d4est_mesh_data_sizes_t* loc_sizes
)
{
  s->field_sizes.local_nodes = loc_sizes->local_nodes;
  d4est_dictionary_allprefixed_ptr(&s->fields, "",
                                   &d4est_mesh_data_realloc_fields_callback, &s->field_sizes);
}

d4est_mesh_data_t*
d4est_mesh_data_init
(
 int mpi_rank,
 d4est_mesh_data_sizes_t* loc_sizes
)
{
  d4est_mesh_data_t* s = D4EST_ALLOC(d4est_mesh_data_t, 1);

  s->mpi_rank = mpi_rank;
  s->field_sizes.local_nodes = loc_sizes->local_nodes;

  d4est_dictionary_init(&s->fields);
  
  return s;
}

void
d4est_mesh_data_destroy
(
 d4est_mesh_data_t *s
)
{
  /* delete all fields */
  d4est_dictionary_allprefixed_ptr(&s->fields, "",
                                   &d4est_mesh_data_free_fields_callback, NULL);

  d4est_dictionary_clear(&s->fields);

  s->mpi_rank = -1;
  s->field_sizes.local_nodes = -1;

  D4EST_FREE(s);
}
