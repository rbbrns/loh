#include <Python.h>
#include "pycore_pystate.h"         // _PyInterpreterState_GET()
#include "pycore_runtime.h"         // _PyRuntime
#include "pycore_unicodeobject.h"   // _PyUnicode_InternImmortal()

#include "pegen.h"
#include "string_parser.h"          // _PyPegen_decode_string()


void *
_PyPegen_dummy_name(Parser *p, ...)
{
    return &_PyRuntime.parser.dummy_name;
}

/* Creates a single-element asdl_seq* that contains a */
asdl_seq *
_PyPegen_singleton_seq(Parser *p, void *a)
{
    assert(a != NULL);
    asdl_seq *seq = (asdl_seq*)_Py_asdl_generic_seq_new(1, p->arena);
    if (!seq) {
        return NULL;
    }
    asdl_seq_SET_UNTYPED(seq, 0, a);
    return seq;
}

asdl_int_seq *
_PyPegen_singleton_int_seq(Parser *p, int a)
{
    asdl_int_seq *seq = _Py_asdl_int_seq_new(1, p->arena);
    if (!seq) {
        return NULL;
    }
    asdl_seq_SET(seq, 0, a);
    return seq;
}

/* Creates a copy of seq and prepends a to it */
asdl_seq *
_PyPegen_seq_insert_in_front(Parser *p, void *a, asdl_seq *seq)
{
    assert(a != NULL);
    if (!seq) {
        return _PyPegen_singleton_seq(p, a);
    }

    asdl_seq *new_seq = (asdl_seq*)_Py_asdl_generic_seq_new(asdl_seq_LEN(seq) + 1, p->arena);
    if (!new_seq) {
        return NULL;
    }

    asdl_seq_SET_UNTYPED(new_seq, 0, a);
    for (Py_ssize_t i = 1, l = asdl_seq_LEN(new_seq); i < l; i++) {
        asdl_seq_SET_UNTYPED(new_seq, i, asdl_seq_GET_UNTYPED(seq, i - 1));
    }
    return new_seq;
}

/* Creates a copy of seq and appends a to it */
asdl_seq *
_PyPegen_seq_append_to_end(Parser *p, asdl_seq *seq, void *a)
{
    assert(a != NULL);
    if (!seq) {
        return _PyPegen_singleton_seq(p, a);
    }

    asdl_seq *new_seq = (asdl_seq*)_Py_asdl_generic_seq_new(asdl_seq_LEN(seq) + 1, p->arena);
    if (!new_seq) {
        return NULL;
    }

    for (Py_ssize_t i = 0, l = asdl_seq_LEN(new_seq); i + 1 < l; i++) {
        asdl_seq_SET_UNTYPED(new_seq, i, asdl_seq_GET_UNTYPED(seq, i));
    }
    asdl_seq_SET_UNTYPED(new_seq, asdl_seq_LEN(new_seq) - 1, a);
    return new_seq;
}

static Py_ssize_t
_get_flattened_seq_size(asdl_seq *seqs)
{
    Py_ssize_t size = 0;
    for (Py_ssize_t i = 0, l = asdl_seq_LEN(seqs); i < l; i++) {
        asdl_seq *inner_seq = asdl_seq_GET_UNTYPED(seqs, i);
        size += asdl_seq_LEN(inner_seq);
    }
    return size;
}

/* Flattens an asdl_seq* of asdl_seq*s */
asdl_seq *
_PyPegen_seq_flatten(Parser *p, asdl_seq *seqs)
{
    Py_ssize_t flattened_seq_size = _get_flattened_seq_size(seqs);
    assert(flattened_seq_size > 0);

    asdl_seq *flattened_seq = (asdl_seq*)_Py_asdl_generic_seq_new(flattened_seq_size, p->arena);
    if (!flattened_seq) {
        return NULL;
    }

    int flattened_seq_idx = 0;
    for (Py_ssize_t i = 0, l = asdl_seq_LEN(seqs); i < l; i++) {
        asdl_seq *inner_seq = asdl_seq_GET_UNTYPED(seqs, i);
        for (Py_ssize_t j = 0, li = asdl_seq_LEN(inner_seq); j < li; j++) {
            asdl_seq_SET_UNTYPED(flattened_seq, flattened_seq_idx++, asdl_seq_GET_UNTYPED(inner_seq, j));
        }
    }
    assert(flattened_seq_idx == flattened_seq_size);

    return flattened_seq;
}

void *
_PyPegen_seq_last_item(asdl_seq *seq)
{
    Py_ssize_t len = asdl_seq_LEN(seq);
    return asdl_seq_GET_UNTYPED(seq, len - 1);
}

void *
_PyPegen_seq_first_item(asdl_seq *seq)
{
    return asdl_seq_GET_UNTYPED(seq, 0);
}

/* Creates a new name of the form <first_name>.<second_name> */
expr_ty
_PyPegen_join_names_with_dot(Parser *p, expr_ty first_name, expr_ty second_name)
{
    assert(first_name != NULL && second_name != NULL);
    PyObject *uni = PyUnicode_FromFormat("%U.%U",
            first_name->v.Name.id, second_name->v.Name.id);
    if (!uni) {
        return NULL;
    }
    PyInterpreterState *interp = _PyInterpreterState_GET();
    _PyUnicode_InternImmortal(interp, &uni);
    if (_PyArena_AddPyObject(p->arena, uni) < 0) {
        Py_DECREF(uni);
        return NULL;
    }

    return _PyAST_Name(uni, Load, EXTRA_EXPR(first_name, second_name));
}

/* Counts the total number of dots in seq's tokens */
int
_PyPegen_seq_count_dots(asdl_seq *seq)
{
    int number_of_dots = 0;
    for (Py_ssize_t i = 0, l = asdl_seq_LEN(seq); i < l; i++) {
        Token *current_expr = asdl_seq_GET_UNTYPED(seq, i);
        switch (current_expr->type) {
            case ELLIPSIS:
                number_of_dots += 3;
                break;
            case DOTDOT:
                number_of_dots += 2;
                break;
            case DOT:
                number_of_dots += 1;
                break;
            default:
                Py_UNREACHABLE();
        }
    }

    return number_of_dots;
}

/* Creates an alias with '*' as the identifier name */
alias_ty
_PyPegen_alias_for_star(Parser *p, int lineno, int col_offset, int end_lineno,
                        int end_col_offset, PyArena *arena) {
    PyObject *str = PyUnicode_InternFromString("*");
    if (!str) {
        return NULL;
    }
    if (_PyArena_AddPyObject(p->arena, str) < 0) {
        Py_DECREF(str);
        return NULL;
    }
    return _PyAST_alias(str, NULL, lineno, col_offset, end_lineno, end_col_offset, arena);
}

/* Creates a new asdl_seq* with the identifiers of all the names in seq */
asdl_identifier_seq *
_PyPegen_map_names_to_ids(Parser *p, asdl_expr_seq *seq)
{
    Py_ssize_t len = asdl_seq_LEN(seq);
    assert(len > 0);

    asdl_identifier_seq *new_seq = _Py_asdl_identifier_seq_new(len, p->arena);
    if (!new_seq) {
        return NULL;
    }
    for (Py_ssize_t i = 0; i < len; i++) {
        expr_ty e = asdl_seq_GET(seq, i);
        asdl_seq_SET(new_seq, i, e->v.Name.id);
    }
    return new_seq;
}

/* Constructs a CmpopExprPair */
CmpopExprPair *
_PyPegen_cmpop_expr_pair(Parser *p, cmpop_ty cmpop, expr_ty expr)
{
    assert(expr != NULL);
    CmpopExprPair *a = _PyArena_Malloc(p->arena, sizeof(CmpopExprPair));
    if (!a) {
        return NULL;
    }
    a->cmpop = cmpop;
    a->expr = expr;
    return a;
}

asdl_int_seq *
_PyPegen_get_cmpops(Parser *p, asdl_seq *seq)
{
    Py_ssize_t len = asdl_seq_LEN(seq);
    assert(len > 0);

    asdl_int_seq *new_seq = _Py_asdl_int_seq_new(len, p->arena);
    if (!new_seq) {
        return NULL;
    }
    for (Py_ssize_t i = 0; i < len; i++) {
        CmpopExprPair *pair = asdl_seq_GET_UNTYPED(seq, i);
        asdl_seq_SET(new_seq, i, pair->cmpop);
    }
    return new_seq;
}

asdl_expr_seq *
_PyPegen_get_exprs(Parser *p, asdl_seq *seq)
{
    Py_ssize_t len = asdl_seq_LEN(seq);
    assert(len > 0);

    asdl_expr_seq *new_seq = _Py_asdl_expr_seq_new(len, p->arena);
    if (!new_seq) {
        return NULL;
    }
    for (Py_ssize_t i = 0; i < len; i++) {
        CmpopExprPair *pair = asdl_seq_GET_UNTYPED(seq, i);
        asdl_seq_SET(new_seq, i, pair->expr);
    }
    return new_seq;
}

/* Creates an asdl_seq* where all the elements have been changed to have ctx as context */
static asdl_expr_seq *
_set_seq_context(Parser *p, asdl_expr_seq *seq, expr_context_ty ctx)
{
    Py_ssize_t len = asdl_seq_LEN(seq);
    if (len == 0) {
        return NULL;
    }

    asdl_expr_seq *new_seq = _Py_asdl_expr_seq_new(len, p->arena);
    if (!new_seq) {
        return NULL;
    }
    for (Py_ssize_t i = 0; i < len; i++) {
        expr_ty e = asdl_seq_GET(seq, i);
        asdl_seq_SET(new_seq, i, _PyPegen_set_expr_context(p, e, ctx));
    }
    return new_seq;
}

static expr_ty
_set_name_context(Parser *p, expr_ty e, expr_context_ty ctx)
{
    return _PyAST_Name(e->v.Name.id, ctx, EXTRA_EXPR(e, e));
}

static expr_ty
_set_tuple_context(Parser *p, expr_ty e, expr_context_ty ctx)
{
    return _PyAST_Tuple(
            _set_seq_context(p, e->v.Tuple.elts, ctx),
            ctx,
            EXTRA_EXPR(e, e));
}

static expr_ty
_set_list_context(Parser *p, expr_ty e, expr_context_ty ctx)
{
    return _PyAST_List(
            _set_seq_context(p, e->v.List.elts, ctx),
            ctx,
            EXTRA_EXPR(e, e));
}

static expr_ty
_set_subscript_context(Parser *p, expr_ty e, expr_context_ty ctx)
{
    return _PyAST_Subscript(e->v.Subscript.value, e->v.Subscript.slice,
                            ctx, EXTRA_EXPR(e, e));
}

static expr_ty
_set_attribute_context(Parser *p, expr_ty e, expr_context_ty ctx)
{
    return _PyAST_Attribute(e->v.Attribute.value, e->v.Attribute.attr,
                            ctx, EXTRA_EXPR(e, e));
}

static expr_ty
_set_starred_context(Parser *p, expr_ty e, expr_context_ty ctx)
{
    return _PyAST_Starred(_PyPegen_set_expr_context(p, e->v.Starred.value, ctx),
                          ctx, EXTRA_EXPR(e, e));
}

/* Creates an `expr_ty` equivalent to `expr` but with `ctx` as context */
expr_ty
_PyPegen_set_expr_context(Parser *p, expr_ty expr, expr_context_ty ctx)
{
    assert(expr != NULL);

    expr_ty new = NULL;
    switch (expr->kind) {
        case Name_kind:
            new = _set_name_context(p, expr, ctx);
            break;
        case Tuple_kind:
            new = _set_tuple_context(p, expr, ctx);
            break;
        case List_kind:
            new = _set_list_context(p, expr, ctx);
            break;
        case Subscript_kind:
            new = _set_subscript_context(p, expr, ctx);
            break;
        case Attribute_kind:
            new = _set_attribute_context(p, expr, ctx);
            break;
        case Starred_kind:
            new = _set_starred_context(p, expr, ctx);
            break;
        default:
            new = expr;
    }
    return new;
}

/* Constructs a KeyValuePair that is used when parsing a dict's key value pairs */
KeyValuePair *
_PyPegen_key_value_pair(Parser *p, expr_ty key, expr_ty value)
{
    KeyValuePair *a = _PyArena_Malloc(p->arena, sizeof(KeyValuePair));
    if (!a) {
        return NULL;
    }
    a->key = key;
    a->value = value;
    return a;
}

/* Extracts all keys from an asdl_seq* of KeyValuePair*'s */
asdl_expr_seq *
_PyPegen_get_keys(Parser *p, asdl_seq *seq)
{
    Py_ssize_t len = asdl_seq_LEN(seq);
    asdl_expr_seq *new_seq = _Py_asdl_expr_seq_new(len, p->arena);
    if (!new_seq) {
        return NULL;
    }
    for (Py_ssize_t i = 0; i < len; i++) {
        KeyValuePair *pair = asdl_seq_GET_UNTYPED(seq, i);
        asdl_seq_SET(new_seq, i, pair->key);
    }
    return new_seq;
}

/* Extracts all values from an asdl_seq* of KeyValuePair*'s */
asdl_expr_seq *
_PyPegen_get_values(Parser *p, asdl_seq *seq)
{
    Py_ssize_t len = asdl_seq_LEN(seq);
    asdl_expr_seq *new_seq = _Py_asdl_expr_seq_new(len, p->arena);
    if (!new_seq) {
        return NULL;
    }
    for (Py_ssize_t i = 0; i < len; i++) {
        KeyValuePair *pair = asdl_seq_GET_UNTYPED(seq, i);
        asdl_seq_SET(new_seq, i, pair->value);
    }
    return new_seq;
}

/* Constructs a KeyPatternPair that is used when parsing mapping & class patterns */
KeyPatternPair *
_PyPegen_key_pattern_pair(Parser *p, expr_ty key, pattern_ty pattern)
{
    KeyPatternPair *a = _PyArena_Malloc(p->arena, sizeof(KeyPatternPair));
    if (!a) {
        return NULL;
    }
    a->key = key;
    a->pattern = pattern;
    return a;
}

/* Extracts all keys from an asdl_seq* of KeyPatternPair*'s */
asdl_expr_seq *
_PyPegen_get_pattern_keys(Parser *p, asdl_seq *seq)
{
    Py_ssize_t len = asdl_seq_LEN(seq);
    asdl_expr_seq *new_seq = _Py_asdl_expr_seq_new(len, p->arena);
    if (!new_seq) {
        return NULL;
    }
    for (Py_ssize_t i = 0; i < len; i++) {
        KeyPatternPair *pair = asdl_seq_GET_UNTYPED(seq, i);
        asdl_seq_SET(new_seq, i, pair->key);
    }
    return new_seq;
}

/* Extracts all patterns from an asdl_seq* of KeyPatternPair*'s */
asdl_pattern_seq *
_PyPegen_get_patterns(Parser *p, asdl_seq *seq)
{
    Py_ssize_t len = asdl_seq_LEN(seq);
    asdl_pattern_seq *new_seq = _Py_asdl_pattern_seq_new(len, p->arena);
    if (!new_seq) {
        return NULL;
    }
    for (Py_ssize_t i = 0; i < len; i++) {
        KeyPatternPair *pair = asdl_seq_GET_UNTYPED(seq, i);
        asdl_seq_SET(new_seq, i, pair->pattern);
    }
    return new_seq;
}

/* Constructs a NameDefaultPair */
NameDefaultPair *
_PyPegen_name_default_pair(Parser *p, arg_ty arg, expr_ty value, Token *tc)
{
    NameDefaultPair *a = _PyArena_Malloc(p->arena, sizeof(NameDefaultPair));
    if (!a) {
        return NULL;
    }
    a->arg = _PyPegen_add_type_comment_to_arg(p, arg, tc);
    a->value = value;
    return a;
}

/* Constructs a SlashWithDefault */
SlashWithDefault *
_PyPegen_slash_with_default(Parser *p, asdl_arg_seq *plain_names, asdl_seq *names_with_defaults)
{
    SlashWithDefault *a = _PyArena_Malloc(p->arena, sizeof(SlashWithDefault));
    if (!a) {
        return NULL;
    }
    a->plain_names = plain_names;
    a->names_with_defaults = names_with_defaults;
    return a;
}

/* Constructs a StarEtc */
StarEtc *
_PyPegen_star_etc(Parser *p, arg_ty vararg, asdl_seq *kwonlyargs, arg_ty kwarg)
{
    StarEtc *a = _PyArena_Malloc(p->arena, sizeof(StarEtc));
    if (!a) {
        return NULL;
    }
    a->vararg = vararg;
    a->kwonlyargs = kwonlyargs;
    a->kwarg = kwarg;
    return a;
}

asdl_seq *
_PyPegen_join_sequences(Parser *p, asdl_seq *a, asdl_seq *b)
{
    Py_ssize_t first_len = asdl_seq_LEN(a);
    Py_ssize_t second_len = asdl_seq_LEN(b);
    asdl_seq *new_seq = (asdl_seq*)_Py_asdl_generic_seq_new(first_len + second_len, p->arena);
    if (!new_seq) {
        return NULL;
    }

    int k = 0;
    for (Py_ssize_t i = 0; i < first_len; i++) {
        asdl_seq_SET_UNTYPED(new_seq, k++, asdl_seq_GET_UNTYPED(a, i));
    }
    for (Py_ssize_t i = 0; i < second_len; i++) {
        asdl_seq_SET_UNTYPED(new_seq, k++, asdl_seq_GET_UNTYPED(b, i));
    }

    return new_seq;
}

static asdl_arg_seq*
_get_names(Parser *p, asdl_seq *names_with_defaults)
{
    Py_ssize_t len = asdl_seq_LEN(names_with_defaults);
    asdl_arg_seq *seq = _Py_asdl_arg_seq_new(len, p->arena);
    if (!seq) {
        return NULL;
    }
    for (Py_ssize_t i = 0; i < len; i++) {
        NameDefaultPair *pair = asdl_seq_GET_UNTYPED(names_with_defaults, i);
        asdl_seq_SET(seq, i, pair->arg);
    }
    return seq;
}

static asdl_expr_seq *
_get_defaults(Parser *p, asdl_seq *names_with_defaults)
{
    Py_ssize_t len = asdl_seq_LEN(names_with_defaults);
    asdl_expr_seq *seq = _Py_asdl_expr_seq_new(len, p->arena);
    if (!seq) {
        return NULL;
    }
    for (Py_ssize_t i = 0; i < len; i++) {
        NameDefaultPair *pair = asdl_seq_GET_UNTYPED(names_with_defaults, i);
        asdl_seq_SET(seq, i, pair->value);
    }
    return seq;
}

static int
_make_posonlyargs(Parser *p,
                  asdl_arg_seq *slash_without_default,
                  SlashWithDefault *slash_with_default,
                  asdl_arg_seq **posonlyargs) {
    if (slash_without_default != NULL) {
        *posonlyargs = slash_without_default;
    }
    else if (slash_with_default != NULL) {
        asdl_arg_seq *slash_with_default_names =
                _get_names(p, slash_with_default->names_with_defaults);
        if (!slash_with_default_names) {
            return -1;
        }
        *posonlyargs = (asdl_arg_seq*)_PyPegen_join_sequences(
                p,
                (asdl_seq*)slash_with_default->plain_names,
                (asdl_seq*)slash_with_default_names);
    }
    else {
        *posonlyargs = _Py_asdl_arg_seq_new(0, p->arena);
    }
    return *posonlyargs == NULL ? -1 : 0;
}

static int
_make_posargs(Parser *p,
              asdl_arg_seq *plain_names,
              asdl_seq *names_with_default,
              asdl_arg_seq **posargs) {

    if (names_with_default != NULL) {
        if (plain_names != NULL) {
            asdl_arg_seq *names_with_default_names = _get_names(p, names_with_default);
            if (!names_with_default_names) {
                return -1;
            }
            *posargs = (asdl_arg_seq*)_PyPegen_join_sequences(
                    p,(asdl_seq*)plain_names, (asdl_seq*)names_with_default_names);
        }
        else {
            *posargs = _get_names(p, names_with_default);
        }
    }
    else {
        if (plain_names != NULL) {
            // With the current grammar, we never get here.
            // If that has changed, remove the assert, and test thoroughly.
            assert(0);
            *posargs = plain_names;
        }
        else {
            *posargs = _Py_asdl_arg_seq_new(0, p->arena);
        }
    }
    return *posargs == NULL ? -1 : 0;
}

static int
_make_posdefaults(Parser *p,
                  SlashWithDefault *slash_with_default,
                  asdl_seq *names_with_default,
                  asdl_expr_seq **posdefaults) {
    if (slash_with_default != NULL && names_with_default != NULL) {
        asdl_expr_seq *slash_with_default_values =
                _get_defaults(p, slash_with_default->names_with_defaults);
        if (!slash_with_default_values) {
            return -1;
        }
        asdl_expr_seq *names_with_default_values = _get_defaults(p, names_with_default);
        if (!names_with_default_values) {
            return -1;
        }
        *posdefaults = (asdl_expr_seq*)_PyPegen_join_sequences(
                p,
                (asdl_seq*)slash_with_default_values,
                (asdl_seq*)names_with_default_values);
    }
    else if (slash_with_default == NULL && names_with_default != NULL) {
        *posdefaults = _get_defaults(p, names_with_default);
    }
    else if (slash_with_default != NULL && names_with_default == NULL) {
        *posdefaults = _get_defaults(p, slash_with_default->names_with_defaults);
    }
    else {
        *posdefaults = _Py_asdl_expr_seq_new(0, p->arena);
    }
    return *posdefaults == NULL ? -1 : 0;
}

static int
_make_kwargs(Parser *p, StarEtc *star_etc,
             asdl_arg_seq **kwonlyargs,
             asdl_expr_seq **kwdefaults) {
    if (star_etc != NULL && star_etc->kwonlyargs != NULL) {
        *kwonlyargs = _get_names(p, star_etc->kwonlyargs);
    }
    else {
        *kwonlyargs = _Py_asdl_arg_seq_new(0, p->arena);
    }

    if (*kwonlyargs == NULL) {
        return -1;
    }

    if (star_etc != NULL && star_etc->kwonlyargs != NULL) {
        *kwdefaults = _get_defaults(p, star_etc->kwonlyargs);
    }
    else {
        *kwdefaults = _Py_asdl_expr_seq_new(0, p->arena);
    }

    if (*kwdefaults == NULL) {
        return -1;
    }

    return 0;
}

/* Constructs an arguments_ty object out of all the parsed constructs in the parameters rule */
arguments_ty
_PyPegen_make_arguments(Parser *p, asdl_arg_seq *slash_without_default,
                        SlashWithDefault *slash_with_default, asdl_arg_seq *plain_names,
                        asdl_seq *names_with_default, StarEtc *star_etc)
{
    asdl_arg_seq *posonlyargs;
    if (_make_posonlyargs(p, slash_without_default, slash_with_default, &posonlyargs) == -1) {
        return NULL;
    }

    asdl_arg_seq *posargs;
    if (_make_posargs(p, plain_names, names_with_default, &posargs) == -1) {
        return NULL;
    }

    asdl_expr_seq *posdefaults;
    if (_make_posdefaults(p,slash_with_default, names_with_default, &posdefaults) == -1) {
        return NULL;
    }

    arg_ty vararg = NULL;
    if (star_etc != NULL && star_etc->vararg != NULL) {
        vararg = star_etc->vararg;
    }

    asdl_arg_seq *kwonlyargs;
    asdl_expr_seq *kwdefaults;
    if (_make_kwargs(p, star_etc, &kwonlyargs, &kwdefaults) == -1) {
        return NULL;
    }

    arg_ty kwarg = NULL;
    if (star_etc != NULL && star_etc->kwarg != NULL) {
        kwarg = star_etc->kwarg;
    }

    return _PyAST_arguments(posonlyargs, posargs, vararg, kwonlyargs,
                            kwdefaults, kwarg, posdefaults, p->arena);
}

