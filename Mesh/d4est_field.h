#ifndef D4EST_FIELD_H
#define D4EST_FIELD_H 

#include <d4est_base.h>

/* more types will be added in the future */
typedef enum {
  VOLUME_NODAL,
  NO_TYPE
} d4est_field_type_t;

typedef struct {

  double* field_data;
  d4est_field_type_t field_type;
  
} d4est_field_t;

#endif
