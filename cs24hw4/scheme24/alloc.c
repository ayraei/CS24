#include "alloc.h"
#include "ptr_vector.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


/*! Change to #define to output garbage-collector statistics. */
#define GC_STATS

/*!
 * Change to #undef to cause the garbage collector to only run when it has to.
 * This dramatically improves performance.
 *
 * However, while testing GC, it's easiest if you try it all the time, so that
 * the number of objects being manipulated is small and easy to understand.
 */
#define ALWAYS_GC


/* Change to #define for other verbose output. */
#undef VERBOSE


void free_value(Value *v);
void free_lambda(Lambda *f);
void free_environment(Environment *env);


/*========================================================*
 * TODO:  Declarations of your functions might go here... *
 *========================================================*/
 
 void mark_environment(Environment *env);
 void mark_value(Value *v);
 void mark_lambda(Lambda *f);
 void mark_eval_stack(PtrVector *p);
 
 void sweep_values();
 void sweep_lambdas();
 void sweep_environments();


/*!
 * A growable vector of pointers to all Value structs that are currently
 * allocated.
 */
static PtrVector allocated_values;


/*!
 * A growable vector of pointers to all Lambda structs that are currently
 * allocated.  Note that each Lambda struct will only have ONE Value struct that
 * points to it.
 */
static PtrVector allocated_lambdas;


/*!
 * A growable vector of pointers to all Environment structs that are currently
 * allocated.
 */
static PtrVector allocated_environments;


#ifndef ALWAYS_GC

/*! Starts at 1MB, and is doubled every time we can't stay within it. */
static long max_allocation_size = 1048576;

#endif


void init_alloc() {
    pv_init(&allocated_values);
    pv_init(&allocated_lambdas);
    pv_init(&allocated_environments);
}


/*!
 * This helper function prints some helpful details about the current allocation
 * status of the program.
 */
void print_alloc_stats(FILE *f) {
    /*
    fprintf(f, "Allocation statistics:\n");
    fprintf(f, "\tAllocated environments:  %u\n", allocated_environments.size);
    fprintf(f, "\tAllocated lambdas:  %u\n", allocated_lambdas.size);
    fprintf(f, "\tAllocated values:  %u\n", allocated_values.size);
    */

    fprintf(f, "%d vals \t%d lambdas \t%d envs\n", allocated_values.size,
        allocated_lambdas.size, allocated_environments.size);
}


/*!
 * This helper function returns the amount of memory currently being used by
 * garbage-collected objects.  It is NOT the total amount of memory being used
 * by the interpreter!
 */ 
long allocation_size() {
    long size = 0;
    
    size += sizeof(Value) * allocated_values.size;
    size += sizeof(Lambda) * allocated_lambdas.size;
    size += sizeof(Value) * allocated_environments.size;
    
    return size;
}


/*!
 * This function heap-allocates a new Value struct, initializes it to be empty,
 * and then records the struct's pointer in the allocated_values vector.
 */
Value * alloc_value(void) {
    Value *v = malloc(sizeof(Value));
    memset(v, 0, sizeof(Value));

    pv_add_elem(&allocated_values, v);

    return v;
}


/*!
 * This function frees a heap-allocated Value struct.  Since a Value struct can
 * represent several different kinds of values, the function looks at the
 * value's type tag to determine if additional memory needs to be freed for the
 * value.
 *
 * Note:  It is assumed that the value's pointer has already been removed from
 *        the allocated_values vector!  If this is not the case, serious errors
 *        will almost certainly occur.
 */
void free_value(Value *v) {
    assert(v != NULL);

    /*
     * If value refers to a lambda, we don't free it here!  Lambdas are freed
     * by the free_lambda() function, and that is called when cleaning up
     * unreachable objects.
     */

    if (v->type == T_String || v->type == T_Atom || v->type == T_Error)
        free(v->string_val);

    free(v);
}



/*!
 * This function heap-allocates a new Lambda struct, initializes it to be empty,
 * and then records the struct's pointer in the allocated_lambdas vector.
 */
Lambda * alloc_lambda(void) {
    Lambda *f = malloc(sizeof(Lambda));
    memset(f, 0, sizeof(Lambda));

    pv_add_elem(&allocated_lambdas, f);

    return f;
}


/*!
 * This function frees a heap-allocated Lambda struct.
 *
 * Note:  It is assumed that the lambda's pointer has already been removed from
 *        the allocated_labmdas vector!  If this is not the case, serious errors
 *        will almost certainly occur.
 */
void free_lambda(Lambda *f) {
    assert(f != NULL);

    /* Lambdas typically reference lists of Value objects for the argument-spec
     * and the body, but we don't need to free these here because they are
     * managed separately.
     */

    free(f);
}


/*!
 * This function heap-allocates a new Environment struct, initializes it to be
 * empty, and then records the struct's pointer in the allocated_environments
 * vector.
 */
Environment * alloc_environment(void) {
    Environment *env = malloc(sizeof(Environment));
    memset(env, 0, sizeof(Environment));

    pv_add_elem(&allocated_environments, env);

    return env;
}


/*!
 * This function frees a heap-allocated Environment struct.  The environment's
 * bindings are also freed since they are owned by the environment, but the
 * binding-values are not freed since they are externally managed.
 *
 * Note:  It is assumed that the environment's pointer has already been removed
 *        from the allocated_environments vector!  If this is not the case,
 *        serious errors will almost certainly occur.
 */
void free_environment(Environment *env) {
    int i;

    /* Free the bindings in the environment first. */
    for (i = 0; i < env->num_bindings; i++) {
        free(env->bindings[i].name);
        /* Don't free the value, since those are handled separately. */
    }
    free(env->bindings);

    /* Now free the environment object itself. */
    free(env);
}


