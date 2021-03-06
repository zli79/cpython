#ifndef PEGEN_H
#define PEGEN_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <token.h>
#include <Python-ast.h>
#include <pyarena.h>

typedef struct _memo {
    int type;
    void *node;
    int mark;
    struct _memo *next;
} Memo;

typedef struct {
    int type;
    PyObject *bytes;
    int lineno, col_offset, end_lineno, end_col_offset;
    Memo *memo;
} Token;

typedef struct {
    char *str;
    int type;
} KeywordToken;

typedef struct {
    struct tok_state *tok;
    Token **tokens;
    int mark;
    int fill, size;
    PyArena *arena;
    KeywordToken **keywords;
    int n_keyword_lists;
    int start_rule;
    int *errcode;
    int parsing_started;
    PyObject* normalize;
    int starting_lineno;
    int starting_col_offset;
    int error_indicator;
} Parser;

typedef struct {
    cmpop_ty cmpop;
    expr_ty expr;
} CmpopExprPair;

typedef struct {
    expr_ty key;
    expr_ty value;
} KeyValuePair;

typedef struct {
    arg_ty arg;
    expr_ty value;
} NameDefaultPair;

typedef struct {
    asdl_seq *plain_names;
    asdl_seq *names_with_defaults; // asdl_seq* of NameDefaultsPair's
} SlashWithDefault;

typedef struct {
    arg_ty vararg;
    asdl_seq *kwonlyargs; // asdl_seq* of NameDefaultsPair's
    arg_ty kwarg;
} StarEtc;

typedef struct {
    operator_ty kind;
} AugOperator;

typedef struct {
    void *element;
    int is_keyword;
} KeywordOrStarred;

void _PyPegen_clear_memo_statistics(void);
PyObject *_PyPegen_get_memo_statistics(void);

int _PyPegen_insert_memo(Parser *p, int mark, int type, void *node);
int _PyPegen_update_memo(Parser *p, int mark, int type, void *node);
int _PyPegen_is_memoized(Parser *p, int type, void *pres);

int _PyPegen_lookahead_with_string(int, void *(func)(Parser *, const char *), Parser *, const char *);
int _PyPegen_lookahead_with_int(int, Token *(func)(Parser *, int), Parser *, int);
int _PyPegen_lookahead(int, void *(func)(Parser *), Parser *);

Token *_PyPegen_expect_token(Parser *p, int type);
Token *_PyPegen_get_last_nonnwhitespace_token(Parser *);
int _PyPegen_fill_token(Parser *p);
void *_PyPegen_async_token(Parser *p);
void *_PyPegen_await_token(Parser *p);
void *_PyPegen_endmarker_token(Parser *p);
expr_ty _PyPegen_name_token(Parser *p);
void *_PyPegen_newline_token(Parser *p);
void *_PyPegen_indent_token(Parser *p);
void *_PyPegen_dedent_token(Parser *p);
expr_ty _PyPegen_number_token(Parser *p);
void *_PyPegen_string_token(Parser *p);
const char *_PyPegen_get_expr_name(expr_ty);
void *_PyPegen_raise_error(Parser *p, PyObject *, const char *errmsg, ...);
void *_PyPegen_dummy_name(Parser *p, ...);

#define UNUSED(expr) do { (void)(expr); } while (0)
#define EXTRA_EXPR(head, tail) head->lineno, head->col_offset, tail->end_lineno, tail->end_col_offset, p->arena
#define EXTRA start_lineno, start_col_offset, end_lineno, end_col_offset, p->arena
#define RAISE_SYNTAX_ERROR(msg, ...) _PyPegen_raise_error(p, PyExc_SyntaxError, msg, ##__VA_ARGS__)
#define RAISE_INDENTATION_ERROR(msg, ...) _PyPegen_raise_error(p, PyExc_IndentationError, msg, ##__VA_ARGS__)

Py_LOCAL_INLINE(void *)
CHECK_CALL(Parser *p, void *result)
{
    if (result == NULL) {
        assert(PyErr_Occurred());
        p->error_indicator = 1;
    }
    return result;
}

