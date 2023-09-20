#include "locals.h"


void locals_init(Locals* const locals) {
    dynarray_init(locals->envs);
}

void locals_push(Locals* locals) {
    dynarray_reserve(locals->envs, 1);
    LocalEnvironment* env = dynarray_last_ref(locals->envs);

    map_init(&env->bound);
}

void locals_pop(Locals* locals) {
    LocalEnvironment* env = dynarray_last_ref(locals->envs);
    map_deinit(&env->bound);
    locals->envs.length -= 1;
}

void locals_insert(Locals* locals, Span name, void* value) {
    LocalEnvironment* env = dynarray_last_ref(locals->envs);
    map_insert(&env->bound, name, value);
}

MaybeAny locals_get(Locals* locals, Span name) {
    for (int i = 0; i < locals->envs.length; i++) {
	LocalEnvironment const* const env = dynarray_get_ref(locals->envs, i);
	if (map_has(&env->bound, name)) {
	    return (MaybeAny){ Yes, map_get(&env->bound, name).value };
	}
    }

    return (MaybeAny){ No };
}