/*!
 * This function performs the garbage collection for the Scheme interpreter.
 * It also contains code to track how many objects were collected on each run,
 * and also it can optionally be set to do GC when the total memory used grows
 * beyond a certain limit.
 */
void collect_garbage() {
    Environment *global_env;
    PtrStack *eval_stack;

#ifdef GC_STATS
    int vals_before, procs_before, envs_before;
    int vals_after, procs_after, envs_after;

    vals_before = allocated_values.size;
    procs_before = allocated_lambdas.size;
    envs_before = allocated_environments.size;
#endif

#ifndef ALWAYS_GC
    /* Don't perform garbage collection if we still have room to grow. */
    if (allocation_size() < max_allocation_size)
        return;
#endif

    /*==========================================================*
     * TODO:  Implement mark-and-sweep garbage collection here! *
     *                                                          *
     * Mark all objects referenceable from either the global    *
     * environment, or from the evaluation stack.  Then sweep   *
     * through all allocated objects, freeing unmarked objects. *
     *==========================================================*/

    global_env = get_global_environment();
    eval_stack = get_eval_stack();

    /* ... TODO ... */
    /* Marking phase. */
    mark_environment(global_env);
    mark_eval_stack(eval_stack);
    
    sweep_values();
    sweep_lambdas();
    //sweep_environments();

#ifndef ALWAYS_GC
    /* If we are still above the maximum allocation size, increase it. */
    if (allocation_size() > max_allocation_size) {
        max_allocation_size *= 2;

        printf("Increasing maximum allocation size to %ld bytes.\n",
            max_allocation_size);
    }
#endif
    
#ifdef GC_STATS
    vals_after = allocated_values.size;
    procs_after = allocated_lambdas.size;
    envs_after = allocated_environments.size;

    printf("GC Results:\n");
    printf("\tBefore: \t%d vals \t%d lambdas \t%d envs\n",
            vals_before, procs_before, envs_before);
    printf("\tAfter:  \t%d vals \t%d lambdas \t%d envs\n",
            vals_after, procs_after, envs_after);
    printf("\tChange: \t%d vals \t%d lambdas \t%d envs\n",
            vals_after - vals_before, procs_after - procs_before,
            envs_after - envs_before);
#endif
}


/*
 * Helper function that marks the environment given, then iterates
 * through its bindings and marks those objects.
 */
void mark_environment(Environment *env) {
    if (env == NULL)
        return;
   int i;

   env->marked = 1;

   for (i = 0; i < env->num_bindings; i++) {
       mark_value(env->bindings[i].value);
   }
}


/*
 * Helper function that marks the value given, then iterates through its
 * contents and marks those objects.
 */
void mark_value(Value *v) {
    if (v == NULL)
        return;
    v->marked = 1;
    if (v->type == T_Lambda) {
        mark_lambda(v->lambda_val);
    }
    if (v->type == T_ConsPair) {
        mark_value(v->cons_val.p_car);
        mark_value(v->cons_val.p_cdr);
    }
}


/*
 * Helper function that marks the lambda given, then iterates through its
 * contents and marks those objects.
 */
void mark_lambda(Lambda *f) {
    if (f == NULL)
        return;
    f->marked = 1;
    if (f->native_impl == 0) {
        mark_value(f->arg_spec);
        mark_value(f->body);
    }
}


/*
 * Helper function that iterates through the evaluation stack and marks
 * objects in it.
 */
void mark_eval_stack(PtrVector *p) {
    unsigned int i, j;
    EvaluationContext *ec;
    PtrVector *lv;
    Value *ppv;
    
    for (i = 0; i < p->size; i++) {
        ec = (EvaluationContext *) pv_get_elem(p, i);
        mark_environment(ec->current_env);
        mark_value(ec->expression);
        mark_value(ec->child_eval_result);
        lv = &(ec->local_vals);
        for (j = 0; j < lv->size; j++) {
            ppv = *((Value **) pv_get_elem(lv, j));
            mark_value(ppv);
        }
    }
}
 
 
/*
 * Helper function that traverses the allocated_values vector and
 * unmarks a marked value, or deletes (free) an unmarked value.
 */
void sweep_values() {
   unsigned int i;
   Value *v;
   
   for (i = 0; i < allocated_values.size; i++) {
       v = (Value *) pv_get_elem(&allocated_values, i);
       if (v != NULL && v->marked == 0) {
           pv_set_elem(&allocated_values, i, NULL);
           free_value(v);
       }
       else if (v != NULL) {
           v->marked = 0;
       }
   }
   
   pv_compact(&allocated_values);
}


/*
 * Helper function that traverses the allocated_lambdas vector and
 * unmarks a marked lambda, or deletes (free) an unmarked lambda.
 */
void sweep_lambdas() {
   unsigned int i;
   Lambda *l;
   
   for (i = 0; i < allocated_lambdas.size; i++) {
       l = (Lambda *) pv_get_elem(&allocated_lambdas, i);
       if (l != NULL && l->marked == 0) {
           pv_set_elem(&allocated_lambdas, i, NULL);
           free_lambda(l);
       }
       else if (l != NULL) {
           l->marked = 0;
       }
   }
   
   pv_compact(&allocated_lambdas);
}


/*
 * Helper function that traverses the allocated_environments vector and
 * unmarks a marked environment, or deletes (free) an unmarked environment.
 */
void sweep_environments() {
   unsigned int i;
   Environment *e;
   
   for (i = 0; i < allocated_environments.size; i++) {
       e = (Environment *) pv_get_elem(&allocated_environments, i);
       if (e != NULL && e->marked == 0) {
           pv_set_elem(&allocated_environments, i, NULL);
           free_environment(e);
       }
       else if (e != NULL) {
           e->marked = 0;
       }
   }
   
   pv_compact(&allocated_environments);
}
