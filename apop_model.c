/** \file apop_model.c	 sets up the estimate structure which outputs from the various regressions and MLEs.*/
/* Copyright (c) 2006--2010 by Ben Klemens.  Licensed under the modified GNU GPL v2; see COPYING and COPYING2.  */

#include "arms.h"
#include "types.h"
#include "mapply.h"
#include "output.h"
#include "internal.h"
#include "settings.h"
#include "likelihoods.h"

/** Allocate an \ref apop_model.

This sets up the output elements of the \c apop_model: the parameters and info. 

At close, the input model has parameters of the correct size.

\li This is the default action for \ref apop_prep. If your model has its own \c prep method, then that gets used instead, but most don't (or call \ref apop_model_clear at the end of their prep routine).

\ref apop_estimate calls \ref apop_prep internally. 

The above two points mean that you probably don't need to call this function directly.

\param data If your params vary with the size of the data set, then the function needs a data set to calibrate against. Otherwise, it's OK to set this to \c NULL.
\param model    The model whose output elements will be modified.
\return A pointer to the same model, should you need it.

\ingroup models  */
apop_model * apop_model_clear(apop_data * data, apop_model *model){
  model->dsize  = (model->dsize == -1 ? data->matrix->size2 : model->dsize);
  int vsize  = model->vbase  == -1 ? data->matrix->size2 : model->vbase;
  int msize1 = model->m1base == -1 ? data->matrix->size2 : model->m1base ;
  int msize2 = model->m2base == -1 ? data->matrix->size2 : model->m2base ;
    apop_data_free(model->parameters);
    apop_data_free(model->info);
    model->parameters	= apop_data_alloc(vsize, msize1, msize2);
    model->info	        = apop_data_alloc(0, 0, 0);
    snprintf(model->info->names->title, 100, "Info");
    model->data         = data;
	return model;
}

/** Free an \ref apop_model structure.

The  \c parameters element is freed.  These are all the things that are completely copied, by \c apop_model_copy, so the parent model is still safe after this is called. \c data is not freed, because the odds are you still need it.

The function runs <tt>free(free_me->more)</tt>, but has no idea what the \c more element
contains; if it points to other structures (like an \ref apop_data set), you need to
free them before calling this function.

If \c free_me is \c NULL, this does nothing.

\param free_me A pointer to the model to be freed.

\ingroup models */
void apop_model_free (apop_model * free_me){
  int   i=0;
    if (!free_me) return;
    apop_data_free(free_me->parameters);
    if (free_me->settings){
        while (free_me->settings[i].name[0]){
            if (free_me->settings[i].free)
                ((void (*)(void*))(free_me->settings[i].free))(free_me->settings[i].setting_group);
            i++;
        }
        free(free_me->settings);
    }
	free(free_me);
}

/** Print the results of an estimation. If your model has a \c print
 method, then I'll use that, else I'll use a default setup.

 Your \c print method can use both by masking itself for a second:
 \code
void print_method(apop_model *in){
  void *temp = in->print;
  in->print = NULL;
  apop_model_show(in);
  in->print = temp;

  printf("Additional info:\n");
  ...
}
 \endcode

\ingroup output */
void apop_model_show (apop_model * print_me){
    if (print_me->print){
        print_me->print(print_me);
        return;
    }
    if (strlen(print_me->name))
        printf ("%s", print_me->name);
    printf("\n\n");
	if (print_me->parameters)
        apop_data_show(print_me->parameters);
    apop_data *cov = apop_data_get_page(print_me->parameters, "<Covariance>");
	if (cov){
		printf("\nThe covariance matrix:\n");
        apop_data_show(cov);
	}
    if (print_me->info)
        apop_data_show(print_me->info);
}

/** Currently an alias for \ref apop_model_show, but when I get
  around to it, it will conform better with the other apop_..._print
  fns.*/
void apop_model_print (apop_model * print_me){
    apop_model_show(print_me);
}

/** Outputs a copy of the \ref apop_model input.
\param in   The model to be copied
\return A pointer to a copy of the original, which you can mangle as you see fit. 

\li If <tt>in.more_size > 0</tt> I <tt>memcpy</tt> the \c more pointer from the original data set.

\ingroup models
*/
apop_model * apop_model_copy(apop_model in){
  apop_model * out = malloc(sizeof(apop_model));
    memcpy(out, &in, sizeof(apop_model));
    if (in.more_size){
        out->more  = malloc(in.more_size);
        memcpy(out->more, in.more, in.more_size);
    }
    int i=0; 
    out->settings = NULL;
    if (in.settings)
        do 
            apop_settings_copy_group(out, &in, in.settings[i].name);
        while (strlen(in.settings[i++].name));
    out->parameters = apop_data_copy(in.parameters);
    out->info = apop_data_copy(in.info);
    return out;
}

