#ifndef D4EST_UTIL_H
#define D4EST_UTIL_H 

/* This file was automatically generated.  Do not edit! */
int d4est_util_compare_strings(const char *str1,const char *str2);
int d4est_util_compare_vecs(double *a,double *b,int N,double eps);
int d4est_util_compare_double(double a,double b,double eps);
void d4est_util_copy_1st_to_2nd(double *v1,double *v2,int N);
double d4est_util_dbl_pow_int(double a,int b);

#endif