arguments_ty
_PyPegen_insert_arg_in_front(Parser *p, arg_ty arg, arguments_ty args)
{
    if (args == NULL) {
        args = _PyPegen_empty_arguments(p);
    }
    return _PyAST_arguments((asdl_arg_seq*)_PyPegen_seq_insert_in_front(p, arg, (asdl_seq*)args->posonlyargs), 
                            args->args, args->vararg, args->kwonlyargs,
                            args->kw_defaults, args->kwarg, args->defaults, p->arena);
}


/* Constructs an empty arguments_ty object, that gets used when a function accepts no
 * arguments. */
arguments_ty
_PyPegen_empty_arguments(Parser *p)
{
    asdl_arg_seq *posonlyargs = _Py_asdl_arg_seq_new(0, p->arena);
    if (!posonlyargs) {
        return NULL;
    }
    asdl_arg_seq *posargs = _Py_asdl_arg_seq_new(0, p->arena);
    if (!posargs) {
        return NULL;
    }
    asdl_expr_seq *posdefaults = _Py_asdl_expr_seq_new(0, p->arena);
    if (!posdefaults) {
        return NULL;
    }
    asdl_arg_seq *kwonlyargs = _Py_asdl_arg_seq_new(0, p->arena);
    if (!kwonlyargs) {
        return NULL;
    }
    asdl_expr_seq *kwdefaults = _Py_asdl_expr_seq_new(0, p->arena);
    if (!kwdefaults) {
        return NULL;
    }

    return _PyAST_arguments(posonlyargs, posargs, NULL, kwonlyargs,
                            kwdefaults, NULL, posdefaults, p->arena);
}

/* Encapsulates the value of an operator_ty into an AugOperator struct */
AugOperator *
_PyPegen_augoperator(Parser *p, operator_ty kind)
{
    AugOperator *a = _PyArena_Malloc(p->arena, sizeof(AugOperator));
    if (!a) {
        return NULL;
    }
    a->kind = kind;
    return a;
}

/* Construct a FunctionDef equivalent to function_def, but with decorators */
stmt_ty
_PyPegen_function_def_decorators(Parser *p, asdl_expr_seq *decorators, stmt_ty function_def)
{
    assert(function_def != NULL);
    if (function_def->kind == AsyncFunctionDef_kind) {
        return _PyAST_AsyncFunctionDef(
            function_def->v.AsyncFunctionDef.name,
            function_def->v.AsyncFunctionDef.args,
            function_def->v.AsyncFunctionDef.body, decorators,
            function_def->v.AsyncFunctionDef.returns,
            function_def->v.AsyncFunctionDef.type_comment,
            function_def->v.AsyncFunctionDef.type_params,
            function_def->lineno, function_def->col_offset,
            function_def->end_lineno, function_def->end_col_offset, p->arena);
    }

    return _PyAST_FunctionDef(
        function_def->v.FunctionDef.name,
        function_def->v.FunctionDef.args,
        function_def->v.FunctionDef.body, decorators,
        function_def->v.FunctionDef.returns,
        function_def->v.FunctionDef.type_comment,
        function_def->v.FunctionDef.type_params,
        function_def->lineno, function_def->col_offset,
        function_def->end_lineno, function_def->end_col_offset, p->arena);
}

/* Construct a ClassDef equivalent to class_def, but with decorators */
stmt_ty
_PyPegen_class_def_decorators(Parser *p, asdl_expr_seq *decorators, stmt_ty class_def)
{
    assert(class_def != NULL);
    return _PyAST_ClassDef(
        class_def->v.ClassDef.name,
        class_def->v.ClassDef.bases, class_def->v.ClassDef.keywords,
        class_def->v.ClassDef.body, decorators,
        class_def->v.ClassDef.type_params,
        class_def->lineno, class_def->col_offset, class_def->end_lineno,
        class_def->end_col_offset, p->arena);
}

/* Construct a KeywordOrStarred */
KeywordOrStarred *
_PyPegen_keyword_or_starred(Parser *p, void *element, int is_keyword)
{
    KeywordOrStarred *a = _PyArena_Malloc(p->arena, sizeof(KeywordOrStarred));
    if (!a) {
        return NULL;
    }
    a->element = element;
    a->is_keyword = is_keyword;
    return a;
}

/* Get the number of starred expressions in an asdl_seq* of KeywordOrStarred*s */
static int
_seq_number_of_starred_exprs(asdl_seq *seq)
{
    int n = 0;
    for (Py_ssize_t i = 0, l = asdl_seq_LEN(seq); i < l; i++) {
        KeywordOrStarred *k = asdl_seq_GET_UNTYPED(seq, i);
        if (!k->is_keyword) {
            n++;
        }
    }
    return n;
}

/* Extract the starred expressions of an asdl_seq* of KeywordOrStarred*s */
asdl_expr_seq *
_PyPegen_seq_extract_starred_exprs(Parser *p, asdl_seq *kwargs)
{
    int new_len = _seq_number_of_starred_exprs(kwargs);
    if (new_len == 0) {
        return NULL;
    }
    asdl_expr_seq *new_seq = _Py_asdl_expr_seq_new(new_len, p->arena);
    if (!new_seq) {
        return NULL;
    }

    int idx = 0;
    for (Py_ssize_t i = 0, len = asdl_seq_LEN(kwargs); i < len; i++) {
        KeywordOrStarred *k = asdl_seq_GET_UNTYPED(kwargs, i);
        if (!k->is_keyword) {
            asdl_seq_SET(new_seq, idx++, k->element);
        }
    }
    return new_seq;
}

/* Return a new asdl_seq* with only the keywords in kwargs */
asdl_keyword_seq*
_PyPegen_seq_delete_starred_exprs(Parser *p, asdl_seq *kwargs)
{
    Py_ssize_t len = asdl_seq_LEN(kwargs);
    Py_ssize_t new_len = len - _seq_number_of_starred_exprs(kwargs);
    if (new_len == 0) {
        return NULL;
    }
    asdl_keyword_seq *new_seq = _Py_asdl_keyword_seq_new(new_len, p->arena);
    if (!new_seq) {
        return NULL;
    }

    int idx = 0;
    for (Py_ssize_t i = 0; i < len; i++) {
        KeywordOrStarred *k = asdl_seq_GET_UNTYPED(kwargs, i);
        if (k->is_keyword) {
            asdl_seq_SET(new_seq, idx++, k->element);
        }
    }
    return new_seq;
}

expr_ty
_PyPegen_ensure_imaginary(Parser *p, expr_ty exp)
{
    if (exp->kind != Constant_kind || !PyComplex_CheckExact(exp->v.Constant.value)) {
        RAISE_SYNTAX_ERROR_KNOWN_LOCATION(exp, "imaginary number required in complex literal");
        return NULL;
    }
    return exp;
}

expr_ty
_PyPegen_ensure_real(Parser *p, expr_ty exp)
{
    if (exp->kind != Constant_kind || PyComplex_CheckExact(exp->v.Constant.value)) {
        RAISE_SYNTAX_ERROR_KNOWN_LOCATION(exp, "real number required in complex literal");
        return NULL;
    }
    return exp;
}

mod_ty
_PyPegen_make_module(Parser *p, asdl_stmt_seq *a) {
    asdl_type_ignore_seq *type_ignores = NULL;
    Py_ssize_t num = p->type_ignore_comments.num_items;
    if (num > 0) {
        // Turn the raw (comment, lineno) pairs into TypeIgnore objects in the arena
        type_ignores = _Py_asdl_type_ignore_seq_new(num, p->arena);
        if (type_ignores == NULL) {
            return NULL;
        }
        for (Py_ssize_t i = 0; i < num; i++) {
            PyObject *tag = _PyPegen_new_type_comment(p, p->type_ignore_comments.items[i].comment);
            if (tag == NULL) {
                return NULL;
            }
            type_ignore_ty ti = _PyAST_TypeIgnore(p->type_ignore_comments.items[i].lineno,
                                                  tag, p->arena);
            if (ti == NULL) {
                return NULL;
            }
            asdl_seq_SET(type_ignores, i, ti);
        }
    }
    return _PyAST_Module(a, type_ignores, p->arena);
}

PyObject *
_PyPegen_new_type_comment(Parser *p, const char *s)
{
    PyObject *res = PyUnicode_DecodeUTF8(s, strlen(s), NULL);
    if (res == NULL) {
        return NULL;
    }
    if (_PyArena_AddPyObject(p->arena, res) < 0) {
        Py_DECREF(res);
        return NULL;
    }
    return res;
}

arg_ty
_PyPegen_add_type_comment_to_arg(Parser *p, arg_ty a, Token *tc)
{
    if (tc == NULL) {
        return a;
    }
    const char *bytes = PyBytes_AsString(tc->bytes);
    if (bytes == NULL) {
        return NULL;
    }
    PyObject *tco = _PyPegen_new_type_comment(p, bytes);
    if (tco == NULL) {
        return NULL;
    }
    return _PyAST_arg(a->arg, a->annotation, tco,
                      a->lineno, a->col_offset, a->end_lineno, a->end_col_offset,
                      p->arena);
}

