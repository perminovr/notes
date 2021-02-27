#ifndef MYDRV_H
#define MYDRV_H

#if defined(__cplusplus)
# define EXTERN_C extern
extern "C" {
//# include ""
}
#else
# define EXTERN_C extern
//# include ""
#endif

typedef struct mydrv_s mydrv_t;
struct mydrv_s {
	void *priv;
};

EXTERN_C mydrv_t *mydrv_create();
EXTERN_C void mydrv_free(mydrv_t *mydrv);

#endif // MYDRV_H