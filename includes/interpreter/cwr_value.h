#ifndef CWR_VALUE_H
#define CWR_VALUE_H

#include <cwr_node.h>
#include <cwr_string.h>

typedef struct cwr_value cwr_value;

typedef struct cwr_array_value {
    cwr_value_type type;
    size_t capacity;
    bool is_reference;

    union {
        float* floats;
        int* integers;
        char* characters;
        cwr_value** values;
    };
} cwr_array_value;

typedef struct cwr_value {
    cwr_value_type type;
    int references_count;

    union {
        float float_n;
        int integer_n;
        char character;
        cwr_array_value array;
    };
} cwr_value;

static void cwr_value_instance_destroy(cwr_value* value);
static void cwr_array_value_destroy(cwr_array_value array_value);

static cwr_array_value cwr_value_create_array_copy(cwr_array_value value) {
    switch (value.type) {
        case cwr_value_character_type:
            return (cwr_array_value) {
                .type = cwr_value_character_type,
                .capacity = value.capacity,
                .characters = cwr_string_duplicate(value.characters)
            };
    }
}

static cwr_value* cwr_value_create_copy(cwr_value* value) {
    cwr_value* target = malloc(sizeof(cwr_value));
    if (value == NULL) {
        return NULL;
    }

    *target = *value;
    target->references_count = 0;
    return target;
}

static cwr_value* cwr_value_create_void() {
    cwr_value* value = malloc(sizeof(cwr_value));
    if (value == NULL) {
        return NULL;
    }

    value->type = cwr_value_void_type;
    value->references_count = 0;
    return value;
}

static cwr_value* cwr_value_create_float(float number) {
    cwr_value* value = malloc(sizeof(cwr_value));
    if (value == NULL) {
        return NULL;
    }

    value->type = cwr_value_float_type;
    value->references_count = 0;
    value->float_n = number;
    return value;
}

static cwr_value* cwr_value_create_integer(int number) {
    cwr_value* value = malloc(sizeof(cwr_value));
    if (value == NULL) {
        return NULL;
    }

    value->type = cwr_value_integer_type;
    value->references_count = 0;
    value->integer_n = number;
    return value;
}

static cwr_value* cwr_value_create_character(char character) {
    cwr_value* value = malloc(sizeof(cwr_value));
    if (value == NULL) {
        return NULL;
    }

    value->type = cwr_value_character_type;
    value->references_count = 0;
    value->character = character;
    return value;
}

static cwr_value* cwr_value_create_array(cwr_array_value array_value) {
    cwr_value* value = malloc(sizeof(cwr_value));
    if (value == NULL) {
        return NULL;
    }

    value->type = cwr_value_array_type;
    value->references_count = 0;
    value->array = array_value;
    return value;
}

static void cwr_value_add_reference(cwr_value* value)  {
    value->references_count++;
}

static void cwr_value_remove_reference(cwr_value* value) {
    if (--value->references_count <= 0) {
        cwr_value_instance_destroy(value);
    }
}

static void cwr_value_set(cwr_value* target, cwr_value* value) {
    switch (target->type) {
        case cwr_value_array_type:
            cwr_array_value_destroy(target->array);
            target->array = value->array;
            break;
        case cwr_value_character_type:
            target->character = value->character;
            break; 
        case cwr_value_integer_type:
            target->integer_n = value->integer_n;
            break;
        case cwr_value_float_type:
            target->float_n = value->float_n;
            break;
    }
}

static cwr_value* cwr_array_value_dereference(cwr_array_value value) {
    if (value.is_reference) {
        return value.values[0];
    }

    cwr_value* target = malloc(sizeof(cwr_value));
    if (target == NULL) {
        return NULL;
    }

    target->references_count = 0;
    switch (value.type) {
        case cwr_value_character_type:
            target->type = cwr_value_character_type;
            target->character = *value.characters;
            return target;
        case cwr_value_integer_type:
            target->type = cwr_value_integer_type;
            target->integer_n = *value.integers;
            return target;
        case cwr_value_float_type:
            target->type = cwr_value_float_type;
            target->float_n = *value.floats;
            return target;
    }
}

static cwr_value* cwr_value_dereference(cwr_value* value) {
    return cwr_array_value_dereference(value->array);
}

static cwr_value* cwr_value_reference(cwr_value* value) {
    cwr_value* target = malloc(sizeof(cwr_value));
    if (target == NULL) {
        return NULL;
    }

    target->type = cwr_value_pointer_type;
    target->references_count = 0;
    target->array = (cwr_array_value) {
        .type = value->type,
        .capacity = 1,
        .is_reference = true,
        .values = malloc(sizeof(cwr_value*))
    };

    if (target->array.values == NULL) {
        free(target);
        return NULL;
    }
    
    target->array.values[0] = value;
    return target;
}

static cwr_value* cwr_value_at(cwr_array_value array_value, size_t index) {
    cwr_value* target = malloc(sizeof(cwr_value));
    if (target == NULL) {
        return NULL;
    }

    target->references_count = 0;
    target->type = array_value.type;
    switch (array_value.type) {
        case cwr_value_character_type:
            target->character = array_value.characters[index];
            return target;
        case cwr_value_float_type:
            target->float_n = array_value.floats[index];
            return target;
        case cwr_value_integer_type:
            target->integer_n = array_value.integers[index];
            return target;
    }
}

static float cwr_value_as_float(cwr_value* value) {
    if (value->type == cwr_value_integer_type) {
        return value->integer_n;
    }

    return value->float_n;
}

static int cwr_value_as_integer(cwr_value* value) {
    if (value->type == cwr_value_float_type) {
        return value->float_n;
    }

    return value->integer_n;
}

static char cwr_value_as_character(cwr_value* value) {
    return value->character;
}

static cwr_array_value cwr_value_as_array(cwr_value value) {
    return value.array;
}

static void cwr_array_value_destroy(cwr_array_value array_value) {
    if (array_value.is_reference) {
        free(array_value.values);
        return;
    }

    switch (array_value.type) {
        case cwr_value_character_type:
            free(array_value.characters);
            break;
        case cwr_value_float_type:
            free(array_value.floats);
            break;
        case cwr_value_integer_type:
            free(array_value.integers);
            break;
    }
}

static void cwr_value_instance_destroy(cwr_value* value) {
    switch (value->type) {
        case cwr_value_pointer_type:
        case cwr_value_array_type:
            cwr_array_value_destroy(value->array);
            break;
    }

    free(value);
}

static void cwr_value_runtime_destroy(cwr_value* value) {
    if (value->references_count > 0) {
        return;
    }

    cwr_value_instance_destroy(value);
}

#endif //CWR_VALUE_H