int
_PyPegen_check_legacy_stmt(Parser *p, expr_ty name) {
    if (name->kind != Name_kind) {
        return 0;
    }
    const char* candidates[2] = {"print", "exec"};
    for (int i=0; i<2; i++) {
        if (PyUnicode_CompareWithASCIIString(name->v.Name.id, candidates[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int
_PyPegen_is_dollar(Parser *p, expr_ty name) {
    if (name->kind != Name_kind) {
        return 0;
    }
    if (PyUnicode_CompareWithASCIIString(name->v.Name.id, "_dollar_item") == 0) {
        return 1;
    }
    return 0;
}

int
_PyPegen_check_barry_as_flufl(Parser *p, Token *t)
{
    // Loh currently treats "!=" normally even when Barry mode is enabled.
    return 0;
}

static ResultTokenWithMetadata *
result_token_with_metadata(Parser *p, void *result, PyObject *metadata)
{
    ResultTokenWithMetadata *res = _PyArena_Malloc(p->arena, sizeof(ResultTokenWithMetadata));
    if (res == NULL) {
        return NULL;
    }
    res->metadata = metadata;
    res->result = result;
    return res;
}

ResultTokenWithMetadata *
_PyPegen_check_fstring_conversion(Parser *p, Token* conv_token, expr_ty conv)
{
    if (conv_token->lineno != conv->lineno || conv_token->end_col_offset != conv->col_offset) {
        return RAISE_SYNTAX_ERROR_KNOWN_RANGE(
            conv_token, conv,
            "%c-string: conversion type must come right after the exclamation mark",
            TOK_GET_STRING_PREFIX(p->tok)
        );
    }

    Py_UCS4 first = PyUnicode_READ_CHAR(conv->v.Name.id, 0);
    if (PyUnicode_GET_LENGTH(conv->v.Name.id) > 1 ||
            !(first == 's' || first == 'r' || first == 'a')) {
        RAISE_SYNTAX_ERROR_KNOWN_LOCATION(conv,
                                            "%c-string: invalid conversion character %R: expected 's', 'r', or 'a'",
                                            TOK_GET_STRING_PREFIX(p->tok),
                                            conv->v.Name.id);
        return NULL;
    }

    return result_token_with_metadata(p, conv, conv_token->metadata);
}

ResultTokenWithMetadata *
_PyPegen_setup_full_format_spec(Parser *p, Token *colon, asdl_expr_seq *spec, int lineno, int col_offset,
                                int end_lineno, int end_col_offset, PyArena *arena)
{
    if (!spec) {
        return NULL;
    }

    // This is needed to keep compatibility with 3.11, where an empty format
    // spec is parsed as an *empty* JoinedStr node, instead of having an empty
    // constant in it.
    Py_ssize_t n_items = asdl_seq_LEN(spec);
    Py_ssize_t non_empty_count = 0;
    for (Py_ssize_t i = 0; i < n_items; i++) {
        expr_ty item = asdl_seq_GET(spec, i);
        non_empty_count += !(item->kind == Constant_kind &&
                             PyUnicode_CheckExact(item->v.Constant.value) &&
                             PyUnicode_GET_LENGTH(item->v.Constant.value) == 0);
    }
    if (non_empty_count != n_items) {
        asdl_expr_seq *resized_spec =
            _Py_asdl_expr_seq_new(non_empty_count, p->arena);
        if (resized_spec == NULL) {
            return NULL;
        }
        Py_ssize_t j = 0;
        for (Py_ssize_t i = 0; i < n_items; i++) {
            expr_ty item = asdl_seq_GET(spec, i);
            if (item->kind == Constant_kind &&
                PyUnicode_CheckExact(item->v.Constant.value) &&
                PyUnicode_GET_LENGTH(item->v.Constant.value) == 0) {
                continue;
            }
            asdl_seq_SET(resized_spec, j++, item);
        }
        assert(j == non_empty_count);
        spec = resized_spec;
    }
    expr_ty res;
    Py_ssize_t n = asdl_seq_LEN(spec);
    if (n == 0 || (n == 1 && asdl_seq_GET(spec, 0)->kind == Constant_kind)) {
        res = _PyAST_JoinedStr(spec, lineno, col_offset, end_lineno,
                                    end_col_offset, p->arena);
    } else {
        res = _PyPegen_concatenate_strings(p, spec,
                             lineno, col_offset, end_lineno,
                             end_col_offset, arena);
    }
    if (!res) {
        return NULL;
    }
    return result_token_with_metadata(p, res, colon->metadata);
}

const char *
_PyPegen_get_expr_name(expr_ty e)
{
    assert(e != NULL);
    switch (e->kind) {
        case Attribute_kind:
            return "attribute";
        case Subscript_kind:
            return "subscript";
        case Starred_kind:
            return "starred";
        case Name_kind:
            return "name";
        case List_kind:
            return "list";
        case Tuple_kind:
            return "tuple";
        case Lambda_kind:
            return "lambda";
        case Call_kind:
            return "function call";
        case Pipe_kind:
            return "pipe expression";
        case BoolOp_kind:
        case BinOp_kind:
        case UnaryOp_kind:
            return "expression";
        case GeneratorExp_kind:
            return "generator expression";
        case Yield_kind:
        case YieldFrom_kind:
            return "yield expression";
        case Await_kind:
            return "await expression";
        case ListComp_kind:
            return "list comprehension";
        case SetComp_kind:
            return "set comprehension";
        case DictComp_kind:
            return "dict comprehension";
        case Dict_kind:
            return "dict literal";
        case Set_kind:
            return "set display";
        case JoinedStr_kind:
        case FormattedValue_kind:
            return "f-string expression";
        case TemplateStr_kind:
        case Interpolation_kind:
            return "t-string expression";
        case Constant_kind: {
            PyObject *value = e->v.Constant.value;
            if (value == Py_None) {
                return "None";
            }
            if (value == Py_False) {
                return "False";
            }
            if (value == Py_True) {
                return "True";
            }
            if (value == Py_Ellipsis) {
                return "ellipsis";
            }
            return "literal";
        }
        case Compare_kind:
            return "comparison";
        case IfExp_kind:
            return "conditional expression";
        case NamedExpr_kind:
            return "named expression";
        default:
            PyErr_Format(PyExc_SystemError,
                         "unexpected expression in assignment %d (line %d)",
                         e->kind, e->lineno);
            return NULL;
    }
}

expr_ty
_PyPegen_get_last_comprehension_item(comprehension_ty comprehension) {
    if (comprehension->ifs == NULL || asdl_seq_LEN(comprehension->ifs) == 0) {
        return comprehension->iter;
    }
    return PyPegen_last_item(comprehension->ifs, expr_ty);
}

expr_ty _PyPegen_collect_call_seqs(Parser *p, asdl_expr_seq *a, asdl_seq *b,
                     int lineno, int col_offset, int end_lineno,
                     int end_col_offset, PyArena *arena) {
    Py_ssize_t args_len = asdl_seq_LEN(a);
    Py_ssize_t total_len = args_len;

    if (b == NULL) {
        return _PyAST_Call(_PyPegen_dummy_name(p), a, NULL, lineno, col_offset,
                        end_lineno, end_col_offset, arena);

    }

    asdl_expr_seq *starreds = _PyPegen_seq_extract_starred_exprs(p, b);
    asdl_keyword_seq *keywords = _PyPegen_seq_delete_starred_exprs(p, b);

    if (starreds) {
        total_len += asdl_seq_LEN(starreds);
    }

    asdl_expr_seq *args = _Py_asdl_expr_seq_new(total_len, arena);
    if (args == NULL) {
        return NULL;
    }

    Py_ssize_t i = 0;
    for (i = 0; i < args_len; i++) {
        asdl_seq_SET(args, i, asdl_seq_GET(a, i));
    }
    for (; i < total_len; i++) {
        asdl_seq_SET(args, i, asdl_seq_GET(starreds, i - args_len));
    }

    return _PyAST_Call(_PyPegen_dummy_name(p), args, keywords, lineno,
                       col_offset, end_lineno, end_col_offset, arena);
}

// AST Error reporting helpers

expr_ty
_PyPegen_get_invalid_target(expr_ty e, TARGETS_TYPE targets_type)
{
    if (e == NULL) {
        return NULL;
    }

#define VISIT_CONTAINER(CONTAINER, TYPE) do { \
        Py_ssize_t len = asdl_seq_LEN((CONTAINER)->v.TYPE.elts);\
        for (Py_ssize_t i = 0; i < len; i++) {\
            expr_ty other = asdl_seq_GET((CONTAINER)->v.TYPE.elts, i);\
            expr_ty child = _PyPegen_get_invalid_target(other, targets_type);\
            if (child != NULL) {\
                return child;\
            }\
        }\
    } while (0)

    // We only need to visit List and Tuple nodes recursively as those
    // are the only ones that can contain valid names in targets when
    // they are parsed as expressions. Any other kind of expression
    // that is a container (like Sets or Dicts) is directly invalid and
    // we don't need to visit it recursively.

    switch (e->kind) {
        case List_kind:
            VISIT_CONTAINER(e, List);
            return NULL;
        case Tuple_kind:
            VISIT_CONTAINER(e, Tuple);
            return NULL;
        case Starred_kind:
            if (targets_type == DEL_TARGETS) {
                return e;
            }
            return _PyPegen_get_invalid_target(e->v.Starred.value, targets_type);
        case Compare_kind:
            // This is needed, because the `a in b` in `for a in b` gets parsed
            // as a comparison, and so we need to search the left side of the comparison
            // for invalid targets.
            if (targets_type == FOR_TARGETS) {
                cmpop_ty cmpop = (cmpop_ty) asdl_seq_GET(e->v.Compare.ops, 0);
                if (cmpop == In) {
                    return _PyPegen_get_invalid_target(e->v.Compare.left, targets_type);
                }
                return NULL;
            }
            return e;
        case Name_kind:
        case Subscript_kind:
        case Attribute_kind:
            return NULL;
        default:
            return e;
    }
}

void *_PyPegen_arguments_parsing_error(Parser *p, expr_ty e) {
    int kwarg_unpacking = 0;
    for (Py_ssize_t i = 0, l = asdl_seq_LEN(e->v.Call.keywords); i < l; i++) {
        keyword_ty keyword = asdl_seq_GET(e->v.Call.keywords, i);
        if (!keyword->arg) {
            kwarg_unpacking = 1;
        }
    }

    const char *msg = NULL;
    if (kwarg_unpacking) {
        msg = "positional argument follows keyword argument unpacking";
    } else {
        msg = "positional argument follows keyword argument";
    }

    return RAISE_SYNTAX_ERROR(msg);
}

void *
_PyPegen_nonparen_genexp_in_call(Parser *p, expr_ty args, asdl_comprehension_seq *comprehensions)
{
    /* The rule that calls this function is 'args for_if_clauses'.
       For the input f(L, x for x in y), L and x are in args and
       the for is parsed as a for_if_clause. We have to check if
       len <= 1, so that input like dict((a, b) for a, b in x)
       gets successfully parsed and then we pass the last
       argument (x in the above example) as the location of the
       error */
    Py_ssize_t len = asdl_seq_LEN(args->v.Call.args);
    if (len <= 1) {
        return NULL;
    }

    comprehension_ty last_comprehension = PyPegen_last_item(comprehensions, comprehension_ty);

    return RAISE_SYNTAX_ERROR_KNOWN_RANGE(
        (expr_ty) asdl_seq_GET(args->v.Call.args, len - 1),
        _PyPegen_get_last_comprehension_item(last_comprehension),
        "Generator expression must be parenthesized"
    );
}

// Fstring stuff

static expr_ty
_PyPegen_decode_fstring_part(Parser* p, int is_raw, expr_ty constant, Token* token) {
    assert(PyUnicode_CheckExact(constant->v.Constant.value));

    const char* bstr = PyUnicode_AsUTF8(constant->v.Constant.value);
    if (bstr == NULL) {
        return NULL;
    }

    size_t len;
    if (strcmp(bstr, "{{") == 0 || strcmp(bstr, "}}") == 0) {
        len = 1;
    } else {
        len = strlen(bstr);
    }

    is_raw = is_raw || strchr(bstr, '\\') == NULL;
    PyObject *str = _PyPegen_decode_string(p, is_raw, bstr, len, token);
    if (str == NULL) {
        _Pypegen_raise_decode_error(p);
        return NULL;
    }
    if (_PyArena_AddPyObject(p->arena, str) < 0) {
        Py_DECREF(str);
        return NULL;
    }
    return _PyAST_Constant(str, NULL, constant->lineno, constant->col_offset,
                           constant->end_lineno, constant->end_col_offset,
                           p->arena);
}

static asdl_expr_seq *
_get_resized_exprs(Parser *p, Token *a, asdl_expr_seq *raw_expressions, Token *b, enum string_kind_t string_kind)
{
    Py_ssize_t n_items = asdl_seq_LEN(raw_expressions);
    Py_ssize_t total_items = n_items;
    for (Py_ssize_t i = 0; i < n_items; i++) {
        expr_ty item = asdl_seq_GET(raw_expressions, i);
        if (item->kind == JoinedStr_kind) {
            total_items += asdl_seq_LEN(item->v.JoinedStr.values) - 1;
        }
    }

    const char* quote_str = PyBytes_AsString(a->bytes);
    if (quote_str == NULL) {
        return NULL;
    }
    int is_raw = strpbrk(quote_str, "rR") != NULL;

    asdl_expr_seq *seq = _Py_asdl_expr_seq_new(total_items, p->arena);
    if (seq == NULL) {
        return NULL;
    }

    Py_ssize_t index = 0;
    for (Py_ssize_t i = 0; i < n_items; i++) {
        expr_ty item = asdl_seq_GET(raw_expressions, i);

        // This should correspond to a JoinedStr node of two elements
        // created _PyPegen_formatted_value. This situation can only be the result of
        // a (f|t)-string debug expression where the first element is a constant with the text and the second
        // a formatted value with the expression.
        if (item->kind == JoinedStr_kind) {
            asdl_expr_seq *values = item->v.JoinedStr.values;
            if (asdl_seq_LEN(values) != 2) {
                PyErr_Format(PyExc_SystemError,
                             string_kind == TSTRING
                             ? "unexpected TemplateStr node without debug data in t-string at line %d"
                             : "unexpected JoinedStr node without debug data in f-string at line %d",
                             item->lineno);
                return NULL;
            }

            expr_ty first = asdl_seq_GET(values, 0);
            assert(first->kind == Constant_kind);
            asdl_seq_SET(seq, index++, first);

            expr_ty second = asdl_seq_GET(values, 1);
            assert((string_kind == TSTRING && second->kind == Interpolation_kind) || second->kind == FormattedValue_kind);
            asdl_seq_SET(seq, index++, second);

            continue;
        }

        if (item->kind == Constant_kind) {
            item = _PyPegen_decode_fstring_part(p, is_raw, item, b);
            if (item == NULL) {
                return NULL;
            }

            /* Tokenizer emits string parts even when the underlying string
            might become an empty value (e.g. FSTRING_MIDDLE with the value \\n)
            so we need to check for them and simplify it here. */
            if (PyUnicode_CheckExact(item->v.Constant.value)
                && PyUnicode_GET_LENGTH(item->v.Constant.value) == 0) {
                continue;
            }
        }
        asdl_seq_SET(seq, index++, item);
    }

    asdl_expr_seq *resized_exprs;
    if (index != total_items) {
        resized_exprs = _Py_asdl_expr_seq_new(index, p->arena);
        if (resized_exprs == NULL) {
            return NULL;
        }
        for (Py_ssize_t i = 0; i < index; i++) {
            asdl_seq_SET(resized_exprs, i, asdl_seq_GET(seq, i));
        }
    }
    else {
        resized_exprs = seq;
    }
    return resized_exprs;
}

expr_ty
_PyPegen_template_str(Parser *p, Token *a, asdl_expr_seq *raw_expressions, Token *b) {

    asdl_expr_seq *resized_exprs = _get_resized_exprs(p, a, raw_expressions, b, TSTRING);
    return _PyAST_TemplateStr(resized_exprs, a->lineno, a->col_offset,
                              b->end_lineno, b->end_col_offset,
                              p->arena);
}

expr_ty
_PyPegen_joined_str(Parser *p, Token* a, asdl_expr_seq* raw_expressions, Token*b) {

    asdl_expr_seq *resized_exprs = _get_resized_exprs(p, a, raw_expressions, b, FSTRING);
    return _PyAST_JoinedStr(resized_exprs, a->lineno, a->col_offset,
                            b->end_lineno, b->end_col_offset,
                            p->arena);
}

expr_ty _PyPegen_decoded_constant_from_token(Parser* p, Token* tok) {
    Py_ssize_t bsize;
    char* bstr;
    if (PyBytes_AsStringAndSize(tok->bytes, &bstr, &bsize) == -1) {
        return NULL;
    }

    // Check if we're inside a raw f-string for format spec decoding
    int is_raw = 0;
    if (INSIDE_FSTRING(p->tok)) {
        tokenizer_mode *mode = TOK_GET_MODE(p->tok);
        is_raw = mode->raw;
    }

    PyObject* str = _PyPegen_decode_string(p, is_raw, bstr, bsize, tok);
    if (str == NULL) {
        return NULL;
    }
    if (_PyArena_AddPyObject(p->arena, str) < 0) {
        Py_DECREF(str);
        return NULL;
    }
    return _PyAST_Constant(str, NULL, tok->lineno, tok->col_offset,
                           tok->end_lineno, tok->end_col_offset,
                           p->arena);
}

expr_ty _PyPegen_constant_from_token(Parser* p, Token* tok) {
    char* bstr = PyBytes_AsString(tok->bytes);
    if (bstr == NULL) {
        return NULL;
    }
    PyObject* str = PyUnicode_FromString(bstr);
    if (str == NULL) {
        return NULL;
    }
    if (_PyArena_AddPyObject(p->arena, str) < 0) {
        Py_DECREF(str);
        return NULL;
    }
    return _PyAST_Constant(str, NULL, tok->lineno, tok->col_offset,
                           tok->end_lineno, tok->end_col_offset,
                           p->arena);
}

expr_ty _PyPegen_constant_from_string(Parser* p, Token* tok) {
    char* the_str = PyBytes_AsString(tok->bytes);
    if (the_str == NULL) {
        return NULL;
    }
    PyObject *s = _PyPegen_parse_string(p, tok);
    if (s == NULL) {
        _Pypegen_raise_decode_error(p);
        return NULL;
    }
    if (_PyArena_AddPyObject(p->arena, s) < 0) {
        Py_DECREF(s);
        return NULL;
    }
    PyObject *kind = NULL;
    if (the_str) {
        if (the_str[0] == 'u' || the_str[0] == 'U') {
            kind = _PyPegen_new_identifier(p, "u");
            if (kind == NULL) {
                return NULL;
            }
        } else if (the_str[0] == 'r' || the_str[0] == 'R') {
            kind = _PyPegen_new_identifier(p, "r");
            if (kind == NULL) {
                return NULL;
            }
        } else if (the_str[0] == 'n' || the_str[0] == 'N') {
            kind = _PyPegen_new_identifier(p, "n");
            if (kind == NULL) {
                return NULL;
            }
        }
    }
    return _PyAST_Constant(s, kind, tok->lineno, tok->col_offset, tok->end_lineno, tok->end_col_offset, p->arena);
}

static int
_get_interpolation_conversion(Parser *p, Token *debug, ResultTokenWithMetadata *conversion,
                              ResultTokenWithMetadata *format)
{
    if (conversion != NULL) {
        expr_ty conversion_expr = (expr_ty) conversion->result;
        assert(conversion_expr->kind == Name_kind);
        Py_UCS4 first = PyUnicode_READ_CHAR(conversion_expr->v.Name.id, 0);
        return Py_SAFE_DOWNCAST(first, Py_UCS4, int);
    }
    else if (debug && !format) {
        /* If no conversion is specified, use !r for debug expressions */
        return (int)'r';
    }
    return -1;
}

static PyObject *
_strip_interpolation_expr(PyObject *exprstr)
{
    Py_ssize_t len = PyUnicode_GET_LENGTH(exprstr);

    for (Py_ssize_t i = len - 1; i >= 0; i--) {
        Py_UCS4 c = PyUnicode_READ_CHAR(exprstr, i);
        if (_PyUnicode_IsWhitespace(c) || c == '=') {
            len--;
        }
        else {
            break;
        }
    }

    return PyUnicode_Substring(exprstr, 0, len);
}

expr_ty _PyPegen_interpolation(Parser *p, expr_ty expression, Token *debug, ResultTokenWithMetadata *conversion,
                                 ResultTokenWithMetadata *format, Token *closing_brace, int lineno, int col_offset,
                                 int end_lineno, int end_col_offset, PyArena *arena) {

    int conversion_val = _get_interpolation_conversion(p, debug, conversion, format);

    /* Find the non whitespace token after the "=" */
    int debug_end_line, debug_end_offset;
    PyObject *debug_metadata;
    constant exprstr;

    if (conversion) {
        debug_end_line = ((expr_ty) conversion->result)->lineno;
        debug_end_offset = ((expr_ty) conversion->result)->col_offset;
        debug_metadata = exprstr = conversion->metadata;
    }
    else if (format) {
        debug_end_line = ((expr_ty) format->result)->lineno;
        debug_end_offset = ((expr_ty) format->result)->col_offset + 1;
        debug_metadata = exprstr = format->metadata;
    }
    else {
        debug_end_line = end_lineno;
        debug_end_offset = end_col_offset;
        debug_metadata = exprstr = closing_brace->metadata;
    }

    assert(exprstr != NULL);
    PyObject *final_exprstr = _strip_interpolation_expr(exprstr);
    if (!final_exprstr || _PyArena_AddPyObject(arena, final_exprstr) < 0) {
        Py_XDECREF(final_exprstr);
        return NULL;
    }

    expr_ty interpolation = _PyAST_Interpolation(
        expression, final_exprstr, conversion_val, format ? (expr_ty) format->result : NULL,
        lineno, col_offset, end_lineno,
        end_col_offset, arena
    );

    if (!debug) {
        return interpolation;
    }

    expr_ty debug_text = _PyAST_Constant(debug_metadata, NULL, lineno, col_offset + 1, debug_end_line,
                                            debug_end_offset - 1, p->arena);
    if (!debug_text) {
        return NULL;
    }

    asdl_expr_seq *values = _Py_asdl_expr_seq_new(2, arena);
    asdl_seq_SET(values, 0, debug_text);
    asdl_seq_SET(values, 1, interpolation);
    return _PyAST_JoinedStr(values, lineno, col_offset, debug_end_line, debug_end_offset, p->arena);
}

expr_ty _PyPegen_formatted_value(Parser *p, expr_ty expression, Token *debug, ResultTokenWithMetadata *conversion,
                                 ResultTokenWithMetadata *format, Token *closing_brace, int lineno, int col_offset,
                                 int end_lineno, int end_col_offset, PyArena *arena) {
    int conversion_val = _get_interpolation_conversion(p, debug, conversion, format);

    expr_ty formatted_value = _PyAST_FormattedValue(
        expression, conversion_val, format ? (expr_ty) format->result : NULL,
        lineno, col_offset, end_lineno,
        end_col_offset, arena
    );

    if (!debug) {
        return formatted_value;
    }

    /* Find the non whitespace token after the "=" */
    int debug_end_line, debug_end_offset;
    PyObject *debug_metadata;

    if (conversion) {
        debug_end_line = ((expr_ty) conversion->result)->lineno;
        debug_end_offset = ((expr_ty) conversion->result)->col_offset;
        debug_metadata = conversion->metadata;
    }
    else if (format) {
        debug_end_line = ((expr_ty) format->result)->lineno;
        debug_end_offset = ((expr_ty) format->result)->col_offset + 1;
        debug_metadata = format->metadata;
    }
    else {
        debug_end_line = end_lineno;
        debug_end_offset = end_col_offset;
        debug_metadata = closing_brace->metadata;
    }
    expr_ty debug_text = _PyAST_Constant(debug_metadata, NULL, lineno, col_offset + 1, debug_end_line,
                                            debug_end_offset - 1, p->arena);
    if (!debug_text) {
        return NULL;
    }

    asdl_expr_seq *values = _Py_asdl_expr_seq_new(2, arena);
    asdl_seq_SET(values, 0, debug_text);
    asdl_seq_SET(values, 1, formatted_value);
    return _PyAST_JoinedStr(values, lineno, col_offset, debug_end_line, debug_end_offset, p->arena);
}

static expr_ty
_build_concatenated_bytes(Parser *p, asdl_expr_seq *strings, int lineno,
                        int col_offset, int end_lineno, int end_col_offset,
                        PyArena *arena)
{
    Py_ssize_t len = asdl_seq_LEN(strings);
    assert(len > 0);

    PyObject* res = Py_GetConstant(Py_CONSTANT_EMPTY_BYTES);

    /* Bytes literals never get a kind, but just for consistency
        since they are represented as Constant nodes, we'll mirror
        the same behavior as unicode strings for determining the
        kind. */
    PyObject* kind = asdl_seq_GET(strings, 0)->v.Constant.kind;
    for (Py_ssize_t i = 0; i < len; i++) {
        expr_ty elem = asdl_seq_GET(strings, i);
        PyBytes_Concat(&res, elem->v.Constant.value);
    }
    if (!res || _PyArena_AddPyObject(arena, res) < 0) {
        Py_XDECREF(res);
        return NULL;
    }
    return _PyAST_Constant(res, kind, lineno, col_offset, end_lineno, end_col_offset, p->arena);
}

static expr_ty
_build_concatenated_unicode(Parser *p, asdl_expr_seq *strings, int lineno,
                        int col_offset, int end_lineno, int end_col_offset,
                        PyArena *arena)
{
    Py_ssize_t len = asdl_seq_LEN(strings);
    assert(len > 1);

    expr_ty first = asdl_seq_GET(strings, 0);

    /* When a string is getting concatenated, the kind of the string
        is determined by the first string in the concatenation
        sequence.

        u"abc" "def" -> u"abcdef"
        "abc" u"abc" ->  "abcabc" */
    PyObject *kind = first->v.Constant.kind;

    PyUnicodeWriter *writer = PyUnicodeWriter_Create(0);
    if (writer == NULL) {
        return NULL;
    }

    for (Py_ssize_t i = 0; i < len; i++) {
        expr_ty current_elem = asdl_seq_GET(strings, i);
        assert(current_elem->kind == Constant_kind);

        if (PyUnicodeWriter_WriteStr(writer,
                                     current_elem->v.Constant.value)) {
            PyUnicodeWriter_Discard(writer);
            return NULL;
        }
    }

    PyObject *final = PyUnicodeWriter_Finish(writer);
    if (final == NULL) {
        return NULL;
    }
    if (_PyArena_AddPyObject(p->arena, final) < 0) {
        Py_DECREF(final);
        return NULL;
    }
    return _PyAST_Constant(final, kind, lineno, col_offset,
                           end_lineno, end_col_offset, arena);
}

static asdl_expr_seq *
_build_concatenated_str(Parser *p, asdl_expr_seq *strings,
                               int lineno, int col_offset, int end_lineno,
                               int end_col_offset, PyArena *arena)
{
    Py_ssize_t len = asdl_seq_LEN(strings);
    assert(len > 0);

    Py_ssize_t n_flattened_elements = 0;
    for (Py_ssize_t i = 0; i < len; i++) {
        expr_ty elem = asdl_seq_GET(strings, i);
        switch(elem->kind) {
            case JoinedStr_kind:
                n_flattened_elements += asdl_seq_LEN(elem->v.JoinedStr.values);
                break;
            case TemplateStr_kind:
                n_flattened_elements += asdl_seq_LEN(elem->v.TemplateStr.values);
                break;
            default:
                n_flattened_elements++;
                break;
        }
    }


    asdl_expr_seq* flattened = _Py_asdl_expr_seq_new(n_flattened_elements, p->arena);
    if (flattened == NULL) {
        return NULL;
    }

    /* build flattened list */
    Py_ssize_t current_pos = 0;
    for (Py_ssize_t i = 0; i < len; i++) {
        expr_ty elem = asdl_seq_GET(strings, i);
        switch(elem->kind) {
            case JoinedStr_kind:
                for (Py_ssize_t j = 0; j < asdl_seq_LEN(elem->v.JoinedStr.values); j++) {
                    expr_ty subvalue = asdl_seq_GET(elem->v.JoinedStr.values, j);
                    if (subvalue == NULL) {
                        return NULL;
                    }
                    asdl_seq_SET(flattened, current_pos++, subvalue);
                }
                break;
            case TemplateStr_kind:
                for (Py_ssize_t j = 0; j < asdl_seq_LEN(elem->v.TemplateStr.values); j++) {
                    expr_ty subvalue = asdl_seq_GET(elem->v.TemplateStr.values, j);
                    if (subvalue == NULL) {
                        return NULL;
                    }
                    asdl_seq_SET(flattened, current_pos++, subvalue);
                }
                break;
            default:
                asdl_seq_SET(flattened, current_pos++, elem);
                break;
        }
    }

    /* calculate folded element count */
    Py_ssize_t n_elements = 0;
    int prev_is_constant = 0;
    for (Py_ssize_t i = 0; i < n_flattened_elements; i++) {
        expr_ty elem = asdl_seq_GET(flattened, i);

        /* The concatenation of a FormattedValue and an empty Constant should
           lead to the FormattedValue itself. Thus, we will not take any empty
           constants into account, just as in `_PyPegen_joined_str` */
        if (elem->kind == Constant_kind &&
            PyUnicode_CheckExact(elem->v.Constant.value) &&
            PyUnicode_GET_LENGTH(elem->v.Constant.value) == 0)
            continue;

        if (!prev_is_constant || elem->kind != Constant_kind) {
            n_elements++;
        }
        prev_is_constant = elem->kind == Constant_kind;
    }

    asdl_expr_seq* values = _Py_asdl_expr_seq_new(n_elements, p->arena);
    if (values == NULL) {
        return NULL;
    }

    /* build folded list */
    current_pos = 0;
    for (Py_ssize_t i = 0; i < n_flattened_elements; i++) {
        expr_ty elem = asdl_seq_GET(flattened, i);

        /* if the current elem and the following are constants,
           fold them and all consequent constants */
        if (elem->kind == Constant_kind) {
            if (i + 1 < n_flattened_elements &&
                asdl_seq_GET(flattened, i + 1)->kind == Constant_kind) {
                expr_ty first_elem = elem;

                /* When a string is getting concatenated, the kind of the string
                   is determined by the first string in the concatenation
                   sequence.

                   u"abc" "def" -> u"abcdef"
                   "abc" u"abc" ->  "abcabc" */
                PyObject *kind = elem->v.Constant.kind;

                PyUnicodeWriter *writer = PyUnicodeWriter_Create(0);
                if (writer == NULL) {
                    return NULL;
                }
                expr_ty last_elem = elem;
                Py_ssize_t j;
                for (j = i; j < n_flattened_elements; j++) {
                    expr_ty current_elem = asdl_seq_GET(flattened, j);
                    if (current_elem->kind == Constant_kind) {
                        if (PyUnicodeWriter_WriteStr(writer,
                                                     current_elem->v.Constant.value)) {
                            PyUnicodeWriter_Discard(writer);
                            return NULL;
                        }
                        last_elem = current_elem;
                    } else {
                        break;
                    }
                }
                i = j - 1;

                PyObject *concat_str = PyUnicodeWriter_Finish(writer);
                if (concat_str == NULL) {
                    return NULL;
                }
                if (_PyArena_AddPyObject(p->arena, concat_str) < 0) {
                    Py_DECREF(concat_str);
                    return NULL;
                }
                elem = _PyAST_Constant(concat_str, kind, first_elem->lineno,
                                       first_elem->col_offset,
                                       last_elem->end_lineno,
                                       last_elem->end_col_offset, p->arena);
                if (elem == NULL) {
                    return NULL;
                }
            }

            /* Drop all empty contanst strings */
            if (PyUnicode_CheckExact(elem->v.Constant.value) &&
                PyUnicode_GET_LENGTH(elem->v.Constant.value) == 0) {
                continue;
            }
        }

        asdl_seq_SET(values, current_pos++, elem);
    }

    assert(current_pos == n_elements);
    return values;
}

static expr_ty
_build_concatenated_joined_str(Parser *p, asdl_expr_seq *strings,
                               int lineno, int col_offset, int end_lineno,
                               int end_col_offset, PyArena *arena)
{
    asdl_expr_seq *values = _build_concatenated_str(p, strings, lineno,
        col_offset, end_lineno, end_col_offset, arena);
    return _PyAST_JoinedStr(values, lineno, col_offset, end_lineno, end_col_offset, p->arena);
}

expr_ty
_PyPegen_concatenate_tstrings(Parser *p, asdl_expr_seq *strings,
                               int lineno, int col_offset, int end_lineno,
                               int end_col_offset, PyArena *arena)
{
    asdl_expr_seq *values = _build_concatenated_str(p, strings, lineno,
        col_offset, end_lineno, end_col_offset, arena);
    return _PyAST_TemplateStr(values, lineno, col_offset, end_lineno,
        end_col_offset, arena);
}

expr_ty
_PyPegen_concatenate_strings(Parser *p, asdl_expr_seq *strings,
                             int lineno, int col_offset, int end_lineno,
                             int end_col_offset, PyArena *arena)
{
    Py_ssize_t len = asdl_seq_LEN(strings);
    assert(len > 0);

    int f_string_found = 0;
    int unicode_string_found = 0;
    int bytes_found = 0;

    Py_ssize_t i = 0;
    for (i = 0; i < len; i++) {
        expr_ty elem = asdl_seq_GET(strings, i);
        switch(elem->kind) {
            case Constant_kind:
                if (PyBytes_CheckExact(elem->v.Constant.value)) {
                    bytes_found = 1;
                } else {
                    unicode_string_found = 1;
                }
                break;
            case JoinedStr_kind:
                f_string_found = 1;
                break;
            case TemplateStr_kind:
                // python.gram handles this; we should never get here
                assert(0);
                break;
            default:
                f_string_found = 1;
                break;
        }
    }

    // Cannot mix unicode and bytes
    if ((unicode_string_found || f_string_found) && bytes_found) {
        RAISE_SYNTAX_ERROR("cannot mix bytes and nonbytes literals");
        return NULL;
    }

    // If it's only bytes or only unicode string, do a simple concat
    if (!f_string_found) {
        if (len == 1) {
            return asdl_seq_GET(strings, 0);
        }
        else if (bytes_found) {
            return _build_concatenated_bytes(p, strings, lineno, col_offset,
                end_lineno, end_col_offset, arena);
        }
        else {
            return _build_concatenated_unicode(p, strings, lineno, col_offset,
                end_lineno, end_col_offset, arena);
        }
    }

    return _build_concatenated_joined_str(p, strings, lineno,
        col_offset, end_lineno, end_col_offset, arena);
}

stmt_ty
_PyPegen_checked_future_import(Parser *p, identifier module, asdl_alias_seq * names, int level,
                  			   int lineno, int col_offset, int end_lineno, int end_col_offset,
                      		   PyArena *arena) {
    if (level == 0 && PyUnicode_CompareWithASCIIString(module, "__future__") == 0) {
        for (Py_ssize_t i = 0; i < asdl_seq_LEN(names); i++) {
            alias_ty alias = asdl_seq_GET(names, i);
            if (PyUnicode_CompareWithASCIIString(alias->name, "barry_as_FLUFL") == 0) {
                p->flags |= PyPARSE_BARRY_AS_BDFL;
            }
        }
    }
    return _PyAST_ImportFrom(module, names, level, lineno, col_offset, end_lineno, end_col_offset, arena);
}

asdl_stmt_seq*
_PyPegen_register_stmts(Parser *p, asdl_stmt_seq* stmts) {
    if (!p->call_invalid_rules) {
        return stmts;
    }
    Py_ssize_t len = asdl_seq_LEN(stmts);
    if (len == 0) {
        return stmts;
    }
    stmt_ty last_stmt = asdl_seq_GET(stmts, len - 1);
    if (p->last_stmt_location.lineno > last_stmt->lineno) {
        return stmts;
    }
    p->last_stmt_location.lineno = last_stmt->lineno;
    p->last_stmt_location.col_offset = last_stmt->col_offset;
    p->last_stmt_location.end_lineno = last_stmt->end_lineno;
    p->last_stmt_location.end_col_offset = last_stmt->end_col_offset;
    return stmts;
}

expr_ty
_PyPegen_safe_navigation(Parser *p, expr_ty primary, expr_ty attr_name) {
    PyObject *var_id = _PyPegen_new_identifier(p, "_loh_safe_val");
    if (!var_id) return NULL;
    
    expr_ty target = _PyAST_Name(var_id, Store, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!target) return NULL;
    
    expr_ty named_expr = _PyAST_NamedExpr(target, primary, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!named_expr) return NULL;
    
    expr_ty none_const = _PyAST_Constant(Py_None, NULL, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!none_const) return NULL;
    
    asdl_int_seq *ops = _Py_asdl_int_seq_new(1, p->arena);
    if (!ops) return NULL;
    asdl_seq_SET(ops, 0, IsNot);
    
    asdl_expr_seq *comparators = _Py_asdl_expr_seq_new(1, p->arena);
    if (!comparators) return NULL;
    asdl_seq_SET(comparators, 0, none_const);
    
    expr_ty test = _PyAST_Compare(named_expr, ops, comparators, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!test) return NULL;
    
    expr_ty load_var = _PyAST_Name(var_id, Load, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!load_var) return NULL;
    
    expr_ty body = _PyAST_Attribute(load_var, attr_name->v.Name.id, Load, primary->lineno, primary->col_offset, attr_name->end_lineno, attr_name->end_col_offset, p->arena);
    if (!body) return NULL;
    
    expr_ty orelse = _PyAST_Constant(Py_None, NULL, primary->lineno, primary->col_offset, attr_name->end_lineno, attr_name->end_col_offset, p->arena);
    if (!orelse) return NULL;
    
    return _PyAST_IfExp(test, body, orelse, primary->lineno, primary->col_offset, attr_name->end_lineno, attr_name->end_col_offset, p->arena);
}

expr_ty
_PyPegen_none_coalesce(Parser *p, expr_ty left, expr_ty right) {
    PyObject *var_id = _PyPegen_new_identifier(p, "_loh_coalesce_val");
    if (!var_id) return NULL;
    
    expr_ty target = _PyAST_Name(var_id, Store, left->lineno, left->col_offset, left->end_lineno, left->end_col_offset, p->arena);
    if (!target) return NULL;
    
    expr_ty named_expr = _PyAST_NamedExpr(target, left, left->lineno, left->col_offset, left->end_lineno, left->end_col_offset, p->arena);
    if (!named_expr) return NULL;
    
    expr_ty none_const = _PyAST_Constant(Py_None, NULL, left->lineno, left->col_offset, left->end_lineno, left->end_col_offset, p->arena);
    if (!none_const) return NULL;
    
    asdl_int_seq *ops = _Py_asdl_int_seq_new(1, p->arena);
    if (!ops) return NULL;
    asdl_seq_SET(ops, 0, IsNot);
    
    asdl_expr_seq *comparators = _Py_asdl_expr_seq_new(1, p->arena);
    if (!comparators) return NULL;
    asdl_seq_SET(comparators, 0, none_const);
    
    expr_ty test = _PyAST_Compare(named_expr, ops, comparators, left->lineno, left->col_offset, left->end_lineno, left->end_col_offset, p->arena);
    if (!test) return NULL;
    
    expr_ty body = _PyAST_Name(var_id, Load, left->lineno, left->col_offset, left->end_lineno, left->end_col_offset, p->arena);
    if (!body) return NULL;
    
    return _PyAST_IfExp(test, body, right, left->lineno, left->col_offset, right->end_lineno, right->end_col_offset, p->arena);
}

expr_ty
_PyPegen_safe_subscript(Parser *p, expr_ty primary, expr_ty slice) {
    PyObject *var_id = _PyPegen_new_identifier(p, "_loh_safe_val");
    if (!var_id) return NULL;
    
    expr_ty target = _PyAST_Name(var_id, Store, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!target) return NULL;
    
    expr_ty named_expr = _PyAST_NamedExpr(target, primary, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!named_expr) return NULL;
    
    expr_ty none_const = _PyAST_Constant(Py_None, NULL, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!none_const) return NULL;
    
    asdl_int_seq *ops = _Py_asdl_int_seq_new(1, p->arena);
    if (!ops) return NULL;
    asdl_seq_SET(ops, 0, IsNot);
    
    asdl_expr_seq *comparators = _Py_asdl_expr_seq_new(1, p->arena);
    if (!comparators) return NULL;
    asdl_seq_SET(comparators, 0, none_const);
    
    expr_ty test = _PyAST_Compare(named_expr, ops, comparators, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!test) return NULL;
    
    expr_ty load_var = _PyAST_Name(var_id, Load, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!load_var) return NULL;
    
    expr_ty body = _PyAST_Subscript(load_var, slice, Load, primary->lineno, primary->col_offset, slice->end_lineno, slice->end_col_offset, p->arena);
    if (!body) return NULL;
    
    expr_ty orelse = _PyAST_Constant(Py_None, NULL, primary->lineno, primary->col_offset, slice->end_lineno, slice->end_col_offset, p->arena);
    if (!orelse) return NULL;
    
    return _PyAST_IfExp(test, body, orelse, primary->lineno, primary->col_offset, slice->end_lineno, slice->end_col_offset, p->arena);
}

expr_ty
_PyPegen_range_literal(Parser *p, expr_ty left, expr_ty right) {
    PyObject *range_id = _PyPegen_new_identifier(p, "range");
    if (!range_id) return NULL;

    expr_ty func = _PyAST_Name(range_id, Load, left->lineno, left->col_offset, right->end_lineno, right->end_col_offset, p->arena);
    if (!func) return NULL;

    asdl_expr_seq *args = _Py_asdl_expr_seq_new(2, p->arena);
    if (!args) return NULL;
    asdl_seq_SET(args, 0, left);
    asdl_seq_SET(args, 1, right);

    return _PyAST_Call(func, args, NULL, left->lineno, left->col_offset, right->end_lineno, right->end_col_offset, p->arena);
}

PyObject *
_PyPegen_make_dot_identifier(Parser *p, PyObject *name)
{
    const char *str = PyUnicode_AsUTF8(name);
    if (!str) {
        return NULL;
    }
    char buf[512];
    snprintf(buf, sizeof(buf), ".%s", str);
    return _PyPegen_new_identifier(p, buf);
}

static asdl_stmt_seq *
desugar_arg_sequence(Parser *p, asdl_arg_seq *arg_list, asdl_stmt_seq *body) {
    if (!arg_list) return body;
    int len = asdl_seq_LEN(arg_list);
    for (int i = len - 1; i >= 0; i--) {
        arg_ty arg = asdl_seq_GET(arg_list, i);
        PyObject *name = arg->arg;
        const char *name_str = PyUnicode_AsUTF8(name);
        if (name_str && name_str[0] == '.' && strlen(name_str) > 1) {
            // Extract real name: name_str + 1
            PyObject *real_name = _PyPegen_new_identifier(p, name_str + 1);
            if (!real_name) return NULL;
            
            // Construct assignment: .name = name
            expr_ty self_name = _PyAST_Name(_PyPegen_new_identifier(p, "."), Load, arg->lineno, arg->col_offset, arg->end_lineno, arg->end_col_offset, p->arena);
            expr_ty target = _PyAST_Attribute(self_name, real_name, Store, arg->lineno, arg->col_offset, arg->end_lineno, arg->end_col_offset, p->arena);
            expr_ty val = _PyAST_Name(real_name, Load, arg->lineno, arg->col_offset, arg->end_lineno, arg->end_col_offset, p->arena);
            asdl_expr_seq *targets = (asdl_expr_seq *)_PyPegen_singleton_seq(p, target);
            stmt_ty assign = _PyAST_Assign(targets, val, NULL, arg->lineno, arg->col_offset, arg->end_lineno, arg->end_col_offset, p->arena);
            
            // Prepend to body
            body = (asdl_stmt_seq *)_PyPegen_seq_insert_in_front(p, assign, (asdl_seq *)body);
            if (!body) return NULL;
            
            // Update the argument name to the clean name
            arg->arg = real_name;
        }
    }
    return body;
}

arg_ty
_PyPegen_make_aliased_arg(Parser *p, expr_ty primary, expr_ty alias, expr_ty annotation)
{
    if (!primary || primary->kind != Name_kind || !alias || alias->kind != Name_kind) {
        return NULL;
    }
    const char *primary_str = PyUnicode_AsUTF8(primary->v.Name.id);
    const char *alias_str = PyUnicode_AsUTF8(alias->v.Name.id);
    if (!primary_str || !alias_str) {
        return NULL;
    }
    char buf[512];
    snprintf(buf, sizeof(buf), "%s|%s", primary_str, alias_str);
    PyObject *combined_id = _PyPegen_new_identifier(p, buf);
    if (!combined_id) {
        return NULL;
    }
    return _PyAST_arg(combined_id, annotation, NULL, primary->lineno, primary->col_offset, alias->end_lineno, alias->end_col_offset, p->arena);
}

stmt_ty
_PyPegen_make_filtered_for(Parser *p, int is_async, expr_ty target, expr_ty iter, expr_ty cond, asdl_stmt_seq *body, asdl_stmt_seq *orelse, PyObject *type_comment, int lineno, int col, int end_lineno, int end_col, PyArena *arena)
{
    if (cond) {
        // Construct `if not (cond): continue`
        // 1. UnaryOp(Not, cond)
        expr_ty not_cond = _PyAST_UnaryOp(Not, cond, lineno, col, end_lineno, end_col, arena);
        if (!not_cond) return NULL;
        
        // 2. Continue
        stmt_ty continue_stmt = _PyAST_Continue(lineno, col, end_lineno, end_col, arena);
        if (!continue_stmt) return NULL;
        
        // 3. body of if statement (sequence containing continue)
        asdl_stmt_seq *if_body = (asdl_stmt_seq *)_PyPegen_singleton_seq(p, continue_stmt);
        if (!if_body) return NULL;
        
        // 4. If statement
        stmt_ty if_stmt = _PyAST_If(not_cond, if_body, NULL, lineno, col, end_lineno, end_col, arena);
        if (!if_stmt) return NULL;
        
        // 5. Prepend to body
        body = (asdl_stmt_seq *)_PyPegen_seq_insert_in_front(p, if_stmt, (asdl_seq *)body);
        if (!body) return NULL;
    }
    
    if (is_async) {
        return _PyAST_AsyncFor(target, iter, body, orelse, type_comment, lineno, col, end_lineno, end_col, arena);
    } else {
        return _PyAST_For(target, iter, body, orelse, type_comment, lineno, col, end_lineno, end_col, arena);
    }
}


static asdl_arg_seq *
append_arg_seq(Parser *p, asdl_arg_seq *seq, arg_ty element)
{
    int len = seq ? asdl_seq_LEN(seq) : 0;
    asdl_arg_seq *new_seq = _Py_asdl_arg_seq_new(len + 1, p->arena);
    if (!new_seq) return NULL;
    for (int i = 0; i < len; i++) {
        asdl_seq_SET(new_seq, i, asdl_seq_GET(seq, i));
    }
    asdl_seq_SET(new_seq, len, element);
    return new_seq;
}

static asdl_expr_seq *
append_expr_seq(Parser *p, asdl_expr_seq *seq, expr_ty element)
{
    int len = seq ? asdl_seq_LEN(seq) : 0;
    asdl_expr_seq *new_seq = _Py_asdl_expr_seq_new(len + 1, p->arena);
    if (!new_seq) return NULL;
    for (int i = 0; i < len; i++) {
        asdl_seq_SET(new_seq, i, asdl_seq_GET(seq, i));
    }
    asdl_seq_SET(new_seq, len, element);
    return new_seq;
}

static stmt_ty
make_alias_error_check(Parser *p, PyObject *name_id, PyObject *alias_id, PyObject *sentinel_id, const char *func_name, int lineno, int col, int end_lineno, int end_col)
{
    PyArena *arena = p->arena;
    
    // name is not _LOH_SENTINEL
    expr_ty name_node = _PyAST_Name(name_id, Load, lineno, col, end_lineno, end_col, arena);
    expr_ty sentinel_node = _PyAST_Name(sentinel_id, Load, lineno, col, end_lineno, end_col, arena);
    asdl_int_seq *ops1 = _Py_asdl_int_seq_new(1, arena);
    asdl_seq_SET(ops1, 0, IsNot);
    asdl_expr_seq *comps1 = _Py_asdl_expr_seq_new(1, arena);
    asdl_seq_SET(comps1, 0, sentinel_node);
    expr_ty cond1 = _PyAST_Compare(name_node, ops1, comps1, lineno, col, end_lineno, end_col, arena);
    
    // alias_name is not _LOH_SENTINEL
    expr_ty alias_node = _PyAST_Name(alias_id, Load, lineno, col, end_lineno, end_col, arena);
    expr_ty sentinel_node2 = _PyAST_Name(sentinel_id, Load, lineno, col, end_lineno, end_col, arena);
    asdl_int_seq *ops2 = _Py_asdl_int_seq_new(1, arena);
    asdl_seq_SET(ops2, 0, IsNot);
    asdl_expr_seq *comps2 = _Py_asdl_expr_seq_new(1, arena);
    asdl_seq_SET(comps2, 0, sentinel_node2);
    expr_ty cond2 = _PyAST_Compare(alias_node, ops2, comps2, lineno, col, end_lineno, end_col, arena);
    
    // cond1 and cond2
    asdl_expr_seq *bool_values = _Py_asdl_expr_seq_new(2, arena);
    asdl_seq_SET(bool_values, 0, cond1);
    asdl_seq_SET(bool_values, 1, cond2);
    expr_ty test = _PyAST_BoolOp(And, bool_values, lineno, col, end_lineno, end_col, arena);
    
    // Construct TypeError("...")
    char msg[512];
    snprintf(msg, sizeof(msg), "%s() got multiple values for alias parameter '%s'/'%s'",
             func_name ? func_name : "function", PyUnicode_AsUTF8(name_id), PyUnicode_AsUTF8(alias_id));
    PyObject *msg_obj = PyUnicode_FromString(msg);
    expr_ty msg_const = _PyAST_Constant(msg_obj, NULL, lineno, col, end_lineno, end_col, arena);
    expr_ty type_error_name = _PyAST_Name(_PyPegen_new_identifier(p, "TypeError"), Load, lineno, col, end_lineno, end_col, arena);
    asdl_expr_seq *call_args = _Py_asdl_expr_seq_new(1, arena);
    asdl_seq_SET(call_args, 0, msg_const);
    expr_ty call_expr = _PyAST_Call(type_error_name, call_args, NULL, lineno, col, end_lineno, end_col, arena);
    stmt_ty raise_stmt = _PyAST_Raise(call_expr, NULL, lineno, col, end_lineno, end_col, arena);
    
    asdl_stmt_seq *if_body = (asdl_stmt_seq *)_PyPegen_singleton_seq(p, raise_stmt);
    return _PyAST_If(test, if_body, NULL, lineno, col, end_lineno, end_col, arena);
}

static stmt_ty
make_alias_resolution(Parser *p, PyObject *name_id, PyObject *alias_id, PyObject *sentinel_id, expr_ty default_expr, const char *func_name, int lineno, int col, int end_lineno, int end_col)
{
    PyArena *arena = p->arena;
    
    // name = alias_name
    expr_ty name_store = _PyAST_Name(name_id, Store, lineno, col, end_lineno, end_col, arena);
    asdl_expr_seq *targets1 = (asdl_expr_seq *)_PyPegen_singleton_seq(p, name_store);
    expr_ty alias_load = _PyAST_Name(alias_id, Load, lineno, col, end_lineno, end_col, arena);
    stmt_ty assign_alias = _PyAST_Assign(targets1, alias_load, NULL, lineno, col, end_lineno, end_col, arena);
    
    asdl_stmt_seq *inner_if_body = (asdl_stmt_seq *)_PyPegen_singleton_seq(p, assign_alias);
    asdl_stmt_seq *inner_if_orelse = NULL;
    
    if (default_expr) {
        // name = default_expr
        expr_ty name_store2 = _PyAST_Name(name_id, Store, lineno, col, end_lineno, end_col, arena);
        asdl_expr_seq *targets2 = (asdl_expr_seq *)_PyPegen_singleton_seq(p, name_store2);
        stmt_ty assign_default = _PyAST_Assign(targets2, default_expr, NULL, lineno, col, end_lineno, end_col, arena);
        inner_if_orelse = (asdl_stmt_seq *)_PyPegen_singleton_seq(p, assign_default);
    } else {
        // raise TypeError("missing 1 required argument")
        char msg[512];
        snprintf(msg, sizeof(msg), "%s() missing 1 required argument: '%s' or '%s'",
                 func_name ? func_name : "function", PyUnicode_AsUTF8(name_id), PyUnicode_AsUTF8(alias_id));
        PyObject *msg_obj = PyUnicode_FromString(msg);
        expr_ty msg_const = _PyAST_Constant(msg_obj, NULL, lineno, col, end_lineno, end_col, arena);
        expr_ty type_error_name = _PyAST_Name(_PyPegen_new_identifier(p, "TypeError"), Load, lineno, col, end_lineno, end_col, arena);
        asdl_expr_seq *call_args = _Py_asdl_expr_seq_new(1, arena);
        asdl_seq_SET(call_args, 0, msg_const);
        expr_ty call_expr = _PyAST_Call(type_error_name, call_args, NULL, lineno, col, end_lineno, end_col, arena);
        stmt_ty raise_stmt = _PyAST_Raise(call_expr, NULL, lineno, col, end_lineno, end_col, arena);
        inner_if_orelse = (asdl_stmt_seq *)_PyPegen_singleton_seq(p, raise_stmt);
    }
    
    // alias_name is not _LOH_SENTINEL
    expr_ty alias_node = _PyAST_Name(alias_id, Load, lineno, col, end_lineno, end_col, arena);
    expr_ty sentinel_node = _PyAST_Name(sentinel_id, Load, lineno, col, end_lineno, end_col, arena);
    asdl_int_seq *ops1 = _Py_asdl_int_seq_new(1, arena);
    asdl_seq_SET(ops1, 0, IsNot);
    asdl_expr_seq *comps1 = _Py_asdl_expr_seq_new(1, arena);
    asdl_seq_SET(comps1, 0, sentinel_node);
    expr_ty inner_cond = _PyAST_Compare(alias_node, ops1, comps1, lineno, col, end_lineno, end_col, arena);
    
    stmt_ty inner_if = _PyAST_If(inner_cond, inner_if_body, inner_if_orelse, lineno, col, end_lineno, end_col, arena);
    
    // name is _LOH_SENTINEL
    expr_ty name_node = _PyAST_Name(name_id, Load, lineno, col, end_lineno, end_col, arena);
    expr_ty sentinel_node2 = _PyAST_Name(sentinel_id, Load, lineno, col, end_lineno, end_col, arena);
    asdl_int_seq *ops2 = _Py_asdl_int_seq_new(1, arena);
    asdl_seq_SET(ops2, 0, Is);
    asdl_expr_seq *comps2 = _Py_asdl_expr_seq_new(1, arena);
    asdl_seq_SET(comps2, 0, sentinel_node2);
    expr_ty outer_cond = _PyAST_Compare(name_node, ops2, comps2, lineno, col, end_lineno, end_col, arena);
    
    asdl_stmt_seq *outer_if_body = (asdl_stmt_seq *)_PyPegen_singleton_seq(p, inner_if);
    return _PyAST_If(outer_cond, outer_if_body, NULL, lineno, col, end_lineno, end_col, arena);
}

static stmt_ty
make_alias_delete(Parser *p, PyObject *alias_id, int lineno, int col, int end_lineno, int end_col)
{
    PyArena *arena = p->arena;
    expr_ty alias_node = _PyAST_Name(alias_id, Del, lineno, col, end_lineno, end_col, arena);
    asdl_expr_seq *targets = (asdl_expr_seq *)_PyPegen_singleton_seq(p, alias_node);
    return _PyAST_Delete(targets, lineno, col, end_lineno, end_col, arena);
}

static asdl_stmt_seq *
desugar_parameter_aliases(Parser *p, PyObject *func_name, arguments_ty args, asdl_stmt_seq *body)
{
    if (!args) return body;
    
    PyObject *sentinel_id = _PyPegen_new_identifier(p, "_LOH_SENTINEL");
    if (!sentinel_id) return body;
    
    const char *func_name_str = func_name ? PyUnicode_AsUTF8(func_name) : "function";

    // 1. Process positional-only and positional-or-keyword arguments
    int posonly_len = args->posonlyargs ? asdl_seq_LEN(args->posonlyargs) : 0;
    int args_len = args->args ? asdl_seq_LEN(args->args) : 0;
    int total_pos = posonly_len + args_len;
    
    if (total_pos > 0) {
        int M = args->defaults ? asdl_seq_LEN(args->defaults) : 0;
        
        // Temporarily store original defaults and new defaults
        expr_ty *orig_defaults = PyMem_Malloc(total_pos * sizeof(expr_ty));
        expr_ty *new_defaults = PyMem_Malloc(total_pos * sizeof(expr_ty));
        if (!orig_defaults || !new_defaults) {
            if (orig_defaults) PyMem_Free(orig_defaults);
            if (new_defaults) PyMem_Free(new_defaults);
            return NULL;
        }
        for (int i = 0; i < total_pos; i++) {
            orig_defaults[i] = (i >= total_pos - M) ? asdl_seq_GET(args->defaults, i - (total_pos - M)) : NULL;
            new_defaults[i] = orig_defaults[i];
        }
        
        // Loop in reverse to prepend resolution logic in the correct execution order
        for (int i = total_pos - 1; i >= 0; i--) {
            arg_ty arg;
            if (i < posonly_len) {
                arg = asdl_seq_GET(args->posonlyargs, i);
            } else {
                arg = asdl_seq_GET(args->args, i - posonly_len);
            }
            
            const char *name_str = PyUnicode_AsUTF8(arg->arg);
            if (name_str && strchr(name_str, '|')) {
                char buf[512];
                strncpy(buf, name_str, sizeof(buf));
                buf[sizeof(buf) - 1] = '\0';
                char *pipe = strchr(buf, '|');
                *pipe = '\0';
                
                PyObject *primary_id = _PyPegen_new_identifier(p, buf);
                PyObject *alias_id = _PyPegen_new_identifier(p, pipe + 1);
                if (!primary_id || !alias_id) {
                    PyMem_Free(orig_defaults);
                    PyMem_Free(new_defaults);
                    return NULL;
                }
                
                int lineno = arg->lineno;
                int col = arg->col_offset;
                int end_lineno = arg->end_lineno;
                int end_col = arg->end_col_offset;
                PyArena *arena = p->arena;
                
                // Save original default value
                expr_ty default_expr = orig_defaults[i];
                
                // Update primary argument name
                arg->arg = primary_id;
                
                // Update default value for primary in signature to _LOH_SENTINEL
                expr_ty sentinel_node = _PyAST_Name(sentinel_id, Load, lineno, col, end_lineno, end_col, arena);
                new_defaults[i] = sentinel_node;
                
                // Append alias argument as keyword-only parameter
                arg_ty alias_arg = _PyAST_arg(alias_id, NULL, NULL, lineno, col, end_lineno, end_col, arena);
                args->kwonlyargs = append_arg_seq(p, args->kwonlyargs, alias_arg);
                expr_ty sentinel_node_kw = _PyAST_Name(sentinel_id, Load, lineno, col, end_lineno, end_col, arena);
                args->kw_defaults = append_expr_seq(p, args->kw_defaults, sentinel_node_kw);
                
                if (!args->kwonlyargs || !args->kw_defaults) {
                    PyMem_Free(orig_defaults);
                    PyMem_Free(new_defaults);
                    return NULL;
                }
                
                // Prepend resolution logic to body
                stmt_ty del_stmt = make_alias_delete(p, alias_id, lineno, col, end_lineno, end_col);
                stmt_ty res_stmt = make_alias_resolution(p, primary_id, alias_id, sentinel_id, default_expr, func_name_str, lineno, col, end_lineno, end_col);
                stmt_ty err_stmt = make_alias_error_check(p, primary_id, alias_id, sentinel_id, func_name_str, lineno, col, end_lineno, end_col);
                
                body = (asdl_stmt_seq *)_PyPegen_seq_insert_in_front(p, del_stmt, (asdl_seq *)body);
                body = (asdl_stmt_seq *)_PyPegen_seq_insert_in_front(p, res_stmt, (asdl_seq *)body);
                body = (asdl_stmt_seq *)_PyPegen_seq_insert_in_front(p, err_stmt, (asdl_seq *)body);
                
                if (!body) {
                    PyMem_Free(orig_defaults);
                    PyMem_Free(new_defaults);
                    return NULL;
                }
            }
        }
        
        // Reconstruct args->defaults
        int first_default_idx = -1;
        for (int i = 0; i < total_pos; i++) {
            if (new_defaults[i] != NULL) {
                first_default_idx = i;
                break;
            }
        }
        if (first_default_idx != -1) {
            int new_defaults_len = total_pos - first_default_idx;
            asdl_expr_seq *new_defaults_seq = _Py_asdl_expr_seq_new(new_defaults_len, p->arena);
            if (!new_defaults_seq) {
                PyMem_Free(orig_defaults);
                PyMem_Free(new_defaults);
                return NULL;
            }
            for (int i = 0; i < new_defaults_len; i++) {
                asdl_seq_SET(new_defaults_seq, i, new_defaults[first_default_idx + i]);
            }
            args->defaults = new_defaults_seq;
        } else {
            args->defaults = NULL;
        }
        
        PyMem_Free(orig_defaults);
        PyMem_Free(new_defaults);
    }
    
    // 2. Process keyword-only arguments (args->kwonlyargs)
    if (args->kwonlyargs) {
        int kw_len = asdl_seq_LEN(args->kwonlyargs);
        // Loop in reverse
        for (int i = kw_len - 1; i >= 0; i--) {
            arg_ty arg = asdl_seq_GET(args->kwonlyargs, i);
            const char *name_str = PyUnicode_AsUTF8(arg->arg);
            if (name_str && strchr(name_str, '|')) {
                char buf[512];
                strncpy(buf, name_str, sizeof(buf));
                buf[sizeof(buf) - 1] = '\0';
                char *pipe = strchr(buf, '|');
                *pipe = '\0';
                
                PyObject *primary_id = _PyPegen_new_identifier(p, buf);
                PyObject *alias_id = _PyPegen_new_identifier(p, pipe + 1);
                if (!primary_id || !alias_id) {
                    return NULL;
                }
                
                int lineno = arg->lineno;
                int col = arg->col_offset;
                int end_lineno = arg->end_lineno;
                int end_col = arg->end_col_offset;
                PyArena *arena = p->arena;
                
                // Save original default value
                expr_ty default_expr = (args->kw_defaults && i < asdl_seq_LEN(args->kw_defaults)) ? asdl_seq_GET(args->kw_defaults, i) : NULL;
                
                // Update primary argument name
                arg->arg = primary_id;
                
                // Update default value for primary in signature to _LOH_SENTINEL
                expr_ty sentinel_node = _PyAST_Name(sentinel_id, Load, lineno, col, end_lineno, end_col, arena);
                if (args->kw_defaults && i < asdl_seq_LEN(args->kw_defaults)) {
                    asdl_seq_SET(args->kw_defaults, i, sentinel_node);
                }
                
                // Append alias argument as keyword-only parameter
                arg_ty alias_arg = _PyAST_arg(alias_id, NULL, NULL, lineno, col, end_lineno, end_col, arena);
                args->kwonlyargs = append_arg_seq(p, args->kwonlyargs, alias_arg);
                expr_ty sentinel_node_kw = _PyAST_Name(sentinel_id, Load, lineno, col, end_lineno, end_col, arena);
                args->kw_defaults = append_expr_seq(p, args->kw_defaults, sentinel_node_kw);
                
                if (!args->kwonlyargs || !args->kw_defaults) {
                    return NULL;
                }
                
                // Prepend resolution logic to body
                stmt_ty del_stmt = make_alias_delete(p, alias_id, lineno, col, end_lineno, end_col);
                stmt_ty res_stmt = make_alias_resolution(p, primary_id, alias_id, sentinel_id, default_expr, func_name_str, lineno, col, end_lineno, end_col);
                stmt_ty err_stmt = make_alias_error_check(p, primary_id, alias_id, sentinel_id, func_name_str, lineno, col, end_lineno, end_col);
                
                body = (asdl_stmt_seq *)_PyPegen_seq_insert_in_front(p, del_stmt, (asdl_seq *)body);
                body = (asdl_stmt_seq *)_PyPegen_seq_insert_in_front(p, res_stmt, (asdl_seq *)body);
                body = (asdl_stmt_seq *)_PyPegen_seq_insert_in_front(p, err_stmt, (asdl_seq *)body);
                
                if (!body) {
                    return NULL;
                }
            }
        }
    }
    
    return body;
}

asdl_stmt_seq *
_PyPegen_desugar_parameter_properties(Parser *p, PyObject *func_name, arguments_ty args, asdl_stmt_seq *body)
{
    body = _PyPegen_desugar_lazy_defaults(p, args, body);
    if (!body) return NULL;

    body = desugar_parameter_aliases(p, func_name, args, body);
    if (!body) return NULL;
    
    if (args) {
        body = desugar_arg_sequence(p, args->kwonlyargs, body);
        if (!body) return NULL;
        body = desugar_arg_sequence(p, args->args, body);
        if (!body) return NULL;
        body = desugar_arg_sequence(p, args->posonlyargs, body);
        if (!body) return NULL;
    }
    return body;
}

expr_ty
_PyPegen_replace_dot_in_expr(Parser *p, expr_ty expr, PyObject *inst_id) {
    if (!expr) {
        return NULL;
    }
    switch (expr->kind) {
        case Name_kind: {
            const char *name_str = PyUnicode_AsUTF8(expr->v.Name.id);
            if (name_str && strcmp(name_str, ".") == 0) {
                return _PyAST_Name(inst_id, expr->v.Name.ctx, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
            }
            return expr;
        }
        case Attribute_kind: {
            expr_ty new_val = _PyPegen_replace_dot_in_expr(p, expr->v.Attribute.value, inst_id);
            if (!new_val) return NULL;
            return _PyAST_Attribute(new_val, expr->v.Attribute.attr, expr->v.Attribute.ctx, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case Call_kind: {
            expr_ty new_func = _PyPegen_replace_dot_in_expr(p, expr->v.Call.func, inst_id);
            if (!new_func) return NULL;
            asdl_expr_seq *new_args = NULL;
            if (expr->v.Call.args) {
                int len = asdl_seq_LEN(expr->v.Call.args);
                new_args = _Py_asdl_expr_seq_new(len, p->arena);
                if (!new_args) return NULL;
                for (int i = 0; i < len; i++) {
                    expr_ty arg = asdl_seq_GET(expr->v.Call.args, i);
                    expr_ty new_arg = _PyPegen_replace_dot_in_expr(p, arg, inst_id);
                    if (!new_arg) return NULL;
                    asdl_seq_SET(new_args, i, new_arg);
                }
            }
            asdl_keyword_seq *new_keywords = NULL;
            if (expr->v.Call.keywords) {
                int len = asdl_seq_LEN(expr->v.Call.keywords);
                new_keywords = _Py_asdl_keyword_seq_new(len, p->arena);
                if (!new_keywords) return NULL;
                for (int i = 0; i < len; i++) {
                    keyword_ty kw = asdl_seq_GET(expr->v.Call.keywords, i);
                    expr_ty new_val = _PyPegen_replace_dot_in_expr(p, kw->value, inst_id);
                    if (!new_val) return NULL;
                    keyword_ty new_kw = _PyAST_keyword(kw->arg, new_val, kw->lineno, kw->col_offset, kw->end_lineno, kw->end_col_offset, p->arena);
                    if (!new_kw) return NULL;
                    asdl_seq_SET(new_keywords, i, new_kw);
                }
            }
            return _PyAST_Call(new_func, new_args, new_keywords, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case BinOp_kind: {
            expr_ty left = _PyPegen_replace_dot_in_expr(p, expr->v.BinOp.left, inst_id);
            expr_ty right = _PyPegen_replace_dot_in_expr(p, expr->v.BinOp.right, inst_id);
            if (!left || !right) return NULL;
            return _PyAST_BinOp(left, expr->v.BinOp.op, right, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case UnaryOp_kind: {
            expr_ty operand = _PyPegen_replace_dot_in_expr(p, expr->v.UnaryOp.operand, inst_id);
            if (!operand) return NULL;
            return _PyAST_UnaryOp(expr->v.UnaryOp.op, operand, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case Compare_kind: {
            expr_ty left = _PyPegen_replace_dot_in_expr(p, expr->v.Compare.left, inst_id);
            if (!left) return NULL;
            asdl_expr_seq *new_comparators = NULL;
            if (expr->v.Compare.comparators) {
                int len = asdl_seq_LEN(expr->v.Compare.comparators);
                new_comparators = _Py_asdl_expr_seq_new(len, p->arena);
                if (!new_comparators) return NULL;
                for (int i = 0; i < len; i++) {
                    expr_ty comp = asdl_seq_GET(expr->v.Compare.comparators, i);
                    expr_ty new_comp = _PyPegen_replace_dot_in_expr(p, comp, inst_id);
                    if (!new_comp) return NULL;
                    asdl_seq_SET(new_comparators, i, new_comp);
                }
            }
            return _PyAST_Compare(left, expr->v.Compare.ops, new_comparators, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case Subscript_kind: {
            expr_ty value = _PyPegen_replace_dot_in_expr(p, expr->v.Subscript.value, inst_id);
            expr_ty slice = _PyPegen_replace_dot_in_expr(p, expr->v.Subscript.slice, inst_id);
            if (!value || !slice) return NULL;
            return _PyAST_Subscript(value, slice, expr->v.Subscript.ctx, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case Slice_kind: {
            expr_ty lower = _PyPegen_replace_dot_in_expr(p, expr->v.Slice.lower, inst_id);
            expr_ty upper = _PyPegen_replace_dot_in_expr(p, expr->v.Slice.upper, inst_id);
            expr_ty step = _PyPegen_replace_dot_in_expr(p, expr->v.Slice.step, inst_id);
            return _PyAST_Slice(lower, upper, step, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case IfExp_kind: {
            expr_ty test = _PyPegen_replace_dot_in_expr(p, expr->v.IfExp.test, inst_id);
            expr_ty body = _PyPegen_replace_dot_in_expr(p, expr->v.IfExp.body, inst_id);
            expr_ty orelse = _PyPegen_replace_dot_in_expr(p, expr->v.IfExp.orelse, inst_id);
            if (!test || !body || !orelse) return NULL;
            return _PyAST_IfExp(test, body, orelse, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case List_kind: {
            asdl_expr_seq *new_elts = NULL;
            if (expr->v.List.elts) {
                int len = asdl_seq_LEN(expr->v.List.elts);
                new_elts = _Py_asdl_expr_seq_new(len, p->arena);
                if (!new_elts) return NULL;
                for (int i = 0; i < len; i++) {
                    expr_ty elt = asdl_seq_GET(expr->v.List.elts, i);
                    expr_ty new_elt = _PyPegen_replace_dot_in_expr(p, elt, inst_id);
                    if (!new_elt) return NULL;
                    asdl_seq_SET(new_elts, i, new_elt);
                }
            }
            return _PyAST_List(new_elts, expr->v.List.ctx, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case Tuple_kind: {
            asdl_expr_seq *new_elts = NULL;
            if (expr->v.Tuple.elts) {
                int len = asdl_seq_LEN(expr->v.Tuple.elts);
                new_elts = _Py_asdl_expr_seq_new(len, p->arena);
                if (!new_elts) return NULL;
                for (int i = 0; i < len; i++) {
                    expr_ty elt = asdl_seq_GET(expr->v.Tuple.elts, i);
                    expr_ty new_elt = _PyPegen_replace_dot_in_expr(p, elt, inst_id);
                    if (!new_elt) return NULL;
                    asdl_seq_SET(new_elts, i, new_elt);
                }
            }
            return _PyAST_Tuple(new_elts, expr->v.Tuple.ctx, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case Dict_kind: {
            asdl_expr_seq *new_keys = NULL;
            if (expr->v.Dict.keys) {
                int len = asdl_seq_LEN(expr->v.Dict.keys);
                new_keys = _Py_asdl_expr_seq_new(len, p->arena);
                if (!new_keys) return NULL;
                for (int i = 0; i < len; i++) {
                    expr_ty key = asdl_seq_GET(expr->v.Dict.keys, i);
                    if (key) {
                        expr_ty new_key = _PyPegen_replace_dot_in_expr(p, key, inst_id);
                        if (!new_key) return NULL;
                        asdl_seq_SET(new_keys, i, new_key);
                    } else {
                        asdl_seq_SET(new_keys, i, NULL);
                    }
                }
            }
            asdl_expr_seq *new_values = NULL;
            if (expr->v.Dict.values) {
                int len = asdl_seq_LEN(expr->v.Dict.values);
                new_values = _Py_asdl_expr_seq_new(len, p->arena);
                if (!new_values) return NULL;
                for (int i = 0; i < len; i++) {
                    expr_ty val = asdl_seq_GET(expr->v.Dict.values, i);
                    expr_ty new_val = _PyPegen_replace_dot_in_expr(p, val, inst_id);
                    if (!new_val) return NULL;
                    asdl_seq_SET(new_values, i, new_val);
                }
            }
            return _PyAST_Dict(new_keys, new_values, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case Set_kind: {
            asdl_expr_seq *new_elts = NULL;
            if (expr->v.Set.elts) {
                int len = asdl_seq_LEN(expr->v.Set.elts);
                new_elts = _Py_asdl_expr_seq_new(len, p->arena);
                if (!new_elts) return NULL;
                for (int i = 0; i < len; i++) {
                    expr_ty elt = asdl_seq_GET(expr->v.Set.elts, i);
                    expr_ty new_elt = _PyPegen_replace_dot_in_expr(p, elt, inst_id);
                    if (!new_elt) return NULL;
                    asdl_seq_SET(new_elts, i, new_elt);
                }
            }
            return _PyAST_Set(new_elts, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case Starred_kind: {
            expr_ty value = _PyPegen_replace_dot_in_expr(p, expr->v.Starred.value, inst_id);
            if (!value) return NULL;
            return _PyAST_Starred(value, expr->v.Starred.ctx, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case NamedExpr_kind: {
            expr_ty target = _PyPegen_replace_dot_in_expr(p, expr->v.NamedExpr.target, inst_id);
            expr_ty value = _PyPegen_replace_dot_in_expr(p, expr->v.NamedExpr.value, inst_id);
            if (!target || !value) return NULL;
            return _PyAST_NamedExpr(target, value, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
        }
        case Constant_kind:
            return expr;
        default:
            return expr;
    }
}

expr_ty
_PyPegen_make_initializer_block(Parser *p, expr_ty primary, asdl_stmt_seq *block_stmts) {
    static unsigned int inst_counter = 0;
    char name_buf[64];
    snprintf(name_buf, sizeof(name_buf), "_loh_inst_%u", inst_counter++);
    PyObject *inst_id = _PyPegen_new_identifier(p, name_buf);
    if (!inst_id) return NULL;

    int block_len = block_stmts ? asdl_seq_LEN(block_stmts) : 0;
    int elts_len = block_len + 2;
    asdl_expr_seq *elts = _Py_asdl_expr_seq_new(elts_len, p->arena);
    if (!elts) return NULL;

    // Element 0: (_loh_inst := primary)
    expr_ty inst_target = _PyAST_Name(inst_id, Store, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!inst_target) return NULL;
    expr_ty named_expr = _PyAST_NamedExpr(inst_target, primary, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!named_expr) return NULL;
    asdl_seq_SET(elts, 0, named_expr);

    // Elements 1..block_len: rewritten statements
    for (int i = 0; i < block_len; i++) {
        stmt_ty stmt = asdl_seq_GET(block_stmts, i);
        expr_ty expr_val = NULL;
        switch (stmt->kind) {
            case Assign_kind: {
                asdl_expr_seq *targets = stmt->v.Assign.targets;
                if (asdl_seq_LEN(targets) > 0) {
                    expr_ty target = asdl_seq_GET(targets, 0);
                    if (target->kind == Attribute_kind) {
                        expr_ty receiver = target->v.Attribute.value;
                        if (receiver->kind == Name_kind && strcmp(PyUnicode_AsUTF8(receiver->v.Name.id), ".") == 0) {
                            // Convert to setattr(_loh_inst, "attr", value)
                            PyObject *setattr_id = _PyPegen_new_identifier(p, "setattr");
                            expr_ty setattr_func = _PyAST_Name(setattr_id, Load, target->lineno, target->col_offset, target->end_lineno, target->end_col_offset, p->arena);
                            
                            expr_ty inst_arg = _PyAST_Name(inst_id, Load, target->lineno, target->col_offset, target->end_lineno, target->end_col_offset, p->arena);
                            expr_ty attr_const = _PyAST_Constant(target->v.Attribute.attr, NULL, target->lineno, target->col_offset, target->end_lineno, target->end_col_offset, p->arena);
                            
                            expr_ty stmt_val = _PyPegen_replace_dot_in_expr(p, stmt->v.Assign.value, inst_id);
                            
                            asdl_expr_seq *args = _Py_asdl_expr_seq_new(3, p->arena);
                            asdl_seq_SET(args, 0, inst_arg);
                            asdl_seq_SET(args, 1, attr_const);
                            asdl_seq_SET(args, 2, stmt_val);
                            
                            expr_val = _PyAST_Call(setattr_func, args, NULL, stmt->lineno, stmt->col_offset, stmt->end_lineno, stmt->end_col_offset, p->arena);
                        }
                    }
                }
                break;
            }
            case AugAssign_kind: {
                expr_ty target = stmt->v.AugAssign.target;
                if (target->kind == Attribute_kind) {
                    expr_ty receiver = target->v.Attribute.value;
                    if (receiver->kind == Name_kind && strcmp(PyUnicode_AsUTF8(receiver->v.Name.id), ".") == 0) {
                        // Convert to setattr(_loh_inst, "attr", getattr(_loh_inst, "attr") op value)
                        PyObject *setattr_id = _PyPegen_new_identifier(p, "setattr");
                        expr_ty setattr_func = _PyAST_Name(setattr_id, Load, target->lineno, target->col_offset, target->end_lineno, target->end_col_offset, p->arena);
                        
                        expr_ty inst_arg = _PyAST_Name(inst_id, Load, target->lineno, target->col_offset, target->end_lineno, target->end_col_offset, p->arena);
                        expr_ty attr_const = _PyAST_Constant(target->v.Attribute.attr, NULL, target->lineno, target->col_offset, target->end_lineno, target->end_col_offset, p->arena);
                        
                        expr_ty current_val = _PyAST_Attribute(inst_arg, target->v.Attribute.attr, Load, target->lineno, target->col_offset, target->end_lineno, target->end_col_offset, p->arena);
                        expr_ty right_val = _PyPegen_replace_dot_in_expr(p, stmt->v.AugAssign.value, inst_id);
                        expr_ty new_val = _PyAST_BinOp(current_val, stmt->v.AugAssign.op, right_val, stmt->lineno, stmt->col_offset, stmt->end_lineno, stmt->end_col_offset, p->arena);
                        
                        asdl_expr_seq *args = _Py_asdl_expr_seq_new(3, p->arena);
                        asdl_seq_SET(args, 0, inst_arg);
                        asdl_seq_SET(args, 1, attr_const);
                        asdl_seq_SET(args, 2, new_val);
                        
                        expr_val = _PyAST_Call(setattr_func, args, NULL, stmt->lineno, stmt->col_offset, stmt->end_lineno, stmt->end_col_offset, p->arena);
                    }
                }
                break;
            }
            case Delete_kind: {
                asdl_expr_seq *targets = stmt->v.Delete.targets;
                if (asdl_seq_LEN(targets) > 0) {
                    expr_ty target = asdl_seq_GET(targets, 0);
                    if (target->kind == Attribute_kind) {
                        expr_ty receiver = target->v.Attribute.value;
                        if (receiver->kind == Name_kind && strcmp(PyUnicode_AsUTF8(receiver->v.Name.id), ".") == 0) {
                            // Convert to delattr(_loh_inst, "attr")
                            PyObject *delattr_id = _PyPegen_new_identifier(p, "delattr");
                            expr_ty delattr_func = _PyAST_Name(delattr_id, Load, target->lineno, target->col_offset, target->end_lineno, target->end_col_offset, p->arena);
                            
                            expr_ty inst_arg = _PyAST_Name(inst_id, Load, target->lineno, target->col_offset, target->end_lineno, target->end_col_offset, p->arena);
                            expr_ty attr_const = _PyAST_Constant(target->v.Attribute.attr, NULL, target->lineno, target->col_offset, target->end_lineno, target->end_col_offset, p->arena);
                            
                            asdl_expr_seq *args = _Py_asdl_expr_seq_new(2, p->arena);
                            asdl_seq_SET(args, 0, inst_arg);
                            asdl_seq_SET(args, 1, attr_const);
                            
                            expr_val = _PyAST_Call(delattr_func, args, NULL, stmt->lineno, stmt->col_offset, stmt->end_lineno, stmt->end_col_offset, p->arena);
                        }
                    }
                }
                break;
            }
            case Expr_kind: {
                expr_val = _PyPegen_replace_dot_in_expr(p, stmt->v.Expr.value, inst_id);
                break;
            }
            case Pass_kind: {
                expr_val = _PyAST_Constant(Py_None, NULL, stmt->lineno, stmt->col_offset, stmt->end_lineno, stmt->end_col_offset, p->arena);
                break;
            }
            default:
                break;
        }
        if (!expr_val) {
            expr_val = _PyAST_Constant(Py_None, NULL, stmt->lineno, stmt->col_offset, stmt->end_lineno, stmt->end_col_offset, p->arena);
        }
        asdl_seq_SET(elts, i + 1, expr_val);
    }

    // Element elts_len - 1: _loh_inst
    expr_ty final_inst = _PyAST_Name(inst_id, Load, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!final_inst) return NULL;
    asdl_seq_SET(elts, elts_len - 1, final_inst);

    expr_ty list_expr = _PyAST_List(elts, Load, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    if (!list_expr) return NULL;

    expr_ty index_const = _PyAST_Constant(PyLong_FromLong(0), NULL, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
    _PyArena_AddPyObject(p->arena, index_const->v.Constant.value);
    
    return _PyAST_Subscript(list_expr, index_const, Load, primary->lineno, primary->col_offset, primary->end_lineno, primary->end_col_offset, p->arena);
}

int
_PyPegen_lookahead_for_colon(Parser *p)
{
    int mark = p->mark;
    while (1) {
        if (mark == p->fill) {
            if (_PyPegen_fill_token(p) < 0) {
                return 0;
            }
        }
        Token *t = p->tokens[mark];
        if (t == NULL) {
            break;
        }
        if (t->type == ENDMARKER || t->type == NEWLINE || t->type == SEMI) {
            break;
        }
        if (t->type == COLON) {
            return 1;
        }
        mark++;
    }
    return 0;
}

static int
is_safe_navigation_expr(expr_ty e) {
    if (!e || e->kind != IfExp_kind) {
        return 0;
    }
    expr_ty test = e->v.IfExp.test;
    expr_ty orelse = e->v.IfExp.orelse;
    if (!test || !orelse) {
        return 0;
    }
    if (orelse->kind != Constant_kind || orelse->v.Constant.value != Py_None) {
        return 0;
    }
    if (test->kind != Compare_kind) {
        return 0;
    }
    if (asdl_seq_LEN(test->v.Compare.ops) != 1 || asdl_seq_LEN(test->v.Compare.comparators) != 1) {
        return 0;
    }
    int op = asdl_seq_GET(test->v.Compare.ops, 0);
    if (op != IsNot) {
        return 0;
    }
    expr_ty left = test->v.Compare.left;
    if (left->kind != NamedExpr_kind) {
        return 0;
    }
    expr_ty target = left->v.NamedExpr.target;
    if (target->kind != Name_kind) {
        return 0;
    }
    if (!_PyUnicode_EqualToASCIIString(target->v.Name.id, "_loh_safe_val")) {
        return 0;
    }
    return 1;
}

stmt_ty
_PyPegen_make_assign(Parser *p, asdl_expr_seq *targets, expr_ty value, Token *tc) {
    int has_safe = 0;
    for (int i = 0; i < asdl_seq_LEN(targets); i++) {
        expr_ty t = asdl_seq_GET(targets, i);
        if (is_safe_navigation_expr(t)) {
            has_safe = 1;
            break;
        }
    }
    
    expr_ty first_target = asdl_seq_GET(targets, 0);
    int lineno = first_target->lineno;
    int col_offset = first_target->col_offset;
    int end_lineno = value->end_lineno;
    int end_col_offset = value->end_col_offset;
    
    if (!has_safe) {
        return _PyAST_Assign(targets, value, tc ? NEW_TYPE_COMMENT(p, tc) : NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
    }
    
    if (asdl_seq_LEN(targets) == 1) {
        expr_ty t = asdl_seq_GET(targets, 0);
        if (is_safe_navigation_expr(t)) {
            expr_ty test = t->v.IfExp.test;
            expr_ty body = t->v.IfExp.body;
            if (body->kind == Attribute_kind) {
                body->v.Attribute.ctx = Store;
            } else if (body->kind == Subscript_kind) {
                body->v.Subscript.ctx = Store;
            }
            asdl_expr_seq *b_targets = (asdl_expr_seq *)_PyPegen_singleton_seq(p, body);
            stmt_ty assign = _PyAST_Assign(b_targets, value, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
            if (!assign) return NULL;
            
            asdl_stmt_seq *if_body = _Py_asdl_stmt_seq_new(1, p->arena);
            if (!if_body) return NULL;
            asdl_seq_SET(if_body, 0, assign);
            return _PyAST_If(test, if_body, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
        }
    }
    
    PyObject *tmp_id = _PyPegen_new_identifier(p, "_loh_assign_tmp");
    if (!tmp_id) return NULL;
    
    expr_ty tmp_store = _PyAST_Name(tmp_id, Store, lineno, col_offset, end_lineno, end_col_offset, p->arena);
    if (!tmp_store) return NULL;
    asdl_expr_seq *tmp_targets = (asdl_expr_seq *)_PyPegen_singleton_seq(p, tmp_store);
    if (!tmp_targets) return NULL;
    stmt_ty init_tmp = _PyAST_Assign(tmp_targets, value, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
    if (!init_tmp) return NULL;
    
    asdl_stmt_seq *stmts = _Py_asdl_stmt_seq_new(asdl_seq_LEN(targets) + 1, p->arena);
    if (!stmts) return NULL;
    asdl_seq_SET(stmts, 0, init_tmp);
    
    expr_ty tmp_load = _PyAST_Name(tmp_id, Load, lineno, col_offset, end_lineno, end_col_offset, p->arena);
    if (!tmp_load) return NULL;
    
    for (int i = 0; i < asdl_seq_LEN(targets); i++) {
        expr_ty t = asdl_seq_GET(targets, i);
        if (is_safe_navigation_expr(t)) {
            expr_ty test = t->v.IfExp.test;
            expr_ty body = t->v.IfExp.body;
            if (body->kind == Attribute_kind) {
                body->v.Attribute.ctx = Store;
            } else if (body->kind == Subscript_kind) {
                body->v.Subscript.ctx = Store;
            }
            asdl_expr_seq *b_targets = (asdl_expr_seq *)_PyPegen_singleton_seq(p, body);
            if (!b_targets) return NULL;
            stmt_ty assign = _PyAST_Assign(b_targets, tmp_load, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
            if (!assign) return NULL;
            
            asdl_stmt_seq *if_body = _Py_asdl_stmt_seq_new(1, p->arena);
            if (!if_body) return NULL;
            asdl_seq_SET(if_body, 0, assign);
            stmt_ty if_stmt = _PyAST_If(test, if_body, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
            if (!if_stmt) return NULL;
            asdl_seq_SET(stmts, i + 1, if_stmt);
        } else {
            asdl_expr_seq *b_targets = (asdl_expr_seq *)_PyPegen_singleton_seq(p, t);
            if (!b_targets) return NULL;
            stmt_ty assign = _PyAST_Assign(b_targets, tmp_load, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
            if (!assign) return NULL;
            asdl_seq_SET(stmts, i + 1, assign);
        }
    }
    
    expr_ty one_const = _PyAST_Constant(Py_True, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
    if (!one_const) return NULL;
    return _PyAST_If(one_const, stmts, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
}

stmt_ty
_PyPegen_make_augassign(Parser *p, expr_ty target, operator_ty op, expr_ty value) {
    int lineno = target->lineno;
    int col_offset = target->col_offset;
    int end_lineno = value->end_lineno;
    int end_col_offset = value->end_col_offset;
    
    if (is_safe_navigation_expr(target)) {
        expr_ty test = target->v.IfExp.test;
        expr_ty body = target->v.IfExp.body;
        if (body->kind == Attribute_kind) {
            body->v.Attribute.ctx = Store;
        } else if (body->kind == Subscript_kind) {
            body->v.Subscript.ctx = Store;
        }
        stmt_ty augassign = _PyAST_AugAssign(body, op, value, lineno, col_offset, end_lineno, end_col_offset, p->arena);
        if (!augassign) return NULL;
        
        asdl_stmt_seq *if_body = _Py_asdl_stmt_seq_new(1, p->arena);
        if (!if_body) return NULL;
        asdl_seq_SET(if_body, 0, augassign);
        return _PyAST_If(test, if_body, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
    }
    return _PyAST_AugAssign(target, op, value, lineno, col_offset, end_lineno, end_col_offset, p->arena);
}

expr_ty
_PyPegen_rescue_expr(Parser *p, expr_ty expr, expr_ty exc_type, expr_ty exc_name, expr_ty fallback) {
    PyObject *rescue_id = _PyPegen_new_identifier(p, "_loh_rescue");
    if (!rescue_id) return NULL;
    
    expr_ty func = _PyAST_Name(rescue_id, Load, expr->lineno, expr->col_offset, fallback->end_lineno, fallback->end_col_offset, p->arena);
    if (!func) return NULL;
    
    // Construct body_lambda: lambda: expr
    arguments_ty body_args = _PyPegen_empty_arguments(p);
    if (!body_args) return NULL;
    expr_ty body_lambda = _PyAST_Lambda(body_args, expr, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
    if (!body_lambda) return NULL;
    
    // Construct exc_val
    expr_ty exc_val;
    if (exc_type == NULL) {
        PyObject *exc_id = _PyPegen_new_identifier(p, "Exception");
        if (!exc_id) return NULL;
        exc_val = _PyAST_Name(exc_id, Load, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
    } else {
        exc_val = exc_type;
    }
    if (!exc_val) return NULL;
    
    // Construct fallback_lambda: lambda exc_name: fallback
    PyObject *exc_name_id = NULL;
    if (exc_name != NULL) {
        exc_name_id = exc_name->v.Name.id;
    } else {
        exc_name_id = _PyPegen_new_identifier(p, "_");
        if (!exc_name_id) return NULL;
    }
    arg_ty arg = _PyAST_arg(exc_name_id, NULL, NULL, fallback->lineno, fallback->col_offset, fallback->end_lineno, fallback->end_col_offset, p->arena);
    if (!arg) return NULL;
    
    asdl_arg_seq *posargs = _Py_asdl_arg_seq_new(1, p->arena);
    if (!posargs) return NULL;
    asdl_seq_SET(posargs, 0, arg);
    
    asdl_arg_seq *posonlyargs = _Py_asdl_arg_seq_new(0, p->arena);
    if (!posonlyargs) return NULL;
    asdl_expr_seq *posdefaults = _Py_asdl_expr_seq_new(0, p->arena);
    if (!posdefaults) return NULL;
    asdl_arg_seq *kwonlyargs = _Py_asdl_arg_seq_new(0, p->arena);
    if (!kwonlyargs) return NULL;
    asdl_expr_seq *kwdefaults = _Py_asdl_expr_seq_new(0, p->arena);
    if (!kwdefaults) return NULL;
    
    arguments_ty fallback_args = _PyAST_arguments(posonlyargs, posargs, NULL, kwonlyargs, kwdefaults, NULL, posdefaults, p->arena);
    if (!fallback_args) return NULL;
    
    expr_ty fallback_lambda = _PyAST_Lambda(fallback_args, fallback, fallback->lineno, fallback->col_offset, fallback->end_lineno, fallback->end_col_offset, p->arena);
    if (!fallback_lambda) return NULL;
    
    // Call _loh_rescue(body_lambda, exc_val, fallback_lambda)
    asdl_expr_seq *args = _Py_asdl_expr_seq_new(3, p->arena);
    if (!args) return NULL;
    asdl_seq_SET(args, 0, body_lambda);
    asdl_seq_SET(args, 1, exc_val);
    asdl_seq_SET(args, 2, fallback_lambda);
    
    return _PyAST_Call(func, args, NULL, expr->lineno, expr->col_offset, fallback->end_lineno, fallback->end_col_offset, p->arena);
}

expr_ty
_PyPegen_make_star_expr(Parser *p, expr_ty a) {
    if (!a) return NULL;
    if (a->kind == Subscript_kind) {
        PyObject *func_id = _PyPegen_new_identifier(p, "_loh_star_subscript");
        if (!func_id) return NULL;
        expr_ty func = _PyAST_Name(func_id, Load, a->lineno, a->col_offset, a->end_lineno, a->end_col_offset, p->arena);
        if (!func) return NULL;
        asdl_expr_seq *args = _Py_asdl_expr_seq_new(2, p->arena);
        if (!args) return NULL;
        asdl_seq_SET(args, 0, a->v.Subscript.value);
        asdl_seq_SET(args, 1, a->v.Subscript.slice);
        return _PyAST_Call(func, args, NULL, a->lineno, a->col_offset, a->end_lineno, a->end_col_offset, p->arena);
    } else {
        PyObject *func_id = _PyPegen_new_identifier(p, "_loh_star");
        if (!func_id) return NULL;
        expr_ty func = _PyAST_Name(func_id, Load, a->lineno, a->col_offset, a->end_lineno, a->end_col_offset, p->arena);
        if (!func) return NULL;
        asdl_expr_seq *args = _Py_asdl_expr_seq_new(1, p->arena);
        if (!args) return NULL;
        asdl_seq_SET(args, 0, a);
        return _PyAST_Call(func, args, NULL, a->lineno, a->col_offset, a->end_lineno, a->end_col_offset, p->arena);
    }
}

expr_ty
_PyPegen_make_double_star_expr(Parser *p, expr_ty a) {
    if (!a) return NULL;
    if (a->kind == Subscript_kind) {
        PyObject *func_id = _PyPegen_new_identifier(p, "_loh_double_star_subscript");
        if (!func_id) return NULL;
        expr_ty func = _PyAST_Name(func_id, Load, a->lineno, a->col_offset, a->end_lineno, a->end_col_offset, p->arena);
        if (!func) return NULL;
        asdl_expr_seq *args = _Py_asdl_expr_seq_new(2, p->arena);
        if (!args) return NULL;
        asdl_seq_SET(args, 0, a->v.Subscript.value);
        asdl_seq_SET(args, 1, a->v.Subscript.slice);
        return _PyAST_Call(func, args, NULL, a->lineno, a->col_offset, a->end_lineno, a->end_col_offset, p->arena);
    } else {
        PyObject *func_id = _PyPegen_new_identifier(p, "_loh_double_star");
        if (!func_id) return NULL;
        expr_ty func = _PyAST_Name(func_id, Load, a->lineno, a->col_offset, a->end_lineno, a->end_col_offset, p->arena);
        if (!func) return NULL;
        asdl_expr_seq *args = _Py_asdl_expr_seq_new(1, p->arena);
        if (!args) return NULL;
        asdl_seq_SET(args, 0, a);
        return _PyAST_Call(func, args, NULL, a->lineno, a->col_offset, a->end_lineno, a->end_col_offset, p->arena);
    }
}

expr_ty
_PyPegen_empty_subscript(Parser *p, expr_ty a) {
    PyObject *func_id = _PyPegen_new_identifier(p, "_loh_empty_subscript");
    if (!func_id) return NULL;
    expr_ty func = _PyAST_Name(func_id, Load, a->lineno, a->col_offset, a->end_lineno, a->end_col_offset, p->arena);
    if (!func) return NULL;
    asdl_expr_seq *args = _Py_asdl_expr_seq_new(1, p->arena);
    if (!args) return NULL;
    asdl_seq_SET(args, 0, a);
    return _PyAST_Call(func, args, NULL, a->lineno, a->col_offset, a->end_lineno, a->end_col_offset, p->arena);
}

stmt_ty
_PyPegen_make_starred_assign(Parser *p, expr_ty target, expr_ty value) {
    int lineno = target->lineno;
    int col_offset = target->col_offset;
    int end_lineno = value->end_lineno;
    int end_col_offset = value->end_col_offset;

    expr_ty load_target = _PyPegen_set_expr_context(p, target, Load);
    if (!load_target) return NULL;

    expr_ty call_expr = NULL;
    if (load_target->kind == Subscript_kind) {
        PyObject *func_id = _PyPegen_new_identifier(p, "_loh_assign_star_subscript");
        if (!func_id) return NULL;
        expr_ty func = _PyAST_Name(func_id, Load, lineno, col_offset, end_lineno, end_col_offset, p->arena);
        if (!func) return NULL;
        asdl_expr_seq *args = _Py_asdl_expr_seq_new(3, p->arena);
        if (!args) return NULL;
        asdl_seq_SET(args, 0, load_target->v.Subscript.value);
        asdl_seq_SET(args, 1, load_target->v.Subscript.slice);
        asdl_seq_SET(args, 2, value);
        call_expr = _PyAST_Call(func, args, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
    } else {
        PyObject *func_id = _PyPegen_new_identifier(p, "_loh_assign_star");
        if (!func_id) return NULL;
        expr_ty func = _PyAST_Name(func_id, Load, lineno, col_offset, end_lineno, end_col_offset, p->arena);
        if (!func) return NULL;
        asdl_expr_seq *args = _Py_asdl_expr_seq_new(2, p->arena);
        if (!args) return NULL;
        asdl_seq_SET(args, 0, load_target);
        asdl_seq_SET(args, 1, value);
        call_expr = _PyAST_Call(func, args, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
    }
    if (!call_expr) return NULL;
    return _PyAST_Expr(call_expr, lineno, col_offset, end_lineno, end_col_offset, p->arena);
}

stmt_ty
_PyPegen_make_double_starred_assign(Parser *p, expr_ty target, expr_ty value) {
    int lineno = target->lineno;
    int col_offset = target->col_offset;
    int end_lineno = value->end_lineno;
    int end_col_offset = value->end_col_offset;

    expr_ty load_target = _PyPegen_set_expr_context(p, target, Load);
    if (!load_target) return NULL;

    expr_ty call_expr = NULL;
    if (load_target->kind == Subscript_kind) {
        PyObject *func_id = _PyPegen_new_identifier(p, "_loh_assign_double_star_subscript");
        if (!func_id) return NULL;
        expr_ty func = _PyAST_Name(func_id, Load, lineno, col_offset, end_lineno, end_col_offset, p->arena);
        if (!func) return NULL;
        asdl_expr_seq *args = _Py_asdl_expr_seq_new(3, p->arena);
        if (!args) return NULL;
        asdl_seq_SET(args, 0, load_target->v.Subscript.value);
        asdl_seq_SET(args, 1, load_target->v.Subscript.slice);
        asdl_seq_SET(args, 2, value);
        call_expr = _PyAST_Call(func, args, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
    } else {
        PyObject *func_id = _PyPegen_new_identifier(p, "_loh_assign_double_star");
        if (!func_id) return NULL;
        expr_ty func = _PyAST_Name(func_id, Load, lineno, col_offset, end_lineno, end_col_offset, p->arena);
        if (!func) return NULL;
        asdl_expr_seq *args = _Py_asdl_expr_seq_new(2, p->arena);
        if (!args) return NULL;
        asdl_seq_SET(args, 0, load_target);
        asdl_seq_SET(args, 1, value);
        call_expr = _PyAST_Call(func, args, NULL, lineno, col_offset, end_lineno, end_col_offset, p->arena);
    }
    if (!call_expr) return NULL;
    return _PyAST_Expr(call_expr, lineno, col_offset, end_lineno, end_col_offset, p->arena);
}

static void desugar_double_dot_arguments(Parser *p, arguments_ty args);
static void desugar_double_dot_pattern(Parser *p, pattern_ty pattern);
static void desugar_double_dot_generators(Parser *p, asdl_comprehension_seq *generators);

expr_ty
_PyPegen_desugar_double_dot_expr(Parser *p, expr_ty expr) {
    if (!expr) {
        return NULL;
    }
    if (!p->has_double_dot) {
        return expr;
    }
    switch (expr->kind) {
        case Name_kind: {
            const char *name_str = PyUnicode_AsUTF8(expr->v.Name.id);
            if (name_str && strcmp(name_str, "..") == 0) {
                PyObject *super_id = _PyPegen_new_identifier(p, "super");
                if (!super_id) return NULL;
                expr_ty super_func = _PyAST_Name(super_id, Load, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
                if (!super_func) return NULL;
                return _PyAST_Call(super_func, NULL, NULL, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
            }
            return expr;
        }
        case Attribute_kind: {
            expr->v.Attribute.value = _PyPegen_desugar_double_dot_expr(p, expr->v.Attribute.value);
            return expr;
        }
        case Call_kind: {
            // Check if calling `..` directly (i.e. `..(args)`)
            if (expr->v.Call.func && expr->v.Call.func->kind == Name_kind) {
                const char *name_str = PyUnicode_AsUTF8(expr->v.Call.func->v.Name.id);
                if (name_str && strcmp(name_str, "..") == 0) {
                    // Desugar to `super().__init__(args)`
                    PyObject *super_id = _PyPegen_new_identifier(p, "super");
                    if (!super_id) return NULL;
                    expr_ty super_func = _PyAST_Name(super_id, Load, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
                    if (!super_func) return NULL;
                    expr_ty super_call = _PyAST_Call(super_func, NULL, NULL, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
                    if (!super_call) return NULL;
                    PyObject *init_id = _PyPegen_new_identifier(p, "__init__");
                    if (!init_id) return NULL;
                    expr_ty init_attr = _PyAST_Attribute(super_call, init_id, Load, expr->lineno, expr->col_offset, expr->end_lineno, expr->end_col_offset, p->arena);
                    if (!init_attr) return NULL;
                    expr->v.Call.func = init_attr;
                }
            }
            expr->v.Call.func = _PyPegen_desugar_double_dot_expr(p, expr->v.Call.func);
            if (expr->v.Call.args) {
                int len = asdl_seq_LEN(expr->v.Call.args);
                for (int i = 0; i < len; i++) {
                    expr_ty arg = asdl_seq_GET(expr->v.Call.args, i);
                    asdl_seq_SET(expr->v.Call.args, i, _PyPegen_desugar_double_dot_expr(p, arg));
                }
            }
            if (expr->v.Call.keywords) {
                int len = asdl_seq_LEN(expr->v.Call.keywords);
                for (int i = 0; i < len; i++) {
                    keyword_ty kw = asdl_seq_GET(expr->v.Call.keywords, i);
                    kw->value = _PyPegen_desugar_double_dot_expr(p, kw->value);
                }
            }
            return expr;
        }
        case BoolOp_kind: {
            if (expr->v.BoolOp.values) {
                int len = asdl_seq_LEN(expr->v.BoolOp.values);
                for (int i = 0; i < len; i++) {
                    expr_ty val = asdl_seq_GET(expr->v.BoolOp.values, i);
                    asdl_seq_SET(expr->v.BoolOp.values, i, _PyPegen_desugar_double_dot_expr(p, val));
                }
            }
            return expr;
        }
        case NamedExpr_kind: {
            expr->v.NamedExpr.target = _PyPegen_desugar_double_dot_expr(p, expr->v.NamedExpr.target);
            expr->v.NamedExpr.value = _PyPegen_desugar_double_dot_expr(p, expr->v.NamedExpr.value);
            return expr;
        }
        case BinOp_kind: {
            expr->v.BinOp.left = _PyPegen_desugar_double_dot_expr(p, expr->v.BinOp.left);
            expr->v.BinOp.right = _PyPegen_desugar_double_dot_expr(p, expr->v.BinOp.right);
            return expr;
        }
        case UnaryOp_kind: {
            expr->v.UnaryOp.operand = _PyPegen_desugar_double_dot_expr(p, expr->v.UnaryOp.operand);
            return expr;
        }
        case Lambda_kind: {
            if (expr->v.Lambda.args) {
                desugar_double_dot_arguments(p, expr->v.Lambda.args);
            }
            expr->v.Lambda.body = _PyPegen_desugar_double_dot_expr(p, expr->v.Lambda.body);
            return expr;
        }
        case IfExp_kind: {
            expr->v.IfExp.test = _PyPegen_desugar_double_dot_expr(p, expr->v.IfExp.test);
            expr->v.IfExp.body = _PyPegen_desugar_double_dot_expr(p, expr->v.IfExp.body);
            expr->v.IfExp.orelse = _PyPegen_desugar_double_dot_expr(p, expr->v.IfExp.orelse);
            return expr;
        }
        case Dict_kind: {
            if (expr->v.Dict.keys) {
                int len = asdl_seq_LEN(expr->v.Dict.keys);
                for (int i = 0; i < len; i++) {
                    expr_ty key = asdl_seq_GET(expr->v.Dict.keys, i);
                    if (key) {
                        asdl_seq_SET(expr->v.Dict.keys, i, _PyPegen_desugar_double_dot_expr(p, key));
                    }
                }
            }
            if (expr->v.Dict.values) {
                int len = asdl_seq_LEN(expr->v.Dict.values);
                for (int i = 0; i < len; i++) {
                    expr_ty val = asdl_seq_GET(expr->v.Dict.values, i);
                    asdl_seq_SET(expr->v.Dict.values, i, _PyPegen_desugar_double_dot_expr(p, val));
                }
            }
            return expr;
        }
        case Set_kind: {
            if (expr->v.Set.elts) {
                int len = asdl_seq_LEN(expr->v.Set.elts);
                for (int i = 0; i < len; i++) {
                    expr_ty elt = asdl_seq_GET(expr->v.Set.elts, i);
                    asdl_seq_SET(expr->v.Set.elts, i, _PyPegen_desugar_double_dot_expr(p, elt));
                }
            }
            return expr;
        }
        case ListComp_kind: {
            expr->v.ListComp.elt = _PyPegen_desugar_double_dot_expr(p, expr->v.ListComp.elt);
            desugar_double_dot_generators(p, expr->v.ListComp.generators);
            return expr;
        }
        case SetComp_kind: {
            expr->v.SetComp.elt = _PyPegen_desugar_double_dot_expr(p, expr->v.SetComp.elt);
            desugar_double_dot_generators(p, expr->v.SetComp.generators);
            return expr;
        }
        case DictComp_kind: {
            expr->v.DictComp.key = _PyPegen_desugar_double_dot_expr(p, expr->v.DictComp.key);
            expr->v.DictComp.value = _PyPegen_desugar_double_dot_expr(p, expr->v.DictComp.value);
            desugar_double_dot_generators(p, expr->v.DictComp.generators);
            return expr;
        }
        case GeneratorExp_kind: {
            expr->v.GeneratorExp.elt = _PyPegen_desugar_double_dot_expr(p, expr->v.GeneratorExp.elt);
            desugar_double_dot_generators(p, expr->v.GeneratorExp.generators);
            return expr;
        }
        case Await_kind: {
            expr->v.Await.value = _PyPegen_desugar_double_dot_expr(p, expr->v.Await.value);
            return expr;
        }
        case Yield_kind: {
            if (expr->v.Yield.value) {
                expr->v.Yield.value = _PyPegen_desugar_double_dot_expr(p, expr->v.Yield.value);
            }
            return expr;
        }
        case YieldFrom_kind: {
            expr->v.YieldFrom.value = _PyPegen_desugar_double_dot_expr(p, expr->v.YieldFrom.value);
            return expr;
        }
        case Compare_kind: {
            expr->v.Compare.left = _PyPegen_desugar_double_dot_expr(p, expr->v.Compare.left);
            if (expr->v.Compare.comparators) {
                int len = asdl_seq_LEN(expr->v.Compare.comparators);
                for (int i = 0; i < len; i++) {
                    expr_ty comp = asdl_seq_GET(expr->v.Compare.comparators, i);
                    asdl_seq_SET(expr->v.Compare.comparators, i, _PyPegen_desugar_double_dot_expr(p, comp));
                }
            }
            return expr;
        }
        case FormattedValue_kind: {
            expr->v.FormattedValue.value = _PyPegen_desugar_double_dot_expr(p, expr->v.FormattedValue.value);
            if (expr->v.FormattedValue.format_spec) {
                expr->v.FormattedValue.format_spec = _PyPegen_desugar_double_dot_expr(p, expr->v.FormattedValue.format_spec);
            }
            return expr;
        }
        case Interpolation_kind: {
            expr->v.Interpolation.value = _PyPegen_desugar_double_dot_expr(p, expr->v.Interpolation.value);
            if (expr->v.Interpolation.format_spec) {
                expr->v.Interpolation.format_spec = _PyPegen_desugar_double_dot_expr(p, expr->v.Interpolation.format_spec);
            }
            return expr;
        }
        case JoinedStr_kind: {
            if (expr->v.JoinedStr.values) {
                int len = asdl_seq_LEN(expr->v.JoinedStr.values);
                for (int i = 0; i < len; i++) {
                    expr_ty val = asdl_seq_GET(expr->v.JoinedStr.values, i);
                    asdl_seq_SET(expr->v.JoinedStr.values, i, _PyPegen_desugar_double_dot_expr(p, val));
                }
            }
            return expr;
        }
        case TemplateStr_kind: {
            if (expr->v.TemplateStr.values) {
                int len = asdl_seq_LEN(expr->v.TemplateStr.values);
                for (int i = 0; i < len; i++) {
                    expr_ty val = asdl_seq_GET(expr->v.TemplateStr.values, i);
                    asdl_seq_SET(expr->v.TemplateStr.values, i, _PyPegen_desugar_double_dot_expr(p, val));
                }
            }
            return expr;
        }
        case Pipe_kind: {
            expr->v.Pipe.left = _PyPegen_desugar_double_dot_expr(p, expr->v.Pipe.left);
            expr->v.Pipe.right = _PyPegen_desugar_double_dot_expr(p, expr->v.Pipe.right);
            return expr;
        }
        case Subscript_kind: {
            expr->v.Subscript.value = _PyPegen_desugar_double_dot_expr(p, expr->v.Subscript.value);
            expr->v.Subscript.slice = _PyPegen_desugar_double_dot_expr(p, expr->v.Subscript.slice);
            return expr;
        }
        case Starred_kind: {
            expr->v.Starred.value = _PyPegen_desugar_double_dot_expr(p, expr->v.Starred.value);
            return expr;
        }
        case List_kind: {
            if (expr->v.List.elts) {
                int len = asdl_seq_LEN(expr->v.List.elts);
                for (int i = 0; i < len; i++) {
                    expr_ty elt = asdl_seq_GET(expr->v.List.elts, i);
                    asdl_seq_SET(expr->v.List.elts, i, _PyPegen_desugar_double_dot_expr(p, elt));
                }
            }
            return expr;
        }
        case Tuple_kind: {
            if (expr->v.Tuple.elts) {
                int len = asdl_seq_LEN(expr->v.Tuple.elts);
                for (int i = 0; i < len; i++) {
                    expr_ty elt = asdl_seq_GET(expr->v.Tuple.elts, i);
                    asdl_seq_SET(expr->v.Tuple.elts, i, _PyPegen_desugar_double_dot_expr(p, elt));
                }
            }
            return expr;
        }
        case Slice_kind: {
            if (expr->v.Slice.lower) {
                expr->v.Slice.lower = _PyPegen_desugar_double_dot_expr(p, expr->v.Slice.lower);
            }
            if (expr->v.Slice.upper) {
                expr->v.Slice.upper = _PyPegen_desugar_double_dot_expr(p, expr->v.Slice.upper);
            }
            if (expr->v.Slice.step) {
                expr->v.Slice.step = _PyPegen_desugar_double_dot_expr(p, expr->v.Slice.step);
            }
            return expr;
        }
        case Constant_kind:
        default:
            return expr;
    }
}

static void
desugar_double_dot_arguments(Parser *p, arguments_ty args) {
    if (!args) return;
    if (args->defaults) {
        int len = asdl_seq_LEN(args->defaults);
        for (int i = 0; i < len; i++) {
            expr_ty def = asdl_seq_GET(args->defaults, i);
            asdl_seq_SET(args->defaults, i, _PyPegen_desugar_double_dot_expr(p, def));
        }
    }
    if (args->kw_defaults) {
        int len = asdl_seq_LEN(args->kw_defaults);
        for (int i = 0; i < len; i++) {
            expr_ty def = asdl_seq_GET(args->kw_defaults, i);
            if (def) {
                asdl_seq_SET(args->kw_defaults, i, _PyPegen_desugar_double_dot_expr(p, def));
            }
        }
    }
    if (args->posonlyargs) {
        int len = asdl_seq_LEN(args->posonlyargs);
        for (int i = 0; i < len; i++) {
            arg_ty arg = asdl_seq_GET(args->posonlyargs, i);
            if (arg->annotation) {
                arg->annotation = _PyPegen_desugar_double_dot_expr(p, arg->annotation);
            }
        }
    }
    if (args->args) {
        int len = asdl_seq_LEN(args->args);
        for (int i = 0; i < len; i++) {
            arg_ty arg = asdl_seq_GET(args->args, i);
            if (arg->annotation) {
                arg->annotation = _PyPegen_desugar_double_dot_expr(p, arg->annotation);
            }
        }
    }
    if (args->kwonlyargs) {
        int len = asdl_seq_LEN(args->kwonlyargs);
        for (int i = 0; i < len; i++) {
            arg_ty arg = asdl_seq_GET(args->kwonlyargs, i);
            if (arg->annotation) {
                arg->annotation = _PyPegen_desugar_double_dot_expr(p, arg->annotation);
            }
        }
    }
    if (args->vararg && args->vararg->annotation) {
        args->vararg->annotation = _PyPegen_desugar_double_dot_expr(p, args->vararg->annotation);
    }
    if (args->kwarg && args->kwarg->annotation) {
        args->kwarg->annotation = _PyPegen_desugar_double_dot_expr(p, args->kwarg->annotation);
    }
}

static void
desugar_double_dot_pattern(Parser *p, pattern_ty pattern) {
    if (!pattern) return;
    switch (pattern->kind) {
        case MatchValue_kind:
            pattern->v.MatchValue.value = _PyPegen_desugar_double_dot_expr(p, pattern->v.MatchValue.value);
            break;
        case MatchSequence_kind:
            if (pattern->v.MatchSequence.patterns) {
                int len = asdl_seq_LEN(pattern->v.MatchSequence.patterns);
                for (int i = 0; i < len; i++) {
                    desugar_double_dot_pattern(p, asdl_seq_GET(pattern->v.MatchSequence.patterns, i));
                }
            }
            break;
        case MatchMapping_kind:
            if (pattern->v.MatchMapping.keys) {
                int len = asdl_seq_LEN(pattern->v.MatchMapping.keys);
                for (int i = 0; i < len; i++) {
                    expr_ty key = asdl_seq_GET(pattern->v.MatchMapping.keys, i);
                    asdl_seq_SET(pattern->v.MatchMapping.keys, i, _PyPegen_desugar_double_dot_expr(p, key));
                }
            }
            if (pattern->v.MatchMapping.patterns) {
                int len = asdl_seq_LEN(pattern->v.MatchMapping.patterns);
                for (int i = 0; i < len; i++) {
                    desugar_double_dot_pattern(p, asdl_seq_GET(pattern->v.MatchMapping.patterns, i));
                }
            }
            break;
        case MatchClass_kind:
            pattern->v.MatchClass.cls = _PyPegen_desugar_double_dot_expr(p, pattern->v.MatchClass.cls);
            if (pattern->v.MatchClass.patterns) {
                int len = asdl_seq_LEN(pattern->v.MatchClass.patterns);
                for (int i = 0; i < len; i++) {
                    desugar_double_dot_pattern(p, asdl_seq_GET(pattern->v.MatchClass.patterns, i));
                }
            }
            if (pattern->v.MatchClass.kwd_patterns) {
                int len = asdl_seq_LEN(pattern->v.MatchClass.kwd_patterns);
                for (int i = 0; i < len; i++) {
                    desugar_double_dot_pattern(p, asdl_seq_GET(pattern->v.MatchClass.kwd_patterns, i));
                }
            }
            break;
        case MatchAs_kind:
            if (pattern->v.MatchAs.pattern) {
                desugar_double_dot_pattern(p, pattern->v.MatchAs.pattern);
            }
            break;
        case MatchOr_kind:
            if (pattern->v.MatchOr.patterns) {
                int len = asdl_seq_LEN(pattern->v.MatchOr.patterns);
                for (int i = 0; i < len; i++) {
                    desugar_double_dot_pattern(p, asdl_seq_GET(pattern->v.MatchOr.patterns, i));
                }
            }
            break;
        default:
            break;
    }
}

static void
desugar_double_dot_generators(Parser *p, asdl_comprehension_seq *generators) {
    if (!generators) return;
    int len = asdl_seq_LEN(generators);
    for (int i = 0; i < len; i++) {
        comprehension_ty gen = asdl_seq_GET(generators, i);
        gen->target = _PyPegen_desugar_double_dot_expr(p, gen->target);
        gen->iter = _PyPegen_desugar_double_dot_expr(p, gen->iter);
        if (gen->ifs) {
            int ifs_len = asdl_seq_LEN(gen->ifs);
            for (int j = 0; j < ifs_len; j++) {
                expr_ty cond = asdl_seq_GET(gen->ifs, j);
                asdl_seq_SET(gen->ifs, j, _PyPegen_desugar_double_dot_expr(p, cond));
            }
        }
    }
}

void
_PyPegen_desugar_double_dot_stmt(Parser *p, stmt_ty stmt) {
    if (!stmt) {
        return;
    }
    switch (stmt->kind) {
        case FunctionDef_kind: {
            if (stmt->v.FunctionDef.decorator_list) {
                int len = asdl_seq_LEN(stmt->v.FunctionDef.decorator_list);
                for (int i = 0; i < len; i++) {
                    expr_ty dec = asdl_seq_GET(stmt->v.FunctionDef.decorator_list, i);
                    asdl_seq_SET(stmt->v.FunctionDef.decorator_list, i, _PyPegen_desugar_double_dot_expr(p, dec));
                }
            }
            if (stmt->v.FunctionDef.returns) {
                stmt->v.FunctionDef.returns = _PyPegen_desugar_double_dot_expr(p, stmt->v.FunctionDef.returns);
            }
            if (stmt->v.FunctionDef.args) {
                desugar_double_dot_arguments(p, stmt->v.FunctionDef.args);
            }
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.FunctionDef.body);
            break;
        }
        case AsyncFunctionDef_kind: {
            if (stmt->v.AsyncFunctionDef.decorator_list) {
                int len = asdl_seq_LEN(stmt->v.AsyncFunctionDef.decorator_list);
                for (int i = 0; i < len; i++) {
                    expr_ty dec = asdl_seq_GET(stmt->v.AsyncFunctionDef.decorator_list, i);
                    asdl_seq_SET(stmt->v.AsyncFunctionDef.decorator_list, i, _PyPegen_desugar_double_dot_expr(p, dec));
                }
            }
            if (stmt->v.AsyncFunctionDef.returns) {
                stmt->v.AsyncFunctionDef.returns = _PyPegen_desugar_double_dot_expr(p, stmt->v.AsyncFunctionDef.returns);
            }
            if (stmt->v.AsyncFunctionDef.args) {
                desugar_double_dot_arguments(p, stmt->v.AsyncFunctionDef.args);
            }
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.AsyncFunctionDef.body);
            break;
        }
        case ClassDef_kind: {
            if (stmt->v.ClassDef.bases) {
                int len = asdl_seq_LEN(stmt->v.ClassDef.bases);
                for (int i = 0; i < len; i++) {
                    expr_ty base = asdl_seq_GET(stmt->v.ClassDef.bases, i);
                    asdl_seq_SET(stmt->v.ClassDef.bases, i, _PyPegen_desugar_double_dot_expr(p, base));
                }
            }
            if (stmt->v.ClassDef.keywords) {
                int len = asdl_seq_LEN(stmt->v.ClassDef.keywords);
                for (int i = 0; i < len; i++) {
                    keyword_ty kw = asdl_seq_GET(stmt->v.ClassDef.keywords, i);
                    kw->value = _PyPegen_desugar_double_dot_expr(p, kw->value);
                }
            }
            if (stmt->v.ClassDef.decorator_list) {
                int len = asdl_seq_LEN(stmt->v.ClassDef.decorator_list);
                for (int i = 0; i < len; i++) {
                    expr_ty dec = asdl_seq_GET(stmt->v.ClassDef.decorator_list, i);
                    asdl_seq_SET(stmt->v.ClassDef.decorator_list, i, _PyPegen_desugar_double_dot_expr(p, dec));
                }
            }
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.ClassDef.body);
            break;
        }
        case Return_kind: {
            if (stmt->v.Return.value) {
                stmt->v.Return.value = _PyPegen_desugar_double_dot_expr(p, stmt->v.Return.value);
            }
            break;
        }
        case Delete_kind: {
            if (stmt->v.Delete.targets) {
                int len = asdl_seq_LEN(stmt->v.Delete.targets);
                for (int i = 0; i < len; i++) {
                    expr_ty target = asdl_seq_GET(stmt->v.Delete.targets, i);
                    asdl_seq_SET(stmt->v.Delete.targets, i, _PyPegen_desugar_double_dot_expr(p, target));
                }
            }
            break;
        }
        case Assign_kind: {
            if (stmt->v.Assign.targets) {
                int len = asdl_seq_LEN(stmt->v.Assign.targets);
                for (int i = 0; i < len; i++) {
                    expr_ty target = asdl_seq_GET(stmt->v.Assign.targets, i);
                    asdl_seq_SET(stmt->v.Assign.targets, i, _PyPegen_desugar_double_dot_expr(p, target));
                }
            }
            stmt->v.Assign.value = _PyPegen_desugar_double_dot_expr(p, stmt->v.Assign.value);
            break;
        }
        case TypeAlias_kind: {
            stmt->v.TypeAlias.name = _PyPegen_desugar_double_dot_expr(p, stmt->v.TypeAlias.name);
            stmt->v.TypeAlias.value = _PyPegen_desugar_double_dot_expr(p, stmt->v.TypeAlias.value);
            break;
        }
        case AugAssign_kind: {
            stmt->v.AugAssign.target = _PyPegen_desugar_double_dot_expr(p, stmt->v.AugAssign.target);
            stmt->v.AugAssign.value = _PyPegen_desugar_double_dot_expr(p, stmt->v.AugAssign.value);
            break;
        }
        case AnnAssign_kind: {
            stmt->v.AnnAssign.target = _PyPegen_desugar_double_dot_expr(p, stmt->v.AnnAssign.target);
            stmt->v.AnnAssign.annotation = _PyPegen_desugar_double_dot_expr(p, stmt->v.AnnAssign.annotation);
            if (stmt->v.AnnAssign.value) {
                stmt->v.AnnAssign.value = _PyPegen_desugar_double_dot_expr(p, stmt->v.AnnAssign.value);
            }
            break;
        }
        case For_kind: {
            stmt->v.For.target = _PyPegen_desugar_double_dot_expr(p, stmt->v.For.target);
            stmt->v.For.iter = _PyPegen_desugar_double_dot_expr(p, stmt->v.For.iter);
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.For.body);
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.For.orelse);
            break;
        }
        case AsyncFor_kind: {
            stmt->v.AsyncFor.target = _PyPegen_desugar_double_dot_expr(p, stmt->v.AsyncFor.target);
            stmt->v.AsyncFor.iter = _PyPegen_desugar_double_dot_expr(p, stmt->v.AsyncFor.iter);
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.AsyncFor.body);
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.AsyncFor.orelse);
            break;
        }
        case While_kind: {
            stmt->v.While.test = _PyPegen_desugar_double_dot_expr(p, stmt->v.While.test);
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.While.body);
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.While.orelse);
            break;
        }
        case If_kind: {
            stmt->v.If.test = _PyPegen_desugar_double_dot_expr(p, stmt->v.If.test);
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.If.body);
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.If.orelse);
            break;
        }
        case With_kind: {
            if (stmt->v.With.items) {
                int len = asdl_seq_LEN(stmt->v.With.items);
                for (int i = 0; i < len; i++) {
                    withitem_ty item = asdl_seq_GET(stmt->v.With.items, i);
                    item->context_expr = _PyPegen_desugar_double_dot_expr(p, item->context_expr);
                    if (item->optional_vars) {
                        item->optional_vars = _PyPegen_desugar_double_dot_expr(p, item->optional_vars);
                    }
                }
            }
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.With.body);
            break;
        }
        case AsyncWith_kind: {
            if (stmt->v.AsyncWith.items) {
                int len = asdl_seq_LEN(stmt->v.AsyncWith.items);
                for (int i = 0; i < len; i++) {
                    withitem_ty item = asdl_seq_GET(stmt->v.AsyncWith.items, i);
                    item->context_expr = _PyPegen_desugar_double_dot_expr(p, item->context_expr);
                    if (item->optional_vars) {
                        item->optional_vars = _PyPegen_desugar_double_dot_expr(p, item->optional_vars);
                    }
                }
            }
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.AsyncWith.body);
            break;
        }
        case Match_kind: {
            stmt->v.Match.subject = _PyPegen_desugar_double_dot_expr(p, stmt->v.Match.subject);
            if (stmt->v.Match.cases) {
                int len = asdl_seq_LEN(stmt->v.Match.cases);
                for (int i = 0; i < len; i++) {
                    match_case_ty mcase = asdl_seq_GET(stmt->v.Match.cases, i);
                    desugar_double_dot_pattern(p, mcase->pattern);
                    if (mcase->guard) {
                        mcase->guard = _PyPegen_desugar_double_dot_expr(p, mcase->guard);
                    }
                    _PyPegen_desugar_double_dot_stmts(p, mcase->body);
                }
            }
            break;
        }
        case Raise_kind: {
            if (stmt->v.Raise.exc) {
                stmt->v.Raise.exc = _PyPegen_desugar_double_dot_expr(p, stmt->v.Raise.exc);
            }
            if (stmt->v.Raise.cause) {
                stmt->v.Raise.cause = _PyPegen_desugar_double_dot_expr(p, stmt->v.Raise.cause);
            }
            break;
        }
        case Try_kind: {
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.Try.body);
            if (stmt->v.Try.handlers) {
                int len = asdl_seq_LEN(stmt->v.Try.handlers);
                for (int i = 0; i < len; i++) {
                    excepthandler_ty handler = asdl_seq_GET(stmt->v.Try.handlers, i);
                    if (handler->v.ExceptHandler.type) {
                        handler->v.ExceptHandler.type = _PyPegen_desugar_double_dot_expr(p, handler->v.ExceptHandler.type);
                    }
                    _PyPegen_desugar_double_dot_stmts(p, handler->v.ExceptHandler.body);
                }
            }
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.Try.orelse);
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.Try.finalbody);
            break;
        }
        case TryStar_kind: {
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.TryStar.body);
            if (stmt->v.TryStar.handlers) {
                int len = asdl_seq_LEN(stmt->v.TryStar.handlers);
                for (int i = 0; i < len; i++) {
                    excepthandler_ty handler = asdl_seq_GET(stmt->v.TryStar.handlers, i);
                    if (handler->v.ExceptHandler.type) {
                        handler->v.ExceptHandler.type = _PyPegen_desugar_double_dot_expr(p, handler->v.ExceptHandler.type);
                    }
                    _PyPegen_desugar_double_dot_stmts(p, handler->v.ExceptHandler.body);
                }
            }
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.TryStar.orelse);
            _PyPegen_desugar_double_dot_stmts(p, stmt->v.TryStar.finalbody);
            break;
        }
        case Assert_kind: {
            stmt->v.Assert.test = _PyPegen_desugar_double_dot_expr(p, stmt->v.Assert.test);
            if (stmt->v.Assert.msg) {
                stmt->v.Assert.msg = _PyPegen_desugar_double_dot_expr(p, stmt->v.Assert.msg);
            }
            break;
        }
        case Expr_kind: {
            stmt->v.Expr.value = _PyPegen_desugar_double_dot_expr(p, stmt->v.Expr.value);
            break;
        }
        case Pass_kind:
        case Break_kind:
        case Continue_kind:
        case Import_kind:
        case ImportFrom_kind:
        case Global_kind:
        case Nonlocal_kind:
        default:
            break;
    }
}

asdl_stmt_seq *
_PyPegen_desugar_double_dot_stmts(Parser *p, asdl_stmt_seq *stmts) {
    if (!stmts) return NULL;
    if (!p->has_double_dot) {
        return stmts;
    }
    int len = asdl_seq_LEN(stmts);
    for (int i = 0; i < len; i++) {
        stmt_ty stmt = asdl_seq_GET(stmts, i);
        _PyPegen_desugar_double_dot_stmt(p, stmt);
    }
    return stmts;
}

stmt_ty
_PyPegen_make_boolean_strict_assign(Parser *p, expr_ty target, int is_true, expr_ty value, int lineno, int col_offset, int end_lineno, int end_col_offset, PyArena *arena) {
    if (!target || !value) return NULL;
    expr_ty load_target = _PyPegen_set_expr_context(p, target, Load);
    if (!load_target) return NULL;

    asdl_int_seq *ops = _Py_asdl_int_seq_new(1, arena);
    if (!ops) return NULL;
    asdl_seq_SET(ops, 0, Eq);
    asdl_expr_seq *comps = _Py_asdl_expr_seq_new(1, arena);
    if (!comps) return NULL;
    expr_ty const_node = _PyAST_Constant(is_true ? Py_True : Py_False, NULL, lineno, col_offset, end_lineno, end_col_offset, arena);
    if (!const_node) return NULL;
    asdl_seq_SET(comps, 0, const_node);

    expr_ty test = _PyAST_Compare(load_target, ops, comps, lineno, col_offset, end_lineno, end_col_offset, arena);
    if (!test) return NULL;

    expr_ty if_expr = _PyAST_IfExp(test, load_target, value, lineno, col_offset, end_lineno, end_col_offset, arena);
    if (!if_expr) return NULL;

    expr_ty store_target = _PyPegen_set_expr_context(p, target, Store);
    if (!store_target) return NULL;
    asdl_expr_seq *targets = _Py_asdl_expr_seq_new(1, arena);
    if (!targets) return NULL;
    asdl_seq_SET(targets, 0, store_target);

    return _PyAST_Assign(targets, if_expr, NULL, lineno, col_offset, end_lineno, end_col_offset, arena);
}

static int
_PyPegen_is_dot_identifier(PyObject *name) {
    const char *str = PyUnicode_AsUTF8(name);
    return str && str[0] == '.';
}

asdl_expr_seq *
_PyPegen_make_alias_names_seq(Parser *p, expr_ty a, expr_ty b) {
    if (a->kind != Name_kind || b->kind != Name_kind) {
        return NULL;
    }
    int a_dot = _PyPegen_is_dot_identifier(a->v.Name.id);
    int b_dot = _PyPegen_is_dot_identifier(b->v.Name.id);
    if (!a_dot && b_dot) {
        return RAISE_SYNTAX_ERROR_KNOWN_RANGE(a, b, "function alias name cannot have dot prefix unless the primary name has it");
    }
    asdl_expr_seq *seq = (asdl_expr_seq *)_PyPegen_singleton_seq(p, a);
    if (!seq) {
        return NULL;
    }
    return (asdl_expr_seq *)_PyPegen_seq_append_to_end(p, (asdl_seq *)seq, b);
}

asdl_expr_seq *
_PyPegen_append_alias_name(Parser *p, asdl_expr_seq *a, expr_ty b) {
    if (!a || asdl_seq_LEN(a) == 0 || b->kind != Name_kind) {
        return NULL;
    }
    expr_ty first = (expr_ty)asdl_seq_GET(a, 0);
    int first_dot = _PyPegen_is_dot_identifier(first->v.Name.id);
    int b_dot = _PyPegen_is_dot_identifier(b->v.Name.id);
    if (!first_dot && b_dot) {
        return RAISE_SYNTAX_ERROR_KNOWN_RANGE(first, b, "function alias name cannot have dot prefix unless the primary name has it");
    }
    return (asdl_expr_seq *)_PyPegen_seq_append_to_end(p, (asdl_seq *)a, b);
}

asdl_expr_seq *
_PyPegen_make_alias_names_seq_multiple(Parser *p, expr_ty a, expr_ty b, asdl_seq *c) {
    asdl_expr_seq *seq = _PyPegen_make_alias_names_seq(p, a, b);
    if (!seq) {
        return NULL;
    }
    if (c) {
        int len = asdl_seq_LEN(c);
        for (int i = 0; i < len; i++) {
            expr_ty name = asdl_seq_GET((asdl_expr_seq *)c, i);
            seq = _PyPegen_append_alias_name(p, seq, name);
            if (!seq) {
                return NULL;
            }
        }
    }
    return seq;
}

asdl_stmt_seq *
_PyPegen_make_aliased_function_def(Parser *p, asdl_expr_seq *names, arguments_ty params, expr_ty returns,
                                  Token *func_type_comment, asdl_type_param_seq *type_params,
                                  asdl_stmt_seq *body, int is_async, int lineno, int col_offset,
                                  int end_lineno, int end_col_offset, PyArena *arena)
{
    if (!names || asdl_seq_LEN(names) < 2) {
        return NULL;
    }

    expr_ty primary_name_node = (expr_ty)asdl_seq_GET(names, 0);
    const char *primary_full_str = PyUnicode_AsUTF8(primary_name_node->v.Name.id);
    if (!primary_full_str) {
        return NULL;
    }

    int has_dot = (primary_full_str[0] == '.');
    PyObject *primary_id = NULL;
    if (has_dot) {
        primary_id = _PyPegen_new_identifier(p, primary_full_str + 1);
        if (!primary_id) return NULL;
        // Prepend '.' as self argument
        PyObject *dot_id = _PyPegen_new_identifier(p, ".");
        if (!dot_id) return NULL;
        arg_ty dot_arg = _PyAST_arg(dot_id, NULL, NULL, lineno, col_offset, end_lineno, end_col_offset, arena);
        if (!dot_arg) return NULL;
        params = _PyPegen_insert_arg_in_front(p, dot_arg, params);
        if (!params) return NULL;
    } else {
        primary_id = primary_name_node->v.Name.id;
    }

    if (!params) {
        params = _PyPegen_empty_arguments(p);
    }

    // Desugar parameter properties (like default values punning/destructuring alias desugaring etc. if any)
    asdl_stmt_seq *desugared_body = _PyPegen_desugar_parameter_properties(p, primary_id, params, body);

    stmt_ty func_stmt = NULL;
    if (is_async) {
        func_stmt = _PyAST_AsyncFunctionDef(primary_id, params, desugared_body, NULL, returns,
                                           func_type_comment ? func_type_comment->metadata : NULL,
                                           type_params, lineno, col_offset, end_lineno, end_col_offset, arena);
    } else {
        func_stmt = _PyAST_FunctionDef(primary_id, params, desugared_body, NULL, returns,
                                       func_type_comment ? func_type_comment->metadata : NULL,
                                       type_params, lineno, col_offset, end_lineno, end_col_offset, arena);
    }
    if (!func_stmt) return NULL;

    int total_aliases = asdl_seq_LEN(names) - 1;
    asdl_stmt_seq *stmts = _Py_asdl_stmt_seq_new(total_aliases + 1, arena);
    if (!stmts) return NULL;
    asdl_seq_SET(stmts, 0, func_stmt);

    // Primary name for right-hand side of assignments (always without dot prefix)
    expr_ty rhs_name = _PyAST_Name(primary_id, Load, lineno, col_offset, end_lineno, end_col_offset, arena);
    if (!rhs_name) return NULL;

    for (int i = 0; i < total_aliases; i++) {
        expr_ty alias_name_node = (expr_ty)asdl_seq_GET(names, i + 1);
        const char *alias_full_str = PyUnicode_AsUTF8(alias_name_node->v.Name.id);
        if (!alias_full_str) return NULL;
        PyObject *alias_id = NULL;
        if (alias_full_str[0] == '.') {
            alias_id = _PyPegen_new_identifier(p, alias_full_str + 1);
        } else {
            alias_id = alias_name_node->v.Name.id;
        }
        if (!alias_id) return NULL;

        expr_ty lhs_name = _PyAST_Name(alias_id, Store, lineno, col_offset, end_lineno, end_col_offset, arena);
        if (!lhs_name) return NULL;
        asdl_expr_seq *targets = (asdl_expr_seq *)_PyPegen_singleton_seq(p, lhs_name);
        if (!targets) return NULL;

        stmt_ty assign_stmt = _PyAST_Assign(targets, rhs_name, NULL, lineno, col_offset, end_lineno, end_col_offset, arena);
        if (!assign_stmt) return NULL;
        asdl_seq_SET(stmts, i + 1, assign_stmt);
    }

    return stmts;
}

asdl_stmt_seq *
_PyPegen_aliased_function_def_decorators(Parser *p, asdl_expr_seq *decorators, asdl_stmt_seq *aliased_f)
{
    if (aliased_f == NULL || asdl_seq_LEN(aliased_f) == 0) {
        return aliased_f;
    }
    stmt_ty func_def = asdl_seq_GET(aliased_f, 0);
    stmt_ty decorated_func = _PyPegen_function_def_decorators(p, decorators, func_def);
    if (!decorated_func) return NULL;
    asdl_seq_SET(aliased_f, 0, decorated_func);
    return aliased_f;
}

expr_ty
_PyPegen_make_lazy_expr(Parser *p, expr_ty expr)
{
    if (expr == NULL) {
        return NULL;
    }
    int lineno = expr->lineno;
    int col = expr->col_offset;
    int end_lineno = expr->end_lineno;
    int end_col = expr->end_col_offset;
    PyArena *arena = p->arena;

    arguments_ty empty_args = _PyPegen_empty_arguments(p);
    if (empty_args == NULL) {
        return NULL;
    }
    expr_ty lambda_node = _PyAST_Lambda(empty_args, expr, lineno, col, end_lineno, end_col, arena);
    if (lambda_node == NULL) {
        return NULL;
    }

    PyObject *lazy_name_id = _PyPegen_new_identifier(p, "_LohLazy");
    if (lazy_name_id == NULL) {
        return NULL;
    }
    expr_ty func_name = _PyAST_Name(lazy_name_id, Load, lineno, col, end_lineno, end_col, arena);
    if (func_name == NULL) {
        return NULL;
    }

    asdl_expr_seq *args = (asdl_expr_seq *)_PyPegen_singleton_seq(p, lambda_node);
    if (args == NULL) {
        return NULL;
    }
    
    return _PyAST_Call(func_name, args, NULL, lineno, col, end_lineno, end_col, arena);
}

asdl_stmt_seq *
_PyPegen_desugar_lazy_defaults(Parser *p, arguments_ty args, asdl_stmt_seq *body)
{
    if (args == NULL) {
        return body;
    }
    PyArena *arena = p->arena;

    PyObject *sentinel_id = _PyPegen_new_identifier(p, "_LOH_SENTINEL");
    if (sentinel_id == NULL) {
        return NULL;
    }

    // 1. Process keyword-only arguments (right-to-left)
    if (args->kwonlyargs && args->kw_defaults) {
        int kw_len = asdl_seq_LEN(args->kwonlyargs);
        for (int i = kw_len - 1; i >= 0; i--) {
            expr_ty def = asdl_seq_GET(args->kw_defaults, i);
            if (def != NULL && def->kind == Call_kind) {
                expr_ty func = def->v.Call.func;
                if (func->kind == Name_kind && strcmp(PyUnicode_AsUTF8(func->v.Name.id), "_LohLazy") == 0) {
                    if (def->v.Call.args && asdl_seq_LEN(def->v.Call.args) > 0) {
                        expr_ty lambda_node = asdl_seq_GET(def->v.Call.args, 0);
                        if (lambda_node->kind == Lambda_kind) {
                            expr_ty inner_expr = lambda_node->v.Lambda.body;
                            arg_ty param_arg = asdl_seq_GET(args->kwonlyargs, i);

                            int lineno = param_arg->lineno;
                            int col = param_arg->col_offset;
                            int end_lineno = param_arg->end_lineno;
                            int end_col = param_arg->end_col_offset;

                            expr_ty sentinel_node = _PyAST_Name(sentinel_id, Load, lineno, col, end_lineno, end_col, arena);
                            asdl_seq_SET(args->kw_defaults, i, sentinel_node);

                            expr_ty param_load = _PyAST_Name(param_arg->arg, Load, lineno, col, end_lineno, end_col, arena);
                            expr_ty cmp = _PyAST_Compare(param_load, 
                                                         _PyPegen_singleton_int_seq(p, Is), 
                                                         (asdl_expr_seq *)_PyPegen_singleton_seq(p, sentinel_node), 
                                                         lineno, col, end_lineno, end_col, arena);
                            
                            expr_ty param_store = _PyAST_Name(param_arg->arg, Store, lineno, col, end_lineno, end_col, arena);
                            asdl_expr_seq *targets = (asdl_expr_seq *)_PyPegen_singleton_seq(p, param_store);
                            stmt_ty assign_stmt = _PyAST_Assign(targets, inner_expr, NULL, lineno, col, end_lineno, end_col, arena);
                            asdl_stmt_seq *if_body = (asdl_stmt_seq *)_PyPegen_singleton_seq(p, assign_stmt);
                            stmt_ty if_stmt = _PyAST_If(cmp, if_body, NULL, lineno, col, end_lineno, end_col, arena);

                            body = (asdl_stmt_seq *)_PyPegen_seq_insert_in_front(p, if_stmt, (asdl_seq *)body);
                            if (body == NULL) {
                                return NULL;
                            }
                        }
                    }
                }
            }
        }
    }

    // 2. Process positional/positional-only arguments (right-to-left)
    if (args->defaults) {
        int posonly_len = args->posonlyargs ? asdl_seq_LEN(args->posonlyargs) : 0;
        int positional_len = args->args ? asdl_seq_LEN(args->args) : 0;
        int total_pos = posonly_len + positional_len;
        int defaults_len = asdl_seq_LEN(args->defaults);

        for (int i = defaults_len - 1; i >= 0; i--) {
            expr_ty def = asdl_seq_GET(args->defaults, i);
            if (def != NULL && def->kind == Call_kind) {
                expr_ty func = def->v.Call.func;
                if (func->kind == Name_kind && strcmp(PyUnicode_AsUTF8(func->v.Name.id), "_LohLazy") == 0) {
                    if (def->v.Call.args && asdl_seq_LEN(def->v.Call.args) > 0) {
                        expr_ty lambda_node = asdl_seq_GET(def->v.Call.args, 0);
                        if (lambda_node->kind == Lambda_kind) {
                            expr_ty inner_expr = lambda_node->v.Lambda.body;

                            int param_idx = total_pos - defaults_len + i;
                            arg_ty param_arg = NULL;
                            if (param_idx < posonly_len) {
                                param_arg = asdl_seq_GET(args->posonlyargs, param_idx);
                            } else {
                                param_arg = asdl_seq_GET(args->args, param_idx - posonly_len);
                            }

                            int lineno = param_arg->lineno;
                            int col = param_arg->col_offset;
                            int end_lineno = param_arg->end_lineno;
                            int end_col = param_arg->end_col_offset;

                            expr_ty sentinel_node = _PyAST_Name(sentinel_id, Load, lineno, col, end_lineno, end_col, arena);
                            asdl_seq_SET(args->defaults, i, sentinel_node);

                            expr_ty param_load = _PyAST_Name(param_arg->arg, Load, lineno, col, end_lineno, end_col, arena);
                            expr_ty cmp = _PyAST_Compare(param_load, 
                                                         _PyPegen_singleton_int_seq(p, Is), 
                                                         (asdl_expr_seq *)_PyPegen_singleton_seq(p, sentinel_node), 
                                                         lineno, col, end_lineno, end_col, arena);
                            
                            expr_ty param_store = _PyAST_Name(param_arg->arg, Store, lineno, col, end_lineno, end_col, arena);
                            asdl_expr_seq *targets = (asdl_expr_seq *)_PyPegen_singleton_seq(p, param_store);
                            stmt_ty assign_stmt = _PyAST_Assign(targets, inner_expr, NULL, lineno, col, end_lineno, end_col, arena);
                            asdl_stmt_seq *if_body = (asdl_stmt_seq *)_PyPegen_singleton_seq(p, assign_stmt);
                            stmt_ty if_stmt = _PyAST_If(cmp, if_body, NULL, lineno, col, end_lineno, end_col, arena);

                            body = (asdl_stmt_seq *)_PyPegen_seq_insert_in_front(p, if_stmt, (asdl_seq *)body);
                            if (body == NULL) {
                                return NULL;
                            }
                        }
                    }
                }
            }
        }
    }

    return body;
}

expr_ty
_PyPegen_make_loh_op(Parser *p, int kind, expr_ty val) {
    PyArena *arena = p->arena;
    int lineno = val ? val->lineno : p->tok->lineno;
    int col = val ? val->col_offset : p->tok->col_offset;
    int end_lineno = val ? val->end_lineno : lineno;
    int end_col = val ? val->end_col_offset : col;


    expr_ty func_name = _PyAST_Name(
        _PyPegen_new_identifier(p, "_LohOp"),
        Load,
        lineno, col, end_lineno, end_col, arena
    );

    expr_ty kind_constant = _PyAST_Constant(
        PyLong_FromLong(kind),
        NULL,
        lineno, col, end_lineno, end_col, arena
    );

    expr_ty val_node = val ? val : _PyAST_Constant(Py_None, NULL, lineno, col, end_lineno, end_col, arena);

    asdl_expr_seq *args = (asdl_expr_seq *)_PyPegen_singleton_seq(p, kind_constant);
    args = (asdl_expr_seq *)_PyPegen_seq_append_to_end(p, (asdl_seq *)args, val_node);

    return _PyAST_Call(
        func_name,
        args,
        NULL,
        lineno, col, end_lineno, end_col, arena
    );
}

expr_ty
_PyPegen_make_deep_copy_expr(Parser *p, expr_ty a) {
    if (a == NULL) return NULL;
    PyArena *arena = p->arena;
    int lineno = a->lineno;
    int col = a->col_offset;
    int end_lineno = a->end_lineno;
    int end_col = a->end_col_offset;

    expr_ty func_name = _PyAST_Name(
        _PyPegen_new_identifier(p, "_loh_deepcopy"),
        Load,
        lineno, col, end_lineno, end_col, arena
    );
    if (func_name == NULL) return NULL;

    asdl_expr_seq *args = (asdl_expr_seq *)_PyPegen_singleton_seq(p, a);

    return _PyAST_Call(
        func_name,
        args,
        NULL,
        lineno, col, end_lineno, end_col, arena
    );
}

expr_ty
_PyPegen_make_deep_copy_starred(Parser *p, expr_ty a) {
    expr_ty deep_call = _PyPegen_make_deep_copy_expr(p, a);
    if (deep_call == NULL) return NULL;
    return _PyAST_Starred(deep_call, Load, a->lineno, a->col_offset, a->end_lineno, a->end_col_offset, p->arena);
}

expr_ty
_PyPegen_make_implicit_target(Parser *p, int arity)


{
    PyArena *arena = p->arena;
    if (arity <= 1) {
        return _PyAST_Name(_PyPegen_new_identifier(p, "_dollar_item"), Store, 1, 0, 1, 0, arena);
    }
    if (arity == 2) {
        asdl_expr_seq *elts = _PyPegen_singleton_seq(p, _PyAST_Name(_PyPegen_new_identifier(p, "_dollar_item"), Store, 1, 0, 1, 0, arena));
        elts = (asdl_expr_seq *)_PyPegen_seq_append_to_end(p, elts, _PyAST_Name(_PyPegen_new_identifier(p, "_dollar_item_2"), Store, 1, 0, 1, 0, arena));
        return _PyAST_Tuple(elts, Store, 1, 0, 1, 0, arena);
    }
    asdl_expr_seq *elts = _PyPegen_singleton_seq(p, _PyAST_Name(_PyPegen_new_identifier(p, "_dollar_item"), Store, 1, 0, 1, 0, arena));
    elts = (asdl_expr_seq *)_PyPegen_seq_append_to_end(p, elts, _PyAST_Name(_PyPegen_new_identifier(p, "_dollar_item_2"), Store, 1, 0, 1, 0, arena));
    elts = (asdl_expr_seq *)_PyPegen_seq_append_to_end(p, elts, _PyAST_Name(_PyPegen_new_identifier(p, "_dollar_item_3"), Store, 1, 0, 1, 0, arena));
    return _PyAST_Tuple(elts, Store, 1, 0, 1, 0, arena);
}

static int
_PyPegen_detect_dollar_arity(expr_ty expr)
{
    if (expr == NULL) return 1;
    switch (expr->kind) {
    case Name_kind:
        if (expr->v.Name.id == NULL) return 1;
        const char *id = PyUnicode_AsUTF8(expr->v.Name.id);
        if (id != NULL) {
            if (strcmp(id, "_dollar_item_3") == 0) return 3;
            if (strcmp(id, "_dollar_item_2") == 0) return 2;
        }
        return 1;
    case BinOp_kind: {
        int r1 = _PyPegen_detect_dollar_arity(expr->v.BinOp.left);
        int r2 = _PyPegen_detect_dollar_arity(expr->v.BinOp.right);
        return r1 > r2 ? r1 : r2;
    }
    case UnaryOp_kind:
        return _PyPegen_detect_dollar_arity(expr->v.UnaryOp.operand);
    case Attribute_kind:
        return _PyPegen_detect_dollar_arity(expr->v.Attribute.value);
    case Subscript_kind: {
        int r1 = _PyPegen_detect_dollar_arity(expr->v.Subscript.value);
        int r2 = _PyPegen_detect_dollar_arity(expr->v.Subscript.slice);
        return r1 > r2 ? r1 : r2;
    }
    case Compare_kind: {
        int max = _PyPegen_detect_dollar_arity(expr->v.Compare.left);
        if (expr->v.Compare.comparators) {
            for (int i = 0; i < asdl_seq_LEN(expr->v.Compare.comparators); i++) {
                int r = _PyPegen_detect_dollar_arity((expr_ty)asdl_seq_GET(expr->v.Compare.comparators, i));
                if (r > max) max = r;
            }
        }
        return max;
    }
    case Call_kind: {
        int max = _PyPegen_detect_dollar_arity(expr->v.Call.func);
        if (expr->v.Call.args) {
            for (int i = 0; i < asdl_seq_LEN(expr->v.Call.args); i++) {
                int r = _PyPegen_detect_dollar_arity((expr_ty)asdl_seq_GET(expr->v.Call.args, i));
                if (r > max) max = r;
            }
        }
        return max;
    }
    case FormattedValue_kind:
        return _PyPegen_detect_dollar_arity(expr->v.FormattedValue.value);
    case JoinedStr_kind: {
        int max = 1;
        if (expr->v.JoinedStr.values) {
            for (int i = 0; i < asdl_seq_LEN(expr->v.JoinedStr.values); i++) {
                int r = _PyPegen_detect_dollar_arity((expr_ty)asdl_seq_GET(expr->v.JoinedStr.values, i));
                if (r > max) max = r;
            }
        }
        return max;
    }
    case Tuple_kind: {
        int max = 1;
        if (expr->v.Tuple.elts) {
            for (int i = 0; i < asdl_seq_LEN(expr->v.Tuple.elts); i++) {
                int r = _PyPegen_detect_dollar_arity((expr_ty)asdl_seq_GET(expr->v.Tuple.elts, i));
                if (r > max) max = r;
            }
        }
        return max;
    }
    case List_kind: {
        int max = 1;
        if (expr->v.List.elts) {
            for (int i = 0; i < asdl_seq_LEN(expr->v.List.elts); i++) {
                int r = _PyPegen_detect_dollar_arity((expr_ty)asdl_seq_GET(expr->v.List.elts, i));
                if (r > max) max = r;
            }
        }
        return max;
    }
    case Dict_kind: {
        int max = 1;
        if (expr->v.Dict.keys) {
            for (int i = 0; i < asdl_seq_LEN(expr->v.Dict.keys); i++) {
                expr_ty k = (expr_ty)asdl_seq_GET(expr->v.Dict.keys, i);
                if (k) {
                    int r = _PyPegen_detect_dollar_arity(k);
                    if (r > max) max = r;
                }
            }
        }
        if (expr->v.Dict.values) {
            for (int i = 0; i < asdl_seq_LEN(expr->v.Dict.values); i++) {
                expr_ty v = (expr_ty)asdl_seq_GET(expr->v.Dict.values, i);
                if (v) {
                    int r = _PyPegen_detect_dollar_arity(v);
                    if (r > max) max = r;
                }
            }
        }
        return max;
    }
    default:
        return 1;
    }
}

expr_ty
_PyPegen_make_implicit_target_auto(Parser *p, expr_ty elt)
{
    int arity = _PyPegen_detect_dollar_arity(elt);
    return _PyPegen_make_implicit_target(p, arity);
}

asdl_comprehension_seq *
_PyPegen_adjust_comprehension_arity(Parser *p, expr_ty elt, asdl_comprehension_seq *clauses)
{
    if (clauses == NULL || asdl_seq_LEN(clauses) == 0) return clauses;
    comprehension_ty first = (comprehension_ty)asdl_seq_GET(clauses, 0);
    if (first->target && first->target->kind == Name_kind && first->target->v.Name.id) {
        const char *id = PyUnicode_AsUTF8(first->target->v.Name.id);
        if (id && strcmp(id, "_dollar_item") == 0) {
            int arity = _PyPegen_detect_dollar_arity(elt);
            if (arity > 1) {
                first->target = _PyPegen_make_implicit_target(p, arity);
            }
        }
    }
    return clauses;
}

asdl_comprehension_seq *
_PyPegen_adjust_dict_comprehension_arity(Parser *p, expr_ty key, expr_ty val, asdl_comprehension_seq *clauses)
{
    if (clauses == NULL || asdl_seq_LEN(clauses) == 0) return clauses;
    comprehension_ty first = (comprehension_ty)asdl_seq_GET(clauses, 0);
    if (first->target && first->target->kind == Name_kind && first->target->v.Name.id) {
        const char *id = PyUnicode_AsUTF8(first->target->v.Name.id);
        if (id && strcmp(id, "_dollar_item") == 0) {
            int r1 = _PyPegen_detect_dollar_arity(key);
            int r2 = _PyPegen_detect_dollar_arity(val);
            int arity = r1 > r2 ? r1 : r2;
            if (arity > 1) {
                first->target = _PyPegen_make_implicit_target(p, arity);
            }
        }
    }
    return clauses;
}

expr_ty
_PyPegen_make_task_spawn(Parser *p, expr_ty expr)
{
    if (expr == NULL) return NULL;
    PyArena *arena = p->arena;
    int lineno = expr->lineno;
    int col = expr->col_offset;
    int end_lineno = expr->end_lineno;
    int end_col = expr->end_col_offset;

    expr_ty asyncio_name = _PyAST_Name(
        _PyPegen_new_identifier(p, "asyncio"),
        Load,
        lineno, col, end_lineno, end_col, arena
    );
    expr_ty func_attr = _PyAST_Attribute(
        asyncio_name,
        _PyPegen_new_identifier(p, "create_task"),
        Load,
        lineno, col, end_lineno, end_col, arena
    );
    asdl_expr_seq *args = (asdl_expr_seq *)_PyPegen_singleton_seq(p, expr);
    return _PyAST_Call(
        func_attr,
        args,
        NULL,
        lineno, col, end_lineno, end_col, arena
    );
}

expr_ty
_PyPegen_make_task_gather(Parser *p, expr_ty expr)
{
    if (expr == NULL) return NULL;
    PyArena *arena = p->arena;
    int lineno = expr->lineno;
    int col = expr->col_offset;
    int end_lineno = expr->end_lineno;
    int end_col = expr->end_col_offset;

    if (expr->kind == List_kind || expr->kind == Tuple_kind || expr->kind == Starred_kind) {
        expr_ty asyncio_name = _PyAST_Name(
            _PyPegen_new_identifier(p, "asyncio"),
            Load,
            lineno, col, end_lineno, end_col, arena
        );
        expr_ty func_attr = _PyAST_Attribute(
            asyncio_name,
            _PyPegen_new_identifier(p, "gather"),
            Load,
            lineno, col, end_lineno, end_col, arena
        );

        asdl_expr_seq *args = NULL;
        if (expr->kind == List_kind) {
            args = expr->v.List.elts;
        }
        else if (expr->kind == Tuple_kind) {
            args = expr->v.Tuple.elts;
        }
        else {
            args = (asdl_expr_seq *)_PyPegen_singleton_seq(p, expr);
        }

        expr_ty call = _PyAST_Call(
            func_attr,
            args,
            NULL,
            lineno, col, end_lineno, end_col, arena
        );
        return _PyAST_Await(call, lineno, col, end_lineno, end_col, arena);
    }
    else {
        return _PyAST_Await(expr, lineno, col, end_lineno, end_col, arena);
    }
}