/* This is needed for helper functions that are allowed to
   return NULL without an error. Example: _PyPegen_seq_extract_starred_exprs */
Py_LOCAL_INLINE(void *)
CHECK_CALL_NULL_ALLOWED(Parser *p, void *result)
{
    if (result == NULL && PyErr_Occurred()) {
        p->error_indicator = 1;
    }
    return result;
}

#define CHECK(result) CHECK_CALL(p, result)
#define CHECK_NULL_ALLOWED(result) CHECK_CALL_NULL_ALLOWED(p, result)

PyObject *_PyPegen_new_identifier(Parser *, char *);
Parser *_PyPegen_Parser_New(struct tok_state *, int, int *, PyArena *);
void _PyPegen_Parser_Free(Parser *);
mod_ty _PyPegen_run_parser_from_file_pointer(FILE *, int, PyObject *, const char *,
                                    const char *, const char *, int *, PyArena *);
void *_PyPegen_run_parser(Parser *);
mod_ty _PyPegen_run_parser_from_file(const char *, int, PyObject *, PyArena *);
mod_ty _PyPegen_run_parser_from_string(const char *, int, PyObject *, int, PyArena *);
void *_PyPegen_interactive_exit(Parser *);
asdl_seq *_PyPegen_singleton_seq(Parser *, void *);
asdl_seq *_PyPegen_seq_insert_in_front(Parser *, void *, asdl_seq *);
asdl_seq *_PyPegen_seq_flatten(Parser *, asdl_seq *);
expr_ty _PyPegen_join_names_with_dot(Parser *, expr_ty, expr_ty);
int _PyPegen_seq_count_dots(asdl_seq *);
alias_ty _PyPegen_alias_for_star(Parser *);
asdl_seq *_PyPegen_map_names_to_ids(Parser *, asdl_seq *);
CmpopExprPair *_PyPegen_cmpop_expr_pair(Parser *, cmpop_ty, expr_ty);
asdl_int_seq *_PyPegen_get_cmpops(Parser *p, asdl_seq *);
asdl_seq *_PyPegen_get_exprs(Parser *, asdl_seq *);
expr_ty _PyPegen_set_expr_context(Parser *, expr_ty, expr_context_ty);
KeyValuePair *_PyPegen_key_value_pair(Parser *, expr_ty, expr_ty);
asdl_seq *_PyPegen_get_keys(Parser *, asdl_seq *);
asdl_seq *_PyPegen_get_values(Parser *, asdl_seq *);
NameDefaultPair *_PyPegen_name_default_pair(Parser *, arg_ty, expr_ty);
SlashWithDefault *_PyPegen_slash_with_default(Parser *, asdl_seq *, asdl_seq *);
StarEtc *_PyPegen_star_etc(Parser *, arg_ty, asdl_seq *, arg_ty);
arguments_ty _PyPegen_make_arguments(Parser *, asdl_seq *, SlashWithDefault *,
                            asdl_seq *, asdl_seq *, StarEtc *);
arguments_ty _PyPegen_empty_arguments(Parser *);
AugOperator *_PyPegen_augoperator(Parser*, operator_ty type);
stmt_ty _PyPegen_function_def_decorators(Parser *, asdl_seq *, stmt_ty);
stmt_ty _PyPegen_class_def_decorators(Parser *, asdl_seq *, stmt_ty);
KeywordOrStarred *_PyPegen_keyword_or_starred(Parser *, void *, int);
asdl_seq *_PyPegen_seq_extract_starred_exprs(Parser *, asdl_seq *);
asdl_seq *_PyPegen_seq_delete_starred_exprs(Parser *, asdl_seq *);
expr_ty _PyPegen_concatenate_strings(Parser *p, asdl_seq *);
asdl_seq *_PyPegen_join_sequences(Parser *, asdl_seq *, asdl_seq *);
void *_PyPegen_arguments_parsing_error(Parser *, expr_ty);

void *_PyPegen_parse(Parser *);

#endif
