#ifndef D4EST_BASE_H
#define D4EST_BASE_H 

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <float.h>
#include <string.h>
#include <sc.h>
#include <math.h>


#ifndef D4EST_DIM
#error "D4EST_DIM (dimension) not defined"
#elif (D4EST_DIM>3||D4EST_DIM<2)
#error "D4EST_DIM must be 2 or 3"
#elif D4EST_DIM==3
#include <p4est_to_p8est.h>
#endif

#ifndef P4_TO_P8
#include <p4est_vtk.h>
#include <p4est_bits.h>
#include <p4est_extended.h>
#include <p4est_iterate.h>
#include <p4est_lnodes.h>
#include <p4est_nodes.h>
#else
#include <p8est_vtk.h>
#include <p8est_bits.h>
#include <p8est_extended.h>
#include <p8est_iterate.h>
#include <p8est_lnodes.h>
#include <p8est_nodes.h>
#endif

#define D4EST_NOOP()                            \
  do                                            \
    {                                           \
    } while (0)

#define D4EST_ASSERT(c) SC_CHECK_ABORT ((c), "Assertion '" #c "'")
#define D4EST_ABORT(c) SC_ABORT(c)

/* #define D4EST_ALLOC(a,b) malloc((sizeof(a))*b) */
/* #define D4EST_FREE(a) free(a) */
/* #define D4EST_ALLOC_ZERO(a,b) calloc(a,b) */
/* #define D4EST_REALLOC(a,b,c) realloc(a,(sizeof(b))*c) */

#define D4EST_ALLOC(a,b) P4EST_ALLOC(a,b)
#define D4EST_FREE(a) P4EST_FREE(a)
#define D4EST_ALLOC_ZERO(a,b) P4EST_ALLOC_ZERO(a,b)
#define D4EST_REALLOC(a,b,c) P4EST_REALLOC(a,b,c)

/* Safer asprintf, see 21st century C book */
#define D4EST_ASPRINTF(write_to, ...) {              \
    char *tmp_string_for_extend = (write_to);        \
    asprintf(&(write_to), __VA_ARGS__);              \
    free(tmp_string_for_extend);                     \
  }



#endif
