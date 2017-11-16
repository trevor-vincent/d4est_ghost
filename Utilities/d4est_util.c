#include <d4est_util.h>

double
d4est_util_dbl_pow_int(double a, int b){
   if (b == 0)
    {
        /* base case: anything to the 0 power is 1 */
        return 1.0;
    }
    else if (a == 0.0)
    {
        /* save us some time, 0 to any power other than 0 is 0 */
        return 0.0;
    }
    else if (b < 0)
    {
        /* b is negative, take the reciprocal of the positive version */
        return 1.0 / d4est_util_dbl_pow_int(a, -b);
    }
    else
    {
        /* b is positive, normal recursion */
        double result = d4est_util_dbl_pow_int(a, b / 2);
        result *= result;
        if (b % 2 != 0)
        {
            /* account for the truncation of b / 2 due to integer division */
            result *= a;
        }
        return result;
    }
}



void
d4est_util_copy_1st_to_2nd (double *v1, double *v2, int N)
{
  memcpy (v2, v1, N * sizeof (double));
}
