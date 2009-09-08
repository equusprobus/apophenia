/** \file stats.h */
/* Copyright (c) 2005--2009 by Ben Klemens.  Licensed under the modified GNU GPL v2; see COPYING and COPYING2. */
#ifndef APOP_STATS_H
#define APOP_STATS_H
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_histogram.h>
#include <gsl/gsl_statistics_double.h>
#include "linear_algebra.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define APOP_SUBMATRIX(m, srow, scol, nrows, ncols, o) gsl_matrix apop_mm_##o = gsl_matrix_submatrix(m, (srow), (scol), (nrows),(ncols)).matrix;\
gsl_matrix * o = &( apop_mm_##o );

#define APOP_MATRIX_ROW(m, row, v) gsl_vector apop_vv_##v = gsl_matrix_row(m, (row)).vector;\
gsl_vector * v = &( apop_vv_##v );

#define APOP_MATRIX_COL(m, col, v) gsl_vector apop_vv_##v = gsl_matrix_column(m, (col)).vector;\
gsl_vector * v = &( apop_vv_##v );

#define APOP_ROW_T(m, row, v) gsl_vector apop_vv_##v = gsl_matrix_row((m)->matrix, apop_name_find((m)->names, row, 'r')).vector;\
gsl_vector * v = &( apop_vv_##v );

#define APOP_COL_T(m, col, v) gsl_vector apop_vv_##v = gsl_matrix_column((m)->matrix, apop_name_find((m)->names, col, 'c')).vector;\
gsl_vector * v = &( apop_vv_##v );

#define APOP_ROW(m, row, v) gsl_vector apop_vv_##v = gsl_matrix_row((m)->matrix, (row)).vector;\
gsl_vector * v = &( apop_vv_##v );

#define APOP_COL(m, col, v) gsl_vector apop_vv_##v = gsl_matrix_column((m)->matrix, (col)).vector;\
gsl_vector * v = &( apop_vv_##v );

#define Apop_col APOP_COL 
#define Apop_row APOP_ROW
#define Apop_col_t APOP_COL_T
#define Apop_row_t APOP_ROW_T
#define Apop_matrix_col APOP_MATRIX_COL 
#define Apop_matrix_row APOP_MATRIX_ROW
#define Apop_submatrix APOP_SUBMATRIX
#define apop_vector_kurt(in) apop_vector_kurtosis(in)

long double apop_vector_sum(const gsl_vector *in);
double apop_var(const gsl_vector *in);
double apop_vector_var_m(const gsl_vector *in, const double mean);
double apop_vector_cov(const gsl_vector *ina, const gsl_vector *inb);
double apop_vector_correlation(const gsl_vector *ina, const gsl_vector *inb);
double apop_vector_kurtosis_pop(const gsl_vector *in);
double apop_vector_kurtosis(const gsl_vector *in);
double apop_vector_skew(const gsl_vector *in);
double apop_vector_skew_pop(const gsl_vector *in);
double apop_vector_weighted_mean(const gsl_vector *, const gsl_vector *);
double apop_vector_weighted_var(const gsl_vector *v, const gsl_vector *w);
double apop_vector_weighted_cov(const gsl_vector *, const gsl_vector *, const gsl_vector *);
double apop_vector_weighted_skew(const gsl_vector *v, const gsl_vector *w);
double apop_vector_weighted_kurt(const gsl_vector *v, const gsl_vector *w);

#define apop_sum(in) apop_vector_sum(in)
#define apop_var(in) apop_vector_var(in) 
#define apop_vector_covar(in) apop_vector_cov(in) 
#define apop_mean(in) apop_vector_mean(in)
#define apop_vector_mean(in)  gsl_stats_mean((in)->data,(in)->stride, (in)->size)
#define apop_vector_var(in)  gsl_stats_variance((in)->data,(in)->stride, (in)->size)

APOP_VAR_DECLARE double apop_vector_distance(const gsl_vector *ina, const gsl_vector *inb, const char metric, const double norm);
double apop_vector_grid_distance(const gsl_vector *ina, const gsl_vector *inb);


APOP_VAR_DECLARE void apop_vector_normalize(gsl_vector *in, gsl_vector **out, const char normalization_type);
void apop_matrix_normalize(gsl_matrix *data, const char row_or_col, const char normalization);

APOP_VAR_DECLARE double apop_random_double(double min, double max, gsl_rng *r);
APOP_VAR_DECLARE int apop_random_int(double min, double max, const gsl_rng *r);

APOP_VAR_DECLARE gsl_matrix * apop_matrix_covariance(gsl_matrix *in, const char normalize);
APOP_VAR_DECLARE  gsl_matrix * apop_matrix_correlation(gsl_matrix *in, const char normalize);
apop_data * apop_data_covariance(const apop_data *in);
apop_data * apop_data_correlation(const apop_data *in);
long double apop_matrix_sum(const gsl_matrix *m);
double apop_matrix_mean(const gsl_matrix *data);
double apop_matrix_var_m(const gsl_matrix *data, double mean);
void apop_matrix_mean_and_var(const gsl_matrix *data, double *mean, double *var);
apop_data * apop_data_summarize(apop_data *data);

//from apop_fisher.c:
apop_data *apop_test_fisher_exact(apop_data *intab);

//from apop_t_f_chi.c:
APOP_VAR_DECLARE int apop_matrix_is_positive_semidefinite(gsl_matrix *m, char semi);
double apop_matrix_to_positive_semidefinite(gsl_matrix *m);
double apop_multivariate_gamma(double a, double p);
double apop_multivariate_lngamma(double a, double p);

//from the regression code:
#define apop_F_test apop_f_test
#define apop_estimate_r_squared(in) apop_estimate_coefficient_of_determination(in)

apop_data *	apop_t_test(gsl_vector *a, gsl_vector *b);
apop_data *	apop_paired_t_test(gsl_vector *a, gsl_vector *b);


apop_data * apop_text_unique_elements(const apop_data *d, size_t col);
gsl_vector * apop_vector_unique_elements(const gsl_vector *v);
apop_data *apop_text_to_factors(apop_data *d, size_t textcol, int datacol);

APOP_VAR_DECLARE apop_data * apop_data_to_dummies(apop_data *d, int col, char type, int keep_first);
APOP_VAR_DECLARE apop_data * apop_f_test (apop_model *est, apop_data *contrast, int normalize);

apop_model *apop_estimate_fixed_effects_OLS(apop_data *data, gsl_vector *categories);

apop_data *apop_estimate_coefficient_of_determination (apop_model *in);
apop_data *apop_estimate_r_squared (apop_model *in);
void apop_estimate_parameter_t_tests (apop_model *est);

//apop_testing.c
apop_data* apop_anova(char *table, char *data, char *grouping1, char *grouping2);

#define apop_ANOVA(table, data, grouping1, grouping2) apop_anova(table, data, grouping1, grouping2)

#ifdef	__cplusplus
}
#endif
#endif

/** \def Apop_submatrix(m, srow, scol, nrow, ncol)
Pull a pointer to a submatrix into a \c gsl_matrix 

 \param m The root matrix
 \param srow the first row (in the root matrix) of the top of the submatrix
 \param scol the first column (in the root matrix) of the left edge of the submatrix
 \param nrow number of rows in the submatrix
 \param ncol number of columns in the submatrix
\hideinitializer */

/** \def Apop_row_t(m, row_name, v)
 After this call, \c v will hold a vector view of the <tt>row</tt>th row of \c m.
 Unlike \ref Apop_row, the second argument is a row name, that I'll look up using \ref apop_name_find.
\hideinitializer */

/** \def Apop_col_t(m, col_name, v)
 After this call, \c v will hold a vector view of the <tt>col</tt>th column of \c m.
 Unlike \ref Apop_col, the second argument is a column name, that I'll look up using \ref apop_name_find.
\hideinitializer */

/** \def Apop_matrix_row(m, row, v)
 After this call, \c v will hold a vector view of the <tt>row</tt>th row of \c m.
\hideinitializer */

/** \def Apop_matrix_col(m, col, v)
 After this call, \c v will hold a vector view of the <tt>col</tt>th column of \c m.
\hideinitializer */

/** \def Apop_row(d, row, v)
 After this call, \c v will hold a vector view of the <tt>row</tt>th row of \ref apop_data set \c d.
\hideinitializer */

/** \def Apop_col(d, col, v)
 After this call, \c v will hold a vector view of the <tt>col</tt>th column of \ref apop_data set \c d.
\hideinitializer */

/** \def apop_mean(v)
 Returns the mean of the elements of the vector \c v.
\hideinitializer */

/** \def apop_vector_mean(v)
 Returns the mean of the elements of the vector \c v.
\hideinitializer */

/** \def apop_vector_var(v)
 Returns the sample variance of the elements of the vector \c v.
\hideinitializer */