/** \def apop_model_set_parameters
Take in an unparameterized \c apop_model and return a new \c apop_model with the given parameters. This would have been called apop_model_parametrize, but the OED lists four acceptable spellings for parameterise, so it's not a great candidate for a function name.

For example, if you need a N(0,1) quickly: 
\code
apop_model *std_normal = apop_model_set_parameters(apop_normal, 0, 1);
\endcode

This doesn't take in data, so it won't work with models that take the number of parameters from the data, and it will only set the vector of the model's parameter \ref apop_data set. This is most standard models, so that's not a real problem either.
If you have a situation where these options are out, you'll have to do something like
<tt>apop_model *new = apop_model_copy(in); apop_model_clear(your_data, in);</tt> and then set \c in->parameters using your data.

  \param in An unparameterized model, like \ref apop_normal or \ref apop_poisson.
  \param ... The list of parameters.
  \return A copy of the input model, with parameters set.

\hideinitializer   \ingroup models
  */

apop_model *apop_model_set_parameters_base(apop_model in, double ap[]){
    apop_assert((in.vbase != -1) && (in.m1base != -1) && (in.m2base != -1),  NULL, 0, 's', "This function only works with models whose number of params does not depend on data size. You'll have to use apop_model *new = apop_model_copy(in); apop_model_clear(your_data, in); and then set in->parameters using your data.");
    apop_model *out = apop_model_copy(in);
    apop_prep(NULL, out);
    apop_data_fill_base(out->parameters, ap);
    return out; 
}

/** estimate the parameters of a model given data.

This function copies the input model, preps it, and calls \c
m.estimate(d,&m). If your model has no \c estimate method, then I
assume \c apop_maximum_likelihood(d, m), with the default MLE params.

I assume that you are using this function rather than directly calling the
model's the \c estimate method directly. For example, the \c estimate
method may assume that \c apop_prep has already been called.

\param d    The data
\param m    The model
\return     A pointer to an output model, which typically matches the input model but has its \c parameters element filled in.

\ingroup models
*/
apop_model *apop_estimate(apop_data *d, apop_model m){
    apop_model *out = apop_model_copy(m);
    apop_prep(d, out);
    if (out->estimate)
        return out->estimate(d, out); 
    return apop_maximum_likelihood(d, out);
}

/** Find the probability of a data/parametrized model pair.

\param d    The data
\param m    The parametrized model, which must have either a \c log_likelihood or a \c p method.

\ingroup models
*/
double apop_p(apop_data *d, apop_model *m){
    Nullcheck_m(m);
    if (m->p)
        return m->p(d, m);
    else if (m->log_likelihood)
        return exp(m->log_likelihood(d, m));
    apop_error(0, 's', "You asked for the log likelihood of a model that has neither p nor log_likelihood methods.\n");
    return 0;
}

/** Find the log likelihood of a data/parametrized model pair.

\param d    The data
\param m    The parametrized model, which must have either a \c log_likelihood or a \c p method.

\ingroup models
*/
double apop_log_likelihood(apop_data *d, apop_model *m){
    Nullcheck_m(m); //Nullcheck_p(m); //Too many models don't use the params.
    if (m->log_likelihood)
        return m->log_likelihood(d, m);
    else if (m->p)
        return log(m->p(d, m));
    apop_error(0, 's', "You asked for the log likelihood of a model that has neither p nor log_likelihood methods.\n");
    return 0;
}

/** Find the vector of derivatives of the log likelihood of a data/parametrized model pair.

\param d    The data
\param out  The score to be returned. I expect you to have allocated this already.
\param m    The parametrized model, which must have either a \c log_likelihood or a \c p method.

\ingroup models
*/
void apop_score(apop_data *d, gsl_vector *out, apop_model *m){
    Nullcheck_m(m); // Nullcheck_p(m);
    if (m->score){
        m->score(d, out, m);
        return;
    }
    gsl_vector * numeric_default = apop_numerical_gradient(d, m);
    gsl_vector_memcpy(out, numeric_default);
    gsl_vector_free(numeric_default);
}

#include "settings.h"

apop_pm_settings *apop_pm_settings_init(apop_pm_settings in){
   apop_pm_settings *out = malloc(sizeof (apop_pm_settings));
    apop_varad_setting(in, out, base, NULL);
    apop_varad_setting(in, out, index, 0);
    apop_varad_setting(in, out, rng, apop_rng_alloc(apop_opts.rng_seed++));
    if (in.rng)
        out->own_rng = 0;
    else
        out->own_rng = 1;
    apop_varad_setting(in, out, draws, 1e4);
    return out;
}

