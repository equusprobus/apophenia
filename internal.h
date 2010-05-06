/* These are functions used here and there to write Apophenia. They're
 not incredibly useful, or even very good form, so they're not public. Cut
 & paste `em into your own code if you'd like.
 */


/* Many Apop functions try to treat the vector and matrix equally, which
 requires knowing which exists and what the sizes are. */
#define Get_vmsizes(d) \
    int firstcol = (d)->vector ? -1 : 0; \
    int vsize = (d)->vector ? (d)->vector->size : 0; \
    int wsize = (d)->weights ? (d)->weights->size : 0; \
    int msize1 = (d)->matrix ? (d)->matrix->size1 : 0; \
    int msize2 = (d)->matrix ? (d)->matrix->size2 : 0; \
    int tsize = vsize + msize1*msize2; \
    if (tsize||wsize||firstcol) /*prevent unused variable complaints */;

// Define a static variable, and initialize on first use.
#define Staticdef(type, name, def) static type (name) = NULL; if (!(name)) (name) = (def);

// Check for NULL and complain if so.
#define Nullcheck(in) Apop_assert_s(in, "%s is NULL.", #in)
#define Nullcheck_m(in) Apop_assert_s(in, "%s is a NULL model.", #in)
#define Nullcheck_p(in) Apop_assert_s((in)->parameters, "%s is a model with NULL parameters. Please set the parameters and try again.", #in)
#define Nullcheck_d(in) Apop_assert_s(in, "%s is a NULL data set.", #in)