void *apop_pm_settings_copy(apop_pm_settings *copyme) {
    apop_pm_settings *out = malloc(sizeof(apop_pm_settings));
    *out = *copyme;
    out->rng = apop_rng_alloc(apop_opts.rng_seed++);
    out->own_rng = 1;
    return out; }

void apop_pm_settings_free(apop_pm_settings *freeme) {
    if (freeme->own_rng)
        gsl_rng_free(freeme->rng);
    free(freeme);
}

/** Get a model describing the distribution of the given parameter estimates.

  For many models, the parameter estimates are well-known, such as the
  \f$t\f$-distribution of the parameters for OLS.

  For models where the distribution of \f$\hat{}p\f$ is not known, if you give me data, I
  will return a \ref apop_normal or \ref apop_multivariate_normal model, using the parameter estimates as mean and \ref apop_bootstrap_cov for the variances.

  If you don't give me data, then I will assume that this is a stochastic model where 
  re-running the model will produce different parameter estimates each time. In this case, I will
  run the model 1e4 times and return a \ref apop_pmf model with the resulting parameter
  distributions.

  Before calling this, I expect that you have already run \ref apop_estimate to produce \f$\hat{}p\f$.

  The \ref apop_pm_settings structure dictates details of how the model is generated.
  For example, if you want only the distribution of the third parameter, and you know the
  distribution will be a PMF generated via random draws, then set settings and call the
  model via:
  \code
    apop_model_group_add(your_model, apop_pm, .index =3, .draws=3e5);
    apop_model *dist = apop_parameter_model(your_data, your_model);
  \endcode

  \li \c index gives the position of the parameter (in \ref apop_data_pack order)
  in which you are interested. Thus, if this is zero or more, then you will get a
  univariate output distribution describing a single parameter. If <tt>index == -1</tt>,
  then I will give you the multivariate distribution across all parameters.  The default
  is zero (i.e. the univariate distribution of the zeroth parameter).
  
  \li \c rng If the method requires random draws (as the default bootstrap will), then use this.
    If you provide \c NULL and one is needed, see the \ref autorng section on how one is provided for you.

  \li \c draws If there is no closed-form solution and bootstrap is inappropriate, then
  the last resort is a large numbr of random draws of the model, summarized into a PMF. Default: 1,000 draws.

*/ 
apop_model *apop_parameter_model(apop_data *d, apop_model *m){
    apop_pm_settings *settings = apop_settings_get_group(m, apop_pm);
    if (!settings)
        settings = apop_model_add_group(m, apop_pm, .base= m);
    if (m->parameter_model)
        return m->parameter_model(d, m);
    if (d){
        Get_vmsizes(m->parameters);//vsize, msize1, msize2
        apop_model *out = apop_model_copy(apop_multivariate_normal);
        out->m1base = out->vbase = out->m2base = out->dsize = vsize+msize1+msize2;
        out->parameters = apop_bootstrap_cov(d, *m, settings->rng, settings->draws);
        out->parameters->vector = apop_data_pack(m->parameters);
        if (settings->index == -1)
            return out;
        else {
            apop_model *out2 = apop_model_set_parameters(apop_normal, 
                    apop_data_get(out->parameters, settings->index, -1), //mean
                    apop_data_get(out->parameters, settings->index, settings->index)//var
                    );
            apop_model_free(out);
            return out2;
        }
    }
    //else
    Get_vmsizes(m->parameters);//vsize, msize1, msize2
    apop_data *param_draws = apop_data_alloc(0, settings->draws, vsize+msize1+msize2);
    for (int i=0; i < settings->draws; i++){
        apop_model *mm = apop_estimate (NULL, *m);//If you're here, d==NULL.
        Apop_row(param_draws, i, onerow);
        apop_data_pack(mm->parameters, onerow);
        apop_model_free(mm);
    }
    if (settings->index == -1)
        return apop_estimate(param_draws, apop_pmf);
    else {
        apop_data *param_draws1 = apop_data_alloc(settings->draws, 0,0);
        apop_col(param_draws, settings->index, the_draws);
        gsl_vector_memcpy(param_draws1->vector, the_draws);
        apop_data_free(param_draws);
        return apop_estimate(param_draws1, apop_pmf);
    }
}

/** draw from a model. If the model has its own RNG, then you're good to
 go; if not, use \ref apop_arms_draw to generate random draws.

That function has a lot of caveats: most notably, the input data will
be univariate, and your likelihood function must be nonnegative and sum
to one. If those aren't appropriate, then don't use this default. [A
more forgiving default is on the to-do list.]

 \ingroup models
*/
void apop_draw(double *out, gsl_rng *r, apop_model *m){
    if (m->draw)
        m->draw(out,r, m); 
    else
        apop_arms_draw(out, r, m);
}

/** The default prep is to simply call \c apop_model_clear. If the
 function has a prep method, then that gets called instead.

\ingroup models
 */
void apop_prep(apop_data *d, apop_model *m){
    if (m->prep)
        m->prep(d, m);
    else
        apop_model_clear(d, m);
}

static double disnan(double in) {return gsl_isnan(in);}

/** A prediction supplies E(a missing value | original data, already-estimated parameters, and other supplied data elements ).

For a regression, one would first estimate the parameters of the model, then supply a row of predictors <b>X</b>. The value of the dependent variable \f$y\f$ is unknown, so the system would predict that value. [In some models, this may not be the expected value, but is a best value for the missing item using some other meaning of `best'.]

For a univariate model (i.e. a model in one-dimensional data space), there is only one variable to omit and fill in, so the prediction problem reduces to the expected value: E(a missing value | original data, already-estimated parameters).

In other cases, prediction is the missing data problem: for three-dimensional data, you may supply the input (34, \c NaN, 12), and the parameterized model provides the most likely value of the middle
parameter.

\li If you give me a \c NULL data set, I will assume you want all values filled in---the expected value.

\li If you give me data with NaNs, I will take those as the points to
be predicted given the provided data.

If the model has no \c predict method, the default is to use the \ref apop_ml_impute function to do the work.

\return If you gave me a non-\c NULL data set, I will return that, with the zeroth column or the NaNs filled in.  If \c NULL input, I will allocate an \ref apop_data set and fill it with the expected values.

There may be a second page (i.e., a \ref apop_data set attached to the <tt>->more</tt> pointer of the main) listing confidence and standard error information. See your specific model documentation for details.

This segment of the framework is in beta---subject to revision of the details.

\ingroup models
  */
apop_data *apop_predict(apop_data *d, apop_model *m){
    apop_data *prediction = NULL;
    apop_data *out = d ? d : apop_data_alloc(0, 1, m->dsize);
    if (!d)
        gsl_matrix_set_all(out->matrix, GSL_NAN);
    if (m->predict)
        prediction = m->predict(out, m);
    if (prediction)
        return prediction;
    if (!apop_map_sum(out, disnan))
        return out;
    apop_model *f = apop_ml_imputation(out, m);
    apop_model_free(f);
    return out;
}

/* Are all the elements of v less than or equal to the corresponding elements of the reference vector? */
static int lte(gsl_vector *v, gsl_vector *ref){
    for (int i=0; i< v->size; i++) 
        if(v->data[i] > gsl_vector_get(ref, i))
            return 0;
    return 1;
}

apop_cdf_settings *apop_cdf_settings_init(apop_cdf_settings in){
    apop_cdf_settings *out = malloc(sizeof(apop_cdf_settings));
    apop_varad_setting(in, out, cdf_model, NULL);
    apop_varad_setting(in, out, draws, 1e4);
    apop_varad_setting(in, out, rng, apop_rng_alloc(++apop_opts.rng_seed));
    return out;
}

void apop_cdf_settings_free(apop_cdf_settings *in){
    gsl_rng_free(in->rng);
    apop_model_free(in->cdf_model);
    free(in);
}

void *apop_cdf_settings_copy(apop_cdf_settings *in){
    apop_cdf_settings *out = malloc(sizeof(apop_cdf_settings));
    *out = *in;
    return out;
}

/** Input a data point in canonical form and a model; returns the area of the model's PDF beneath the given point.

  By default, I just make random draws from the PDF and return the percentage of those
  draws beneath or equal to the given point. Many models have closed-form solutions that
  make no use of random draws. 
  */
double apop_cdf(apop_data *d, apop_model *m){
    if (m->cdf)
        return m->cdf(d, m);
    apop_cdf_settings *cs = Apop_settings_get_group(m, apop_cdf);
    if (!cs)
        cs = Apop_model_add_group(m, apop_cdf);
    int tally = 0; 
    gsl_vector *v= gsl_vector_alloc(d->matrix->size2);
    Apop_row(d, 0, ref);
    for (int i=0; i< cs->draws; i++){
        apop_draw(v->data, cs->rng, m);
        tally += lte(v, ref);
    }
    gsl_vector_free(v);
    return tally/(double)cs->draws;
}
