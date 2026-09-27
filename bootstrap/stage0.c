#include "runtime.h"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"

FLValue is_digit_p(FLValue c);
static FLValue __fl_wrap_is_digit_p(FLClosure*, int, FLValue*);
FLValue is_alpha_p(FLValue c);
static FLValue __fl_wrap_is_alpha_p(FLClosure*, int, FLValue*);
FLValue is_alnum_p(FLValue c);
static FLValue __fl_wrap_is_alnum_p(FLClosure*, int, FLValue*);
FLValue is_space_p(FLValue c);
static FLValue __fl_wrap_is_space_p(FLClosure*, int, FLValue*);
FLValue is_symbol_char_p(FLValue c);
static FLValue __fl_wrap_is_symbol_char_p(FLClosure*, int, FLValue*);
FLValue make_state(FLValue src);
static FLValue __fl_wrap_make_state(FLClosure*, int, FLValue*);
FLValue peek_at(FLValue st, FLValue offset);
static FLValue __fl_wrap_peek_at(FLClosure*, int, FLValue*);
FLValue peek(FLValue st);
static FLValue __fl_wrap_peek(FLClosure*, int, FLValue*);
FLValue at_end_p(FLValue st);
static FLValue __fl_wrap_at_end_p(FLClosure*, int, FLValue*);
FLValue advance(FLValue st);
static FLValue __fl_wrap_advance(FLClosure*, int, FLValue*);
FLValue emit(FLValue st, FLValue kind, FLValue value, FLValue sl, FLValue sc);
static FLValue __fl_wrap_emit(FLClosure*, int, FLValue*);
FLValue skip_comment_loop(FLValue _cur);
static FLValue __fl_wrap_skip_comment_loop(FLClosure*, int, FLValue*);
FLValue skip_comment(FLValue st);
static FLValue __fl_wrap_skip_comment(FLClosure*, int, FLValue*);
FLValue skip_ws_loop(FLValue _cur);
static FLValue __fl_wrap_skip_ws_loop(FLClosure*, int, FLValue*);
FLValue skip_ws(FLValue st);
static FLValue __fl_wrap_skip_ws(FLClosure*, int, FLValue*);
FLValue read_number_iter(FLValue _cur, FLValue _res_acc, FLValue _dot, FLValue line, FLValue col);
static FLValue __fl_wrap_read_number_iter(FLClosure*, int, FLValue*);
FLValue read_number_body(FLValue st, FLValue acc, FLValue has_dot, FLValue line, FLValue col);
static FLValue __fl_wrap_read_number_body(FLClosure*, int, FLValue*);
FLValue read_number(FLValue st);
static FLValue __fl_wrap_read_number(FLClosure*, int, FLValue*);
FLValue translate_esc(FLValue c);
static FLValue __fl_wrap_translate_esc(FLClosure*, int, FLValue*);
FLValue read_string_iter(FLValue _cur, FLValue _res_acc, FLValue line, FLValue col);
static FLValue __fl_wrap_read_string_iter(FLClosure*, int, FLValue*);
FLValue read_string_body(FLValue st, FLValue acc, FLValue line, FLValue col);
static FLValue __fl_wrap_read_string_body(FLClosure*, int, FLValue*);
FLValue read_string(FLValue st);
static FLValue __fl_wrap_read_string(FLClosure*, int, FLValue*);
FLValue read_symbol_iter(FLValue _cur, FLValue _res_acc, FLValue line, FLValue col, FLValue kind);
static FLValue __fl_wrap_read_symbol_iter(FLClosure*, int, FLValue*);
FLValue read_symbol_body_kind(FLValue st, FLValue acc, FLValue line, FLValue col, FLValue kind);
static FLValue __fl_wrap_read_symbol_body_kind(FLClosure*, int, FLValue*);
FLValue read_symbol(FLValue st);
static FLValue __fl_wrap_read_symbol(FLClosure*, int, FLValue*);
FLValue read_variable(FLValue st);
static FLValue __fl_wrap_read_variable(FLClosure*, int, FLValue*);
FLValue read_keyword(FLValue st);
static FLValue __fl_wrap_read_keyword(FLClosure*, int, FLValue*);
FLValue read_token(FLValue st);
static FLValue __fl_wrap_read_token(FLClosure*, int, FLValue*);
FLValue lex_loop(FLValue _cur);
static FLValue __fl_wrap_lex_loop(FLClosure*, int, FLValue*);
FLValue lex(FLValue src);
static FLValue __fl_wrap_lex(FLClosure*, int, FLValue*);
FLValue make_literal(FLValue __fl_kw_type, FLValue value, FLValue line);
static FLValue __fl_wrap_make_literal(FLClosure*, int, FLValue*);
FLValue make_variable(FLValue name, FLValue line);
static FLValue __fl_wrap_make_variable(FLClosure*, int, FLValue*);
FLValue make_keyword(FLValue name, FLValue line);
static FLValue __fl_wrap_make_keyword(FLClosure*, int, FLValue*);
FLValue make_sexpr(FLValue op, FLValue args, FLValue line);
static FLValue __fl_wrap_make_sexpr(FLClosure*, int, FLValue*);
FLValue make_number(FLValue v, FLValue line);
static FLValue __fl_wrap_make_number(FLClosure*, int, FLValue*);
FLValue make_string(FLValue v, FLValue line);
static FLValue __fl_wrap_make_string(FLClosure*, int, FLValue*);
FLValue make_bool(FLValue v, FLValue line);
static FLValue __fl_wrap_make_bool(FLClosure*, int, FLValue*);
FLValue make_null(FLValue line);
static FLValue __fl_wrap_make_null(FLClosure*, int, FLValue*);
FLValue make_symbol(FLValue v, FLValue line);
static FLValue __fl_wrap_make_symbol(FLClosure*, int, FLValue*);
FLValue make_block(FLValue __fl_kw_type, FLValue name, FLValue fields, FLValue line);
static FLValue __fl_wrap_make_block(FLClosure*, int, FLValue*);
FLValue make_array_block(FLValue items, FLValue line);
static FLValue __fl_wrap_make_array_block(FLClosure*, int, FLValue*);
FLValue make_map_block(FLValue items, FLValue line);
static FLValue __fl_wrap_make_map_block(FLClosure*, int, FLValue*);
FLValue make_pattern_literal(FLValue value, FLValue line);
static FLValue __fl_wrap_make_pattern_literal(FLClosure*, int, FLValue*);
FLValue make_pattern_variable(FLValue name, FLValue line);
static FLValue __fl_wrap_make_pattern_variable(FLClosure*, int, FLValue*);
FLValue make_pattern_wildcard(FLValue line);
static FLValue __fl_wrap_make_pattern_wildcard(FLClosure*, int, FLValue*);
FLValue make_pattern_list(FLValue items, FLValue __fl_kw_rest, FLValue line);
static FLValue __fl_wrap_make_pattern_list(FLClosure*, int, FLValue*);
FLValue make_pattern_struct(FLValue type_name, FLValue fields, FLValue line);
static FLValue __fl_wrap_make_pattern_struct(FLClosure*, int, FLValue*);
FLValue make_pattern_or(FLValue alternatives, FLValue line);
static FLValue __fl_wrap_make_pattern_or(FLClosure*, int, FLValue*);
FLValue make_pattern_range(FLValue start, FLValue end, FLValue line);
static FLValue __fl_wrap_make_pattern_range(FLClosure*, int, FLValue*);
FLValue make_pattern_match(FLValue value, FLValue cases, FLValue line);
static FLValue __fl_wrap_make_pattern_match(FLClosure*, int, FLValue*);
FLValue make_match_case(FLValue pattern, FLValue guard, FLValue body, FLValue line);
static FLValue __fl_wrap_make_match_case(FLClosure*, int, FLValue*);
FLValue make_function_value(FLValue params, FLValue body, FLValue captured_env, FLValue name);
static FLValue __fl_wrap_make_function_value(FLClosure*, int, FLValue*);
FLValue make_type_class(FLValue name, FLValue generics, FLValue methods, FLValue line);
static FLValue __fl_wrap_make_type_class(FLClosure*, int, FLValue*);
FLValue make_type_class_instance(FLValue class_name, FLValue type_name, FLValue impls, FLValue line);
static FLValue __fl_wrap_make_type_class_instance(FLClosure*, int, FLValue*);
FLValue make_module_block(FLValue name, FLValue exports, FLValue body, FLValue line);
static FLValue __fl_wrap_make_module_block(FLClosure*, int, FLValue*);
FLValue make_import_block(FLValue path, FLValue alias, FLValue names, FLValue line);
static FLValue __fl_wrap_make_import_block(FLClosure*, int, FLValue*);
FLValue make_open_block(FLValue module_name, FLValue line);
static FLValue __fl_wrap_make_open_block(FLClosure*, int, FLValue*);
FLValue make_search_block(FLValue query, FLValue fields, FLValue line);
static FLValue __fl_wrap_make_search_block(FLClosure*, int, FLValue*);
FLValue make_learn_block(FLValue topic, FLValue fields, FLValue line);
static FLValue __fl_wrap_make_learn_block(FLClosure*, int, FLValue*);
FLValue make_reasoning_block(FLValue name, FLValue fields, FLValue line);
static FLValue __fl_wrap_make_reasoning_block(FLClosure*, int, FLValue*);
FLValue make_async_function(FLValue name, FLValue params, FLValue body, FLValue line);
static FLValue __fl_wrap_make_async_function(FLClosure*, int, FLValue*);
FLValue make_await(FLValue expr, FLValue line);
static FLValue __fl_wrap_make_await(FLClosure*, int, FLValue*);
FLValue make_try(FLValue body, FLValue catch, FLValue finally, FLValue line);
static FLValue __fl_wrap_make_try(FLClosure*, int, FLValue*);
FLValue make_catch(FLValue param, FLValue body, FLValue line);
static FLValue __fl_wrap_make_catch(FLClosure*, int, FLValue*);
FLValue make_throw(FLValue expr, FLValue line);
static FLValue __fl_wrap_make_throw(FLClosure*, int, FLValue*);
FLValue make_template_string(FLValue value, FLValue expressions, FLValue line);
static FLValue __fl_wrap_make_template_string(FLClosure*, int, FLValue*);
FLValue make_loop(FLValue init, FLValue condition, FLValue __fl_kw_update, FLValue body, FLValue line);
static FLValue __fl_wrap_make_loop(FLClosure*, int, FLValue*);
FLValue make_page(FLValue name, FLValue path, FLValue fields, FLValue line);
static FLValue __fl_wrap_make_page(FLClosure*, int, FLValue*);
FLValue make_route(FLValue method, FLValue path, FLValue handler, FLValue line);
static FLValue __fl_wrap_make_route(FLClosure*, int, FLValue*);
FLValue make_component(FLValue name, FLValue fields, FLValue line);
static FLValue __fl_wrap_make_component(FLClosure*, int, FLValue*);
FLValue make_form(FLValue name, FLValue fields, FLValue line);
static FLValue __fl_wrap_make_form(FLClosure*, int, FLValue*);
FLValue deep_equal_p(FLValue a, FLValue b);
static FLValue __fl_wrap_deep_equal_p(FLClosure*, int, FLValue*);
FLValue deep_equal_list_p(FLValue a, FLValue b, FLValue i);
static FLValue __fl_wrap_deep_equal_list_p(FLClosure*, int, FLValue*);
FLValue deep_equal_map_p(FLValue a, FLValue b);
static FLValue __fl_wrap_deep_equal_map_p(FLClosure*, int, FLValue*);
FLValue keys_no_line(FLValue m);
static FLValue __fl_wrap_keys_no_line(FLClosure*, int, FLValue*);
FLValue deep_equal_map_keys_p(FLValue a, FLValue b, FLValue ks, FLValue i);
static FLValue __fl_wrap_deep_equal_map_keys_p(FLClosure*, int, FLValue*);
FLValue json_keys(FLValue m);
static FLValue __fl_wrap_json_keys(FLClosure*, int, FLValue*);
FLValue p_make(FLValue tokens);
static FLValue __fl_wrap_p_make(FLClosure*, int, FLValue*);
FLValue p_peek(FLValue p);
static FLValue __fl_wrap_p_peek(FLClosure*, int, FLValue*);
FLValue p_peek_at(FLValue p, FLValue offset);
static FLValue __fl_wrap_p_peek_at(FLClosure*, int, FLValue*);
FLValue p_end_p(FLValue p);
static FLValue __fl_wrap_p_end_p(FLClosure*, int, FLValue*);
FLValue p_advance(FLValue p);
static FLValue __fl_wrap_p_advance(FLClosure*, int, FLValue*);
FLValue p_with_ast(FLValue p, FLValue ast);
static FLValue __fl_wrap_p_with_ast(FLClosure*, int, FLValue*);
FLValue p_append_ast(FLValue p, FLValue node);
static FLValue __fl_wrap_p_append_ast(FLClosure*, int, FLValue*);
FLValue r_pair(FLValue p, FLValue node);
static FLValue __fl_wrap_r_pair(FLClosure*, int, FLValue*);
FLValue string_contains_p(FLValue s, FLValue substr);
static FLValue __fl_wrap_string_contains_p(FLClosure*, int, FLValue*);
FLValue parse_atom(FLValue p);
static FLValue __fl_wrap_parse_atom(FLClosure*, int, FLValue*);
FLValue hash_fn_op(FLValue node);
static FLValue __fl_wrap_hash_fn_op(FLClosure*, int, FLValue*);
FLValue replace_pct(FLValue node);
static FLValue __fl_wrap_replace_pct(FLClosure*, int, FLValue*);
FLValue replace_pct_list(FLValue lst, FLValue i, FLValue acc);
static FLValue __fl_wrap_replace_pct_list(FLClosure*, int, FLValue*);
FLValue parse_hash_fn(FLValue p);
static FLValue __fl_wrap_parse_hash_fn(FLClosure*, int, FLValue*);
FLValue make_pipe_call(FLValue lhs, FLValue rhs, FLValue line);
static FLValue __fl_wrap_make_pipe_call(FLClosure*, int, FLValue*);
FLValue parse_pipe_chain(FLValue p, FLValue lhs);
static FLValue __fl_wrap_parse_pipe_chain(FLClosure*, int, FLValue*);
FLValue parse_expr_base(FLValue p);
static FLValue __fl_wrap_parse_expr_base(FLClosure*, int, FLValue*);
FLValue parse_expr(FLValue p);
static FLValue __fl_wrap_parse_expr(FLClosure*, int, FLValue*);
FLValue parse_sexpr(FLValue p);
static FLValue __fl_wrap_parse_sexpr(FLClosure*, int, FLValue*);
FLValue parse_consume_rparen(FLValue p);
static FLValue __fl_wrap_parse_consume_rparen(FLClosure*, int, FLValue*);
FLValue parse_args(FLValue p, FLValue acc);
static FLValue __fl_wrap_parse_args(FLClosure*, int, FLValue*);
FLValue parse_bracket(FLValue p);
static FLValue __fl_wrap_parse_bracket(FLClosure*, int, FLValue*);
FLValue is_block_type_p(FLValue s);
static FLValue __fl_wrap_is_block_type_p(FLClosure*, int, FLValue*);
FLValue upper_case(FLValue s);
static FLValue __fl_wrap_upper_case(FLClosure*, int, FLValue*);
FLValue parse_array(FLValue p, FLValue line);
static FLValue __fl_wrap_parse_array(FLClosure*, int, FLValue*);
FLValue parse_consume_rbracket(FLValue p);
static FLValue __fl_wrap_parse_consume_rbracket(FLClosure*, int, FLValue*);
FLValue parse_named_block(FLValue p, FLValue line);
static FLValue __fl_wrap_parse_named_block(FLClosure*, int, FLValue*);
FLValue parse_optional_name(FLValue p);
static FLValue __fl_wrap_parse_optional_name(FLClosure*, int, FLValue*);
FLValue parse_block_fields(FLValue p, FLValue acc);
static FLValue __fl_wrap_parse_block_fields(FLClosure*, int, FLValue*);
FLValue parse_map(FLValue p);
static FLValue __fl_wrap_parse_map(FLClosure*, int, FLValue*);
FLValue parse_consume_rbrace(FLValue p);
static FLValue __fl_wrap_parse_consume_rbrace(FLClosure*, int, FLValue*);
FLValue parse_all(FLValue p);
static FLValue __fl_wrap_parse_all(FLClosure*, int, FLValue*);
FLValue parse(FLValue tokens);
static FLValue __fl_wrap_parse(FLClosure*, int, FLValue*);
FLValue get_block_items(FLValue node);
static FLValue __fl_wrap_get_block_items(FLClosure*, int, FLValue*);
FLValue c_esc(FLValue s);
static FLValue __fl_wrap_c_esc(FLClosure*, int, FLValue*);
FLValue c_reserved_p(FLValue s);
static FLValue __fl_wrap_c_reserved_p(FLClosure*, int, FLValue*);
FLValue c_name(FLValue n);
static FLValue __fl_wrap_c_name(FLClosure*, int, FLValue*);
FLValue cgc(FLValue n);
static FLValue __fl_wrap_cgc(FLClosure*, int, FLValue*);
FLValue cgc_op_wrapper(FLValue sym);
static FLValue __fl_wrap_cgc_op_wrapper(FLClosure*, int, FLValue*);
FLValue template_to_parts(FLValue s, FLValue prefix, FLValue acc);
static FLValue __fl_wrap_template_to_parts(FLClosure*, int, FLValue*);
FLValue cgc_template_string(FLValue n);
static FLValue __fl_wrap_cgc_template_string(FLClosure*, int, FLValue*);
FLValue cgc_literal(FLValue n);
static FLValue __fl_wrap_cgc_literal(FLClosure*, int, FLValue*);
FLValue cgc_block(FLValue n);
static FLValue __fl_wrap_cgc_block(FLClosure*, int, FLValue*);
FLValue cgc_func_block(FLValue n);
static FLValue __fl_wrap_cgc_func_block(FLClosure*, int, FLValue*);
FLValue cgc_params(FLValue it);
static FLValue __fl_wrap_cgc_params(FLClosure*, int, FLValue*);
FLValue cgc_params_loop(FLValue _it, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_cgc_params_loop(FLClosure*, int, FLValue*);
FLValue cgc_extract_name(FLValue node);
static FLValue __fl_wrap_cgc_extract_name(FLClosure*, int, FLValue*);
FLValue cgc_try(FLValue n);
static FLValue __fl_wrap_cgc_try(FLClosure*, int, FLValue*);
FLValue cgc_fncall(FLValue fn_c, FLValue args);
static FLValue __fl_wrap_cgc_fncall(FLClosure*, int, FLValue*);
FLValue cgc_sexpr(FLValue n);
static FLValue __fl_wrap_cgc_sexpr(FLClosure*, int, FLValue*);
FLValue cgc_dispatch(FLValue op, FLValue args);
static FLValue __fl_wrap_cgc_dispatch(FLClosure*, int, FLValue*);
FLValue cgc_swap_b(FLValue args);
static FLValue __fl_wrap_cgc_swap_b(FLClosure*, int, FLValue*);
FLValue cgc_is_get_p(FLValue n);
static FLValue __fl_wrap_cgc_is_get_p(FLClosure*, int, FLValue*);
FLValue cgc_warn_nil_get(FLValue op, FLValue args);
static FLValue __fl_wrap_cgc_warn_nil_get(FLClosure*, int, FLValue*);
FLValue cgc_is_str_lit_p(FLValue n);
static FLValue __fl_wrap_cgc_is_str_lit_p(FLClosure*, int, FLValue*);
FLValue cgc_is_num_lit_p(FLValue n);
static FLValue __fl_wrap_cgc_is_num_lit_p(FLClosure*, int, FLValue*);
FLValue cgc_warn_type_mix(FLValue op, FLValue args);
static FLValue __fl_wrap_cgc_warn_type_mix(FLClosure*, int, FLValue*);
FLValue cgc_dispatch_fallback(FLValue op, FLValue args);
static FLValue __fl_wrap_cgc_dispatch_fallback(FLClosure*, int, FLValue*);
FLValue cgc_fn_argv_decls(FLValue items, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_fn_argv_decls(FLClosure*, int, FLValue*);
FLValue cgc_fn_param_names(FLValue items, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_fn_param_names(FLClosure*, int, FLValue*);
FLValue cgc_collect_vars(FLValue node, FLValue acc);
static FLValue __fl_wrap_cgc_collect_vars(FLClosure*, int, FLValue*);
FLValue cgc_collect_vars_loop(FLValue _args, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_cgc_collect_vars_loop(FLClosure*, int, FLValue*);
FLValue cgc_fn_env_decls(FLValue _caps, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_cgc_fn_env_decls(FLClosure*, int, FLValue*);
FLValue cgc_env_arr(FLValue _caps, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_cgc_env_arr(FLClosure*, int, FLValue*);
FLValue cgc_fn_caps_filter(FLValue all_vars, FLValue param_names, FLValue outer, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_fn_caps_filter(FLClosure*, int, FLValue*);
FLValue cgc_fn(FLValue args);
static FLValue __fl_wrap_cgc_fn(FLClosure*, int, FLValue*);
FLValue cgc_list(FLValue args);
static FLValue __fl_wrap_cgc_list(FLClosure*, int, FLValue*);
FLValue cgc_str(FLValue args);
static FLValue __fl_wrap_cgc_str(FLClosure*, int, FLValue*);
FLValue cgc_str_arg(FLValue args);
static FLValue __fl_wrap_cgc_str_arg(FLClosure*, int, FLValue*);
FLValue cgc_if(FLValue args);
static FLValue __fl_wrap_cgc_if(FLClosure*, int, FLValue*);
FLValue cgc_cond(FLValue args);
static FLValue __fl_wrap_cgc_cond(FLClosure*, int, FLValue*);
FLValue cgc_cond_nested(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_cond_nested(FLClosure*, int, FLValue*);
FLValue cgc_cond_flat(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_cond_flat(FLClosure*, int, FLValue*);
FLValue cgc_do(FLValue args);
static FLValue __fl_wrap_cgc_do(FLClosure*, int, FLValue*);
FLValue cgc_let(FLValue args);
static FLValue __fl_wrap_cgc_let(FLClosure*, int, FLValue*);
FLValue cgc_let_unique_name(FLValue n);
static FLValue __fl_wrap_cgc_let_unique_name(FLClosure*, int, FLValue*);
FLValue cgc_let_1d(FLValue it, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_let_1d(FLClosure*, int, FLValue*);
FLValue cgc_let_2d(FLValue it, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_let_2d(FLClosure*, int, FLValue*);
FLValue cgc_body(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_body(FLClosure*, int, FLValue*);
FLValue cgc_defn_stmts(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_defn_stmts(FLClosure*, int, FLValue*);
FLValue cgc_node_has_recur(FLValue node);
static FLValue __fl_wrap_cgc_node_has_recur(FLClosure*, int, FLValue*);
FLValue cgc_any_has_recur(FLValue args, FLValue i);
static FLValue __fl_wrap_cgc_any_has_recur(FLClosure*, int, FLValue*);
FLValue cgc_recur_goto_stmt(FLValue args, FLValue vars, FLValue label);
static FLValue __fl_wrap_cgc_recur_goto_stmt(FLClosure*, int, FLValue*);
FLValue cgc_with_recur_goto(FLValue node, FLValue vars, FLValue label);
static FLValue __fl_wrap_cgc_with_recur_goto(FLClosure*, int, FLValue*);
FLValue cgc_if_wr_goto(FLValue args, FLValue vars, FLValue label);
static FLValue __fl_wrap_cgc_if_wr_goto(FLClosure*, int, FLValue*);
FLValue cgc_cond_wr_goto(FLValue args, FLValue vars, FLValue label);
static FLValue __fl_wrap_cgc_cond_wr_goto(FLClosure*, int, FLValue*);
FLValue cgc_cond_nested_wr_goto(FLValue args, FLValue vars, FLValue label, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_cond_nested_wr_goto(FLClosure*, int, FLValue*);
FLValue cgc_thread_first(FLValue args);
static FLValue __fl_wrap_cgc_thread_first(FLClosure*, int, FLValue*);
FLValue cgc_tf_build(FLValue inner, FLValue args, FLValue i);
static FLValue __fl_wrap_cgc_tf_build(FLClosure*, int, FLValue*);
FLValue cgc_thread_last(FLValue args);
static FLValue __fl_wrap_cgc_thread_last(FLClosure*, int, FLValue*);
FLValue cgc_tl_build(FLValue inner, FLValue args, FLValue i);
static FLValue __fl_wrap_cgc_tl_build(FLClosure*, int, FLValue*);
FLValue cgc_case(FLValue args);
static FLValue __fl_wrap_cgc_case(FLClosure*, int, FLValue*);
FLValue cgc_case_rev(FLValue val_c, FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_case_rev(FLClosure*, int, FLValue*);
FLValue cgc_if_let(FLValue args);
static FLValue __fl_wrap_cgc_if_let(FLClosure*, int, FLValue*);
FLValue cgc_when_let(FLValue args);
static FLValue __fl_wrap_cgc_when_let(FLClosure*, int, FLValue*);
FLValue match_wildcard_p(FLValue node);
static FLValue __fl_wrap_match_wildcard_p(FLClosure*, int, FLValue*);
FLValue match_bind_var_p(FLValue node);
static FLValue __fl_wrap_match_bind_var_p(FLClosure*, int, FLValue*);
FLValue match_vec_pat_p(FLValue node);
static FLValue __fl_wrap_match_vec_pat_p(FLClosure*, int, FLValue*);
FLValue match_pat_nil_p(FLValue node);
static FLValue __fl_wrap_match_pat_nil_p(FLClosure*, int, FLValue*);
FLValue match_pat_var_p(FLValue node);
static FLValue __fl_wrap_match_pat_var_p(FLClosure*, int, FLValue*);
FLValue cgc_match_vec_cond(FLValue val_c, FLValue pat_node);
static FLValue __fl_wrap_cgc_match_vec_cond(FLClosure*, int, FLValue*);
FLValue cgc_match_rev(FLValue val_c, FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_match_rev(FLClosure*, int, FLValue*);
FLValue cgc_match(FLValue args);
static FLValue __fl_wrap_cgc_match(FLClosure*, int, FLValue*);
FLValue cgc_doseq(FLValue args);
static FLValue __fl_wrap_cgc_doseq(FLClosure*, int, FLValue*);
FLValue cgc_dotimes(FLValue args);
static FLValue __fl_wrap_cgc_dotimes(FLClosure*, int, FLValue*);
FLValue cgc_doto(FLValue args);
static FLValue __fl_wrap_cgc_doto(FLClosure*, int, FLValue*);
FLValue cgc_doto_calls(FLValue obj_c, FLValue forms, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_doto_calls(FLClosure*, int, FLValue*);
FLValue cgc_for(FLValue args);
static FLValue __fl_wrap_cgc_for(FLClosure*, int, FLValue*);
FLValue cgc_cond_flat_wr_goto(FLValue args, FLValue vars, FLValue label, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_cond_flat_wr_goto(FLClosure*, int, FLValue*);
FLValue cgc_do_wr_goto(FLValue args, FLValue vars, FLValue label);
static FLValue __fl_wrap_cgc_do_wr_goto(FLClosure*, int, FLValue*);
FLValue cgc_stmts_wr_goto(FLValue args, FLValue vars, FLValue label, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_stmts_wr_goto(FLClosure*, int, FLValue*);
FLValue cgc_let_wr_goto(FLValue args, FLValue vars, FLValue label);
static FLValue __fl_wrap_cgc_let_wr_goto(FLClosure*, int, FLValue*);
FLValue cgc_body_wr_goto(FLValue args, FLValue vars, FLValue label, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_body_wr_goto(FLClosure*, int, FLValue*);
FLValue cgc_defn_stmts_tco(FLValue args, FLValue vars, FLValue label, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_defn_stmts_tco(FLClosure*, int, FLValue*);
FLValue cgc_defn(FLValue args);
static FLValue __fl_wrap_cgc_defn(FLClosure*, int, FLValue*);
FLValue cgc_define(FLValue args);
static FLValue __fl_wrap_cgc_define(FLClosure*, int, FLValue*);
FLValue cgc_binop_chain(FLValue args, FLValue fn);
static FLValue __fl_wrap_cgc_binop_chain(FLClosure*, int, FLValue*);
FLValue cgc_binop_fold(FLValue args, FLValue fn, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_binop_fold(FLClosure*, int, FLValue*);
FLValue cgc_and(FLValue args);
static FLValue __fl_wrap_cgc_and(FLClosure*, int, FLValue*);
FLValue cgc_and_fold(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_and_fold(FLClosure*, int, FLValue*);
FLValue cgc_or(FLValue args);
static FLValue __fl_wrap_cgc_or(FLClosure*, int, FLValue*);
FLValue cgc_or_fold(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_or_fold(FLClosure*, int, FLValue*);
FLValue cgc_args(FLValue args);
static FLValue __fl_wrap_cgc_args(FLClosure*, int, FLValue*);
FLValue cgc_args_loop(FLValue _args, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_cgc_args_loop(FLClosure*, int, FLValue*);
FLValue cgc_stmts(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_stmts(FLClosure*, int, FLValue*);
FLValue cgc_forward_decls(FLValue nodes);
static FLValue __fl_wrap_cgc_forward_decls(FLClosure*, int, FLValue*);
FLValue cgc_forward_loop(FLValue _nodes, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_cgc_forward_loop(FLClosure*, int, FLValue*);
FLValue cgc_wrapper_call_args(FLValue _items, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_cgc_wrapper_call_args(FLClosure*, int, FLValue*);
FLValue cgc_top_level(FLValue _nodes, FLValue _i, FLValue _stmts, FLValue _fns);
static FLValue __fl_wrap_cgc_top_level(FLClosure*, int, FLValue*);
FLValue cgc_lambda_fwd_loop(FLValue _defs, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_cgc_lambda_fwd_loop(FLClosure*, int, FLValue*);
FLValue cgc_lambda_fwds();
static FLValue __fl_wrap_cgc_lambda_fwds(FLClosure*, int, FLValue*);
FLValue cgc_join_lambda_loop(FLValue _defs, FLValue _i, FLValue _acc);
static FLValue __fl_wrap_cgc_join_lambda_loop(FLClosure*, int, FLValue*);
FLValue cgc_join_lambdas();
static FLValue __fl_wrap_cgc_join_lambdas(FLClosure*, int, FLValue*);
FLValue cgc_join_wrappers();
static FLValue __fl_wrap_cgc_join_wrappers(FLClosure*, int, FLValue*);
FLValue cgc_join_globals();
static FLValue __fl_wrap_cgc_join_globals(FLClosure*, int, FLValue*);
FLValue generate_c(FLValue nodes);
static FLValue __fl_wrap_generate_c(FLClosure*, int, FLValue*);
FLValue cgc_set_b(FLValue args);
static FLValue __fl_wrap_cgc_set_b(FLClosure*, int, FLValue*);
FLValue cgc_while(FLValue args);
static FLValue __fl_wrap_cgc_while(FLClosure*, int, FLValue*);
FLValue loop_extract_vars(FLValue items, FLValue i, FLValue acc);
static FLValue __fl_wrap_loop_extract_vars(FLClosure*, int, FLValue*);
FLValue loop_make_decls(FLValue items, FLValue i, FLValue acc);
static FLValue __fl_wrap_loop_make_decls(FLClosure*, int, FLValue*);
FLValue cgc_loop(FLValue args);
static FLValue __fl_wrap_cgc_loop(FLClosure*, int, FLValue*);
FLValue cgc_recur_temps(FLValue args, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_recur_temps(FLClosure*, int, FLValue*);
FLValue cgc_recur_assigns(FLValue vars, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_recur_assigns(FLClosure*, int, FLValue*);
FLValue cgc_recur_stmt(FLValue args, FLValue vars);
static FLValue __fl_wrap_cgc_recur_stmt(FLClosure*, int, FLValue*);
FLValue cgc_with_recur(FLValue node, FLValue vars);
static FLValue __fl_wrap_cgc_with_recur(FLClosure*, int, FLValue*);
FLValue cgc_if_wr(FLValue args, FLValue vars);
static FLValue __fl_wrap_cgc_if_wr(FLClosure*, int, FLValue*);
FLValue cgc_cond_wr(FLValue args, FLValue vars);
static FLValue __fl_wrap_cgc_cond_wr(FLClosure*, int, FLValue*);
FLValue cgc_cond_nested_wr(FLValue args, FLValue vars, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_cond_nested_wr(FLClosure*, int, FLValue*);
FLValue cgc_do_wr(FLValue args, FLValue vars);
static FLValue __fl_wrap_cgc_do_wr(FLClosure*, int, FLValue*);
FLValue cgc_stmts_wr(FLValue args, FLValue vars, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_stmts_wr(FLClosure*, int, FLValue*);
FLValue cgc_let_wr(FLValue args, FLValue vars);
static FLValue __fl_wrap_cgc_let_wr(FLClosure*, int, FLValue*);
FLValue cgc_body_wr(FLValue args, FLValue vars, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_body_wr(FLClosure*, int, FLValue*);
FLValue cgc_array_block(FLValue n);
static FLValue __fl_wrap_cgc_array_block(FLClosure*, int, FLValue*);
FLValue cgc_map_entry_c(FLValue ent);
static FLValue __fl_wrap_cgc_map_entry_c(FLClosure*, int, FLValue*);
FLValue cgc_map_entries_c(FLValue ents, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_map_entries_c(FLClosure*, int, FLValue*);
FLValue cgc_map_key_c(FLValue key_node);
static FLValue __fl_wrap_cgc_map_key_c(FLClosure*, int, FLValue*);
FLValue cgc_map_items_c(FLValue items, FLValue i, FLValue acc);
static FLValue __fl_wrap_cgc_map_items_c(FLClosure*, int, FLValue*);
FLValue cgc_map_from_items(FLValue items);
static FLValue __fl_wrap_cgc_map_from_items(FLClosure*, int, FLValue*);
FLValue cgc_map_block(FLValue n);
static FLValue __fl_wrap_cgc_map_block(FLClosure*, int, FLValue*);
FLValue ir_err(FLValue msg, FLValue n);
static FLValue __fl_wrap_ir_err(FLClosure*, int, FLValue*);
FLValue ir_chk(FLValue n);
static FLValue __fl_wrap_ir_chk(FLClosure*, int, FLValue*);
FLValue includes_item(FLValue arr, FLValue val);
static FLValue __fl_wrap_includes_item(FLClosure*, int, FLValue*);
FLValue ir_validate(FLValue nodes);
static FLValue __fl_wrap_ir_validate(FLClosure*, int, FLValue*);
FLValue path_dir(FLValue path);
static FLValue __fl_wrap_path_dir(FLClosure*, int, FLValue*);
FLValue append_all(FLValue acc, FLValue items);
static FLValue __fl_wrap_append_all(FLClosure*, int, FLValue*);
FLValue expand_loads(FLValue nodes, FLValue base_dir);
static FLValue __fl_wrap_expand_loads(FLClosure*, int, FLValue*);
FLValue ast_json_str(FLValue s);
static FLValue __fl_wrap_ast_json_str(FLClosure*, int, FLValue*);
FLValue ast_node_to_json(FLValue node);
static FLValue __fl_wrap_ast_node_to_json(FLClosure*, int, FLValue*);
FLValue ast_tree_lines(FLValue node, FLValue prefix, FLValue is_last);
static FLValue __fl_wrap_ast_tree_lines(FLClosure*, int, FLValue*);
FLValue ast_emit(FLValue input);
static FLValue __fl_wrap_ast_emit(FLClosure*, int, FLValue*);
FLValue cgc_run(FLValue argv);
static FLValue __fl_wrap_cgc_run(FLClosure*, int, FLValue*);
static FLValue lambda_id_atom;

static FLValue lambda_defs_atom;

static FLValue outer_params_atom;

static FLValue known_fncall_targets_atom;

static FLValue known_defns_atom;

static FLValue wrapper_defs_atom;

static FLValue global_decls_atom;

static FLValue defn_arity_atom;

static FLValue cgc_defn_depth_atom;

static FLValue cgc_hoisted_fns_atom;

static FLValue ir_kind_set;

static FLValue ir_lit_types;

static FLValue loaded_paths_atom;


static FLValue __fl_anon_0(FLClosure*, int, FLValue*);
static FLValue __fl_anon_1(FLClosure*, int, FLValue*);
static FLValue __fl_anon_2(FLClosure*, int, FLValue*);
static FLValue __fl_anon_3(FLClosure*, int, FLValue*);
static FLValue __fl_anon_4(FLClosure*, int, FLValue*);
static FLValue __fl_anon_5(FLClosure*, int, FLValue*);
static FLValue __fl_anon_6(FLClosure*, int, FLValue*);
static FLValue __fl_anon_7(FLClosure*, int, FLValue*);
static FLValue __fl_anon_8(FLClosure*, int, FLValue*);
static FLValue __fl_anon_9(FLClosure*, int, FLValue*);
static FLValue __fl_anon_10(FLClosure*, int, FLValue*);
static FLValue __fl_anon_11(FLClosure*, int, FLValue*);
static FLValue __fl_anon_12(FLClosure*, int, FLValue*);
static FLValue __fl_anon_13(FLClosure*, int, FLValue*);
static FLValue __fl_anon_14(FLClosure*, int, FLValue*);
static FLValue __fl_anon_15(FLClosure*, int, FLValue*);
static FLValue __fl_anon_16(FLClosure*, int, FLValue*);
static FLValue __fl_anon_17(FLClosure*, int, FLValue*);

static FLValue __fl_anon_0(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue k __attribute__((unused)) = argv[0];
    return fl_not(fl_eq(k, fl_str_val("line")));
}

static FLValue __fl_anon_2(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue op = _self->env[0];
    FLValue a __attribute__((unused)) = argv[0];
    return (fl_truthy(cgc_is_get_p(a)) ? fl_println(fl_str_n(3, fl_str_val("[FL Warn] nil 전파: "), op, fl_str_val(" 인자에 (get ...) — null? 체크 권장"))) : fl_nil());
}

static FLValue __fl_anon_3(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue p __attribute__((unused)) = argv[0];
    return fl_atom_reset(known_fncall_targets_atom, fl_vec_push(fl_atom_deref(known_fncall_targets_atom), p));
}

static FLValue __fl_anon_4(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue items = _self->env[0];
    FLValue val_c = _self->env[1];
    FLValue i __attribute__((unused)) = argv[0];
    return ((__extension__ ({
    FLValue elem = get(items, i);
    FLValue getter = fl_str_n(5, fl_str_val("get("), val_c, fl_str_val(", fl_int("), i, fl_str_val("))"));
    (fl_truthy(match_pat_nil_p(elem)) ? (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("cond"), fl_str_n(3, fl_str_val("fl_truthy(null_p("), getter, fl_str_val("))")), fl_str_val("bind"), fl_str_val("")}; fl_map_from_pairs(__fl_kv, 2); })) : (fl_truthy(match_pat_var_p(elem)) ? (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("cond"), fl_str_val(""), fl_str_val("bind"), fl_str_n(5, fl_str_val("FLValue "), c_name(get(elem, fl_str_val("name"))), fl_str_val(" = "), getter, fl_str_val(";"))}; fl_map_from_pairs(__fl_kv, 2); })) : (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("cond"), fl_str_n(5, fl_str_val("fl_truthy(fl_eq("), getter, fl_str_val(", "), cgc(elem), fl_str_val("))")), fl_str_val("bind"), fl_str_val("")}; fl_map_from_pairs(__fl_kv, 2); }))));
})));
}

static FLValue __fl_anon_5(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue s __attribute__((unused)) = argv[0];
    return fl_gt(length(s), fl_int(0));
}

static FLValue __fl_anon_6(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue r __attribute__((unused)) = argv[0];
    return get(r, fl_str_val("cond"));
}

static FLValue __fl_anon_7(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue a __attribute__((unused)) = argv[0];
    FLValue b __attribute__((unused)) = argv[1];
    return fl_str_n(3, a, fl_str_val(" && "), b);
}

static FLValue __fl_anon_8(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue a __attribute__((unused)) = argv[0];
    FLValue b __attribute__((unused)) = argv[1];
    return fl_str_n(3, a, fl_str_val(" "), b);
}

static FLValue __fl_anon_9(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue s __attribute__((unused)) = argv[0];
    return fl_gt(length(s), fl_int(0));
}

static FLValue __fl_anon_10(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue r __attribute__((unused)) = argv[0];
    return get(r, fl_str_val("bind"));
}

static FLValue __fl_anon_13(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue func_def = _self->env[0];
    FLValue acc __attribute__((unused)) = argv[0];
    return fl_str_n(3, acc, func_def, fl_str_val("\n\n"));
}

static FLValue __fl_anon_15(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue v __attribute__((unused)) = argv[0];
    return fl_atom_reset(outer_params_atom, fl_vec_push(fl_atom_deref(outer_params_atom), v));
}

static FLValue __fl_anon_16(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue abs_path = _self->env[0];
    FLValue v __attribute__((unused)) = argv[0];
    return fl_vec_push(v, abs_path);
}

static FLValue __fl_anon_17(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue items = _self->env[0];
    FLValue prefix = _self->env[1];
    FLValue next = _self->env[2];
    FLValue n = _self->env[3];
    FLValue acc __attribute__((unused)) = argv[0];
    FLValue i __attribute__((unused)) = argv[1];
    return append_all(acc, ast_tree_lines(get(items, i), fl_str_n(2, prefix, next), fl_eq(i, fl_sub(n, fl_int(1)))));
}

static FLValue __fl_anon_18(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue args = _self->env[0];
    FLValue prefix = _self->env[1];
    FLValue next = _self->env[2];
    FLValue n = _self->env[3];
    FLValue acc __attribute__((unused)) = argv[0];
    FLValue i __attribute__((unused)) = argv[1];
    return append_all(acc, ast_tree_lines(get(args, i), fl_str_n(2, prefix, next), fl_eq(i, fl_sub(n, fl_int(1)))));
}

static FLValue __fl_anon_19(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue __fl_ign_0 __attribute__((unused)) = argv[0];
    return fl_vec_new();
}

static FLValue __fl_anon_20(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue acc __attribute__((unused)) = argv[0];
    FLValue node __attribute__((unused)) = argv[1];
    return append_all(acc, ast_tree_lines(node, fl_str_val(""), fl_bool(true)));
}

static FLValue __fl_anon_21(FLClosure* _self, int _argc, FLValue* argv) {
    (void)_self; (void)_argc;
    FLValue __fl_ign_0 __attribute__((unused)) = argv[0];
    return fl_vec_new();
}

static FLValue __fl_wrap_is_digit_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return is_digit_p(argv[0]);
}

static FLValue __fl_wrap_is_alpha_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return is_alpha_p(argv[0]);
}

static FLValue __fl_wrap_is_alnum_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return is_alnum_p(argv[0]);
}

static FLValue __fl_wrap_is_space_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return is_space_p(argv[0]);
}

static FLValue __fl_wrap_is_symbol_char_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return is_symbol_char_p(argv[0]);
}

static FLValue __fl_wrap_make_state(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_state(argv[0]);
}

static FLValue __fl_wrap_peek_at(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return peek_at(argv[0], argv[1]);
}

static FLValue __fl_wrap_peek(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return peek(argv[0]);
}

static FLValue __fl_wrap_at_end_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return at_end_p(argv[0]);
}

static FLValue __fl_wrap_advance(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return advance(argv[0]);
}

static FLValue __fl_wrap_emit(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return emit(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_skip_comment_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return skip_comment_loop(argv[0]);
}

static FLValue __fl_wrap_skip_comment(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return skip_comment(argv[0]);
}

static FLValue __fl_wrap_skip_ws_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return skip_ws_loop(argv[0]);
}

static FLValue __fl_wrap_skip_ws(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return skip_ws(argv[0]);
}

static FLValue __fl_wrap_read_number_iter(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return read_number_iter(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_read_number_body(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return read_number_body(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_read_number(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return read_number(argv[0]);
}

static FLValue __fl_wrap_translate_esc(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return translate_esc(argv[0]);
}

static FLValue __fl_wrap_read_string_iter(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return read_string_iter(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_read_string_body(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return read_string_body(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_read_string(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return read_string(argv[0]);
}

static FLValue __fl_wrap_read_symbol_iter(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return read_symbol_iter(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_read_symbol_body_kind(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return read_symbol_body_kind(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_read_symbol(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return read_symbol(argv[0]);
}

static FLValue __fl_wrap_read_variable(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return read_variable(argv[0]);
}

static FLValue __fl_wrap_read_keyword(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return read_keyword(argv[0]);
}

static FLValue __fl_wrap_read_token(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return read_token(argv[0]);
}

static FLValue __fl_wrap_lex_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return lex_loop(argv[0]);
}

static FLValue __fl_wrap_lex(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return lex(argv[0]);
}

static FLValue __fl_wrap_make_literal(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_literal(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_make_variable(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_variable(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_keyword(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_keyword(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_sexpr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_sexpr(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_make_number(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_number(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_string(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_string(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_bool(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_bool(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_null(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_null(argv[0]);
}

static FLValue __fl_wrap_make_symbol(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_symbol(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_block(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_make_array_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_array_block(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_map_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_map_block(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_pattern_literal(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_pattern_literal(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_pattern_variable(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_pattern_variable(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_pattern_wildcard(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_pattern_wildcard(argv[0]);
}

static FLValue __fl_wrap_make_pattern_list(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_pattern_list(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_make_pattern_struct(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_pattern_struct(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_make_pattern_or(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_pattern_or(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_pattern_range(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_pattern_range(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_make_pattern_match(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_pattern_match(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_make_match_case(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_match_case(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_make_function_value(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_function_value(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_make_type_class(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_type_class(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_make_type_class_instance(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_type_class_instance(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_make_module_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_module_block(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_make_import_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_import_block(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_make_open_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_open_block(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_search_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_search_block(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_make_learn_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_learn_block(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_make_reasoning_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_reasoning_block(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_make_async_function(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_async_function(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_make_await(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_await(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_try(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_try(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_make_catch(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_catch(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_make_throw(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_throw(argv[0], argv[1]);
}

static FLValue __fl_wrap_make_template_string(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_template_string(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_make_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_loop(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_make_page(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_page(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_make_route(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_route(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_make_component(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_component(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_make_form(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_form(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_deep_equal_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return deep_equal_p(argv[0], argv[1]);
}

static FLValue __fl_wrap_deep_equal_list_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return deep_equal_list_p(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_deep_equal_map_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return deep_equal_map_p(argv[0], argv[1]);
}

static FLValue __fl_wrap_keys_no_line(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return keys_no_line(argv[0]);
}

static FLValue __fl_wrap_deep_equal_map_keys_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return deep_equal_map_keys_p(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_json_keys(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return json_keys(argv[0]);
}

static FLValue __fl_wrap_p_make(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return p_make(argv[0]);
}

static FLValue __fl_wrap_p_peek(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return p_peek(argv[0]);
}

static FLValue __fl_wrap_p_peek_at(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return p_peek_at(argv[0], argv[1]);
}

static FLValue __fl_wrap_p_end_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return p_end_p(argv[0]);
}

static FLValue __fl_wrap_p_advance(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return p_advance(argv[0]);
}

static FLValue __fl_wrap_p_with_ast(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return p_with_ast(argv[0], argv[1]);
}

static FLValue __fl_wrap_p_append_ast(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return p_append_ast(argv[0], argv[1]);
}

static FLValue __fl_wrap_r_pair(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return r_pair(argv[0], argv[1]);
}

static FLValue __fl_wrap_string_contains_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return string_contains_p(argv[0], argv[1]);
}

static FLValue __fl_wrap_parse_atom(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_atom(argv[0]);
}

static FLValue __fl_wrap_hash_fn_op(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return hash_fn_op(argv[0]);
}

static FLValue __fl_wrap_replace_pct(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return replace_pct(argv[0]);
}

static FLValue __fl_wrap_replace_pct_list(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return replace_pct_list(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_parse_hash_fn(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_hash_fn(argv[0]);
}

static FLValue __fl_wrap_make_pipe_call(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return make_pipe_call(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_parse_pipe_chain(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_pipe_chain(argv[0], argv[1]);
}

static FLValue __fl_wrap_parse_expr_base(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_expr_base(argv[0]);
}

static FLValue __fl_wrap_parse_expr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_expr(argv[0]);
}

static FLValue __fl_wrap_parse_sexpr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_sexpr(argv[0]);
}

static FLValue __fl_wrap_parse_consume_rparen(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_consume_rparen(argv[0]);
}

static FLValue __fl_wrap_parse_args(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_args(argv[0], argv[1]);
}

static FLValue __fl_wrap_parse_bracket(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_bracket(argv[0]);
}

static FLValue __fl_wrap_is_block_type_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return is_block_type_p(argv[0]);
}

static FLValue __fl_wrap_upper_case(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return upper_case(argv[0]);
}

static FLValue __fl_wrap_parse_array(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_array(argv[0], argv[1]);
}

static FLValue __fl_wrap_parse_consume_rbracket(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_consume_rbracket(argv[0]);
}

static FLValue __fl_wrap_parse_named_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_named_block(argv[0], argv[1]);
}

static FLValue __fl_wrap_parse_optional_name(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_optional_name(argv[0]);
}

static FLValue __fl_wrap_parse_block_fields(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_block_fields(argv[0], argv[1]);
}

static FLValue __fl_wrap_parse_map(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_map(argv[0]);
}

static FLValue __fl_wrap_parse_consume_rbrace(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_consume_rbrace(argv[0]);
}

static FLValue __fl_wrap_parse_all(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse_all(argv[0]);
}

static FLValue __fl_wrap_parse(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return parse(argv[0]);
}

static FLValue __fl_wrap_get_block_items(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return get_block_items(argv[0]);
}

static FLValue __fl_wrap_c_esc(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return c_esc(argv[0]);
}

static FLValue __fl_wrap_c_reserved_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return c_reserved_p(argv[0]);
}

static FLValue __fl_wrap_c_name(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return c_name(argv[0]);
}

static FLValue __fl_wrap_cgc(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc(argv[0]);
}

static FLValue __fl_wrap_cgc_op_wrapper(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_op_wrapper(argv[0]);
}

static FLValue __fl_wrap_template_to_parts(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return template_to_parts(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_template_string(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_template_string(argv[0]);
}

static FLValue __fl_wrap_cgc_literal(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_literal(argv[0]);
}

static FLValue __fl_wrap_cgc_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_block(argv[0]);
}

static FLValue __fl_wrap_cgc_func_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_func_block(argv[0]);
}

static FLValue __fl_wrap_cgc_params(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_params(argv[0]);
}

static FLValue __fl_wrap_cgc_params_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_params_loop(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_extract_name(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_extract_name(argv[0]);
}

static FLValue __fl_wrap_cgc_try(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_try(argv[0]);
}

static FLValue __fl_wrap_cgc_fncall(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_fncall(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_sexpr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_sexpr(argv[0]);
}

static FLValue __fl_wrap_cgc_dispatch(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_dispatch(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_swap_b(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_swap_b(argv[0]);
}

static FLValue __fl_wrap_cgc_is_get_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_is_get_p(argv[0]);
}

static FLValue __fl_wrap_cgc_warn_nil_get(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_warn_nil_get(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_is_str_lit_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_is_str_lit_p(argv[0]);
}

static FLValue __fl_wrap_cgc_is_num_lit_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_is_num_lit_p(argv[0]);
}

static FLValue __fl_wrap_cgc_warn_type_mix(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_warn_type_mix(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_dispatch_fallback(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_dispatch_fallback(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_fn_argv_decls(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_fn_argv_decls(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_fn_param_names(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_fn_param_names(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_collect_vars(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_collect_vars(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_collect_vars_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_collect_vars_loop(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_fn_env_decls(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_fn_env_decls(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_env_arr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_env_arr(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_fn_caps_filter(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_fn_caps_filter(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_cgc_fn(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_fn(argv[0]);
}

static FLValue __fl_wrap_cgc_list(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_list(argv[0]);
}

static FLValue __fl_wrap_cgc_str(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_str(argv[0]);
}

static FLValue __fl_wrap_cgc_str_arg(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_str_arg(argv[0]);
}

static FLValue __fl_wrap_cgc_if(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_if(argv[0]);
}

static FLValue __fl_wrap_cgc_cond(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_cond(argv[0]);
}

static FLValue __fl_wrap_cgc_cond_nested(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_cond_nested(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_cond_flat(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_cond_flat(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_do(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_do(argv[0]);
}

static FLValue __fl_wrap_cgc_let(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_let(argv[0]);
}

static FLValue __fl_wrap_cgc_let_unique_name(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_let_unique_name(argv[0]);
}

static FLValue __fl_wrap_cgc_let_1d(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_let_1d(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_let_2d(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_let_2d(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_body(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_body(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_defn_stmts(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_defn_stmts(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_node_has_recur(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_node_has_recur(argv[0]);
}

static FLValue __fl_wrap_cgc_any_has_recur(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_any_has_recur(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_recur_goto_stmt(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_recur_goto_stmt(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_with_recur_goto(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_with_recur_goto(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_if_wr_goto(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_if_wr_goto(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_cond_wr_goto(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_cond_wr_goto(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_cond_nested_wr_goto(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_cond_nested_wr_goto(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_cgc_thread_first(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_thread_first(argv[0]);
}

static FLValue __fl_wrap_cgc_tf_build(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_tf_build(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_thread_last(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_thread_last(argv[0]);
}

static FLValue __fl_wrap_cgc_tl_build(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_tl_build(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_case(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_case(argv[0]);
}

static FLValue __fl_wrap_cgc_case_rev(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_case_rev(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_cgc_if_let(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_if_let(argv[0]);
}

static FLValue __fl_wrap_cgc_when_let(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_when_let(argv[0]);
}

static FLValue __fl_wrap_match_wildcard_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return match_wildcard_p(argv[0]);
}

static FLValue __fl_wrap_match_bind_var_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return match_bind_var_p(argv[0]);
}

static FLValue __fl_wrap_match_vec_pat_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return match_vec_pat_p(argv[0]);
}

static FLValue __fl_wrap_match_pat_nil_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return match_pat_nil_p(argv[0]);
}

static FLValue __fl_wrap_match_pat_var_p(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return match_pat_var_p(argv[0]);
}

static FLValue __fl_wrap_cgc_match_vec_cond(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_match_vec_cond(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_match_rev(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_match_rev(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_cgc_match(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_match(argv[0]);
}

static FLValue __fl_wrap_cgc_doseq(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_doseq(argv[0]);
}

static FLValue __fl_wrap_cgc_dotimes(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_dotimes(argv[0]);
}

static FLValue __fl_wrap_cgc_doto(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_doto(argv[0]);
}

static FLValue __fl_wrap_cgc_doto_calls(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_doto_calls(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_cgc_for(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_for(argv[0]);
}

static FLValue __fl_wrap_cgc_cond_flat_wr_goto(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_cond_flat_wr_goto(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_cgc_do_wr_goto(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_do_wr_goto(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_stmts_wr_goto(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_stmts_wr_goto(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_cgc_let_wr_goto(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_let_wr_goto(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_body_wr_goto(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_body_wr_goto(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_cgc_defn_stmts_tco(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_defn_stmts_tco(argv[0], argv[1], argv[2], argv[3], argv[4]);
}

static FLValue __fl_wrap_cgc_defn(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_defn(argv[0]);
}

static FLValue __fl_wrap_cgc_define(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_define(argv[0]);
}

static FLValue __fl_wrap_cgc_binop_chain(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_binop_chain(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_binop_fold(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_binop_fold(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_cgc_and(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_and(argv[0]);
}

static FLValue __fl_wrap_cgc_and_fold(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_and_fold(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_or(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_or(argv[0]);
}

static FLValue __fl_wrap_cgc_or_fold(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_or_fold(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_args(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_args(argv[0]);
}

static FLValue __fl_wrap_cgc_args_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_args_loop(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_stmts(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_stmts(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_forward_decls(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_forward_decls(argv[0]);
}

static FLValue __fl_wrap_cgc_forward_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_forward_loop(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_wrapper_call_args(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_wrapper_call_args(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_top_level(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_top_level(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_cgc_lambda_fwd_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_lambda_fwd_loop(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_lambda_fwds(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_lambda_fwds();
}

static FLValue __fl_wrap_cgc_join_lambda_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_join_lambda_loop(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_join_lambdas(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_join_lambdas();
}

static FLValue __fl_wrap_cgc_join_wrappers(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_join_wrappers();
}

static FLValue __fl_wrap_cgc_join_globals(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_join_globals();
}

static FLValue __fl_wrap_generate_c(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return generate_c(argv[0]);
}

static FLValue __fl_wrap_cgc_set_b(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_set_b(argv[0]);
}

static FLValue __fl_wrap_cgc_while(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_while(argv[0]);
}

static FLValue __fl_wrap_loop_extract_vars(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return loop_extract_vars(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_loop_make_decls(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return loop_make_decls(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_loop(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_loop(argv[0]);
}

static FLValue __fl_wrap_cgc_recur_temps(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_recur_temps(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_recur_assigns(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_recur_assigns(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_recur_stmt(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_recur_stmt(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_with_recur(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_with_recur(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_if_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_if_wr(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_cond_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_cond_wr(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_cond_nested_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_cond_nested_wr(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_cgc_do_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_do_wr(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_stmts_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_stmts_wr(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_cgc_let_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_let_wr(argv[0], argv[1]);
}

static FLValue __fl_wrap_cgc_body_wr(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_body_wr(argv[0], argv[1], argv[2], argv[3]);
}

static FLValue __fl_wrap_cgc_array_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_array_block(argv[0]);
}

static FLValue __fl_wrap_cgc_map_entry_c(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_map_entry_c(argv[0]);
}

static FLValue __fl_wrap_cgc_map_entries_c(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_map_entries_c(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_map_key_c(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_map_key_c(argv[0]);
}

static FLValue __fl_wrap_cgc_map_items_c(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_map_items_c(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_cgc_map_from_items(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_map_from_items(argv[0]);
}

static FLValue __fl_wrap_cgc_map_block(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_map_block(argv[0]);
}

static FLValue __fl_wrap_ir_err(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return ir_err(argv[0], argv[1]);
}

static FLValue __fl_wrap_ir_chk(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return ir_chk(argv[0]);
}

static FLValue __fl_wrap_includes_item(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return includes_item(argv[0], argv[1]);
}

static FLValue __fl_wrap_ir_validate(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return ir_validate(argv[0]);
}

static FLValue __fl_wrap_path_dir(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return path_dir(argv[0]);
}

static FLValue __fl_wrap_append_all(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return append_all(argv[0], argv[1]);
}

static FLValue __fl_wrap_expand_loads(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return expand_loads(argv[0], argv[1]);
}

static FLValue __fl_wrap_ast_json_str(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return ast_json_str(argv[0]);
}

static FLValue __fl_wrap_ast_node_to_json(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return ast_node_to_json(argv[0]);
}

static FLValue __fl_wrap_ast_tree_lines(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return ast_tree_lines(argv[0], argv[1], argv[2]);
}

static FLValue __fl_wrap_ast_emit(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return ast_emit(argv[0]);
}

static FLValue __fl_wrap_cgc_run(FLClosure* _s, int _ac, FLValue* argv) {
    (void)_s; (void)_ac;
    return cgc_run(argv[0]);
}


FLValue is_digit_p(FLValue c) {
    fl_push_frame_ln("is_digit_p", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(null_p(c)) ? fl_bool(false) : (fl_truthy(fl_gte(c, fl_str_val("0"))) ? fl_lte(c, fl_str_val("9")) : fl_bool(false))); fl_pop_frame(); return __fl_ret__; }
}

FLValue is_alpha_p(FLValue c) {
    fl_push_frame_ln("is_alpha_p", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(null_p(c)) ? fl_bool(false) : fl_or((fl_truthy(fl_gte(c, fl_str_val("a"))) ? fl_lte(c, fl_str_val("z")) : fl_bool(false)), (fl_truthy(fl_gte(c, fl_str_val("A"))) ? fl_lte(c, fl_str_val("Z")) : fl_bool(false)))); fl_pop_frame(); return __fl_ret__; }
}

FLValue is_alnum_p(FLValue c) {
    fl_push_frame_ln("is_alnum_p", __LINE__);
    { FLValue __fl_ret__ = fl_or(is_digit_p(c), is_alpha_p(c)); fl_pop_frame(); return __fl_ret__; }
}

FLValue is_space_p(FLValue c) {
    fl_push_frame_ln("is_space_p", __LINE__);
    { FLValue __fl_ret__ = fl_or(fl_or(fl_or(fl_eq(c, fl_str_val(" ")), fl_eq(c, fl_str_val("\t"))), fl_eq(c, fl_str_val("\n"))), fl_eq(c, fl_str_val("\r"))); fl_pop_frame(); return __fl_ret__; }
}

FLValue is_symbol_char_p(FLValue c) {
    fl_push_frame_ln("is_symbol_char_p", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(null_p(c)) ? fl_bool(false) : fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(fl_or(is_alnum_p(c), fl_eq(c, fl_str_val("-"))), fl_eq(c, fl_str_val("_"))), fl_eq(c, fl_str_val("?"))), fl_eq(c, fl_str_val("!"))), fl_eq(c, fl_str_val("/"))), fl_eq(c, fl_str_val("."))), fl_eq(c, fl_str_val("<"))), fl_eq(c, fl_str_val(">"))), fl_eq(c, fl_str_val("="))), fl_eq(c, fl_str_val("+"))), fl_eq(c, fl_str_val("*"))), fl_eq(c, fl_str_val("%"))), fl_eq(c, fl_str_val("&"))), fl_eq(c, fl_str_val("|"))), fl_eq(c, fl_str_val("^"))), fl_eq(c, fl_str_val("~")))); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_state(FLValue src) {
    fl_push_frame_ln("make_state", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("src"), src, fl_str_val("idx"), fl_int(0), fl_str_val("line"), fl_int(1), fl_str_val("col"), fl_int(1), fl_str_val("tokens"), fl_vec_new()}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue peek_at(FLValue st, FLValue offset) {
    fl_push_frame_ln("peek_at", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue src = get(st, fl_str_val("src"));
    FLValue i = fl_add(get(st, fl_str_val("idx")), offset);
    (fl_truthy(fl_gte(i, length(src))) ? fl_nil() : char_at(src, i));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue peek(FLValue st) {
    fl_push_frame_ln("peek", __LINE__);
    { FLValue __fl_ret__ = peek_at(st, fl_int(0)); fl_pop_frame(); return __fl_ret__; }
}

FLValue at_end_p(FLValue st) {
    fl_push_frame_ln("at_end_p", __LINE__);
    { FLValue __fl_ret__ = fl_gte(get(st, fl_str_val("idx")), length(get(st, fl_str_val("src")))); fl_pop_frame(); return __fl_ret__; }
}

FLValue advance(FLValue st) {
    fl_push_frame_ln("advance", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue c = peek(st);
    (fl_truthy(fl_eq(c, fl_str_val("\n"))) ? (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("src"), get(st, fl_str_val("src")), fl_str_val("idx"), fl_add(get(st, fl_str_val("idx")), fl_int(1)), fl_str_val("line"), fl_add(get(st, fl_str_val("line")), fl_int(1)), fl_str_val("col"), fl_int(1), fl_str_val("tokens"), get(st, fl_str_val("tokens"))}; fl_map_from_pairs(__fl_kv, 5); })) : (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("src"), get(st, fl_str_val("src")), fl_str_val("idx"), fl_add(get(st, fl_str_val("idx")), fl_int(1)), fl_str_val("line"), get(st, fl_str_val("line")), fl_str_val("col"), fl_add(get(st, fl_str_val("col")), fl_int(1)), fl_str_val("tokens"), get(st, fl_str_val("tokens"))}; fl_map_from_pairs(__fl_kv, 5); })));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue emit(FLValue st, FLValue kind, FLValue value, FLValue sl, FLValue sc) {
    fl_push_frame_ln("emit", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("src"), get(st, fl_str_val("src")), fl_str_val("idx"), get(st, fl_str_val("idx")), fl_str_val("line"), get(st, fl_str_val("line")), fl_str_val("col"), get(st, fl_str_val("col")), fl_str_val("tokens"), fl_vec_push(get(st, fl_str_val("tokens")), (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), kind, fl_str_val("type"), kind, fl_str_val("value"), value, fl_str_val("line"), sl, fl_str_val("col"), sc}; fl_map_from_pairs(__fl_kv, 5); })))}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue skip_comment_loop(FLValue _cur) {
    fl_push_frame_ln("skip_comment_loop", __LINE__);
    __fl_tco_skip_comment_loop:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _cur;
    FLValue cur = __fl_loop_tmp_0;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_or(at_end_p(cur), fl_eq(peek(cur), fl_str_val("\n")))) ? (fl_truthy(at_end_p(cur)) ? cur : advance(cur)) : (__extension__ ({
    FLValue _fl_t0 = advance(cur);
    cur = _fl_t0;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue skip_comment(FLValue st) {
    fl_push_frame_ln("skip_comment", __LINE__);
    { FLValue __fl_ret__ = skip_comment_loop(st); fl_pop_frame(); return __fl_ret__; }
}

FLValue skip_ws_loop(FLValue _cur) {
    fl_push_frame_ln("skip_ws_loop", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _cur;
    FLValue cur = __fl_loop_tmp_0;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(at_end_p(cur)) ? cur : ((__extension__ ({
    FLValue c = peek(cur);
    (fl_truthy(is_space_p(c)) ? (__extension__ ({
    FLValue _fl_t0 = advance(cur);
    cur = _fl_t0;
    _fl_looping = 1; fl_nil();
})) : (fl_truthy(fl_eq(c, fl_str_val(";"))) ? (__extension__ ({
    FLValue _fl_t0 = skip_comment(cur);
    cur = _fl_t0;
    _fl_looping = 1; fl_nil();
})) : cur));
}))));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue skip_ws(FLValue st) {
    fl_push_frame_ln("skip_ws", __LINE__);
    { FLValue __fl_ret__ = skip_ws_loop(st); fl_pop_frame(); return __fl_ret__; }
}

FLValue read_number_iter(FLValue _cur, FLValue _res_acc, FLValue _dot, FLValue line, FLValue col) {
    fl_push_frame_ln("read_number_iter", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _cur;
    FLValue cur = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _res_acc;
    FLValue res_acc = __fl_loop_tmp_2;
    FLValue __fl_loop_tmp_4 = _dot;
    FLValue dot = __fl_loop_tmp_4;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(at_end_p(cur)) ? emit(cur, fl_str_val("Number"), res_acc, line, col) : ((__extension__ ({
    FLValue c = peek(cur);
    (fl_truthy(is_digit_p(c)) ? (__extension__ ({
    FLValue _fl_t0 = advance(cur);
    FLValue _fl_t1 = fl_str_n(2, res_acc, c);
    FLValue _fl_t2 = dot;
    cur = _fl_t0;
    res_acc = _fl_t1;
    dot = _fl_t2;
    _fl_looping = 1; fl_nil();
})) : (fl_truthy((fl_truthy(fl_eq(c, fl_str_val("."))) ? fl_not(dot) : fl_bool(false))) ? (__extension__ ({
    FLValue _fl_t0 = advance(cur);
    FLValue _fl_t1 = fl_str_n(2, res_acc, c);
    FLValue _fl_t2 = fl_bool(true);
    cur = _fl_t0;
    res_acc = _fl_t1;
    dot = _fl_t2;
    _fl_looping = 1; fl_nil();
})) : emit(cur, fl_str_val("Number"), res_acc, line, col)));
}))));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue read_number_body(FLValue st, FLValue acc, FLValue has_dot, FLValue line, FLValue col) {
    fl_push_frame_ln("read_number_body", __LINE__);
    { FLValue __fl_ret__ = read_number_iter(st, acc, has_dot, line, col); fl_pop_frame(); return __fl_ret__; }
}

FLValue read_number(FLValue st) {
    fl_push_frame_ln("read_number", __LINE__);
    { FLValue __fl_ret__ = read_number_body(st, fl_str_val(""), fl_bool(false), get(st, fl_str_val("line")), get(st, fl_str_val("col"))); fl_pop_frame(); return __fl_ret__; }
}

FLValue translate_esc(FLValue c) {
    fl_push_frame_ln("translate_esc", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(c, fl_str_val("n"))) ? fl_str_val("\n") : (fl_truthy(fl_eq(c, fl_str_val("t"))) ? fl_str_val("\t") : (fl_truthy(fl_eq(c, fl_str_val("r"))) ? fl_str_val("\r") : (fl_truthy(fl_eq(c, fl_str_val("\""))) ? fl_str_val("\"") : (fl_truthy(fl_eq(c, fl_str_val("\\"))) ? fl_str_val("\\") : c))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue read_string_iter(FLValue _cur, FLValue _res_acc, FLValue line, FLValue col) {
    fl_push_frame_ln("read_string_iter", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _cur;
    FLValue cur = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _res_acc;
    FLValue res_acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(at_end_p(cur)) ? emit(cur, fl_str_val("String"), res_acc, line, col) : ((__extension__ ({
    FLValue c = peek(cur);
    (fl_truthy(fl_eq(c, fl_str_val("\""))) ? emit(advance(cur), fl_str_val("String"), res_acc, line, col) : (fl_truthy(fl_eq(c, fl_str_val("\\"))) ? ((__extension__ ({
    FLValue st2 = advance(cur);
    FLValue c2 = peek(st2);
    (__extension__ ({
    FLValue _fl_t0 = advance(st2);
    FLValue _fl_t1 = fl_str_n(2, res_acc, translate_esc(c2));
    cur = _fl_t0;
    res_acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}));
}))) : (__extension__ ({
    FLValue _fl_t0 = advance(cur);
    FLValue _fl_t1 = fl_str_n(2, res_acc, c);
    cur = _fl_t0;
    res_acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}))));
}))));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue read_string_body(FLValue st, FLValue acc, FLValue line, FLValue col) {
    fl_push_frame_ln("read_string_body", __LINE__);
    { FLValue __fl_ret__ = read_string_iter(st, acc, line, col); fl_pop_frame(); return __fl_ret__; }
}

FLValue read_string(FLValue st) {
    fl_push_frame_ln("read_string", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue line = get(st, fl_str_val("line"));
    FLValue col = get(st, fl_str_val("col"));
    FLValue st1 = advance(st);
    read_string_body(st1, fl_str_val(""), line, col);
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue read_symbol_iter(FLValue _cur, FLValue _res_acc, FLValue line, FLValue col, FLValue kind) {
    fl_push_frame_ln("read_symbol_iter", __LINE__);
    __fl_tco_read_symbol_iter:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _cur;
    FLValue cur = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _res_acc;
    FLValue res_acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(at_end_p(cur)) ? emit(cur, kind, res_acc, line, col) : ((__extension__ ({
    FLValue c = peek(cur);
    (fl_truthy(is_symbol_char_p(c)) ? (__extension__ ({
    FLValue _fl_t0 = advance(cur);
    FLValue _fl_t1 = fl_str_n(2, res_acc, c);
    cur = _fl_t0;
    res_acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})) : emit(cur, kind, res_acc, line, col));
}))));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue read_symbol_body_kind(FLValue st, FLValue acc, FLValue line, FLValue col, FLValue kind) {
    fl_push_frame_ln("read_symbol_body_kind", __LINE__);
    { FLValue __fl_ret__ = read_symbol_iter(st, acc, line, col, kind); fl_pop_frame(); return __fl_ret__; }
}

FLValue read_symbol(FLValue st) {
    fl_push_frame_ln("read_symbol", __LINE__);
    { FLValue __fl_ret__ = read_symbol_body_kind(st, fl_str_val(""), get(st, fl_str_val("line")), get(st, fl_str_val("col")), fl_str_val("Symbol")); fl_pop_frame(); return __fl_ret__; }
}

FLValue read_variable(FLValue st) {
    fl_push_frame_ln("read_variable", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue line = get(st, fl_str_val("line"));
    FLValue col = get(st, fl_str_val("col"));
    FLValue st1 = advance(st);
    read_symbol_body_kind(st1, fl_str_val(""), line, col, fl_str_val("Variable"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue read_keyword(FLValue st) {
    fl_push_frame_ln("read_keyword", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue line = get(st, fl_str_val("line"));
    FLValue col = get(st, fl_str_val("col"));
    FLValue st1 = advance(st);
    read_symbol_body_kind(st1, fl_str_val(""), line, col, fl_str_val("Keyword"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue read_token(FLValue st) {
    fl_push_frame_ln("read_token", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue st1 = skip_ws(st);
    (fl_truthy(at_end_p(st1)) ? st1 : ((__extension__ ({
    FLValue c = peek(st1);
    FLValue line = get(st1, fl_str_val("line"));
    FLValue col = get(st1, fl_str_val("col"));
    (fl_truthy(fl_eq(c, fl_str_val("("))) ? emit(advance(st1), fl_str_val("LParen"), c, line, col) : (fl_truthy(fl_eq(c, fl_str_val(")"))) ? emit(advance(st1), fl_str_val("RParen"), c, line, col) : (fl_truthy(fl_eq(c, fl_str_val("["))) ? emit(advance(st1), fl_str_val("LBracket"), c, line, col) : (fl_truthy(fl_eq(c, fl_str_val("]"))) ? emit(advance(st1), fl_str_val("RBracket"), c, line, col) : (fl_truthy(fl_eq(c, fl_str_val("{"))) ? emit(advance(st1), fl_str_val("LBrace"), c, line, col) : (fl_truthy(fl_eq(c, fl_str_val("}"))) ? emit(advance(st1), fl_str_val("RBrace"), c, line, col) : (fl_truthy(fl_eq(c, fl_str_val("\""))) ? read_string(st1) : (fl_truthy(fl_eq(c, fl_str_val("$"))) ? read_variable(st1) : (fl_truthy(fl_eq(c, fl_str_val(":"))) ? read_keyword(st1) : (fl_truthy(is_digit_p(c)) ? read_number(st1) : (fl_truthy((fl_truthy(fl_eq(c, fl_str_val("-"))) ? is_digit_p(peek_at(st1, fl_int(1))) : fl_bool(false))) ? read_number_body(advance(st1), fl_str_val("-"), fl_bool(false), line, col) : (fl_truthy(fl_eq(c, fl_str_val("#"))) ? (fl_truthy(fl_eq(peek_at(st1, fl_int(1)), fl_str_val("("))) ? emit(advance(advance(st1)), fl_str_val("HashParen"), fl_str_val("#("), line, col) : emit(advance(st1), fl_str_val("Unknown"), fl_str_val("#"), line, col)) : (fl_truthy(fl_eq(c, fl_str_val("|"))) ? (fl_truthy(fl_eq(peek_at(st1, fl_int(1)), fl_str_val(">"))) ? emit(advance(advance(st1)), fl_str_val("Pipe"), fl_str_val("|>"), line, col) : emit(advance(st1), fl_str_val("Unknown"), fl_str_val("|"), line, col)) : (fl_truthy(is_symbol_char_p(c)) ? read_symbol(st1) : emit(advance(st1), fl_str_val("Unknown"), c, line, col)))))))))))))));
}))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue lex_loop(FLValue _cur) {
    fl_push_frame_ln("lex_loop", __LINE__);
    __fl_tco_lex_loop:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _cur;
    FLValue cur = __fl_loop_tmp_0;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = ((__extension__ ({
    FLValue ws = skip_ws(cur);
    (fl_truthy(at_end_p(ws)) ? get(ws, fl_str_val("tokens")) : (__extension__ ({
    FLValue _fl_t0 = read_token(ws);
    cur = _fl_t0;
    _fl_looping = 1; fl_nil();
})));
})));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue lex(FLValue src) {
    fl_push_frame_ln("lex", __LINE__);
    { FLValue __fl_ret__ = lex_loop(make_state(src)); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_literal(FLValue __fl_kw_type, FLValue value, FLValue line) {
    fl_push_frame_ln("make_literal", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("literal"), fl_str_val("type"), __fl_kw_type, fl_str_val("value"), value, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_variable(FLValue name, FLValue line) {
    fl_push_frame_ln("make_variable", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("variable"), fl_str_val("name"), name, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_keyword(FLValue name, FLValue line) {
    fl_push_frame_ln("make_keyword", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("keyword"), fl_str_val("name"), name, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_sexpr(FLValue op, FLValue args, FLValue line) {
    fl_push_frame_ln("make_sexpr", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("sexpr"), fl_str_val("op"), op, fl_str_val("args"), args, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_number(FLValue v, FLValue line) {
    fl_push_frame_ln("make_number", __LINE__);
    { FLValue __fl_ret__ = make_literal(fl_str_val("number"), v, line); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_string(FLValue v, FLValue line) {
    fl_push_frame_ln("make_string", __LINE__);
    { FLValue __fl_ret__ = make_literal(fl_str_val("string"), v, line); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_bool(FLValue v, FLValue line) {
    fl_push_frame_ln("make_bool", __LINE__);
    { FLValue __fl_ret__ = make_literal(fl_str_val("boolean"), v, line); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_null(FLValue line) {
    fl_push_frame_ln("make_null", __LINE__);
    { FLValue __fl_ret__ = make_literal(fl_str_val("null"), fl_nil(), line); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_symbol(FLValue v, FLValue line) {
    fl_push_frame_ln("make_symbol", __LINE__);
    { FLValue __fl_ret__ = make_literal(fl_str_val("symbol"), v, line); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_block(FLValue __fl_kw_type, FLValue name, FLValue fields, FLValue line) {
    fl_push_frame_ln("make_block", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("block"), fl_str_val("type"), __fl_kw_type, fl_str_val("name"), name, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_array_block(FLValue items, FLValue line) {
    fl_push_frame_ln("make_array_block", __LINE__);
    { FLValue __fl_ret__ = make_block(fl_str_val("Array"), fl_nil(), (__extension__ ({ FLValue __fl_kv[2] = {fl_str_val("items"), items}; fl_map_from_pairs(__fl_kv, 1); })), line); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_map_block(FLValue items, FLValue line) {
    fl_push_frame_ln("make_map_block", __LINE__);
    { FLValue __fl_ret__ = make_block(fl_str_val("Map"), fl_nil(), (__extension__ ({ FLValue __fl_kv[2] = {fl_str_val("items"), items}; fl_map_from_pairs(__fl_kv, 1); })), line); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_pattern_literal(FLValue value, FLValue line) {
    fl_push_frame_ln("make_pattern_literal", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("pattern-literal"), fl_str_val("value"), value, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_pattern_variable(FLValue name, FLValue line) {
    fl_push_frame_ln("make_pattern_variable", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("pattern-variable"), fl_str_val("name"), name, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_pattern_wildcard(FLValue line) {
    fl_push_frame_ln("make_pattern_wildcard", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("kind"), fl_str_val("pattern-wildcard"), fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 2); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_pattern_list(FLValue items, FLValue __fl_kw_rest, FLValue line) {
    fl_push_frame_ln("make_pattern_list", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("pattern-list"), fl_str_val("items"), items, fl_str_val("rest"), __fl_kw_rest, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_pattern_struct(FLValue type_name, FLValue fields, FLValue line) {
    fl_push_frame_ln("make_pattern_struct", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("pattern-struct"), fl_str_val("type"), type_name, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_pattern_or(FLValue alternatives, FLValue line) {
    fl_push_frame_ln("make_pattern_or", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("pattern-or"), fl_str_val("alternatives"), alternatives, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_pattern_range(FLValue start, FLValue end, FLValue line) {
    fl_push_frame_ln("make_pattern_range", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("pattern-range"), fl_str_val("start"), start, fl_str_val("end"), end, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_pattern_match(FLValue value, FLValue cases, FLValue line) {
    fl_push_frame_ln("make_pattern_match", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("pattern-match"), fl_str_val("value"), value, fl_str_val("cases"), cases, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_match_case(FLValue pattern, FLValue guard, FLValue body, FLValue line) {
    fl_push_frame_ln("make_match_case", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("match-case"), fl_str_val("pattern"), pattern, fl_str_val("guard"), guard, fl_str_val("body"), body, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_function_value(FLValue params, FLValue body, FLValue captured_env, FLValue name) {
    fl_push_frame_ln("make_function_value", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("function-value"), fl_str_val("params"), params, fl_str_val("body"), body, fl_str_val("capturedEnv"), captured_env, fl_str_val("name"), name}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_type_class(FLValue name, FLValue generics, FLValue methods, FLValue line) {
    fl_push_frame_ln("make_type_class", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("type-class"), fl_str_val("name"), name, fl_str_val("generics"), generics, fl_str_val("methods"), methods, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_type_class_instance(FLValue class_name, FLValue type_name, FLValue impls, FLValue line) {
    fl_push_frame_ln("make_type_class_instance", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("type-class-instance"), fl_str_val("class"), class_name, fl_str_val("type"), type_name, fl_str_val("impls"), impls, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_module_block(FLValue name, FLValue exports, FLValue body, FLValue line) {
    fl_push_frame_ln("make_module_block", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("module"), fl_str_val("name"), name, fl_str_val("exports"), exports, fl_str_val("body"), body, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_import_block(FLValue path, FLValue alias, FLValue names, FLValue line) {
    fl_push_frame_ln("make_import_block", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("import"), fl_str_val("path"), path, fl_str_val("alias"), alias, fl_str_val("names"), names, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_open_block(FLValue module_name, FLValue line) {
    fl_push_frame_ln("make_open_block", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("open"), fl_str_val("module"), module_name, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_search_block(FLValue query, FLValue fields, FLValue line) {
    fl_push_frame_ln("make_search_block", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("search-block"), fl_str_val("query"), query, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_learn_block(FLValue topic, FLValue fields, FLValue line) {
    fl_push_frame_ln("make_learn_block", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("learn-block"), fl_str_val("topic"), topic, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_reasoning_block(FLValue name, FLValue fields, FLValue line) {
    fl_push_frame_ln("make_reasoning_block", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("reasoning-block"), fl_str_val("name"), name, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_async_function(FLValue name, FLValue params, FLValue body, FLValue line) {
    fl_push_frame_ln("make_async_function", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("async-function"), fl_str_val("name"), name, fl_str_val("params"), params, fl_str_val("body"), body, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_await(FLValue expr, FLValue line) {
    fl_push_frame_ln("make_await", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("await"), fl_str_val("expr"), expr, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_try(FLValue body, FLValue catch, FLValue finally, FLValue line) {
    fl_push_frame_ln("make_try", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("try"), fl_str_val("body"), body, fl_str_val("catch"), catch, fl_str_val("finally"), finally, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_catch(FLValue param, FLValue body, FLValue line) {
    fl_push_frame_ln("make_catch", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("catch"), fl_str_val("param"), param, fl_str_val("body"), body, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_throw(FLValue expr, FLValue line) {
    fl_push_frame_ln("make_throw", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("throw"), fl_str_val("expr"), expr, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_template_string(FLValue value, FLValue expressions, FLValue line) {
    fl_push_frame_ln("make_template_string", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("template-string"), fl_str_val("value"), value, fl_str_val("parts"), fl_vec_new(), fl_str_val("expressions"), expressions, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_loop(FLValue init, FLValue condition, FLValue __fl_kw_update, FLValue body, FLValue line) {
    fl_push_frame_ln("make_loop", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[12] = {fl_str_val("kind"), fl_str_val("loop"), fl_str_val("init"), init, fl_str_val("condition"), condition, fl_str_val("update"), __fl_kw_update, fl_str_val("body"), body, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 6); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_page(FLValue name, FLValue path, FLValue fields, FLValue line) {
    fl_push_frame_ln("make_page", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("page"), fl_str_val("name"), name, fl_str_val("path"), path, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_route(FLValue method, FLValue path, FLValue handler, FLValue line) {
    fl_push_frame_ln("make_route", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[10] = {fl_str_val("kind"), fl_str_val("route"), fl_str_val("method"), method, fl_str_val("path"), path, fl_str_val("handler"), handler, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 5); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_component(FLValue name, FLValue fields, FLValue line) {
    fl_push_frame_ln("make_component", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("component"), fl_str_val("name"), name, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_form(FLValue name, FLValue fields, FLValue line) {
    fl_push_frame_ln("make_form", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[8] = {fl_str_val("kind"), fl_str_val("form"), fl_str_val("name"), name, fl_str_val("fields"), fields, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 4); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue deep_equal_p(FLValue a, FLValue b) {
    fl_push_frame_ln("deep_equal_p", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy((fl_truthy(null_p(a)) ? null_p(b) : fl_bool(false))) ? fl_bool(true) : (fl_truthy(fl_or(null_p(a), null_p(b))) ? fl_bool(false) : (fl_truthy((fl_truthy(list_p(a)) ? list_p(b) : fl_bool(false))) ? deep_equal_list_p(a, b, fl_int(0)) : (fl_truthy((fl_truthy(fl_map_p(a)) ? fl_map_p(b) : fl_bool(false))) ? deep_equal_map_p(a, b) : fl_eq(a, b))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue deep_equal_list_p(FLValue a, FLValue b, FLValue i) {
    fl_push_frame_ln("deep_equal_list_p", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_not(fl_eq(length(a), length(b)))) ? fl_bool(false) : (fl_truthy(fl_gte(i, length(a))) ? fl_bool(true) : (fl_truthy(fl_not(deep_equal_p(get(a, i), get(b, i)))) ? fl_bool(false) : deep_equal_list_p(a, b, fl_add(i, fl_int(1)))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue deep_equal_map_p(FLValue a, FLValue b) {
    fl_push_frame_ln("deep_equal_map_p", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue ka = keys_no_line(a);
    FLValue kb = keys_no_line(b);
    (fl_truthy(fl_not(fl_eq(length(ka), length(kb)))) ? fl_bool(false) : deep_equal_map_keys_p(a, b, ka, fl_int(0)));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue keys_no_line(FLValue m) {
    fl_push_frame_ln("keys_no_line", __LINE__);
    { FLValue __fl_ret__ = fl_filter_fn(fl_fn_new(__fl_anon_0, 0, NULL), json_keys(m)); fl_pop_frame(); return __fl_ret__; }
}

FLValue deep_equal_map_keys_p(FLValue a, FLValue b, FLValue ks, FLValue i) {
    fl_push_frame_ln("deep_equal_map_keys_p", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(ks))) ? fl_bool(true) : (fl_truthy(((__extension__ ({
    FLValue k = get(ks, i);
    fl_not(deep_equal_p(get(a, k), get(b, k)));
})))) ? fl_bool(false) : deep_equal_map_keys_p(a, b, ks, fl_add(i, fl_int(1))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue json_keys(FLValue m) {
    fl_push_frame_ln("json_keys", __LINE__);
    { FLValue __fl_ret__ = fl_map_keys(m); fl_pop_frame(); return __fl_ret__; }
}

FLValue p_make(FLValue tokens) {
    fl_push_frame_ln("p_make", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("tokens"), tokens, fl_str_val("idx"), fl_int(0), fl_str_val("ast"), fl_vec_new()}; fl_map_from_pairs(__fl_kv, 3); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue p_peek(FLValue p) {
    fl_push_frame_ln("p_peek", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue i = get(p, fl_str_val("idx"));
    FLValue t = get(p, fl_str_val("tokens"));
    (fl_truthy(fl_gte(i, length(t))) ? fl_nil() : get(t, i));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue p_peek_at(FLValue p, FLValue offset) {
    fl_push_frame_ln("p_peek_at", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue i = fl_add(get(p, fl_str_val("idx")), offset);
    FLValue t = get(p, fl_str_val("tokens"));
    (fl_truthy(fl_gte(i, length(t))) ? fl_nil() : get(t, i));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue p_end_p(FLValue p) {
    fl_push_frame_ln("p_end_p", __LINE__);
    { FLValue __fl_ret__ = fl_gte(get(p, fl_str_val("idx")), length(get(p, fl_str_val("tokens")))); fl_pop_frame(); return __fl_ret__; }
}

FLValue p_advance(FLValue p) {
    fl_push_frame_ln("p_advance", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("tokens"), get(p, fl_str_val("tokens")), fl_str_val("idx"), fl_add(get(p, fl_str_val("idx")), fl_int(1)), fl_str_val("ast"), get(p, fl_str_val("ast"))}; fl_map_from_pairs(__fl_kv, 3); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue p_with_ast(FLValue p, FLValue ast) {
    fl_push_frame_ln("p_with_ast", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("tokens"), get(p, fl_str_val("tokens")), fl_str_val("idx"), get(p, fl_str_val("idx")), fl_str_val("ast"), ast}; fl_map_from_pairs(__fl_kv, 3); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue p_append_ast(FLValue p, FLValue node) {
    fl_push_frame_ln("p_append_ast", __LINE__);
    { FLValue __fl_ret__ = p_with_ast(p, fl_vec_push(get(p, fl_str_val("ast")), node)); fl_pop_frame(); return __fl_ret__; }
}

FLValue r_pair(FLValue p, FLValue node) {
    fl_push_frame_ln("r_pair", __LINE__);
    { FLValue __fl_ret__ = (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("p"), p, fl_str_val("node"), node}; fl_map_from_pairs(__fl_kv, 2); })); fl_pop_frame(); return __fl_ret__; }
}

FLValue string_contains_p(FLValue s, FLValue substr) {
    fl_push_frame_ln("string_contains_p", __LINE__);
    { FLValue __fl_ret__ = fl_not(fl_eq(fl_int(-1), str_index_of(s, substr))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_atom(FLValue p) {
    fl_push_frame_ln("parse_atom", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue t = p_peek(p);
    FLValue k = get(t, fl_str_val("kind"));
    FLValue v = get(t, fl_str_val("value"));
    FLValue line = get(t, fl_str_val("line"));
    (fl_truthy(fl_eq(k, fl_str_val("Number"))) ? r_pair(p_advance(p), make_literal(fl_str_val("number"), v, line)) : (fl_truthy(fl_eq(k, fl_str_val("String"))) ? (fl_truthy(fl_or(string_contains_p(v, fl_str_n(2, fl_str_val("$"), fl_str_val("{"))), string_contains_p(v, fl_str_n(2, fl_str_val("#"), fl_str_val("{"))))) ? r_pair(p_advance(p), make_template_string(v, fl_vec_new(), line)) : r_pair(p_advance(p), make_literal(fl_str_val("string"), v, line))) : (fl_truthy(fl_eq(k, fl_str_val("Symbol"))) ? (fl_truthy(fl_eq(v, fl_str_val("true"))) ? r_pair(p_advance(p), make_literal(fl_str_val("boolean"), fl_bool(true), line)) : (fl_truthy(fl_eq(v, fl_str_val("false"))) ? r_pair(p_advance(p), make_literal(fl_str_val("boolean"), fl_bool(false), line)) : (fl_truthy(fl_or(fl_eq(v, fl_str_val("nil")), fl_eq(v, fl_str_val("null")))) ? r_pair(p_advance(p), make_literal(fl_str_val("nil"), fl_nil(), line)) : r_pair(p_advance(p), make_literal(fl_str_val("symbol"), v, line))))) : (fl_truthy(fl_eq(k, fl_str_val("Variable"))) ? r_pair(p_advance(p), make_variable(v, line)) : (fl_truthy(fl_eq(k, fl_str_val("Keyword"))) ? r_pair(p_advance(p), make_keyword(v, line)) : r_pair(p_advance(p), make_literal(fl_str_val("unknown"), v, line)))))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue hash_fn_op(FLValue node) {
    fl_push_frame_ln("hash_fn_op", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("literal"))) ? get(node, fl_str_val("value")) : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("variable"))) ? fl_str_n(2, fl_str_val("$"), get(node, fl_str_val("name"))) : fl_str_val("__apply__"))); fl_pop_frame(); return __fl_ret__; }
}

FLValue replace_pct(FLValue node) {
    fl_push_frame_ln("replace_pct", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy((fl_truthy((fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("literal"))) ? fl_eq(get(node, fl_str_val("type")), fl_str_val("symbol")) : fl_bool(false))) ? fl_eq(get(node, fl_str_val("value")), fl_str_val("%")) : fl_bool(false))) ? make_variable(fl_str_val("__pct__"), get(node, fl_str_val("line"))) : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("sexpr"))) ? make_sexpr(get(node, fl_str_val("op")), replace_pct_list(get(node, fl_str_val("args")), fl_int(0), fl_vec_new()), get(node, fl_str_val("line"))) : node)); fl_pop_frame(); return __fl_ret__; }
}

FLValue replace_pct_list(FLValue lst, FLValue i, FLValue acc) {
    fl_push_frame_ln("replace_pct_list", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(lst))) ? acc : replace_pct_list(lst, fl_add(i, fl_int(1)), fl_vec_push(acc, replace_pct(get(lst, i))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_hash_fn(FLValue p) {
    fl_push_frame_ln("parse_hash_fn", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue tok = p_peek(p);
    FLValue line = get(tok, fl_str_val("line"));
    FLValue p1 = p_advance(p);
    FLValue collected = parse_args(p1, fl_vec_new());
    FLValue p2 = get(collected, fl_str_val("p"));
    FLValue raw = get(collected, fl_str_val("node"));
    FLValue p3 = parse_consume_rparen(p2);
    FLValue body = (fl_truthy(fl_eq(length(raw), fl_int(0))) ? make_literal(fl_str_val("nil"), fl_nil(), line) : (fl_truthy(fl_eq(length(raw), fl_int(1))) ? get(raw, fl_int(0)) : make_sexpr(hash_fn_op(get(raw, fl_int(0))), fl_vec_rest(raw), line)));
    FLValue replaced = replace_pct(body);
    r_pair(p3, make_sexpr(fl_str_val("fn"), (__extension__ ({ FLValue __fl_lst[2] = {make_array_block((__extension__ ({ FLValue __fl_lst[1] = {make_variable(fl_str_val("__pct__"), line)}; fl_vec_from(__fl_lst, 1); })), line), replaced}; fl_vec_from(__fl_lst, 2); })), line));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue make_pipe_call(FLValue lhs, FLValue rhs, FLValue line) {
    fl_push_frame_ln("make_pipe_call", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(get(rhs, fl_str_val("kind")), fl_str_val("sexpr"))) ? make_sexpr(get(rhs, fl_str_val("op")), fl_concat(get(rhs, fl_str_val("args")), (__extension__ ({ FLValue __fl_lst[1] = {lhs}; fl_vec_from(__fl_lst, 1); }))), line) : make_sexpr(get(rhs, fl_str_val("value")), (__extension__ ({ FLValue __fl_lst[1] = {lhs}; fl_vec_from(__fl_lst, 1); })), line)); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_pipe_chain(FLValue p, FLValue lhs) {
    fl_push_frame_ln("parse_pipe_chain", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue nt = p_peek(p);
    (fl_truthy((fl_truthy(fl_not(null_p(nt))) ? fl_eq(get(nt, fl_str_val("kind")), fl_str_val("Pipe")) : fl_bool(false))) ? ((__extension__ ({
    FLValue p2 = p_advance(p);
    FLValue rhs_res = parse_expr_base(p2);
    FLValue p3 = get(rhs_res, fl_str_val("p"));
    FLValue rhs = get(rhs_res, fl_str_val("node"));
    FLValue line = get(nt, fl_str_val("line"));
    parse_pipe_chain(p3, make_pipe_call(lhs, rhs, line));
}))) : r_pair(p, lhs));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_expr_base(FLValue p) {
    fl_push_frame_ln("parse_expr_base", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue t = p_peek(p);
    FLValue k = get(t, fl_str_val("kind"));
    FLValue v = get(t, fl_str_val("value"));
    (fl_truthy(fl_eq(k, fl_str_val("LParen"))) ? parse_sexpr(p) : (fl_truthy(fl_eq(k, fl_str_val("LBracket"))) ? parse_bracket(p) : (fl_truthy(fl_eq(k, fl_str_val("LBrace"))) ? parse_map(p) : (fl_truthy(fl_eq(k, fl_str_val("HashParen"))) ? parse_hash_fn(p) : (fl_truthy((fl_truthy(fl_eq(k, fl_str_val("Unknown"))) ? fl_eq(v, fl_str_val("@")) : fl_bool(false))) ? ((__extension__ ({
    FLValue p1 = p_advance(p);
    FLValue res = parse_expr_base(p1);
    r_pair(get(res, fl_str_val("p")), make_sexpr(fl_str_val("deref"), (__extension__ ({ FLValue __fl_lst[1] = {get(res, fl_str_val("node"))}; fl_vec_from(__fl_lst, 1); })), get(t, fl_str_val("line"))));
}))) : parse_atom(p))))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_expr(FLValue p) {
    fl_push_frame_ln("parse_expr", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue res = parse_expr_base(p);
    parse_pipe_chain(get(res, fl_str_val("p")), get(res, fl_str_val("node")));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_sexpr(FLValue p) {
    fl_push_frame_ln("parse_sexpr", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue start_tok = p_peek(p);
    FLValue line = get(start_tok, fl_str_val("line"));
    FLValue p1 = p_advance(p);
    FLValue __fl_kw_first __attribute__((unused)) = parse_args(p1, fl_vec_new());
    FLValue p2 = get(__fl_kw_first, fl_str_val("p"));
    FLValue args = get(__fl_kw_first, fl_str_val("node"));
    (fl_truthy(fl_eq(length(args), fl_int(0))) ? r_pair(parse_consume_rparen(p2), make_sexpr(fl_str_val(""), fl_vec_new(), line)) : ((__extension__ ({
    FLValue op_node = get(args, fl_int(0));
    FLValue op = (fl_truthy(fl_eq(get(op_node, fl_str_val("kind")), fl_str_val("literal"))) ? get(op_node, fl_str_val("value")) : (fl_truthy(fl_eq(get(op_node, fl_str_val("kind")), fl_str_val("variable"))) ? fl_str_n(2, fl_str_val("$"), get(op_node, fl_str_val("name"))) : fl_str_val("__apply__")));
    FLValue __fl_kw_rest __attribute__((unused)) = (fl_truthy(fl_eq(op, fl_str_val("__apply__"))) ? args : substring(args, fl_int(1), length(args)));
    (fl_truthy(fl_eq(op, fl_str_val("try"))) ? ((__extension__ ({
    FLValue body = (fl_truthy(fl_gt(length(__fl_kw_rest), fl_int(0))) ? get(__fl_kw_rest, fl_int(0)) : fl_nil());
    FLValue catch_clause = (fl_truthy(fl_gt(length(__fl_kw_rest), fl_int(1))) ? get(__fl_kw_rest, fl_int(1)) : fl_nil());
    FLValue finally_clause = (fl_truthy(fl_gt(length(__fl_kw_rest), fl_int(2))) ? get(__fl_kw_rest, fl_int(2)) : fl_nil());
    r_pair(parse_consume_rparen(p2), make_try(body, catch_clause, finally_clause, line));
}))) : (fl_truthy(fl_eq(op, fl_str_val("loop"))) ? ((__extension__ ({
    FLValue loop_array = (fl_truthy(fl_gt(length(__fl_kw_rest), fl_int(0))) ? get(__fl_kw_rest, fl_int(0)) : fl_nil());
    FLValue _body_start __attribute__((unused)) = (fl_truthy(fl_gt(length(__fl_kw_rest), fl_int(1))) ? get(__fl_kw_rest, fl_int(1)) : fl_nil());
    (fl_truthy(null_p(loop_array)) ? r_pair(parse_consume_rparen(p2), make_sexpr(fl_str_val("loop"), fl_vec_new(), line)) : (fl_truthy(fl_not(fl_eq(get(loop_array, fl_str_val("kind")), fl_str_val("array")))) ? r_pair(parse_consume_rparen(p2), make_sexpr(fl_str_val("loop"), __fl_kw_rest, line)) : ((__extension__ ({
    FLValue items = get(loop_array, fl_str_val("items"));
    FLValue init = (fl_truthy(fl_gt(length(items), fl_int(0))) ? get(items, fl_int(0)) : fl_nil());
    FLValue condition = (fl_truthy(fl_gt(length(items), fl_int(1))) ? get(items, fl_int(1)) : fl_nil());
    FLValue __fl_kw_update __attribute__((unused)) = (fl_truthy(fl_gt(length(items), fl_int(2))) ? get(items, fl_int(2)) : fl_nil());
    FLValue body_exprs = substring(__fl_kw_rest, fl_int(1), length(__fl_kw_rest));
    r_pair(parse_consume_rparen(p2), make_loop(init, condition, __fl_kw_update, (fl_truthy(fl_eq(length(body_exprs), fl_int(1))) ? get(body_exprs, fl_int(0)) : make_sexpr(fl_str_val("do"), body_exprs, line)), line));
})))));
}))) : (fl_truthy(fl_eq(op, fl_str_val("and"))) ? r_pair(parse_consume_rparen(p2), (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("and"), fl_str_val("args"), __fl_kw_rest, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); }))) : (fl_truthy(fl_eq(op, fl_str_val("or"))) ? r_pair(parse_consume_rparen(p2), (__extension__ ({ FLValue __fl_kv[6] = {fl_str_val("kind"), fl_str_val("or"), fl_str_val("args"), __fl_kw_rest, fl_str_val("line"), line}; fl_map_from_pairs(__fl_kv, 3); }))) : (fl_truthy(fl_eq(op, fl_str_val("throw"))) ? r_pair(parse_consume_rparen(p2), make_throw((fl_truthy(fl_gt(length(__fl_kw_rest), fl_int(0))) ? get(__fl_kw_rest, fl_int(0)) : make_sexpr(fl_str_val("nil"), fl_vec_new(), line)), line)) : r_pair(parse_consume_rparen(p2), make_sexpr(op, __fl_kw_rest, line)))))));
}))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_consume_rparen(FLValue p) {
    fl_push_frame_ln("parse_consume_rparen", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue t = p_peek(p);
    (fl_truthy((fl_truthy(fl_not(null_p(t))) ? fl_eq(get(t, fl_str_val("kind")), fl_str_val("RParen")) : fl_bool(false))) ? p_advance(p) : p);
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_args(FLValue p, FLValue acc) {
    fl_push_frame_ln("parse_args", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue t = p_peek(p);
    (fl_truthy(null_p(t)) ? r_pair(p, acc) : (fl_truthy(fl_eq(get(t, fl_str_val("kind")), fl_str_val("RParen"))) ? r_pair(p, acc) : (fl_truthy(fl_eq(get(t, fl_str_val("kind")), fl_str_val("RBracket"))) ? r_pair(p, acc) : (fl_truthy(fl_eq(get(t, fl_str_val("kind")), fl_str_val("RBrace"))) ? r_pair(p, acc) : ((__extension__ ({
    FLValue one = parse_expr(p);
    parse_args(get(one, fl_str_val("p")), fl_vec_push(acc, get(one, fl_str_val("node"))));
})))))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_bracket(FLValue p) {
    fl_push_frame_ln("parse_bracket", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue tok = p_peek(p);
    FLValue line = get(tok, fl_str_val("line"));
    FLValue p1 = p_advance(p);
    FLValue next = p_peek(p1);
    (fl_truthy((fl_truthy((fl_truthy((fl_truthy(fl_not(null_p(next))) ? fl_eq(get(next, fl_str_val("kind")), fl_str_val("Symbol")) : fl_bool(false))) ? is_block_type_p(get(next, fl_str_val("value"))) : fl_bool(false))) ? fl_eq(get(next, fl_str_val("value")), upper_case(get(next, fl_str_val("value")))) : fl_bool(false))) ? parse_named_block(p1, line) : parse_array(p1, line));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue is_block_type_p(FLValue s) {
    fl_push_frame_ln("is_block_type_p", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue c = char_at(s, fl_int(0));
    (fl_truthy((fl_truthy(fl_gte(c, fl_str_val("A"))) ? fl_lte(c, fl_str_val("Z")) : fl_bool(false))) ? fl_not(fl_str_includes(s, fl_str_val("_"))) : fl_bool(false));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue upper_case(FLValue s) {
    fl_push_frame_ln("upper_case", __LINE__);
    { FLValue __fl_ret__ = s; fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_array(FLValue p, FLValue line) {
    fl_push_frame_ln("parse_array", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue collected = parse_args(p, fl_vec_new());
    FLValue p2 = get(collected, fl_str_val("p"));
    FLValue items = get(collected, fl_str_val("node"));
    r_pair(parse_consume_rbracket(p2), make_array_block(items, line));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_consume_rbracket(FLValue p) {
    fl_push_frame_ln("parse_consume_rbracket", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue t = p_peek(p);
    (fl_truthy((fl_truthy(fl_not(null_p(t))) ? fl_eq(get(t, fl_str_val("kind")), fl_str_val("RBracket")) : fl_bool(false))) ? p_advance(p) : p);
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_named_block(FLValue p, FLValue line) {
    fl_push_frame_ln("parse_named_block", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue type_tok = p_peek(p);
    FLValue __fl_kw_type __attribute__((unused)) = get(type_tok, fl_str_val("value"));
    FLValue p1 = p_advance(p);
    FLValue name_info = parse_optional_name(p1);
    FLValue p2 = get(name_info, fl_str_val("p"));
    FLValue name = get(name_info, fl_str_val("node"));
    FLValue fields_info = parse_block_fields(p2, fl_map_new());
    FLValue p3 = get(fields_info, fl_str_val("p"));
    FLValue fields = get(fields_info, fl_str_val("node"));
    r_pair(parse_consume_rbracket(p3), make_block(__fl_kw_type, name, fields, line));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_optional_name(FLValue p) {
    fl_push_frame_ln("parse_optional_name", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue t = p_peek(p);
    (fl_truthy((fl_truthy((fl_truthy(fl_not(null_p(t))) ? fl_eq(get(t, fl_str_val("kind")), fl_str_val("Symbol")) : fl_bool(false))) ? fl_not(fl_eq(char_at(get(t, fl_str_val("value")), fl_int(0)), fl_str_val(":"))) : fl_bool(false))) ? r_pair(p_advance(p), get(t, fl_str_val("value"))) : r_pair(p, fl_nil()));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_block_fields(FLValue p, FLValue acc) {
    fl_push_frame_ln("parse_block_fields", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(p_end_p(p)) ? r_pair(p, acc) : ((__extension__ ({
    FLValue t = p_peek(p);
    (fl_truthy(fl_eq(get(t, fl_str_val("kind")), fl_str_val("Keyword"))) ? ((__extension__ ({
    FLValue key = get(t, fl_str_val("value"));
    FLValue p1 = p_advance(p);
    FLValue val = parse_expr(p1);
    parse_block_fields(get(val, fl_str_val("p")), fl_map_set(acc, key, get(val, fl_str_val("node"))));
}))) : r_pair(p, acc));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_map(FLValue p) {
    fl_push_frame_ln("parse_map", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue tok = p_peek(p);
    FLValue line = get(tok, fl_str_val("line"));
    FLValue p1 = p_advance(p);
    FLValue collected = parse_args(p1, fl_vec_new());
    FLValue p2 = get(collected, fl_str_val("p"));
    FLValue items = get(collected, fl_str_val("node"));
    r_pair(parse_consume_rbrace(p2), make_map_block(items, line));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_consume_rbrace(FLValue p) {
    fl_push_frame_ln("parse_consume_rbrace", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue t = p_peek(p);
    (fl_truthy((fl_truthy(fl_not(null_p(t))) ? fl_eq(get(t, fl_str_val("kind")), fl_str_val("RBrace")) : fl_bool(false))) ? p_advance(p) : p);
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse_all(FLValue p) {
    fl_push_frame_ln("parse_all", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(p_end_p(p)) ? get(p, fl_str_val("ast")) : ((__extension__ ({
    FLValue one = parse_expr(p);
    FLValue p2 = p_append_ast(get(one, fl_str_val("p")), get(one, fl_str_val("node")));
    parse_all(p2);
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue parse(FLValue tokens) {
    fl_push_frame_ln("parse", __LINE__);
    { FLValue __fl_ret__ = parse_all(p_make(tokens)); fl_pop_frame(); return __fl_ret__; }
}

FLValue get_block_items(FLValue node) {
    fl_push_frame_ln("get_block_items", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(null_p(node)) ? fl_vec_new() : (fl_truthy((fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("block"))) ? fl_eq(get(node, fl_str_val("type")), fl_str_val("Array")) : fl_bool(false))) ? get(get(node, fl_str_val("fields")), fl_str_val("items")) : node)); fl_pop_frame(); return __fl_ret__; }
}

FLValue c_esc(FLValue s) {
    fl_push_frame_ln("c_esc", __LINE__);
    { FLValue __fl_ret__ = str_replace(str_replace(str_replace(str_replace(str_replace(s, fl_str_val("\\"), fl_str_val("\\\\")), fl_str_val("\""), fl_str_val("\\\"")), fl_str_val("\n"), fl_str_val("\\n")), fl_str_val("\t"), fl_str_val("\\t")), fl_str_val("\r"), fl_str_val("\\r")); fl_pop_frame(); return __fl_ret__; }
}

FLValue c_reserved_p(FLValue s) {
    fl_push_frame_ln("c_reserved_p", __LINE__);
    { FLValue __fl_ret__ = fl_includes_item((__extension__ ({ FLValue __fl_arr[72] = {fl_str_val("else"), fl_str_val("return"), fl_str_val("for"), fl_str_val("while"), fl_str_val("do"), fl_str_val("int"), fl_str_val("long"), fl_str_val("short"), fl_str_val("void"), fl_str_val("char"), fl_str_val("float"), fl_str_val("double"), fl_str_val("struct"), fl_str_val("union"), fl_str_val("enum"), fl_str_val("register"), fl_str_val("static"), fl_str_val("const"), fl_str_val("if"), fl_str_val("switch"), fl_str_val("case"), fl_str_val("break"), fl_str_val("extern"), fl_str_val("continue"), fl_str_val("default"), fl_str_val("goto"), fl_str_val("sizeof"), fl_str_val("auto"), fl_str_val("inline"), fl_str_val("unsigned"), fl_str_val("signed"), fl_str_val("volatile"), fl_str_val("typedef"), fl_str_val("restrict"), fl_str_val("bool"), fl_str_val("true"), fl_str_val("false"), fl_str_val("inc"), fl_str_val("dec"), fl_str_val("get"), fl_str_val("length"), fl_str_val("keys"), fl_str_val("vals"), fl_str_val("first"), fl_str_val("last"), fl_str_val("rest"), fl_str_val("nth"), fl_str_val("count"), fl_str_val("sort"), fl_str_val("reverse"), fl_str_val("range"), fl_str_val("merge"), fl_str_val("flatten"), fl_str_val("some"), fl_str_val("every"), fl_str_val("drop"), fl_str_val("take"), fl_str_val("zip"), fl_str_val("trim"), fl_str_val("join"), fl_str_val("split"), fl_str_val("num"), fl_str_val("atom"), fl_str_val("deref"), fl_str_val("identity"), fl_str_val("not"), fl_str_val("and"), fl_str_val("or"), fl_str_val("type"), fl_str_val("new"), fl_str_val("delete"), fl_str_val("update")}; fl_vec_from(__fl_arr, 72); })), s); fl_pop_frame(); return __fl_ret__; }
}

FLValue c_name(FLValue n) {
    fl_push_frame_ln("c_name", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue raw = str_replace(str_replace(str_replace(str_replace(n, fl_str_val("->"), fl_str_val("_to_")), fl_str_val("-"), fl_str_val("_")), fl_str_val("?"), fl_str_val("_p")), fl_str_val("!"), fl_str_val("_b"));
    (fl_truthy(c_reserved_p(raw)) ? fl_str_n(2, fl_str_val("__fl_kw_"), raw) : raw);
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc(FLValue n) {
    fl_push_frame_ln("cgc", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(null_p(n)) ? fl_str_val("fl_nil()") : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("raw-c"))) ? get(n, fl_str_val("code")) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("literal"))) ? cgc_literal(n) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("template-string"))) ? cgc_template_string(n) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("variable"))) ? ((__extension__ ({
    FLValue name = get(n, fl_str_val("name"));
    FLValue cn = c_name(name);
    FLValue line = get(n, fl_str_val("line"));
    (fl_truthy(fl_includes_item(fl_deref(known_defns_atom), cn)) ? fl_str_n(3, fl_str_val("fl_fn_new(__fl_wrap_"), cn, fl_str_val(", 0, NULL)")) : (fl_truthy(fl_not(fl_or(fl_includes_item(fl_deref(known_fncall_targets_atom), cn), fl_includes_item(fl_deref(outer_params_atom), cn)))) ? (__extension__ ({ fl_println(fl_str_n(3, fl_str_val("[FL Warn] 정의되지 않은 이름: "), name, (fl_truthy(null_p(line)) ? fl_str_val("") : fl_str_n(3, fl_str_val(" (line "), line, fl_str_val(")"))))); cn;  })) : cn));
}))) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("keyword"))) ? fl_str_n(3, fl_str_val("fl_str_val(\""), c_esc(get(n, fl_str_val("name"))), fl_str_val("\")")) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("sexpr"))) ? cgc_sexpr(n) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("block"))) ? cgc_block(n) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("and"))) ? cgc_and(get(n, fl_str_val("args"))) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("or"))) ? cgc_or(get(n, fl_str_val("args"))) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("try"))) ? cgc_try(n) : (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("throw"))) ? ((__extension__ ({
    FLValue line = get(n, fl_str_val("line"));
    FLValue expr_c = cgc(get(n, fl_str_val("expr")));
    (fl_truthy((fl_truthy(line) ? fl_gt(line, fl_int(0)) : fl_bool(false))) ? fl_str_n(5, fl_str_val("(__fl_throw_line="), to_string(line), fl_str_val(", fl_throw("), expr_c, fl_str_val("), fl_nil())")) : fl_str_n(3, fl_str_val("(fl_throw("), expr_c, fl_str_val("), fl_nil())")));
}))) : (__extension__ ({ fl_println(fl_str_n(4, fl_str_val("[CGC-ERR] unsupported IR kind="), get(n, fl_str_val("kind")), fl_str_val(" line="), get(n, fl_str_val("line")))); fl_str_val("fl_nil()");  })))))))))))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_op_wrapper(FLValue sym) {
    fl_push_frame_ln("cgc_op_wrapper", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(sym, fl_str_val("+"))) ? fl_str_val("fl_fn_new(__fl_op_add_w, 0, NULL)") : (fl_truthy(fl_eq(sym, fl_str_val("-"))) ? fl_str_val("fl_fn_new(__fl_op_sub_w, 0, NULL)") : (fl_truthy(fl_eq(sym, fl_str_val("*"))) ? fl_str_val("fl_fn_new(__fl_op_mul_w, 0, NULL)") : (fl_truthy(fl_eq(sym, fl_str_val("/"))) ? fl_str_val("fl_fn_new(__fl_op_div_w, 0, NULL)") : (fl_truthy(fl_eq(sym, fl_str_val("%"))) ? fl_str_val("fl_fn_new(__fl_op_mod_w, 0, NULL)") : (fl_truthy(fl_eq(sym, fl_str_val("mod"))) ? fl_str_val("fl_fn_new(__fl_op_mod_w, 0, NULL)") : (fl_truthy(fl_eq(sym, fl_str_val("="))) ? fl_str_val("fl_fn_new(__fl_op_eq_w, 0, NULL)") : (fl_truthy(fl_eq(sym, fl_str_val("!="))) ? fl_str_val("fl_fn_new(__fl_op_neq_w, 0, NULL)") : (fl_truthy(fl_eq(sym, fl_str_val("<"))) ? fl_str_val("fl_fn_new(__fl_op_lt_w, 0, NULL)") : (fl_truthy(fl_eq(sym, fl_str_val(">"))) ? fl_str_val("fl_fn_new(__fl_op_gt_w, 0, NULL)") : (fl_truthy(fl_eq(sym, fl_str_val("<="))) ? fl_str_val("fl_fn_new(__fl_op_lte_w, 0, NULL)") : (fl_truthy(fl_eq(sym, fl_str_val(">="))) ? fl_str_val("fl_fn_new(__fl_op_gte_w, 0, NULL)") : fl_nil())))))))))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue template_to_parts(FLValue s, FLValue prefix, FLValue acc) {
    fl_push_frame_ln("template_to_parts", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue idx = str_index_of(s, prefix);
    (fl_truthy(fl_eq(idx, fl_int(-1))) ? (fl_truthy(fl_gt(length(s), fl_int(0))) ? fl_vec_push(acc, fl_str_n(3, fl_str_val("fl_str_val(\""), c_esc(s), fl_str_val("\")"))) : acc) : ((__extension__ ({
    FLValue before = substring(s, fl_int(0), idx);
    FLValue __fl_kw_rest __attribute__((unused)) = substring(s, fl_add(idx, fl_int(2)), length(s));
    FLValue close = str_index_of(__fl_kw_rest, fl_str_val("}"));
    FLValue varname = str_trim(substring(__fl_kw_rest, fl_int(0), close));
    FLValue after = substring(__fl_kw_rest, fl_add(close, fl_int(1)), length(__fl_kw_rest));
    FLValue with_txt = (fl_truthy(fl_gt(length(before), fl_int(0))) ? fl_vec_push(acc, fl_str_n(3, fl_str_val("fl_str_val(\""), c_esc(before), fl_str_val("\")"))) : acc);
    FLValue with_var = fl_vec_push(with_txt, c_name(varname));
    template_to_parts(after, prefix, with_var);
}))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_template_string(FLValue n) {
    fl_push_frame_ln("cgc_template_string", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue v = get(n, fl_str_val("value"));
    FLValue prefix = (fl_truthy(string_contains_p(v, fl_str_n(2, fl_str_val("$"), fl_str_val("{")))) ? fl_str_n(2, fl_str_val("$"), fl_str_val("{")) : fl_str_n(2, fl_str_val("#"), fl_str_val("{")));
    FLValue parts = template_to_parts(v, prefix, fl_vec_new());
    (fl_truthy(fl_eq(length(parts), fl_int(0))) ? fl_str_val("fl_str_val(\"\")") : (fl_truthy(fl_eq(length(parts), fl_int(1))) ? get(parts, fl_int(0)) : fl_str_n(5, fl_str_val("fl_str_n("), length(parts), fl_str_val(", "), join(parts, fl_str_val(", ")), fl_str_val(")"))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_literal(FLValue n) {
    fl_push_frame_ln("cgc_literal", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue t = get(n, fl_str_val("type"));
    FLValue v = get(n, fl_str_val("value"));
    (fl_truthy(fl_eq(t, fl_str_val("number"))) ? (fl_truthy(fl_str_includes(fl_str_n(1, v), fl_str_val("."))) ? fl_str_n(3, fl_str_val("fl_float("), v, fl_str_val(")")) : fl_str_n(3, fl_str_val("fl_int("), v, fl_str_val(")"))) : (fl_truthy(fl_eq(t, fl_str_val("string"))) ? fl_str_n(3, fl_str_val("fl_str_val(\""), c_esc(v), fl_str_val("\")")) : (fl_truthy(fl_eq(t, fl_str_val("boolean"))) ? (fl_truthy(v) ? fl_str_val("fl_bool(true)") : fl_str_val("fl_bool(false)")) : (fl_truthy(fl_eq(t, fl_str_val("nil"))) ? fl_str_val("fl_nil()") : (fl_truthy(fl_eq(t, fl_str_val("symbol"))) ? (fl_truthy(fl_eq(v, fl_str_val("true"))) ? fl_str_val("fl_bool(true)") : (fl_truthy(fl_eq(v, fl_str_val("false"))) ? fl_str_val("fl_bool(false)") : (fl_truthy(fl_or(fl_eq(v, fl_str_val("nil")), fl_eq(v, fl_str_val("null")))) ? fl_str_val("fl_nil()") : ((__extension__ ({
    FLValue opw = cgc_op_wrapper(v);
    (fl_truthy(fl_not(null_p(opw))) ? opw : ((__extension__ ({
    FLValue cn = c_name(v);
    (fl_truthy(fl_includes_item(fl_deref(known_defns_atom), cn)) ? fl_str_n(3, fl_str_val("fl_fn_new(__fl_wrap_"), cn, fl_str_val(", 0, NULL)")) : cn);
}))));
})))))) : (__extension__ ({ fl_println(fl_str_n(4, fl_str_val("[CGC-ERR] unsupported literal type="), get(n, fl_str_val("type")), fl_str_val(" line="), get(n, fl_str_val("line")))); fl_str_val("fl_nil()");  })))))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_block(FLValue n) {
    fl_push_frame_ln("cgc_block", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue t = get(n, fl_str_val("type"));
    (fl_truthy(fl_eq(t, fl_str_val("FUNC"))) ? cgc_func_block(n) : (fl_truthy(fl_eq(t, fl_str_val("Array"))) ? cgc_array_block(n) : (fl_truthy(fl_eq(t, fl_str_val("Map"))) ? cgc_map_block(n) : (__extension__ ({ fl_println(fl_str_n(4, fl_str_val("[CGC-ERR] unsupported block type="), t, fl_str_val(" line="), get(n, fl_str_val("line")))); fl_str_val("fl_nil()");  })))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_func_block(FLValue n) {
    fl_push_frame_ln("cgc_func_block", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue name = c_name(get(n, fl_str_val("name")));
    FLValue f = get(n, fl_str_val("fields"));
    FLValue params_block = get(f, fl_str_val("params"));
    FLValue body_node = get(f, fl_str_val("body"));
    FLValue ps = cgc_params(get_block_items(params_block));
    FLValue body = cgc(body_node);
    fl_str_n(7, fl_str_val("FLValue "), name, fl_str_val("("), ps, fl_str_val(") {\n    return "), body, fl_str_val(";\n}"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_params(FLValue it) {
    fl_push_frame_ln("cgc_params", __LINE__);
    { FLValue __fl_ret__ = cgc_params_loop(it, fl_int(0), fl_str_val("")); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_params_loop(FLValue _it, FLValue _i, FLValue _acc) {
    fl_push_frame_ln("cgc_params_loop", __LINE__);
    __fl_tco_cgc_params_loop:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_it))) ? acc : ((__extension__ ({
    FLValue raw = cgc_extract_name(get(_it, i));
    FLValue n = (fl_truthy(fl_eq(raw, fl_str_val("_"))) ? fl_str_n(2, fl_str_val("__fl_ign_"), i) : raw);
    FLValue entry = fl_str_n(2, fl_str_val("FLValue "), n);
    (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = (fl_truthy(fl_eq(length(acc), fl_int(0))) ? entry : fl_str_n(3, acc, fl_str_val(", "), entry));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}));
}))));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_extract_name(FLValue node) {
    fl_push_frame_ln("cgc_extract_name", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("variable"))) ? c_name(get(node, fl_str_val("name"))) : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("literal"))) ? c_name(fl_str_n(1, get(node, fl_str_val("value")))) : fl_str_val("_anon"))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_try(FLValue n) {
    fl_push_frame_ln("cgc_try", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue id = fl_deref(lambda_id_atom);
    FLValue body_node = get(n, fl_str_val("body"));
    FLValue catch_node = get(n, fl_str_val("catch"));
    FLValue finally_node = get(n, fl_str_val("finally"));
    FLValue body_c = (fl_truthy(null_p(body_node)) ? fl_str_val("fl_nil()") : cgc(body_node));
    FLValue catch_args = (fl_truthy(null_p(catch_node)) ? fl_vec_new() : get(catch_node, fl_str_val("args")));
    FLValue param_c = (fl_truthy(fl_gt(length(catch_args), fl_int(0))) ? cgc_extract_name(get(catch_args, fl_int(0))) : fl_str_val("_fl_err"));
    FLValue __fl_ign_1 __attribute__((unused)) = fl_atom_reset(outer_params_atom, fl_vec_push(fl_atom_deref(outer_params_atom), param_c));
    FLValue catch_body_c = (fl_truthy(fl_gt(length(catch_args), fl_int(1))) ? cgc(get(catch_args, fl_int(1))) : fl_str_val("fl_nil()"));
    FLValue finally_c = (fl_truthy(null_p(finally_node)) ? fl_str_val("") : ((__extension__ ({
    FLValue fargs = get(finally_node, fl_str_val("args"));
    fl_str_n(3, fl_str_val("    "), cgc_body(fargs, fl_int(0), fl_str_val("")), fl_str_val(";\n"));
}))));
    fl_atom_reset(lambda_id_atom, fl_add(fl_atom_deref(lambda_id_atom), fl_int(1)));
    fl_str_n(42, fl_str_val("(__extension__ ({\n"), fl_str_val("    FLValue _fl_try_"), id, fl_str_val(";\n"), fl_str_val("    if (fl_try_top < FL_TRY_MAX) {\n"), fl_str_val("        FLTryFrame* _fl_frame_"), id, fl_str_val(" = &fl_try_stack[fl_try_top++];\n"), fl_str_val("        if (setjmp(_fl_frame_"), id, fl_str_val("->buf) == 0) {\n"), fl_str_val("            _fl_try_"), id, fl_str_val(" = "), body_c, fl_str_val(";\n"), fl_str_val("            fl_try_top--;\n"), fl_str_val("        } else {\n"), fl_str_val("            fl_try_top--;\n"), fl_str_val("            FLValue "), param_c, fl_str_val(" __attribute__((unused)) = _fl_frame_"), id, fl_str_val("->err;\n"), fl_str_val("            _fl_try_"), id, fl_str_val(" = "), catch_body_c, fl_str_val(";\n"), fl_str_val("        }\n"), fl_str_val("    } else {\n"), fl_str_val("        _fl_try_"), id, fl_str_val(" = "), body_c, fl_str_val(";\n"), fl_str_val("    }\n"), finally_c, fl_str_val("    _fl_try_"), id, fl_str_val(";\n"), fl_str_val("}))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_fncall(FLValue fn_c, FLValue args) {
    fl_push_frame_ln("cgc_fncall", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue argc = length(args);
    FLValue id = fl_deref(lambda_id_atom);
    fl_atom_reset(lambda_id_atom, fl_add(fl_atom_deref(lambda_id_atom), fl_int(1)));
    (fl_truthy(fl_eq(argc, fl_int(0))) ? fl_str_n(3, fl_str_val("fl_fn_call("), fn_c, fl_str_val(", 0, NULL)")) : fl_str_n(14, fl_str_val("(__extension__ ({ FLValue __fl_ca_"), id, fl_str_val("["), argc, fl_str_val("] = {"), cgc_args(args), fl_str_val("};"), fl_str_val(" fl_fn_call("), fn_c, fl_str_val(", "), argc, fl_str_val(", __fl_ca_"), id, fl_str_val("); }))")));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_sexpr(FLValue n) {
    fl_push_frame_ln("cgc_sexpr", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue op = get(n, fl_str_val("op"));
    FLValue args = get(n, fl_str_val("args"));
    (fl_truthy(fl_not(fl_string_p(op))) ? cgc_fncall(cgc(op), args) : (fl_truthy(fl_eq(op, fl_str_val("__apply__"))) ? cgc_fncall(cgc(get(args, fl_int(0))), substring(args, fl_int(1), length(args))) : (fl_truthy((fl_truthy(fl_gt(length(op), fl_int(0))) ? fl_eq(char_at(op, fl_int(0)), fl_str_val("$")) : fl_bool(false))) ? ((__extension__ ({
    FLValue cn = c_name(substring(op, fl_int(1), length(op)));
    cgc_fncall((fl_truthy(fl_includes_item(fl_deref(known_defns_atom), cn)) ? fl_str_n(3, fl_str_val("fl_fn_new(__fl_wrap_"), cn, fl_str_val(", 0, NULL)")) : cn), args);
}))) : cgc_dispatch(op, args))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_dispatch(FLValue op, FLValue args) {
    fl_push_frame_ln("cgc_dispatch", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(op, fl_str_val("if"))) ? cgc_if(args) : (fl_truthy(fl_eq(op, fl_str_val("cond"))) ? cgc_cond(args) : (fl_truthy(fl_eq(op, fl_str_val("do"))) ? cgc_do(args) : (fl_truthy(fl_eq(op, fl_str_val("begin"))) ? cgc_do(args) : (fl_truthy(fl_eq(op, fl_str_val("let"))) ? cgc_let(args) : (fl_truthy(fl_eq(op, fl_str_val("defn"))) ? cgc_defn(args) : (fl_truthy(fl_eq(op, fl_str_val("define"))) ? cgc_define(args) : (fl_truthy(fl_eq(op, fl_str_val("+"))) ? (__extension__ ({ cgc_warn_nil_get(fl_str_val("+"), args); cgc_warn_type_mix(fl_str_val("+"), args); cgc_binop_chain(args, fl_str_val("fl_add"));  })) : (fl_truthy(fl_eq(op, fl_str_val("add"))) ? (__extension__ ({ cgc_warn_nil_get(fl_str_val("add"), args); cgc_binop_chain(args, fl_str_val("fl_add"));  })) : (fl_truthy(fl_eq(op, fl_str_val("-"))) ? (__extension__ ({ cgc_warn_nil_get(fl_str_val("-"), args); (fl_truthy(fl_eq(length(args), fl_int(1))) ? fl_str_n(3, fl_str_val("fl_sub(fl_int(0), "), cgc(get(args, fl_int(0))), fl_str_val(")")) : cgc_binop_chain(args, fl_str_val("fl_sub")));  })) : (fl_truthy(fl_eq(op, fl_str_val("sub"))) ? (__extension__ ({ cgc_warn_nil_get(fl_str_val("sub"), args); cgc_binop_chain(args, fl_str_val("fl_sub"));  })) : (fl_truthy(fl_eq(op, fl_str_val("*"))) ? (__extension__ ({ cgc_warn_nil_get(fl_str_val("*"), args); cgc_warn_type_mix(fl_str_val("*"), args); cgc_binop_chain(args, fl_str_val("fl_mul"));  })) : (fl_truthy(fl_eq(op, fl_str_val("mul"))) ? (__extension__ ({ cgc_warn_nil_get(fl_str_val("mul"), args); cgc_binop_chain(args, fl_str_val("fl_mul"));  })) : (fl_truthy(fl_eq(op, fl_str_val("/"))) ? (__extension__ ({ cgc_warn_nil_get(fl_str_val("/"), args); fl_str_n(5, fl_str_val("fl_div("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")"));  })) : (fl_truthy(fl_eq(op, fl_str_val("div"))) ? (__extension__ ({ cgc_warn_nil_get(fl_str_val("div"), args); fl_str_n(5, fl_str_val("fl_div("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")"));  })) : (fl_truthy(fl_eq(op, fl_str_val("%"))) ? fl_str_n(5, fl_str_val("fl_mod("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("mod"))) ? fl_str_n(5, fl_str_val("fl_mod("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("="))) ? fl_str_n(5, fl_str_val("fl_eq("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("eq"))) ? fl_str_n(5, fl_str_val("fl_eq("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("!="))) ? fl_str_n(5, fl_str_val("fl_neq("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("neq"))) ? fl_str_n(5, fl_str_val("fl_neq("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("<"))) ? fl_str_n(5, fl_str_val("fl_lt("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("lt"))) ? fl_str_n(5, fl_str_val("fl_lt("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val(">"))) ? fl_str_n(5, fl_str_val("fl_gt("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("gt"))) ? fl_str_n(5, fl_str_val("fl_gt("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("<="))) ? fl_str_n(5, fl_str_val("fl_lte("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("lte"))) ? fl_str_n(5, fl_str_val("fl_lte("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val(">="))) ? fl_str_n(5, fl_str_val("fl_gte("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("gte"))) ? fl_str_n(5, fl_str_val("fl_gte("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("and"))) ? cgc_and(args) : (fl_truthy(fl_eq(op, fl_str_val("or"))) ? cgc_or(args) : (fl_truthy(fl_eq(op, fl_str_val("not"))) ? fl_str_n(3, fl_str_val("fl_not("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("println"))) ? fl_str_n(3, fl_str_val("fl_println("), cgc_str_arg(args), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("print"))) ? fl_str_n(3, fl_str_val("fl_print("), cgc_str_arg(args), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str"))) ? cgc_str(args) : (fl_truthy(fl_eq(op, fl_str_val("set!"))) ? cgc_set_b(args) : (fl_truthy(fl_eq(op, fl_str_val("while"))) ? cgc_while(args) : (fl_truthy(fl_eq(op, fl_str_val("atom"))) ? fl_str_n(3, fl_str_val("fl_atom_new("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("deref"))) ? fl_str_n(3, fl_str_val("fl_deref("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("reset!"))) ? fl_str_n(5, fl_str_val("fl_atom_reset("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("swap!"))) ? cgc_swap_b(args) : (fl_truthy(fl_eq(op, fl_str_val("num"))) ? fl_str_n(3, fl_str_val("num("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("includes-item"))) ? fl_str_n(5, fl_str_val("fl_includes_item("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-includes"))) ? fl_str_n(5, fl_str_val("fl_str_includes("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-starts-with"))) ? fl_str_n(5, fl_str_val("fl_str_starts_with("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-ends-with"))) ? fl_str_n(5, fl_str_val("fl_str_ends_with("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("string?"))) ? fl_str_n(3, fl_str_val("fl_string_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("obj-entries"))) ? fl_str_n(3, fl_str_val("fl_map_entries("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("length"))) ? fl_str_n(3, fl_str_val("length("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("count"))) ? fl_str_n(3, fl_str_val("length("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("get"))) ? fl_str_n(5, fl_str_val("get("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("assoc"))) ? fl_str_n(7, fl_str_val("fl_map_set("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("contains?"))) ? fl_str_n(5, fl_str_val("fl_contains_p("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("apply"))) ? fl_str_n(5, fl_str_val("fl_apply("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("assoc-in"))) ? fl_str_n(7, fl_str_val("fl_assoc_in("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("update-in"))) ? fl_str_n(7, fl_str_val("fl_update_in("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("inspect"))) ? fl_str_n(3, fl_str_val("fl_inspect("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("pp"))) ? fl_str_n(3, fl_str_val("fl_pp("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("format"))) ? fl_str_n(5, fl_str_val("fl_format("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("merge"))) ? fl_str_n(5, fl_str_val("fl_merge("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("base64-encode"))) ? fl_str_n(3, fl_str_val("fl_base64_encode("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("base64-decode"))) ? fl_str_n(3, fl_str_val("fl_base64_decode("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("dissoc"))) ? fl_str_n(5, fl_str_val("fl_dissoc("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("keys"))) ? fl_str_n(3, fl_str_val("fl_keys("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("vals"))) ? fl_str_n(3, fl_str_val("fl_vals("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("select-keys"))) ? fl_str_n(5, fl_str_val("fl_select_keys("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("update"))) ? fl_str_n(7, fl_str_val("fl_update("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("reverse"))) ? fl_str_n(3, fl_str_val("reverse("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("sort"))) ? fl_str_n(3, fl_str_val("sort("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("sort-by"))) ? fl_str_n(5, fl_str_val("fl_sort_by("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("range"))) ? fl_str_n(3, fl_str_val("range("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("take"))) ? fl_str_n(5, fl_str_val("take("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("drop"))) ? fl_str_n(5, fl_str_val("drop("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("last"))) ? fl_str_n(3, fl_str_val("fl_vec_last("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("nth"))) ? fl_str_n(5, fl_str_val("get("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("concat"))) ? fl_str_n(5, fl_str_val("fl_concat("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("uuid"))) ? fl_str_val("uuid()") : (fl_truthy(fl_eq(op, fl_str_val("obj-merge"))) ? fl_str_n(5, fl_str_val("fl_map_merge("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("obj-pick"))) ? fl_str_n(5, fl_str_val("select_keys("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("obj-omit"))) ? fl_str_n(5, fl_str_val("fl_obj_omit("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("obj-values"))) ? fl_str_n(3, fl_str_val("vals("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("vals"))) ? fl_str_n(3, fl_str_val("vals("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("entries"))) ? fl_str_n(3, fl_str_val("entries("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("get-in"))) ? fl_str_n(5, fl_str_val("fl_get_in("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("map-vals"))) ? fl_str_n(5, fl_str_val("fl_map_vals_fn("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-split"))) ? fl_str_n(5, fl_str_val("str_split("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-slice"))) ? fl_str_n(7, fl_str_val("substring("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-to-upper"))) ? fl_str_n(3, fl_str_val("str_to_upper("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-to-lower"))) ? fl_str_n(3, fl_str_val("str_to_lower("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-trim"))) ? fl_str_n(3, fl_str_val("str_trim("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-pad-left"))) ? fl_str_n(7, fl_str_val("str_pad_left("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-pad-right"))) ? fl_str_n(7, fl_str_val("str_pad_right("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-index-of"))) ? fl_str_n(5, fl_str_val("str_index_of("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-repeat"))) ? fl_str_n(5, fl_str_val("str_repeat("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-length"))) ? fl_str_n(3, fl_str_val("length("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-to-num"))) ? fl_str_n(3, fl_str_val("fl_str_to_num("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("html-escape"))) ? fl_str_n(3, fl_str_val("fl_html_escape("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("number?"))) ? fl_str_n(3, fl_str_val("fl_number_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("boolean?"))) ? fl_str_n(3, fl_str_val("fl_boolean_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("integer?"))) ? fl_str_n(3, fl_str_val("fl_integer_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("float?"))) ? fl_str_n(3, fl_str_val("fl_float_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("array?"))) ? fl_str_n(3, fl_str_val("fl_array_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("vector?"))) ? fl_str_n(3, fl_str_val("fl_array_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("map?"))) ? fl_str_n(3, fl_str_val("fl_map_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("fn?"))) ? fl_str_n(3, fl_str_val("fl_fn_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("type-of"))) ? fl_str_n(3, fl_str_val("type_of("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("empty?"))) ? fl_str_n(3, fl_str_val("fl_empty_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("not-empty?"))) ? fl_str_n(3, fl_str_val("fl_not_empty_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("nil-or-empty?"))) ? fl_str_n(3, fl_str_val("fl_nil_or_empty_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("loop"))) ? cgc_loop(args) : (fl_truthy(fl_eq(op, fl_str_val("recur"))) ? fl_str_val("/* orphan recur */ fl_nil()") : (fl_truthy(fl_eq(op, fl_str_val("map"))) ? fl_str_n(5, fl_str_val("fl_map_fn("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("filter"))) ? fl_str_n(5, fl_str_val("fl_filter_fn("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("reduce"))) ? fl_str_n(7, fl_str_val("fl_reduce_fn("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("append"))) ? fl_str_n(5, fl_str_val("fl_vec_push("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("push"))) ? fl_str_n(5, fl_str_val("fl_vec_push("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("vec-builder"))) ? fl_str_val("fl_vec_builder_new()") : (fl_truthy(fl_eq(op, fl_str_val("freeze-builder"))) ? fl_str_n(3, fl_str_val("fl_vec_builder_freeze("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("flatten"))) ? fl_str_n(3, fl_str_val("flatten("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("flatten-1"))) ? fl_str_n(3, fl_str_val("flatten("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("distinct"))) ? fl_str_n(3, fl_str_val("distinct("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("zip"))) ? fl_str_n(5, fl_str_val("zip("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("zip-with"))) ? fl_str_n(5, fl_str_val("zip("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("group-by"))) ? fl_str_n(5, fl_str_val("group_by("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("frequencies"))) ? fl_str_n(3, fl_str_val("frequencies("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("every?"))) ? fl_str_n(5, fl_str_val("fl_every_p("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("any?"))) ? fl_str_n(5, fl_str_val("fl_any_p("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("none?"))) ? fl_str_n(5, fl_str_val("fl_none_p("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("find-first"))) ? fl_str_n(5, fl_str_val("fl_find_first("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("count-if"))) ? fl_str_n(5, fl_str_val("fl_count_if("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("repeat"))) ? fl_str_n(5, fl_str_val("fl_repeat("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("map-indexed"))) ? fl_str_n(5, fl_str_val("fl_map_indexed("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("mapcat"))) ? fl_str_n(5, fl_str_val("fl_mapcat("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("keep"))) ? fl_str_n(5, fl_str_val("fl_keep("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("comp"))) ? fl_str_n(5, fl_str_val("fl_comp("), cgc_args(args), fl_str_val(", "), length(args), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("conj"))) ? fl_str_n(5, fl_str_val("fl_conj("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("into"))) ? fl_str_n(5, fl_str_val("fl_into("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("keys"))) ? fl_str_n(3, fl_str_val("fl_map_keys("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("obj-keys"))) ? fl_str_n(3, fl_str_val("fl_map_keys("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("map-entries"))) ? fl_str_n(3, fl_str_val("fl_map_entries("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("null?"))) ? fl_str_n(3, fl_str_val("null_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("nil?"))) ? fl_str_n(3, fl_str_val("null_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("char-at"))) ? fl_str_n(5, fl_str_val("char_at("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("char-code-at"))) ? fl_str_n(5, fl_str_val("char_code_at("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("floor"))) ? fl_str_n(3, fl_str_val("fl_floor("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("ceil"))) ? fl_str_n(3, fl_str_val("fl_ceil("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("abs"))) ? fl_str_n(3, fl_str_val("fl_abs("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("math-sqrt"))) ? fl_str_n(3, fl_str_val("fl_math_sqrt("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("now"))) ? fl_str_val("fl_now()") : (fl_truthy(fl_eq(op, fl_str_val("now-ms"))) ? fl_str_val("fl_now_ms()") : (fl_truthy(fl_eq(op, fl_str_val("str-join"))) ? fl_str_n(5, fl_str_val("join("), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-replace"))) ? fl_str_n(7, fl_str_val("str_replace("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-replace-re"))) ? fl_str_n(7, fl_str_val("str_replace_re("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-replace-all-re"))) ? fl_str_n(7, fl_str_val("str_replace_all_re("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-match"))) ? fl_str_n(5, fl_str_val("str_match("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-match-all"))) ? fl_str_n(5, fl_str_val("str_match_all("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("str-test"))) ? fl_str_n(5, fl_str_val("str_test("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("future"))) ? fl_str_n(3, fl_str_val("fl_future("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("deref-timeout"))) ? fl_str_n(5, fl_str_val("fl_deref_timeout("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("future-done?"))) ? fl_str_n(3, fl_str_val("fl_future_done("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("substring"))) ? fl_str_n(7, fl_str_val("substring("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("trim"))) ? fl_str_n(3, fl_str_val("trim("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("slice"))) ? fl_str_n(7, fl_str_val("substring("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("list"))) ? cgc_list(args) : (fl_truthy(fl_eq(op, fl_str_val("_fl_map_set"))) ? fl_str_n(7, fl_str_val("fl_map_set("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_read"))) ? fl_str_n(3, fl_str_val("fl_file_read("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_write"))) ? fl_str_n(5, fl_str_val("fl_file_write("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-read"))) ? fl_str_n(3, fl_str_val("fl_file_read("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-write"))) ? fl_str_n(5, fl_str_val("fl_file_write("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-exists"))) ? fl_str_n(3, fl_str_val("file_exists("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file_exists"))) ? fl_str_n(3, fl_str_val("file_exists("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_append"))) ? fl_str_n(5, fl_str_val("_fl_file_append("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-append"))) ? fl_str_n(5, fl_str_val("_fl_file_append("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_copy"))) ? fl_str_n(5, fl_str_val("_fl_file_copy("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_delete"))) ? fl_str_n(3, fl_str_val("_fl_file_delete("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-delete"))) ? fl_str_n(3, fl_str_val("_fl_file_delete("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_mkdir"))) ? fl_str_n(3, fl_str_val("_fl_file_mkdir("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-mkdir"))) ? fl_str_n(3, fl_str_val("_fl_file_mkdir("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_rmdir"))) ? fl_str_n(3, fl_str_val("_fl_file_rmdir("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_list"))) ? fl_str_n(3, fl_str_val("_fl_file_list("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-list"))) ? fl_str_n(3, fl_str_val("_fl_file_list("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_size"))) ? fl_str_n(3, fl_str_val("_fl_file_size("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_modified"))) ? fl_str_n(3, fl_str_val("_fl_file_modified("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_rename"))) ? fl_str_n(5, fl_str_val("_fl_file_rename("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_is_file"))) ? fl_str_n(3, fl_str_val("_fl_file_is_file("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-is-file?"))) ? fl_str_n(3, fl_str_val("_fl_file_is_file("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_file_is_dir"))) ? fl_str_n(3, fl_str_val("_fl_file_is_dir("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-is-dir?"))) ? fl_str_n(3, fl_str_val("_fl_file_is_dir("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_env_get"))) ? fl_str_n(3, fl_str_val("_fl_env_get("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("env-get"))) ? fl_str_n(3, fl_str_val("_fl_env_get("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_env_set"))) ? fl_str_n(5, fl_str_val("_fl_env_set("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_env_all"))) ? fl_str_val("_fl_env_all()") : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_run"))) ? fl_str_n(3, fl_str_val("_fl_process_run("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("process-run"))) ? fl_str_n(3, fl_str_val("_fl_process_run("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_run_args"))) ? fl_str_n(5, fl_str_val("_fl_process_run_args("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_exec"))) ? fl_str_n(3, fl_str_val("_fl_process_exec("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_exec_args"))) ? fl_str_n(5, fl_str_val("_fl_process_exec_args("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_spawn"))) ? fl_str_n(5, fl_str_val("_fl_process_spawn("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_kill"))) ? fl_str_n(3, fl_str_val("_fl_process_kill("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_wait"))) ? fl_str_n(3, fl_str_val("_fl_process_wait("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_exists"))) ? fl_str_n(3, fl_str_val("_fl_process_exists("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_getcwd"))) ? fl_str_val("_fl_process_getcwd()") : (fl_truthy(fl_eq(op, fl_str_val("process-cwd"))) ? fl_str_val("_fl_process_getcwd()") : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_chdir"))) ? fl_str_n(3, fl_str_val("_fl_process_chdir("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_pid"))) ? fl_str_val("_fl_process_pid()") : (fl_truthy(fl_eq(op, fl_str_val("process-pid"))) ? fl_str_val("_fl_process_pid()") : (fl_truthy(fl_eq(op, fl_str_val("_fl_process_ppid"))) ? fl_str_val("_fl_process_ppid()") : (fl_truthy(fl_eq(op, fl_str_val("_fl_run_inherit"))) ? fl_str_n(3, fl_str_val("_fl_run_inherit("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("run-inherit"))) ? fl_str_n(3, fl_str_val("_fl_run_inherit("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("cli-args"))) ? fl_str_val("fl_get_argv()") : (fl_truthy(fl_eq(op, fl_str_val("bit-xor"))) ? fl_str_n(5, fl_str_val("fl_bit_xor("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("bit-and"))) ? fl_str_n(5, fl_str_val("fl_bit_and("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("bit-or"))) ? fl_str_n(5, fl_str_val("fl_bit_or("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("bit-shl"))) ? fl_str_n(5, fl_str_val("fl_bit_shl("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("bit-shr"))) ? fl_str_n(5, fl_str_val("fl_bit_shr("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("json-parse"))) ? fl_str_n(3, fl_str_val("fl_json_parse("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("json-stringify"))) ? fl_str_n(3, fl_str_val("fl_json_stringify("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("run-parallel"))) ? fl_str_n(5, fl_str_val("fl_run_parallel("), cgc(get(args, fl_int(0))), fl_str_val(", "), (fl_truthy(fl_gte(length(args), fl_int(2))) ? cgc(get(args, fl_int(1))) : fl_str_val("fl_int(0)")), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-get"))) ? fl_str_n(3, fl_str_val("http_get("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-post"))) ? fl_str_n(5, fl_str_val("http_post("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-put"))) ? fl_str_n(5, fl_str_val("http_put("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-del"))) ? fl_str_n(3, fl_str_val("http_del("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-patch"))) ? fl_str_n(5, fl_str_val("http_patch("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-get-headers"))) ? fl_str_n(5, fl_str_val("http_get_h("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-post-headers"))) ? fl_str_n(7, fl_str_val("http_post_h("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-req"))) ? fl_str_n(9, fl_str_val("http_req("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(", "), cgc(get(args, fl_int(3))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-body"))) ? fl_str_n(3, fl_str_val("http_body("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-status"))) ? fl_str_n(3, fl_str_val("http_status("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("http-ok?"))) ? fl_str_n(3, fl_str_val("http_ok_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("sleep"))) ? fl_str_n(3, fl_str_val("fl_sleep_ms("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("now-ms"))) ? fl_str_val("fl_now_ms()") : (fl_truthy(fl_eq(op, fl_str_val("server-start"))) ? fl_str_n(3, fl_str_val("fl_http_start("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-stop"))) ? fl_str_val("fl_http_stop()") : (fl_truthy(fl_eq(op, fl_str_val("server-html"))) ? fl_str_n(3, fl_str_val("fl_resp_html("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-json"))) ? fl_str_n(3, fl_str_val("fl_resp_json("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-status"))) ? fl_str_n(5, fl_str_val("fl_resp_status("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-redirect"))) ? fl_str_n(3, fl_str_val("fl_resp_redirect("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-html-cookie"))) ? fl_str_n(5, fl_str_val("fl_resp_html_cookie("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-set-cookie"))) ? fl_str_n(7, fl_str_val("fl_resp_set_cookie("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-route"))) ? fl_str_n(7, fl_str_val("fl_http_route("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-get"))) ? fl_str_n(5, fl_str_val("fl_http_route(fl_str_val(\"GET\"), "), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("server-post"))) ? fl_str_n(5, fl_str_val("fl_http_route(fl_str_val(\"POST\"), "), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("db-open"))) ? fl_str_n(3, fl_str_val("fl_db_open("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("db-close"))) ? fl_str_n(3, fl_str_val("fl_db_close("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("db-query"))) ? fl_str_n(7, fl_str_val("fl_db_query("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("db-exec"))) ? fl_str_n(7, fl_str_val("fl_db_exec("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("fn"))) ? cgc_fn(args) : (fl_truthy(fl_eq(op, fl_str_val("cg-and"))) ? fl_str_n(5, fl_str_val("fl_and("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("cg-or"))) ? fl_str_n(5, fl_str_val("fl_or("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("cg-match"))) ? fl_str_val("fl_nil()") : (fl_truthy(fl_eq(op, fl_str_val("when"))) ? ((__extension__ ({
    FLValue cond_c = cgc(get(args, fl_int(0)));
    FLValue body = substring(args, fl_int(1), length(args));
    FLValue body_c = (fl_truthy(fl_eq(length(body), fl_int(1))) ? cgc(get(body, fl_int(0))) : cgc_do(body));
    fl_str_n(5, fl_str_val("(fl_truthy("), cond_c, fl_str_val(") ? "), body_c, fl_str_val(" : fl_nil())"));
}))) : (fl_truthy(fl_eq(op, fl_str_val("unless"))) ? ((__extension__ ({
    FLValue cond_c = cgc(get(args, fl_int(0)));
    FLValue body = substring(args, fl_int(1), length(args));
    FLValue body_c = (fl_truthy(fl_eq(length(body), fl_int(1))) ? cgc(get(body, fl_int(0))) : cgc_do(body));
    fl_str_n(5, fl_str_val("(!fl_truthy("), cond_c, fl_str_val(") ? "), body_c, fl_str_val(" : fl_nil())"));
}))) : (fl_truthy(fl_eq(op, fl_str_val("for-each"))) ? fl_str_n(5, fl_str_val("(fl_for_each("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val("), fl_nil())")) : (fl_truthy(fl_eq(op, fl_str_val("throw"))) ? fl_str_n(3, fl_str_val("(fl_throw("), cgc(get(args, fl_int(0))), fl_str_val("), fl_nil())")) : (fl_truthy(fl_eq(op, fl_str_val("auth-jwt-sign"))) ? fl_str_n(7, fl_str_val("fl_jwt_sign("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("auth-jwt-verify"))) ? fl_str_n(5, fl_str_val("fl_jwt_verify("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("auth-jwt-expired"))) ? fl_str_n(3, fl_str_val("fl_jwt_expired("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("auth-jwt-decode"))) ? fl_str_n(3, fl_str_val("fl_jwt_verify("), cgc(get(args, fl_int(0))), fl_str_val(", fl_str_val(\"\"))")) : (fl_truthy(fl_eq(op, fl_str_val("auth-hash-password"))) ? fl_str_n(3, fl_str_val("fl_hash_password("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("auth-verify-password"))) ? fl_str_n(5, fl_str_val("fl_verify_password("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("->"))) ? cgc_thread_first(args) : (fl_truthy(fl_eq(op, fl_str_val("->>"))) ? cgc_thread_last(args) : (fl_truthy(fl_eq(op, fl_str_val("when"))) ? fl_str_n(5, fl_str_val("(fl_truthy("), cgc(get(args, fl_int(0))), fl_str_val(") ? ("), cgc_body(args, fl_int(1), fl_str_val("")), fl_str_val(") : fl_nil())")) : (fl_truthy(fl_eq(op, fl_str_val("unless"))) ? fl_str_n(5, fl_str_val("(fl_truthy("), cgc(get(args, fl_int(0))), fl_str_val(") ? fl_nil() : ("), cgc_body(args, fl_int(1), fl_str_val("")), fl_str_val("))")) : (fl_truthy(fl_eq(op, fl_str_val("case"))) ? cgc_case(args) : (fl_truthy(fl_eq(op, fl_str_val("match"))) ? cgc_match(args) : (fl_truthy(fl_eq(op, fl_str_val("if-let"))) ? cgc_if_let(args) : (fl_truthy(fl_eq(op, fl_str_val("when-let"))) ? cgc_when_let(args) : (fl_truthy(fl_eq(op, fl_str_val("doseq"))) ? cgc_doseq(args) : (fl_truthy(fl_eq(op, fl_str_val("dotimes"))) ? cgc_dotimes(args) : (fl_truthy(fl_eq(op, fl_str_val("doto"))) ? cgc_doto(args) : (fl_truthy(fl_eq(op, fl_str_val("for"))) ? cgc_for(args) : (fl_truthy(fl_eq(op, fl_str_val("sha256"))) ? fl_str_n(3, fl_str_val("sha256("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("md5"))) ? fl_str_n(3, fl_str_val("md5("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("random"))) ? fl_str_val("fl_random()") : (fl_truthy(fl_eq(op, fl_str_val("max-by"))) ? fl_str_n(5, fl_str_val("max_by("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("min-by"))) ? fl_str_n(5, fl_str_val("min_by("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("clamp"))) ? fl_str_n(7, fl_str_val("clamp("), cgc(get(args, fl_int(0))), fl_str_val(", "), cgc(get(args, fl_int(1))), fl_str_val(", "), cgc(get(args, fl_int(2))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("json-pretty"))) ? fl_str_n(3, fl_str_val("json_pretty("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("file-exists?"))) ? fl_str_n(3, fl_str_val("file_exists_p("), cgc(get(args, fl_int(0))), fl_str_val(")")) : cgc_dispatch_fallback(op, args))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_swap_b(FLValue args) {
    fl_push_frame_ln("cgc_swap_b", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue atom_c = cgc(get(args, fl_int(0)));
    FLValue fn_node = get(args, fl_int(1));
    FLValue extra = substring(args, fl_int(2), length(args));
    ((__extension__ ({
    FLValue is_lambda = (fl_truthy(fl_eq(get(fn_node, fl_str_val("kind")), fl_str_val("sexpr"))) ? fl_eq(get(fn_node, fl_str_val("op")), fl_str_val("fn")) : fl_bool(false));
    (fl_truthy(is_lambda) ? fl_str_n(5, fl_str_val("swap_bang("), atom_c, fl_str_val(", "), cgc(fn_node), fl_str_val(")")) : ((__extension__ ({
    FLValue __fl_kw_deref __attribute__((unused)) = fl_str_n(3, fl_str_val("fl_atom_deref("), atom_c, fl_str_val(")"));
    FLValue op = get(fn_node, fl_str_val("value"));
    FLValue ec = cgc_args(extra);
    ((__extension__ ({
    FLValue rhs = (fl_truthy(fl_eq(op, fl_str_val("+"))) ? fl_str_n(5, fl_str_val("fl_add("), __fl_kw_deref, fl_str_val(", "), ec, fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("-"))) ? fl_str_n(5, fl_str_val("fl_sub("), __fl_kw_deref, fl_str_val(", "), ec, fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("*"))) ? fl_str_n(5, fl_str_val("fl_mul("), __fl_kw_deref, fl_str_val(", "), ec, fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("push"))) ? fl_str_n(5, fl_str_val("fl_vec_push("), __fl_kw_deref, fl_str_val(", "), ec, fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("append"))) ? fl_str_n(5, fl_str_val("fl_vec_push("), __fl_kw_deref, fl_str_val(", "), ec, fl_str_val(")")) : fl_str_n(5, c_name(op), fl_str_val("("), __fl_kw_deref, (fl_truthy(fl_eq(length(extra), fl_int(0))) ? fl_str_val("") : fl_str_n(2, fl_str_val(", "), ec)), fl_str_val(")")))))));
    fl_str_n(5, fl_str_val("fl_atom_reset("), atom_c, fl_str_val(", "), rhs, fl_str_val(")"));
})));
}))));
})));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_is_get_p(FLValue n) {
    fl_push_frame_ln("cgc_is_get_p", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("sexpr"))) ? fl_eq(get(n, fl_str_val("op")), fl_str_val("get")) : fl_bool(false)); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_warn_nil_get(FLValue op, FLValue args) {
    fl_push_frame_ln("cgc_warn_nil_get", __LINE__);
    { FLValue __fl_ret__ = fl_map_fn((__extension__ ({ FLValue __env_2[1] = {op}; fl_fn_new(__fl_anon_2, 1, __env_2); })), args); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_is_str_lit_p(FLValue n) {
    fl_push_frame_ln("cgc_is_str_lit_p", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("literal"))) ? fl_eq(get(n, fl_str_val("type")), fl_str_val("string")) : fl_bool(false)); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_is_num_lit_p(FLValue n) {
    fl_push_frame_ln("cgc_is_num_lit_p", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("literal"))) ? fl_eq(get(n, fl_str_val("type")), fl_str_val("number")) : fl_bool(false)); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_warn_type_mix(FLValue op, FLValue args) {
    fl_push_frame_ln("cgc_warn_type_mix", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue has_str = fl_gt(length(fl_filter_fn(fl_fn_new(__fl_wrap_cgc_is_str_lit_p, 0, NULL), args)), fl_int(0));
    FLValue has_num = fl_gt(length(fl_filter_fn(fl_fn_new(__fl_wrap_cgc_is_num_lit_p, 0, NULL), args)), fl_int(0));
    (fl_truthy((fl_truthy(has_str) ? has_num : fl_bool(false))) ? fl_println(fl_str_n(3, fl_str_val("[FL Warn] 타입 불일치: "), op, fl_str_val(" — 문자열과 숫자 혼용"))) : fl_nil());
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_dispatch_fallback(FLValue op, FLValue args) {
    fl_push_frame_ln("cgc_dispatch_fallback", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(op, fl_str_val("first"))) ? fl_str_n(3, fl_str_val("fl_vec_first("), cgc(get(args, fl_int(0))), fl_str_val(")")) : (fl_truthy(fl_eq(op, fl_str_val("rest"))) ? fl_str_n(3, fl_str_val("fl_vec_rest("), cgc(get(args, fl_int(0))), fl_str_val(")")) : ((__extension__ ({
    FLValue cn = c_name(op);
    FLValue known_arity = get(fl_deref(defn_arity_atom), cn);
    (fl_truthy((fl_truthy(fl_not(null_p(known_arity))) ? fl_not(fl_eq(length(args), known_arity)) : fl_bool(false))) ? fl_println(fl_str_n(6, fl_str_val("[FL Warn] arity: "), op, fl_str_val(" expects "), known_arity, fl_str_val(" args, got "), length(args))) : fl_nil());
    (fl_truthy(fl_includes_item(fl_deref(known_fncall_targets_atom), cn)) ? cgc_fncall(cn, args) : fl_str_n(4, cn, fl_str_val("("), cgc_args(args), fl_str_val(")")));
}))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_fn_argv_decls(FLValue items, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_fn_argv_decls", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(items))) ? acc : ((__extension__ ({
    FLValue raw = c_name(cgc_extract_name(get(items, i)));
    FLValue name = (fl_truthy(fl_eq(raw, fl_str_val("_"))) ? fl_str_n(2, fl_str_val("__fl_ign_"), i) : raw);
    cgc_fn_argv_decls(items, fl_add(i, fl_int(1)), fl_str_n(6, acc, fl_str_val("    FLValue "), name, fl_str_val(" __attribute__((unused)) = argv["), i, fl_str_val("];\n")));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_fn_param_names(FLValue items, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_fn_param_names", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(items))) ? acc : ((__extension__ ({
    FLValue raw = c_name(cgc_extract_name(get(items, i)));
    FLValue name = (fl_truthy(fl_eq(raw, fl_str_val("_"))) ? fl_str_n(2, fl_str_val("__fl_ign_"), i) : raw);
    cgc_fn_param_names(items, fl_add(i, fl_int(1)), fl_vec_push(acc, name));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_collect_vars(FLValue node, FLValue acc) {
    fl_push_frame_ln("cgc_collect_vars", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(null_p(node)) ? acc : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("variable"))) ? ((__extension__ ({
    FLValue n = c_name(get(node, fl_str_val("name")));
    (fl_truthy(fl_includes_item(acc, n)) ? acc : fl_vec_push(acc, n));
}))) : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("sexpr"))) ? ((__extension__ ({
    FLValue op = get(node, fl_str_val("op"));
    FLValue acc2 = (fl_truthy((fl_truthy((fl_truthy(fl_string_p(op)) ? fl_gt(length(op), fl_int(1)) : fl_bool(false))) ? fl_eq(char_at(op, fl_int(0)), fl_str_val("$")) : fl_bool(false))) ? ((__extension__ ({
    FLValue vn = c_name(substring(op, fl_int(1), length(op)));
    (fl_truthy(fl_includes_item(acc, vn)) ? acc : fl_vec_push(acc, vn));
}))) : acc);
    cgc_collect_vars_loop(get(node, fl_str_val("args")), fl_int(0), acc2);
}))) : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("block"))) ? cgc_collect_vars_loop(get_block_items(node), fl_int(0), acc) : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("and"))) ? cgc_collect_vars_loop(get(node, fl_str_val("args")), fl_int(0), acc) : (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("or"))) ? cgc_collect_vars_loop(get(node, fl_str_val("args")), fl_int(0), acc) : acc)))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_collect_vars_loop(FLValue _args, FLValue _i, FLValue _acc) {
    fl_push_frame_ln("cgc_collect_vars_loop", __LINE__);
    __fl_tco_cgc_collect_vars_loop:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_or(null_p(_args), fl_gte(i, length(_args)))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = cgc_collect_vars(get(_args, i), acc);
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_fn_env_decls(FLValue _caps, FLValue _i, FLValue _acc) {
    fl_push_frame_ln("cgc_fn_env_decls", __LINE__);
    __fl_tco_cgc_fn_env_decls:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_caps))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_str_n(6, acc, fl_str_val("    FLValue "), get(_caps, i), fl_str_val(" = _self->env["), i, fl_str_val("];\n"));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_env_arr(FLValue _caps, FLValue _i, FLValue _acc) {
    fl_push_frame_ln("cgc_env_arr", __LINE__);
    __fl_tco_cgc_env_arr:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_caps))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = (fl_truthy(fl_eq(i, fl_int(0))) ? get(_caps, fl_int(0)) : fl_str_n(3, acc, fl_str_val(", "), get(_caps, i)));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_fn_caps_filter(FLValue all_vars, FLValue param_names, FLValue outer, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_fn_caps_filter", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(all_vars))) ? acc : ((__extension__ ({
    FLValue v = get(all_vars, i);
    cgc_fn_caps_filter(all_vars, param_names, outer, fl_add(i, fl_int(1)), (fl_truthy((fl_truthy(fl_not(fl_includes_item(param_names, v))) ? fl_includes_item(outer, v) : fl_bool(false))) ? fl_vec_push(acc, v) : acc));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_fn(FLValue args) {
    fl_push_frame_ln("cgc_fn", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue id = fl_deref(lambda_id_atom);
    FLValue id_bump = fl_atom_reset(lambda_id_atom, fl_add(fl_atom_deref(lambda_id_atom), fl_int(1)));
    FLValue param_items = get_block_items(get(args, fl_int(0)));
    FLValue param_names = cgc_fn_param_names(param_items, fl_int(0), fl_vec_new());
    FLValue body_node = get(args, fl_int(1));
    FLValue all_vars = cgc_collect_vars(body_node, fl_vec_new());
    FLValue outer = fl_deref(outer_params_atom);
    FLValue caps = cgc_fn_caps_filter(all_vars, param_names, outer, fl_int(0), fl_vec_new());
    FLValue nenv = length(caps);
    FLValue env_decls = cgc_fn_env_decls(caps, fl_int(0), fl_str_val(""));
    FLValue _reg __attribute__((unused)) = fl_map_fn(fl_fn_new(__fl_anon_3, 0, NULL), param_names);
    FLValue _fplen __attribute__((unused)) = length(fl_deref(outer_params_atom));
    FLValue _foset __attribute__((unused)) = fl_atom_reset(outer_params_atom, concat(fl_atom_deref(outer_params_atom), param_names));
    FLValue body_c = cgc(body_node);
    FLValue _fores __attribute__((unused)) = fl_atom_reset(outer_params_atom, substring(fl_deref(outer_params_atom), fl_int(0), _fplen));
    FLValue decls = cgc_fn_argv_decls(param_items, fl_int(0), fl_str_val(""));
    FLValue fn_name = fl_str_n(2, fl_str_val("__fl_anon_"), id);
    fl_atom_reset(lambda_defs_atom, fl_vec_push(fl_atom_deref(lambda_defs_atom), fl_str_n(9, fl_str_val("static FLValue "), fn_name, fl_str_val("(FLClosure* _self, int _argc, FLValue* argv) {\n"), fl_str_val("    (void)_self; (void)_argc;\n"), env_decls, decls, fl_str_val("    return "), body_c, fl_str_val(";\n}"))));
    (fl_truthy(fl_eq(nenv, fl_int(0))) ? fl_str_n(3, fl_str_val("fl_fn_new("), fn_name, fl_str_val(", 0, NULL)")) : fl_str_n(14, fl_str_val("(__extension__ ({ FLValue __env_"), id, fl_str_val("["), nenv, fl_str_val("] = {"), cgc_env_arr(caps, fl_int(0), fl_str_val("")), fl_str_val("};"), fl_str_val(" fl_fn_new("), fn_name, fl_str_val(", "), nenv, fl_str_val(", __env_"), id, fl_str_val("); }))")));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_list(FLValue args) {
    fl_push_frame_ln("cgc_list", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_vec_new()") : ((__extension__ ({
    FLValue cnt = length(args);
    FLValue __fl_kw_vals __attribute__((unused)) = cgc_args(args);
    fl_str_n(8, fl_str_val("(__extension__ ({ FLValue __fl_lst["), cnt, fl_str_val("] = {"), __fl_kw_vals, fl_str_val("};"), fl_str_val(" fl_vec_from(__fl_lst, "), cnt, fl_str_val("); }))"));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_str(FLValue args) {
    fl_push_frame_ln("cgc_str", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue n = length(args);
    (fl_truthy(fl_eq(n, fl_int(0))) ? fl_str_val("fl_str_val(\"\")") : (fl_truthy(fl_eq(n, fl_int(1))) ? fl_str_n(3, fl_str_val("fl_str_n(1, "), cgc(get(args, fl_int(0))), fl_str_val(")")) : fl_str_n(5, fl_str_val("fl_str_n("), n, fl_str_val(", "), cgc_args(args), fl_str_val(")"))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_str_arg(FLValue args) {
    fl_push_frame_ln("cgc_str_arg", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(length(args), fl_int(1))) ? cgc(get(args, fl_int(0))) : cgc_str(args)); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_if(FLValue args) {
    fl_push_frame_ln("cgc_if", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue cond = cgc(get(args, fl_int(0)));
    FLValue then = cgc(get(args, fl_int(1)));
    FLValue __fl_kw_else __attribute__((unused)) = (fl_truthy(fl_gte(length(args), fl_int(3))) ? cgc(get(args, fl_int(2))) : fl_str_val("fl_nil()"));
    fl_str_n(7, fl_str_val("(fl_truthy("), cond, fl_str_val(") ? "), then, fl_str_val(" : "), __fl_kw_else, fl_str_val(")"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_cond(FLValue args) {
    fl_push_frame_ln("cgc_cond", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_nil()") : ((__extension__ ({
    FLValue __fl_kw_first __attribute__((unused)) = get(args, fl_int(0));
    FLValue nested = (fl_truthy(fl_eq(get(__fl_kw_first, fl_str_val("kind")), fl_str_val("block"))) ? fl_eq(get(__fl_kw_first, fl_str_val("type")), fl_str_val("Array")) : fl_bool(false));
    (fl_truthy(nested) ? cgc_cond_nested(args, fl_sub(length(args), fl_int(1)), fl_str_val("fl_nil()")) : cgc_cond_flat(args, fl_sub(length(args), fl_int(2)), fl_str_val("fl_nil()")));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_cond_nested(FLValue args, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_cond_nested", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_lt(i, fl_int(0))) ? acc : ((__extension__ ({
    FLValue items = get_block_items(get(args, i));
    FLValue test = get(items, fl_int(0));
    FLValue body = get(items, fl_int(1));
    FLValue is_else = (fl_truthy(fl_eq(get(test, fl_str_val("kind")), fl_str_val("literal"))) ? fl_or(fl_eq(get(test, fl_str_val("value")), fl_bool(true)), fl_eq(get(test, fl_str_val("value")), fl_str_val("true"))) : fl_bool(false));
    cgc_cond_nested(args, fl_sub(i, fl_int(1)), (fl_truthy(is_else) ? cgc(body) : fl_str_n(7, fl_str_val("(fl_truthy("), cgc(test), fl_str_val(") ? "), cgc(body), fl_str_val(" : "), acc, fl_str_val(")"))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_cond_flat(FLValue args, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_cond_flat", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_lt(i, fl_int(0))) ? acc : ((__extension__ ({
    FLValue test = get(args, i);
    FLValue body = get(args, fl_add(i, fl_int(1)));
    FLValue is_else = (fl_truthy(fl_eq(get(test, fl_str_val("kind")), fl_str_val("literal"))) ? fl_or(fl_eq(get(test, fl_str_val("value")), fl_bool(true)), fl_eq(get(test, fl_str_val("value")), fl_str_val("true"))) : fl_bool(false));
    cgc_cond_flat(args, fl_sub(i, fl_int(2)), (fl_truthy(is_else) ? cgc(body) : fl_str_n(7, fl_str_val("(fl_truthy("), cgc(test), fl_str_val(") ? "), cgc(body), fl_str_val(" : "), acc, fl_str_val(")"))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_do(FLValue args) {
    fl_push_frame_ln("cgc_do", __LINE__);
    { FLValue __fl_ret__ = fl_str_n(3, fl_str_val("(__extension__ ({ "), cgc_stmts(args, fl_int(0), fl_str_val("")), fl_str_val(" }))")); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_let(FLValue args) {
    fl_push_frame_ln("cgc_let", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue items = get_block_items(get(args, fl_int(0)));
    FLValue __fl_kw_first __attribute__((unused)) = get(items, fl_int(0));
    FLValue nested = (fl_truthy(fl_eq(get(__fl_kw_first, fl_str_val("kind")), fl_str_val("block"))) ? fl_eq(get(__fl_kw_first, fl_str_val("type")), fl_str_val("Array")) : fl_bool(false));
    FLValue decls = (fl_truthy(nested) ? cgc_let_2d(items, fl_int(0), fl_str_val("")) : cgc_let_1d(items, fl_int(0), fl_str_val("")));
    FLValue body = cgc_body(substring(args, fl_int(1), length(args)), fl_int(0), fl_str_val(""));
    fl_str_n(5, fl_str_val("((__extension__ ({\n"), decls, fl_str_val("    "), body, fl_str_val(";\n})))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_let_unique_name(FLValue n) {
    fl_push_frame_ln("cgc_let_unique_name", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(n, fl_str_val("_"))) ? ((__extension__ ({
    FLValue uid = fl_deref(lambda_id_atom);
    fl_atom_reset(lambda_id_atom, fl_add(fl_atom_deref(lambda_id_atom), fl_int(1)));
    fl_str_n(2, fl_str_val("__fl_ign_"), uid);
}))) : n); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_let_1d(FLValue it, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_let_1d", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(it))) ? acc : ((__extension__ ({
    FLValue n = cgc_let_unique_name(cgc_extract_name(get(it, i)));
    FLValue v = cgc(get(it, fl_add(i, fl_int(1))));
    FLValue attr = (fl_truthy(fl_str_starts_with(n, fl_str_val("_"))) ? fl_str_val(" __attribute__((unused))") : fl_str_val(""));
    fl_atom_reset(known_fncall_targets_atom, fl_vec_push(fl_atom_deref(known_fncall_targets_atom), n));
    fl_atom_reset(outer_params_atom, fl_vec_push(fl_atom_deref(outer_params_atom), n));
    cgc_let_1d(it, fl_add(i, fl_int(2)), fl_str_n(7, acc, fl_str_val("    FLValue "), n, attr, fl_str_val(" = "), v, fl_str_val(";\n")));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_let_2d(FLValue it, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_let_2d", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(it))) ? acc : ((__extension__ ({
    FLValue p = get_block_items(get(it, i));
    FLValue n = cgc_let_unique_name(cgc_extract_name(get(p, fl_int(0))));
    FLValue v = cgc(get(p, fl_int(1)));
    FLValue attr = (fl_truthy(fl_str_starts_with(n, fl_str_val("_"))) ? fl_str_val(" __attribute__((unused))") : fl_str_val(""));
    fl_atom_reset(known_fncall_targets_atom, fl_vec_push(fl_atom_deref(known_fncall_targets_atom), n));
    fl_atom_reset(outer_params_atom, fl_vec_push(fl_atom_deref(outer_params_atom), n));
    cgc_let_2d(it, fl_add(i, fl_int(1)), fl_str_n(7, acc, fl_str_val("    FLValue "), n, attr, fl_str_val(" = "), v, fl_str_val(";\n")));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_body(FLValue args, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_body", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? acc : ((__extension__ ({
    FLValue __fl_kw_last __attribute__((unused)) = fl_eq(i, fl_sub(length(args), fl_int(1)));
    FLValue c = cgc(get(args, i));
    (fl_truthy(__fl_kw_last) ? fl_str_n(2, acc, c) : cgc_body(args, fl_add(i, fl_int(1)), fl_str_n(3, acc, c, fl_str_val(";\n    "))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_defn_stmts(FLValue args, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_defn_stmts", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? acc : ((__extension__ ({
    FLValue __fl_kw_last __attribute__((unused)) = fl_eq(i, fl_sub(length(args), fl_int(1)));
    FLValue c = cgc(get(args, i));
    (fl_truthy(__fl_kw_last) ? fl_str_n(4, acc, fl_str_val("    { FLValue __fl_ret__ = "), c, fl_str_val("; fl_pop_frame(); return __fl_ret__; }\n")) : cgc_defn_stmts(args, fl_add(i, fl_int(1)), fl_str_n(4, acc, fl_str_val("    (void)("), c, fl_str_val(");\n"))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_node_has_recur(FLValue node) {
    fl_push_frame_ln("cgc_node_has_recur", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(null_p(node)) ? fl_bool(false) : (fl_truthy(fl_not(fl_eq(get(node, fl_str_val("kind")), fl_str_val("sexpr")))) ? fl_bool(false) : ((__extension__ ({
    FLValue op = get(node, fl_str_val("op"));
    FLValue args = get(node, fl_str_val("args"));
    (fl_truthy(fl_eq(op, fl_str_val("recur"))) ? fl_bool(true) : cgc_any_has_recur(args, fl_int(0)));
}))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_any_has_recur(FLValue args, FLValue i) {
    fl_push_frame_ln("cgc_any_has_recur", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? fl_bool(false) : (fl_truthy(cgc_node_has_recur(get(args, i))) ? fl_bool(true) : cgc_any_has_recur(args, fl_add(i, fl_int(1))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_recur_goto_stmt(FLValue args, FLValue vars, FLValue label) {
    fl_push_frame_ln("cgc_recur_goto_stmt", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue temps = cgc_recur_temps(args, fl_int(0), fl_str_val(""));
    FLValue assigns = cgc_recur_assigns(vars, fl_int(0), fl_str_val(""));
    fl_str_n(6, fl_str_val("(__extension__ ({\n"), temps, assigns, fl_str_val("    goto "), label, fl_str_val("; fl_nil();\n}))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_with_recur_goto(FLValue node, FLValue vars, FLValue label) {
    fl_push_frame_ln("cgc_with_recur_goto", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(null_p(node)) ? fl_str_val("fl_nil()") : ((__extension__ ({
    FLValue k = get(node, fl_str_val("kind"));
    (fl_truthy(fl_eq(k, fl_str_val("sexpr"))) ? ((__extension__ ({
    FLValue op = get(node, fl_str_val("op"));
    FLValue args = get(node, fl_str_val("args"));
    (fl_truthy(fl_eq(op, fl_str_val("recur"))) ? cgc_recur_goto_stmt(args, vars, label) : (fl_truthy(fl_eq(op, fl_str_val("if"))) ? cgc_if_wr_goto(args, vars, label) : (fl_truthy(fl_eq(op, fl_str_val("cond"))) ? cgc_cond_wr_goto(args, vars, label) : (fl_truthy(fl_eq(op, fl_str_val("do"))) ? cgc_do_wr_goto(args, vars, label) : (fl_truthy(fl_eq(op, fl_str_val("begin"))) ? cgc_do_wr_goto(args, vars, label) : (fl_truthy(fl_eq(op, fl_str_val("let"))) ? cgc_let_wr_goto(args, vars, label) : (fl_truthy(fl_eq(op, fl_str_val("loop"))) ? cgc_loop(args) : (fl_truthy(fl_eq(op, fl_str_val("->"))) ? cgc_thread_first(args) : (fl_truthy(fl_eq(op, fl_str_val("->>"))) ? cgc_thread_last(args) : cgc(node))))))))));
}))) : cgc(node));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_if_wr_goto(FLValue args, FLValue vars, FLValue label) {
    fl_push_frame_ln("cgc_if_wr_goto", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue cond = cgc(get(args, fl_int(0)));
    FLValue then = cgc_with_recur_goto(get(args, fl_int(1)), vars, label);
    FLValue __fl_kw_else __attribute__((unused)) = (fl_truthy(fl_gte(length(args), fl_int(3))) ? cgc_with_recur_goto(get(args, fl_int(2)), vars, label) : fl_str_val("fl_nil()"));
    fl_str_n(7, fl_str_val("(fl_truthy("), cond, fl_str_val(") ? "), then, fl_str_val(" : "), __fl_kw_else, fl_str_val(")"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_cond_wr_goto(FLValue args, FLValue vars, FLValue label) {
    fl_push_frame_ln("cgc_cond_wr_goto", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_nil()") : ((__extension__ ({
    FLValue __fl_kw_first __attribute__((unused)) = get(args, fl_int(0));
    FLValue nested = (fl_truthy(fl_eq(get(__fl_kw_first, fl_str_val("kind")), fl_str_val("block"))) ? fl_eq(get(__fl_kw_first, fl_str_val("type")), fl_str_val("Array")) : fl_bool(false));
    (fl_truthy(nested) ? cgc_cond_nested_wr_goto(args, vars, label, fl_sub(length(args), fl_int(1)), fl_str_val("fl_nil()")) : cgc_cond_flat_wr_goto(args, vars, label, fl_sub(length(args), fl_int(2)), fl_str_val("fl_nil()")));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_cond_nested_wr_goto(FLValue args, FLValue vars, FLValue label, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_cond_nested_wr_goto", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_lt(i, fl_int(0))) ? acc : ((__extension__ ({
    FLValue items = get_block_items(get(args, i));
    FLValue test = get(items, fl_int(0));
    FLValue body = get(items, fl_int(1));
    FLValue is_else = (fl_truthy(fl_eq(get(test, fl_str_val("kind")), fl_str_val("literal"))) ? fl_or(fl_eq(get(test, fl_str_val("value")), fl_bool(true)), fl_eq(get(test, fl_str_val("value")), fl_str_val("true"))) : fl_bool(false));
    cgc_cond_nested_wr_goto(args, vars, label, fl_sub(i, fl_int(1)), (fl_truthy(is_else) ? cgc_with_recur_goto(body, vars, label) : fl_str_n(7, fl_str_val("(fl_truthy("), cgc(test), fl_str_val(") ? "), cgc_with_recur_goto(body, vars, label), fl_str_val(" : "), acc, fl_str_val(")"))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_thread_first(FLValue args) {
    fl_push_frame_ln("cgc_thread_first", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_lte(length(args), fl_int(1))) ? (fl_truthy(fl_eq(length(args), fl_int(1))) ? cgc(get(args, fl_int(0))) : fl_str_val("fl_nil()")) : cgc_tf_build(cgc(get(args, fl_int(0))), args, fl_int(1))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_tf_build(FLValue inner, FLValue args, FLValue i) {
    fl_push_frame_ln("cgc_tf_build", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? inner : ((__extension__ ({
    FLValue form = get(args, i);
    FLValue raw = (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("kind"), fl_str_val("raw-c"), fl_str_val("code"), inner}; fl_map_from_pairs(__fl_kv, 2); }));
    FLValue is_call = fl_eq(get(form, fl_str_val("kind")), fl_str_val("sexpr"));
    FLValue sym_name = (fl_truthy(fl_eq(get(form, fl_str_val("kind")), fl_str_val("variable"))) ? get(form, fl_str_val("name")) : fl_str_n(1, get(form, fl_str_val("value"))));
    FLValue new_inner = (fl_truthy(is_call) ? cgc_dispatch(get(form, fl_str_val("op")), fl_concat((__extension__ ({ FLValue __fl_lst[1] = {raw}; fl_vec_from(__fl_lst, 1); })), get(form, fl_str_val("args")))) : cgc_dispatch_fallback(sym_name, (__extension__ ({ FLValue __fl_lst[1] = {raw}; fl_vec_from(__fl_lst, 1); }))));
    cgc_tf_build(new_inner, args, fl_add(i, fl_int(1)));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_thread_last(FLValue args) {
    fl_push_frame_ln("cgc_thread_last", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_lte(length(args), fl_int(1))) ? (fl_truthy(fl_eq(length(args), fl_int(1))) ? cgc(get(args, fl_int(0))) : fl_str_val("fl_nil()")) : cgc_tl_build(cgc(get(args, fl_int(0))), args, fl_int(1))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_tl_build(FLValue inner, FLValue args, FLValue i) {
    fl_push_frame_ln("cgc_tl_build", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? inner : ((__extension__ ({
    FLValue form = get(args, i);
    FLValue raw = (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("kind"), fl_str_val("raw-c"), fl_str_val("code"), inner}; fl_map_from_pairs(__fl_kv, 2); }));
    FLValue is_call = fl_eq(get(form, fl_str_val("kind")), fl_str_val("sexpr"));
    FLValue sym_name = (fl_truthy(fl_eq(get(form, fl_str_val("kind")), fl_str_val("variable"))) ? get(form, fl_str_val("name")) : fl_str_n(1, get(form, fl_str_val("value"))));
    FLValue new_inner = (fl_truthy(is_call) ? cgc_dispatch(get(form, fl_str_val("op")), fl_vec_push(get(form, fl_str_val("args")), raw)) : cgc_dispatch_fallback(sym_name, (__extension__ ({ FLValue __fl_lst[1] = {raw}; fl_vec_from(__fl_lst, 1); }))));
    cgc_tl_build(new_inner, args, fl_add(i, fl_int(1)));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_case(FLValue args) {
    fl_push_frame_ln("cgc_case", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue val_c = cgc(get(args, fl_int(0)));
    FLValue n = length(args);
    FLValue has_default = fl_eq(fl_mod(n, fl_int(2)), fl_int(0));
    FLValue init_acc = (fl_truthy(has_default) ? cgc(get(args, fl_sub(n, fl_int(1)))) : fl_str_val("fl_nil()"));
    FLValue last_i = fl_sub((fl_truthy(has_default) ? fl_sub(n, fl_int(2)) : fl_sub(n, fl_int(1))), fl_int(1));
    cgc_case_rev(val_c, args, last_i, init_acc);
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_case_rev(FLValue val_c, FLValue args, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_case_rev", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_lt(i, fl_int(1))) ? acc : ((__extension__ ({
    FLValue key_c = cgc(get(args, i));
    FLValue res_c = cgc(get(args, fl_add(i, fl_int(1))));
    cgc_case_rev(val_c, args, fl_sub(i, fl_int(2)), fl_str_n(9, fl_str_val("(fl_truthy(fl_eq("), val_c, fl_str_val(", "), key_c, fl_str_val(")) ? "), res_c, fl_str_val(" : "), acc, fl_str_val(")")));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_if_let(FLValue args) {
    fl_push_frame_ln("cgc_if_let", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue binding = get(args, fl_int(0));
    FLValue then = get(args, fl_int(1));
    FLValue has_else = fl_gt(length(args), fl_int(2));
    FLValue else_c = (fl_truthy(has_else) ? cgc(get(args, fl_int(2))) : fl_str_val("fl_nil()"));
    FLValue items = get_block_items(binding);
    FLValue var_c = c_name(get(get(items, fl_int(0)), fl_str_val("name")));
    FLValue init_c = cgc(get(items, fl_int(1)));
    fl_str_n(12, fl_str_val("(__extension__ ({ FLValue "), var_c, fl_str_val(" = "), init_c, fl_str_val(";"), fl_str_val(" !fl_truthy(null_p("), var_c, fl_str_val(")) ? ("), cgc(then), fl_str_val(") : "), else_c, fl_str_val("; }))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_when_let(FLValue args) {
    fl_push_frame_ln("cgc_when_let", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue binding = get(args, fl_int(0));
    FLValue items = get_block_items(binding);
    FLValue var_c = c_name(get(get(items, fl_int(0)), fl_str_val("name")));
    FLValue init_c = cgc(get(items, fl_int(1)));
    FLValue body_c = cgc_body(args, fl_int(1), fl_str_val(""));
    fl_str_n(10, fl_str_val("(__extension__ ({ FLValue "), var_c, fl_str_val(" = "), init_c, fl_str_val(";"), fl_str_val(" !fl_truthy(null_p("), var_c, fl_str_val(")) ? ("), body_c, fl_str_val(") : fl_nil(); }))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue match_wildcard_p(FLValue node) {
    fl_push_frame_ln("match_wildcard_p", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy((fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("literal"))) ? fl_eq(get(node, fl_str_val("type")), fl_str_val("symbol")) : fl_bool(false))) ? fl_eq(get(node, fl_str_val("value")), fl_str_val("_")) : fl_bool(false)); fl_pop_frame(); return __fl_ret__; }
}

FLValue match_bind_var_p(FLValue node) {
    fl_push_frame_ln("match_bind_var_p", __LINE__);
    { FLValue __fl_ret__ = fl_eq(get(node, fl_str_val("kind")), fl_str_val("variable")); fl_pop_frame(); return __fl_ret__; }
}

FLValue match_vec_pat_p(FLValue node) {
    fl_push_frame_ln("match_vec_pat_p", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("block"))) ? fl_eq(get(node, fl_str_val("type")), fl_str_val("Array")) : fl_bool(false)); fl_pop_frame(); return __fl_ret__; }
}

FLValue match_pat_nil_p(FLValue node) {
    fl_push_frame_ln("match_pat_nil_p", __LINE__);
    { FLValue __fl_ret__ = fl_or((fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("literal"))) ? fl_eq(get(node, fl_str_val("type")), fl_str_val("nil")) : fl_bool(false)), (fl_truthy((fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("literal"))) ? fl_eq(get(node, fl_str_val("type")), fl_str_val("symbol")) : fl_bool(false))) ? fl_or(fl_eq(get(node, fl_str_val("value")), fl_str_val("nil")), fl_eq(get(node, fl_str_val("value")), fl_str_val("null"))) : fl_bool(false))); fl_pop_frame(); return __fl_ret__; }
}

FLValue match_pat_var_p(FLValue node) {
    fl_push_frame_ln("match_pat_var_p", __LINE__);
    { FLValue __fl_ret__ = fl_eq(get(node, fl_str_val("kind")), fl_str_val("variable")); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_match_vec_cond(FLValue val_c, FLValue pat_node) {
    fl_push_frame_ln("cgc_match_vec_cond", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue items = get_block_items(pat_node);
    FLValue n = length(items);
    FLValue len_cond = fl_str_n(5, fl_str_val("fl_truthy(fl_eq(length("), val_c, fl_str_val("), fl_int("), n, fl_str_val(")))"));
    FLValue elem_results = fl_map_fn((__extension__ ({ FLValue __env_4[2] = {items, val_c}; fl_fn_new(__fl_anon_4, 2, __env_4); })), range(n));
    FLValue elem_conds = fl_filter_fn(fl_fn_new(__fl_anon_5, 0, NULL), fl_map_fn(fl_fn_new(__fl_anon_6, 0, NULL), elem_results));
    FLValue cond_str = fl_reduce_fn(fl_fn_new(__fl_anon_7, 0, NULL), len_cond, elem_conds);
    FLValue bind_str = fl_reduce_fn(fl_fn_new(__fl_anon_8, 0, NULL), fl_str_val(""), fl_filter_fn(fl_fn_new(__fl_anon_9, 0, NULL), fl_map_fn(fl_fn_new(__fl_anon_10, 0, NULL), elem_results)));
    (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("cond"), cond_str, fl_str_val("bind"), bind_str}; fl_map_from_pairs(__fl_kv, 2); }));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_match_rev(FLValue val_c, FLValue args, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_match_rev", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_lt(i, fl_int(1))) ? acc : ((__extension__ ({
    FLValue pat = get(args, i);
    FLValue res = get(args, fl_add(i, fl_int(1)));
    cgc_match_rev(val_c, args, fl_sub(i, fl_int(2)), (fl_truthy(match_wildcard_p(pat)) ? cgc(res) : (fl_truthy(match_bind_var_p(pat)) ? fl_str_n(7, fl_str_val("(__extension__ ({ FLValue "), c_name(get(pat, fl_str_val("name"))), fl_str_val(" = "), val_c, fl_str_val("; "), cgc(res), fl_str_val("; }))")) : (fl_truthy(match_vec_pat_p(pat)) ? ((__extension__ ({
    FLValue vc = cgc_match_vec_cond(val_c, pat);
    fl_str_n(9, fl_str_val("(("), get(vc, fl_str_val("cond")), fl_str_val(") ? (__extension__ ({ "), get(vc, fl_str_val("bind")), fl_str_val(" "), cgc(res), fl_str_val("; })) : "), acc, fl_str_val(")"));
}))) : fl_str_n(9, fl_str_val("(fl_truthy(fl_eq("), val_c, fl_str_val(", "), cgc(pat), fl_str_val(")) ? "), cgc(res), fl_str_val(" : "), acc, fl_str_val(")"))))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_match(FLValue args) {
    fl_push_frame_ln("cgc_match", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue n = length(args);
    FLValue last_key = get(args, fl_sub(n, fl_int(2)));
    FLValue last_val = get(args, fl_sub(n, fl_int(1)));
    FLValue has_wild = match_wildcard_p(last_key);
    FLValue __fl_kw_default __attribute__((unused)) = (fl_truthy(has_wild) ? cgc(last_val) : fl_str_val("fl_nil()"));
    FLValue start_i = (fl_truthy(has_wild) ? fl_sub(n, fl_int(4)) : fl_sub(n, fl_int(2)));
    FLValue val_tmp = fl_str_val("__match_val__");
    FLValue inner = cgc_match_rev(val_tmp, args, start_i, __fl_kw_default);
    fl_str_n(7, fl_str_val("(__extension__ ({ FLValue "), val_tmp, fl_str_val(" = "), cgc(get(args, fl_int(0))), fl_str_val("; "), inner, fl_str_val("; }))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_doseq(FLValue args) {
    fl_push_frame_ln("cgc_doseq", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue items = get_block_items(get(args, fl_int(0)));
    FLValue vnode = get(items, fl_int(0));
    FLValue cnode = get(items, fl_int(1));
    FLValue var_c = cgc_extract_name(vnode);
    FLValue __fl_ign_11 __attribute__((unused)) = fl_atom_reset(outer_params_atom, fl_vec_push(fl_atom_deref(outer_params_atom), var_c));
    FLValue coll_c = cgc(cnode);
    FLValue body_c = cgc_body(args, fl_int(1), fl_str_val(""));
    fl_str_n(13, fl_str_val("(__extension__ ({ FLValue _ds_c = "), coll_c, fl_str_val(";"), fl_str_val(" FLValue _ds_len = length(_ds_c);"), fl_str_val(" FLValue _ds_i = fl_int(0);"), fl_str_val(" for (; fl_truthy(fl_lt(_ds_i, _ds_len));"), fl_str_val(" _ds_i = fl_add(_ds_i, fl_int(1))) {"), fl_str_val(" FLValue "), var_c, fl_str_val(" = nth(_ds_c, _ds_i);"), fl_str_val(" "), body_c, fl_str_val("; } fl_nil(); }))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_dotimes(FLValue args) {
    fl_push_frame_ln("cgc_dotimes", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue var_node = get(args, fl_int(0));
    FLValue var_c = cgc_extract_name(var_node);
    FLValue __fl_ign_12 __attribute__((unused)) = fl_atom_reset(outer_params_atom, fl_vec_push(fl_atom_deref(outer_params_atom), var_c));
    FLValue n_c = cgc(get(args, fl_int(1)));
    FLValue body_c = cgc_body(args, fl_int(2), fl_str_val(""));
    fl_str_n(21, fl_str_val("(__extension__ ({ FLValue __n_"), var_c, fl_str_val(" = "), n_c, fl_str_val(";"), fl_str_val(" FLValue "), var_c, fl_str_val(" = fl_int(0);"), fl_str_val(" for (; fl_truthy(fl_lt("), var_c, fl_str_val(", __n_"), var_c, fl_str_val("));"), fl_str_val(" "), var_c, fl_str_val(" = fl_add("), var_c, fl_str_val(", fl_int(1))) { "), body_c, fl_str_val("; }"), fl_str_val(" fl_nil(); }))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_doto(FLValue args) {
    fl_push_frame_ln("cgc_doto", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue obj_c = cgc(get(args, fl_int(0)));
    FLValue forms = substring(args, fl_int(1), length(args));
    FLValue calls = cgc_doto_calls(obj_c, forms, fl_int(0), fl_str_val(""));
    fl_str_n(5, fl_str_val("(__extension__ ({ FLValue __doto = "), obj_c, fl_str_val("; "), calls, fl_str_val(" __doto; }))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_doto_calls(FLValue obj_c, FLValue forms, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_doto_calls", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(forms))) ? acc : ((__extension__ ({
    FLValue form = get(forms, i);
    FLValue raw = (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("kind"), fl_str_val("raw-c"), fl_str_val("code"), fl_str_val("__doto")}; fl_map_from_pairs(__fl_kv, 2); }));
    FLValue call = cgc_dispatch(get(form, fl_str_val("op")), fl_concat((__extension__ ({ FLValue __fl_lst[1] = {raw}; fl_vec_from(__fl_lst, 1); })), get(form, fl_str_val("args"))));
    cgc_doto_calls(obj_c, forms, fl_add(i, fl_int(1)), fl_str_n(3, acc, call, fl_str_val("; ")));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_for(FLValue args) {
    fl_push_frame_ln("cgc_for", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue binding = get(args, fl_int(0));
    FLValue body = get(args, fl_int(1));
    FLValue items = get_block_items(binding);
    FLValue var_node = get(items, fl_int(0));
    FLValue col_node = get(items, fl_int(1));
    FLValue fn_c = cgc_fn((__extension__ ({ FLValue __fl_lst[2] = {(__extension__ ({ FLValue __fl_lst[1] = {var_node}; fl_vec_from(__fl_lst, 1); })), body}; fl_vec_from(__fl_lst, 2); })));
    FLValue col_c = cgc(col_node);
    fl_str_n(5, fl_str_val("fl_map_fn("), fn_c, fl_str_val(", "), col_c, fl_str_val(")"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_cond_flat_wr_goto(FLValue args, FLValue vars, FLValue label, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_cond_flat_wr_goto", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_lt(i, fl_int(0))) ? acc : ((__extension__ ({
    FLValue test = get(args, i);
    FLValue body = get(args, fl_add(i, fl_int(1)));
    FLValue is_else = (fl_truthy(fl_eq(get(test, fl_str_val("kind")), fl_str_val("literal"))) ? fl_or(fl_eq(get(test, fl_str_val("value")), fl_bool(true)), fl_eq(get(test, fl_str_val("value")), fl_str_val("true"))) : fl_bool(false));
    cgc_cond_flat_wr_goto(args, vars, label, fl_sub(i, fl_int(2)), (fl_truthy(is_else) ? cgc_with_recur_goto(body, vars, label) : fl_str_n(7, fl_str_val("(fl_truthy("), cgc(test), fl_str_val(") ? "), cgc_with_recur_goto(body, vars, label), fl_str_val(" : "), acc, fl_str_val(")"))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_do_wr_goto(FLValue args, FLValue vars, FLValue label) {
    fl_push_frame_ln("cgc_do_wr_goto", __LINE__);
    { FLValue __fl_ret__ = fl_str_n(3, fl_str_val("(__extension__ ({ "), cgc_stmts_wr_goto(args, vars, label, fl_int(0), fl_str_val("")), fl_str_val(" }))")); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_stmts_wr_goto(FLValue args, FLValue vars, FLValue label, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_stmts_wr_goto", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? acc : cgc_stmts_wr_goto(args, vars, label, fl_add(i, fl_int(1)), fl_str_n(3, acc, cgc_with_recur_goto(get(args, i), vars, label), fl_str_val("; ")))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_let_wr_goto(FLValue args, FLValue vars, FLValue label) {
    fl_push_frame_ln("cgc_let_wr_goto", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue items = get_block_items(get(args, fl_int(0)));
    FLValue __fl_kw_first __attribute__((unused)) = get(items, fl_int(0));
    FLValue nested = (fl_truthy(fl_eq(get(__fl_kw_first, fl_str_val("kind")), fl_str_val("block"))) ? fl_eq(get(__fl_kw_first, fl_str_val("type")), fl_str_val("Array")) : fl_bool(false));
    FLValue decls = (fl_truthy(nested) ? cgc_let_2d(items, fl_int(0), fl_str_val("")) : cgc_let_1d(items, fl_int(0), fl_str_val("")));
    FLValue body_c = cgc_body_wr_goto(substring(args, fl_int(1), length(args)), vars, label, fl_int(0), fl_str_val(""));
    fl_str_n(5, fl_str_val("((__extension__ ({\n"), decls, fl_str_val("    "), body_c, fl_str_val(";\n})))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_body_wr_goto(FLValue args, FLValue vars, FLValue label, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_body_wr_goto", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? acc : ((__extension__ ({
    FLValue __fl_kw_last __attribute__((unused)) = fl_eq(i, fl_sub(length(args), fl_int(1)));
    FLValue c = cgc_with_recur_goto(get(args, i), vars, label);
    (fl_truthy(__fl_kw_last) ? c : cgc_body_wr_goto(args, vars, label, fl_add(i, fl_int(1)), fl_str_n(3, acc, c, fl_str_val(";\n    "))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_defn_stmts_tco(FLValue args, FLValue vars, FLValue label, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_defn_stmts_tco", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? acc : ((__extension__ ({
    FLValue __fl_kw_last __attribute__((unused)) = fl_eq(i, fl_sub(length(args), fl_int(1)));
    FLValue c = cgc_with_recur_goto(get(args, i), vars, label);
    (fl_truthy(__fl_kw_last) ? fl_str_n(4, acc, fl_str_val("    { FLValue __fl_ret__ = "), c, fl_str_val("; fl_pop_frame(); return __fl_ret__; }\n")) : cgc_defn_stmts_tco(args, vars, label, fl_add(i, fl_int(1)), fl_str_n(4, acc, fl_str_val("    (void)("), c, fl_str_val(");\n"))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_defn(FLValue args) {
    fl_push_frame_ln("cgc_defn", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue is_nested = fl_gt(fl_deref(cgc_defn_depth_atom), fl_int(0));
    fl_atom_reset(cgc_defn_depth_atom, fl_add(fl_atom_deref(cgc_defn_depth_atom), fl_int(1)));
    ((__extension__ ({
    FLValue name = c_name(cgc_extract_name(get(args, fl_int(0))));
    FLValue param_items = get_block_items(get(args, fl_int(1)));
    FLValue ps = cgc_params(param_items);
    FLValue pnames = cgc_fn_param_names(param_items, fl_int(0), fl_vec_new());
    FLValue src_line = get(get(args, fl_int(0)), fl_str_val("line"));
    fl_atom_reset(outer_params_atom, pnames);
    ((__extension__ ({
    FLValue call_args = cgc_wrapper_call_args(param_items, fl_int(0), fl_str_val(""));
    FLValue has_recur = cgc_any_has_recur(args, fl_int(2));
    FLValue line_directive = (fl_truthy(fl_or(is_nested, null_p(src_line))) ? fl_str_val("") : fl_str_n(3, fl_str_val("#line "), src_line, fl_str_val(" \"<fl>\"\n")));
    fl_atom_reset(wrapper_defs_atom, fl_vec_push(fl_atom_deref(wrapper_defs_atom), fl_str_n(9, fl_str_val("static FLValue __fl_wrap_"), name, fl_str_val("(FLClosure* _s, int _ac, FLValue* argv) {\n"), fl_str_val("    (void)_s; (void)_ac;\n"), fl_str_val("    return "), name, fl_str_val("("), call_args, fl_str_val(");\n}"))));
    ((__extension__ ({
    FLValue func_def = (fl_truthy(has_recur) ? ((__extension__ ({
    FLValue label = fl_str_n(2, fl_str_val("__fl_tco_"), name);
    FLValue stmts = cgc_defn_stmts_tco(args, pnames, label, fl_int(2), fl_str_val(""));
    fl_str_n(14, line_directive, fl_str_val("FLValue "), name, fl_str_val("("), ps, fl_str_val(") {\n"), fl_str_val("    fl_push_frame_ln(\""), name, fl_str_val("\", __LINE__);\n"), fl_str_val("    "), label, fl_str_val(":;\n"), stmts, fl_str_val("}"));
}))) : ((__extension__ ({
    FLValue stmts = cgc_defn_stmts(args, fl_int(2), fl_str_val(""));
    fl_str_n(11, line_directive, fl_str_val("FLValue "), name, fl_str_val("("), ps, fl_str_val(") {\n"), fl_str_val("    fl_push_frame_ln(\""), name, fl_str_val("\", __LINE__);\n"), stmts, fl_str_val("}"));
}))));
    ((__extension__ ({
    FLValue result_str = (fl_truthy(is_nested) ? (__extension__ ({ swap_bang(cgc_hoisted_fns_atom, (__extension__ ({ FLValue __env_13[1] = {func_def}; fl_fn_new(__fl_anon_13, 1, __env_13); }))); fl_str_val("fl_nil()");  })) : func_def);
    fl_atom_reset(cgc_defn_depth_atom, fl_sub(fl_atom_deref(cgc_defn_depth_atom), fl_int(1)));
    result_str;
})));
})));
})));
})));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_define(FLValue args) {
    fl_push_frame_ln("cgc_define", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue name = c_name(cgc_extract_name(get(args, fl_int(0))));
    FLValue val = cgc(get(args, fl_int(1)));
    fl_atom_reset(known_fncall_targets_atom, fl_vec_push(fl_atom_deref(known_fncall_targets_atom), name));
    fl_atom_reset(global_decls_atom, fl_vec_push(fl_atom_deref(global_decls_atom), fl_str_n(3, fl_str_val("static FLValue "), name, fl_str_val(";"))));
    fl_str_n(4, name, fl_str_val(" = "), val, fl_str_val(";"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_binop_chain(FLValue args, FLValue fn) {
    fl_push_frame_ln("cgc_binop_chain", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_int(0)") : (fl_truthy(fl_eq(length(args), fl_int(1))) ? cgc(get(args, fl_int(0))) : cgc_binop_fold(args, fn, fl_int(1), cgc(get(args, fl_int(0)))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_binop_fold(FLValue args, FLValue fn, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_binop_fold", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? acc : cgc_binop_fold(args, fn, fl_add(i, fl_int(1)), fl_str_n(6, fn, fl_str_val("("), acc, fl_str_val(", "), cgc(get(args, i)), fl_str_val(")")))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_and(FLValue args) {
    fl_push_frame_ln("cgc_and", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_bool(true)") : (fl_truthy(fl_eq(length(args), fl_int(1))) ? cgc(get(args, fl_int(0))) : cgc_and_fold(args, fl_int(1), cgc(get(args, fl_int(0)))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_and_fold(FLValue args, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_and_fold", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? acc : cgc_and_fold(args, fl_add(i, fl_int(1)), fl_str_n(5, fl_str_val("(fl_truthy("), acc, fl_str_val(") ? "), cgc(get(args, i)), fl_str_val(" : fl_bool(false))")))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_or(FLValue args) {
    fl_push_frame_ln("cgc_or", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_bool(false)") : (fl_truthy(fl_eq(length(args), fl_int(1))) ? cgc(get(args, fl_int(0))) : cgc_or_fold(args, fl_int(1), cgc(get(args, fl_int(0)))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_or_fold(FLValue args, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_or_fold", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? acc : cgc_or_fold(args, fl_add(i, fl_int(1)), fl_str_n(5, fl_str_val("fl_or("), acc, fl_str_val(", "), cgc(get(args, i)), fl_str_val(")")))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_args(FLValue args) {
    fl_push_frame_ln("cgc_args", __LINE__);
    { FLValue __fl_ret__ = cgc_args_loop(args, fl_int(0), fl_str_val("")); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_args_loop(FLValue _args, FLValue _i, FLValue _acc) {
    fl_push_frame_ln("cgc_args_loop", __LINE__);
    __fl_tco_cgc_args_loop:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_or(null_p(_args), fl_gte(i, length(_args)))) ? acc : ((__extension__ ({
    FLValue c = cgc(get(_args, i));
    (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = (fl_truthy(fl_eq(i, fl_int(0))) ? c : fl_str_n(3, acc, fl_str_val(", "), c));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}));
}))));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_stmts(FLValue args, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_stmts", __LINE__);
    __fl_tco_cgc_stmts:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(args))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_str_n(3, acc, cgc(get(args, i)), fl_str_val("; "));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_forward_decls(FLValue nodes) {
    fl_push_frame_ln("cgc_forward_decls", __LINE__);
    { FLValue __fl_ret__ = cgc_forward_loop(nodes, fl_int(0), fl_str_val("")); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_forward_loop(FLValue _nodes, FLValue _i, FLValue _acc) {
    fl_push_frame_ln("cgc_forward_loop", __LINE__);
    __fl_tco_cgc_forward_loop:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_nodes))) ? acc : ((__extension__ ({
    FLValue n = get(_nodes, i);
    (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = (fl_truthy((fl_truthy(fl_eq(get(n, fl_str_val("kind")), fl_str_val("sexpr"))) ? fl_eq(get(n, fl_str_val("op")), fl_str_val("defn")) : fl_bool(false))) ? ((__extension__ ({
    FLValue name = c_name(cgc_extract_name(get(get(n, fl_str_val("args")), fl_int(0))));
    FLValue param_items = get_block_items(get(get(n, fl_str_val("args")), fl_int(1)));
    FLValue ps = cgc_params(param_items);
    fl_atom_reset(known_defns_atom, fl_vec_push(fl_atom_deref(known_defns_atom), name));
    fl_atom_reset(defn_arity_atom, assoc(fl_atom_deref(defn_arity_atom), name, length(param_items)));
    fl_str_n(9, acc, fl_str_val("FLValue "), name, fl_str_val("("), ps, fl_str_val(");\n"), fl_str_val("static FLValue __fl_wrap_"), name, fl_str_val("(FLClosure*, int, FLValue*);\n"));
}))) : acc);
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}));
}))));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_wrapper_call_args(FLValue _items, FLValue _i, FLValue _acc) {
    fl_push_frame_ln("cgc_wrapper_call_args", __LINE__);
    __fl_tco_cgc_wrapper_call_args:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_items))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_str_n(5, acc, (fl_truthy(fl_eq(length(acc), fl_int(0))) ? fl_str_val("") : fl_str_val(", ")), fl_str_val("argv["), i, fl_str_val("]"));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_top_level(FLValue _nodes, FLValue _i, FLValue _stmts, FLValue _fns) {
    fl_push_frame_ln("cgc_top_level", __LINE__);
    __fl_tco_cgc_top_level:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _stmts;
    FLValue stmts = __fl_loop_tmp_2;
    FLValue __fl_loop_tmp_4 = _fns;
    FLValue fns = __fl_loop_tmp_4;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_nodes))) ? ((__extension__ ({
    FLValue hoisted = fl_deref(cgc_hoisted_fns_atom);
    FLValue all_fns = (fl_truthy(fl_gt(length(hoisted), fl_int(0))) ? fl_str_n(3, hoisted, fl_str_val("\n\n"), fns) : fns);
    (__extension__ ({ FLValue __fl_kv[4] = {fl_str_val("stmts"), stmts, fl_str_val("fns"), all_fns}; fl_map_from_pairs(__fl_kv, 2); }));
}))) : ((__extension__ ({
    FLValue n = get(_nodes, i);
    FLValue k = get(n, fl_str_val("kind"));
    FLValue is_defn = (fl_truthy(fl_eq(k, fl_str_val("sexpr"))) ? fl_eq(get(n, fl_str_val("op")), fl_str_val("defn")) : fl_bool(false));
    FLValue is_block_func = (fl_truthy(fl_eq(k, fl_str_val("block"))) ? fl_eq(get(n, fl_str_val("type")), fl_str_val("FUNC")) : fl_bool(false));
    (fl_truthy(fl_or(is_defn, is_block_func)) ? (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = stmts;
    FLValue _fl_t2 = fl_str_n(3, fns, cgc(n), fl_str_val("\n\n"));
    i = _fl_t0;
    stmts = _fl_t1;
    fns = _fl_t2;
    _fl_looping = 1; fl_nil();
})) : ((__extension__ ({
    FLValue line = get(n, fl_str_val("line"));
    FLValue ld = (fl_truthy(null_p(line)) ? fl_str_val("") : fl_str_n(3, fl_str_val("    #line "), line, fl_str_val(" \"<fl>\"\n")));
    (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_str_n(5, stmts, ld, fl_str_val("    "), cgc(n), fl_str_val(";\n"));
    FLValue _fl_t2 = fns;
    i = _fl_t0;
    stmts = _fl_t1;
    fns = _fl_t2;
    _fl_looping = 1; fl_nil();
}));
}))));
}))));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_lambda_fwd_loop(FLValue _defs, FLValue _i, FLValue _acc) {
    fl_push_frame_ln("cgc_lambda_fwd_loop", __LINE__);
    __fl_tco_cgc_lambda_fwd_loop:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_defs))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_str_n(4, acc, fl_str_val("static FLValue __fl_anon_"), i, fl_str_val("(FLClosure*, int, FLValue*);\n"));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_lambda_fwds() {
    fl_push_frame_ln("cgc_lambda_fwds", __LINE__);
    { FLValue __fl_ret__ = cgc_lambda_fwd_loop(fl_deref(lambda_defs_atom), fl_int(0), fl_str_val("")); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_join_lambda_loop(FLValue _defs, FLValue _i, FLValue _acc) {
    fl_push_frame_ln("cgc_join_lambda_loop", __LINE__);
    __fl_tco_cgc_join_lambda_loop:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = _i;
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = _acc;
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(_defs))) ? acc : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_str_n(3, acc, get(_defs, i), fl_str_val("\n\n"));
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_join_lambdas() {
    fl_push_frame_ln("cgc_join_lambdas", __LINE__);
    { FLValue __fl_ret__ = cgc_join_lambda_loop(fl_deref(lambda_defs_atom), fl_int(0), fl_str_val("")); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_join_wrappers() {
    fl_push_frame_ln("cgc_join_wrappers", __LINE__);
    { FLValue __fl_ret__ = cgc_join_lambda_loop(fl_deref(wrapper_defs_atom), fl_int(0), fl_str_val("")); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_join_globals() {
    fl_push_frame_ln("cgc_join_globals", __LINE__);
    { FLValue __fl_ret__ = cgc_join_lambda_loop(fl_deref(global_decls_atom), fl_int(0), fl_str_val("")); fl_pop_frame(); return __fl_ret__; }
}

FLValue generate_c(FLValue nodes) {
    fl_push_frame_ln("generate_c", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue fwd = cgc_forward_decls(nodes);
    FLValue parts = cgc_top_level(nodes, fl_int(0), fl_str_val(""), fl_str_val(""));
    FLValue fns = get(parts, fl_str_val("fns"));
    FLValue stmts = get(parts, fl_str_val("stmts"));
    FLValue gdecls = cgc_join_globals();
    FLValue lfwd = cgc_lambda_fwds();
    FLValue ldefs = cgc_join_lambdas();
    FLValue wdefs = cgc_join_wrappers();
    fl_str_n(17, fl_str_val("#include \"runtime.h\"\n"), fl_str_val("#pragma GCC diagnostic ignored \"-Wunused-function\"\n"), fl_str_val("#pragma GCC diagnostic ignored \"-Wunused-parameter\"\n\n"), fwd, gdecls, fl_str_val("\n"), lfwd, fl_str_val("\n"), ldefs, wdefs, fl_str_val("\n"), fns, fl_str_val("int main(int argc, char** argv) {\n"), fl_str_val("    fl_init_argv(argc, argv);\n"), stmts, fl_str_val("    return 0;\n"), fl_str_val("}\n"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_set_b(FLValue args) {
    fl_push_frame_ln("cgc_set_b", __LINE__);
    { FLValue __fl_ret__ = fl_str_n(5, fl_str_val("(__extension__ ({ "), cgc(get(args, fl_int(0))), fl_str_val(" = "), cgc(get(args, fl_int(1))), fl_str_val("; fl_nil(); }))")); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_while(FLValue args) {
    fl_push_frame_ln("cgc_while", __LINE__);
    { FLValue __fl_ret__ = fl_str_n(5, fl_str_val("(__extension__ ({ while (fl_truthy("), cgc(get(args, fl_int(0))), fl_str_val(")) { "), cgc(get(args, fl_int(1))), fl_str_val("; } fl_nil(); }))")); fl_pop_frame(); return __fl_ret__; }
}

FLValue loop_extract_vars(FLValue items, FLValue i, FLValue acc) {
    fl_push_frame_ln("loop_extract_vars", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(items))) ? acc : loop_extract_vars(items, fl_add(i, fl_int(2)), fl_vec_push(acc, cgc_extract_name(get(items, i))))); fl_pop_frame(); return __fl_ret__; }
}

FLValue loop_make_decls(FLValue items, FLValue i, FLValue acc) {
    fl_push_frame_ln("loop_make_decls", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(items))) ? acc : ((__extension__ ({
    FLValue name = cgc_extract_name(get(items, i));
    FLValue val = cgc(get(items, fl_add(i, fl_int(1))));
    FLValue tmp = fl_str_n(2, fl_str_val("__fl_loop_tmp_"), i);
    loop_make_decls(items, fl_add(i, fl_int(2)), fl_str_n(11, acc, fl_str_val("    FLValue "), tmp, fl_str_val(" = "), val, fl_str_val(";\n"), fl_str_val("    FLValue "), name, fl_str_val(" = "), tmp, fl_str_val(";\n")));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_loop(FLValue args) {
    fl_push_frame_ln("cgc_loop", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue items = get_block_items(get(args, fl_int(0)));
    FLValue body = get(args, fl_int(1));
    FLValue vars = loop_extract_vars(items, fl_int(0), fl_vec_new());
    FLValue decls = loop_make_decls(items, fl_int(0), fl_str_val(""));
    FLValue __fl_ign_14 __attribute__((unused)) = fl_map_fn(fl_fn_new(__fl_anon_15, 0, NULL), vars);
    FLValue body_c = cgc_with_recur(body, vars);
    fl_str_n(8, fl_str_val("(__extension__ ({\n"), decls, fl_str_val("    int _fl_looping = 1; FLValue _fl_result = fl_nil();\n"), fl_str_val("    while (_fl_looping) { _fl_looping = 0;\n"), fl_str_val("    _fl_result = "), body_c, fl_str_val(";\n    }\n"), fl_str_val("    _fl_result;\n}))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_recur_temps(FLValue args, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_recur_temps", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? acc : cgc_recur_temps(args, fl_add(i, fl_int(1)), fl_str_n(6, acc, fl_str_val("    FLValue _fl_t"), i, fl_str_val(" = "), cgc(get(args, i)), fl_str_val(";\n")))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_recur_assigns(FLValue vars, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_recur_assigns", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(vars))) ? acc : cgc_recur_assigns(vars, fl_add(i, fl_int(1)), fl_str_n(6, acc, fl_str_val("    "), get(vars, i), fl_str_val(" = _fl_t"), i, fl_str_val(";\n")))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_recur_stmt(FLValue args, FLValue vars) {
    fl_push_frame_ln("cgc_recur_stmt", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue temps = cgc_recur_temps(args, fl_int(0), fl_str_val(""));
    FLValue assigns = cgc_recur_assigns(vars, fl_int(0), fl_str_val(""));
    fl_str_n(4, fl_str_val("(__extension__ ({\n"), temps, assigns, fl_str_val("    _fl_looping = 1; fl_nil();\n}))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_with_recur(FLValue node, FLValue vars) {
    fl_push_frame_ln("cgc_with_recur", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(null_p(node)) ? fl_str_val("fl_nil()") : ((__extension__ ({
    FLValue k = get(node, fl_str_val("kind"));
    (fl_truthy(fl_eq(k, fl_str_val("sexpr"))) ? ((__extension__ ({
    FLValue op = get(node, fl_str_val("op"));
    FLValue args = get(node, fl_str_val("args"));
    (fl_truthy(fl_eq(op, fl_str_val("recur"))) ? cgc_recur_stmt(args, vars) : (fl_truthy(fl_eq(op, fl_str_val("if"))) ? cgc_if_wr(args, vars) : (fl_truthy(fl_eq(op, fl_str_val("cond"))) ? cgc_cond_wr(args, vars) : (fl_truthy(fl_eq(op, fl_str_val("do"))) ? cgc_do_wr(args, vars) : (fl_truthy(fl_eq(op, fl_str_val("begin"))) ? cgc_do_wr(args, vars) : (fl_truthy(fl_eq(op, fl_str_val("let"))) ? cgc_let_wr(args, vars) : (fl_truthy(fl_eq(op, fl_str_val("loop"))) ? cgc_loop(args) : cgc(node))))))));
}))) : cgc(node));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_if_wr(FLValue args, FLValue vars) {
    fl_push_frame_ln("cgc_if_wr", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue cond = cgc(get(args, fl_int(0)));
    FLValue then = cgc_with_recur(get(args, fl_int(1)), vars);
    FLValue __fl_kw_else __attribute__((unused)) = (fl_truthy(fl_gte(length(args), fl_int(3))) ? cgc_with_recur(get(args, fl_int(2)), vars) : fl_str_val("fl_nil()"));
    fl_str_n(7, fl_str_val("(fl_truthy("), cond, fl_str_val(") ? "), then, fl_str_val(" : "), __fl_kw_else, fl_str_val(")"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_cond_wr(FLValue args, FLValue vars) {
    fl_push_frame_ln("cgc_cond_wr", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(length(args), fl_int(0))) ? fl_str_val("fl_nil()") : ((__extension__ ({
    FLValue __fl_kw_first __attribute__((unused)) = get(args, fl_int(0));
    FLValue nested = (fl_truthy(fl_eq(get(__fl_kw_first, fl_str_val("kind")), fl_str_val("block"))) ? fl_eq(get(__fl_kw_first, fl_str_val("type")), fl_str_val("Array")) : fl_bool(false));
    (fl_truthy(nested) ? cgc_cond_nested_wr(args, vars, fl_sub(length(args), fl_int(1)), fl_str_val("fl_nil()")) : fl_str_val("fl_nil()"));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_cond_nested_wr(FLValue args, FLValue vars, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_cond_nested_wr", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_lt(i, fl_int(0))) ? acc : ((__extension__ ({
    FLValue items = get_block_items(get(args, i));
    FLValue test = get(items, fl_int(0));
    FLValue body = get(items, fl_int(1));
    FLValue is_else = (fl_truthy(fl_eq(get(test, fl_str_val("kind")), fl_str_val("literal"))) ? fl_or(fl_eq(get(test, fl_str_val("value")), fl_bool(true)), fl_eq(get(test, fl_str_val("value")), fl_str_val("true"))) : fl_bool(false));
    cgc_cond_nested_wr(args, vars, fl_sub(i, fl_int(1)), (fl_truthy(is_else) ? cgc_with_recur(body, vars) : fl_str_n(7, fl_str_val("(fl_truthy("), cgc(test), fl_str_val(") ? "), cgc_with_recur(body, vars), fl_str_val(" : "), acc, fl_str_val(")"))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_do_wr(FLValue args, FLValue vars) {
    fl_push_frame_ln("cgc_do_wr", __LINE__);
    { FLValue __fl_ret__ = fl_str_n(3, fl_str_val("(__extension__ ({ "), cgc_stmts_wr(args, vars, fl_int(0), fl_str_val("")), fl_str_val(" }))")); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_stmts_wr(FLValue args, FLValue vars, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_stmts_wr", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? acc : cgc_stmts_wr(args, vars, fl_add(i, fl_int(1)), fl_str_n(3, acc, cgc_with_recur(get(args, i), vars), fl_str_val("; ")))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_let_wr(FLValue args, FLValue vars) {
    fl_push_frame_ln("cgc_let_wr", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue items = get_block_items(get(args, fl_int(0)));
    FLValue __fl_kw_first __attribute__((unused)) = get(items, fl_int(0));
    FLValue nested = (fl_truthy(fl_eq(get(__fl_kw_first, fl_str_val("kind")), fl_str_val("block"))) ? fl_eq(get(__fl_kw_first, fl_str_val("type")), fl_str_val("Array")) : fl_bool(false));
    FLValue decls = (fl_truthy(nested) ? cgc_let_2d(items, fl_int(0), fl_str_val("")) : cgc_let_1d(items, fl_int(0), fl_str_val("")));
    FLValue body_c = cgc_body_wr(substring(args, fl_int(1), length(args)), vars, fl_int(0), fl_str_val(""));
    fl_str_n(5, fl_str_val("((__extension__ ({\n"), decls, fl_str_val("    "), body_c, fl_str_val(";\n})))"));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_body_wr(FLValue args, FLValue vars, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_body_wr", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(args))) ? acc : ((__extension__ ({
    FLValue __fl_kw_last __attribute__((unused)) = fl_eq(i, fl_sub(length(args), fl_int(1)));
    FLValue c = cgc_with_recur(get(args, i), vars);
    (fl_truthy(__fl_kw_last) ? c : cgc_body_wr(args, vars, fl_add(i, fl_int(1)), fl_str_n(3, acc, c, fl_str_val(";\n    "))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_array_block(FLValue n) {
    fl_push_frame_ln("cgc_array_block", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue items = get(get(n, fl_str_val("fields")), fl_str_val("items"));
    (fl_truthy(fl_or(null_p(items), fl_eq(length(items), fl_int(0)))) ? fl_str_val("fl_vec_new()") : ((__extension__ ({
    FLValue cnt = length(items);
    FLValue __fl_kw_vals __attribute__((unused)) = cgc_args(items);
    fl_str_n(8, fl_str_val("(__extension__ ({ FLValue __fl_arr["), cnt, fl_str_val("] = {"), __fl_kw_vals, fl_str_val("};"), fl_str_val(" fl_vec_from(__fl_arr, "), cnt, fl_str_val("); }))"));
}))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_map_entry_c(FLValue ent) {
    fl_push_frame_ln("cgc_map_entry_c", __LINE__);
    { FLValue __fl_ret__ = fl_str_n(4, fl_str_val("fl_str_val(\""), c_esc(get(ent, fl_int(0))), fl_str_val("\"), "), cgc(get(ent, fl_int(1)))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_map_entries_c(FLValue ents, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_map_entries_c", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(ents))) ? acc : ((__extension__ ({
    FLValue sep = (fl_truthy(fl_eq(i, fl_int(0))) ? fl_str_val("") : fl_str_val(", "));
    FLValue ec = cgc_map_entry_c(get(ents, i));
    cgc_map_entries_c(ents, fl_add(i, fl_int(1)), fl_str_n(3, acc, sep, ec));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_map_key_c(FLValue key_node) {
    fl_push_frame_ln("cgc_map_key_c", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_eq(get(key_node, fl_str_val("kind")), fl_str_val("keyword"))) ? fl_str_n(3, fl_str_val("fl_str_val(\""), c_esc(get(key_node, fl_str_val("name"))), fl_str_val("\")")) : cgc(key_node)); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_map_items_c(FLValue items, FLValue i, FLValue acc) {
    fl_push_frame_ln("cgc_map_items_c", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_gte(i, length(items))) ? acc : ((__extension__ ({
    FLValue sep = (fl_truthy(fl_eq(i, fl_int(0))) ? fl_str_val("") : fl_str_val(", "));
    FLValue key_c = cgc_map_key_c(get(items, i));
    FLValue val_c = cgc(get(items, fl_add(i, fl_int(1))));
    cgc_map_items_c(items, fl_add(i, fl_int(2)), fl_str_n(5, acc, sep, key_c, fl_str_val(", "), val_c));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_map_from_items(FLValue items) {
    fl_push_frame_ln("cgc_map_from_items", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(fl_or(null_p(items), fl_eq(length(items), fl_int(0)))) ? fl_str_val("fl_map_new()") : ((__extension__ ({
    FLValue n_pairs = fl_div(length(items), fl_int(2));
    FLValue n_items = length(items);
    FLValue kvs = cgc_map_items_c(items, fl_int(0), fl_str_val(""));
    fl_str_n(8, fl_str_val("(__extension__ ({ FLValue __fl_kv["), n_items, fl_str_val("] = {"), kvs, fl_str_val("};"), fl_str_val(" fl_map_from_pairs(__fl_kv, "), n_pairs, fl_str_val("); }))"));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_map_block(FLValue n) {
    fl_push_frame_ln("cgc_map_block", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue fields = get(n, fl_str_val("fields"));
    FLValue items = get(fields, fl_str_val("items"));
    (fl_truthy((fl_truthy(fl_not(null_p(items))) ? fl_eq(type_of(items), fl_str_val("array")) : fl_bool(false))) ? cgc_map_from_items(items) : ((__extension__ ({
    FLValue ents = fl_map_entries(fields);
    (fl_truthy(fl_or(null_p(ents), fl_eq(length(ents), fl_int(0)))) ? fl_str_val("fl_map_new()") : ((__extension__ ({
    FLValue n_pairs = length(ents);
    FLValue n_items = fl_mul(n_pairs, fl_int(2));
    FLValue kvs = cgc_map_entries_c(ents, fl_int(0), fl_str_val(""));
    fl_str_n(8, fl_str_val("(__extension__ ({ FLValue __fl_kv["), n_items, fl_str_val("] = {"), kvs, fl_str_val("};"), fl_str_val(" fl_map_from_pairs(__fl_kv, "), n_pairs, fl_str_val("); }))"));
}))));
}))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue ir_err(FLValue msg, FLValue n) {
    fl_push_frame_ln("ir_err", __LINE__);
    (void)(fl_println(fl_str_n(6, fl_str_val("[IR-ERR] "), msg, fl_str_val(" | kind="), get(n, fl_str_val("kind")), fl_str_val(" line="), get(n, fl_str_val("line")))));
    { FLValue __fl_ret__ = fl_bool(false); fl_pop_frame(); return __fl_ret__; }
}

FLValue ir_chk(FLValue n) {
    fl_push_frame_ln("ir_chk", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(null_p(n)) ? fl_bool(true) : ((__extension__ ({
    FLValue k = get(n, fl_str_val("kind"));
    (fl_truthy(null_p(k)) ? ir_err(fl_str_val("missing :kind"), n) : (fl_truthy(fl_not(fl_includes_item(ir_kind_set, k))) ? fl_println(fl_str_n(4, fl_str_val("[IR-WARN] unknown kind="), k, fl_str_val(" line="), get(n, fl_str_val("line")))) : (fl_truthy(fl_eq(k, fl_str_val("literal"))) ? ((__extension__ ({
    FLValue t = get(n, fl_str_val("type"));
    FLValue v = get(n, fl_str_val("value"));
    (fl_truthy(null_p(t)) ? ir_err(fl_str_val("literal missing :type"), n) : (fl_truthy(fl_not(fl_includes_item(ir_lit_types, t))) ? ir_err(fl_str_n(2, fl_str_val("literal bad type="), t), n) : (fl_truthy((fl_truthy(fl_eq(t, fl_str_val("boolean"))) ? fl_not(fl_eq(type_of(v), fl_str_val("boolean"))) : fl_bool(false))) ? ir_err(fl_str_n(2, fl_str_val("boolean value must be FL boolean, got "), type_of(v)), n) : (fl_truthy((fl_truthy(fl_eq(t, fl_str_val("nil"))) ? fl_not(null_p(v)) : fl_bool(false))) ? ir_err(fl_str_val("nil literal value must be nil"), n) : fl_bool(true)))));
}))) : (fl_truthy(fl_eq(k, fl_str_val("variable"))) ? (fl_truthy(null_p(get(n, fl_str_val("name")))) ? ir_err(fl_str_val("variable missing :name"), n) : fl_bool(true)) : (fl_truthy(fl_eq(k, fl_str_val("keyword"))) ? (fl_truthy(null_p(get(n, fl_str_val("name")))) ? ir_err(fl_str_val("keyword missing :name"), n) : fl_bool(true)) : (fl_truthy(fl_eq(k, fl_str_val("sexpr"))) ? (fl_truthy(null_p(get(n, fl_str_val("op")))) ? ir_err(fl_str_val("sexpr missing :op"), n) : (fl_truthy(fl_not(fl_eq(type_of(get(n, fl_str_val("op"))), fl_str_val("string")))) ? ir_err(fl_str_val("sexpr :op must be string"), n) : (fl_truthy(fl_or(fl_eq(get(n, fl_str_val("op")), fl_str_val("and")), fl_eq(get(n, fl_str_val("op")), fl_str_val("or")))) ? ir_err(fl_str_n(2, fl_str_val("and/or must be canonical kind node, not sexpr op="), get(n, fl_str_val("op"))), n) : (fl_truthy(null_p(get(n, fl_str_val("args")))) ? ir_err(fl_str_val("sexpr missing :args"), n) : (fl_truthy(fl_not(fl_eq(type_of(get(n, fl_str_val("args"))), fl_str_val("array")))) ? ir_err(fl_str_val("sexpr :args must be array"), n) : fl_bool(true)))))) : (fl_truthy(fl_eq(k, fl_str_val("block"))) ? (fl_truthy(null_p(get(n, fl_str_val("type")))) ? ir_err(fl_str_val("block missing :type"), n) : fl_bool(true)) : (fl_truthy(fl_or(fl_eq(k, fl_str_val("and")), fl_eq(k, fl_str_val("or")))) ? (fl_truthy(fl_or(null_p(get(n, fl_str_val("args"))), fl_not(fl_eq(type_of(get(n, fl_str_val("args"))), fl_str_val("array"))))) ? ir_err(fl_str_n(2, k, fl_str_val(" missing/invalid :args")), n) : fl_bool(true)) : fl_bool(true)))))))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue includes_item(FLValue arr, FLValue val) {
    fl_push_frame_ln("includes_item", __LINE__);
    __fl_tco_includes_item:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = fl_int(0);
    FLValue i = __fl_loop_tmp_0;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(arr))) ? fl_bool(false) : (fl_truthy(fl_eq(get(arr, i), val)) ? fl_bool(true) : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    i = _fl_t0;
    _fl_looping = 1; fl_nil();
}))));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue ir_validate(FLValue nodes) {
    fl_push_frame_ln("ir_validate", __LINE__);
    __fl_tco_ir_validate:;
    { FLValue __fl_ret__ = (fl_truthy(fl_or(null_p(nodes), fl_eq(length(nodes), fl_int(0)))) ? fl_bool(true) : (__extension__ ({
    FLValue __fl_loop_tmp_0 = fl_int(0);
    FLValue i = __fl_loop_tmp_0;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(nodes))) ? fl_bool(true) : (__extension__ ({ ir_chk(get(nodes, i)); (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    i = _fl_t0;
    _fl_looping = 1; fl_nil();
}));  })));
    }
    _fl_result;
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue path_dir(FLValue path) {
    fl_push_frame_ln("path_dir", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue len = length(path);
    FLValue __fl_kw_last __attribute__((unused)) = (__extension__ ({
    FLValue __fl_loop_tmp_0 = fl_sub(len, fl_int(1));
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = fl_int(-1);
    FLValue r = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_lt(i, fl_int(0))) ? r : (fl_truthy(fl_eq(char_at(path, i), fl_str_val("/"))) ? i : (__extension__ ({
    FLValue _fl_t0 = fl_sub(i, fl_int(1));
    FLValue _fl_t1 = r;
    i = _fl_t0;
    r = _fl_t1;
    _fl_looping = 1; fl_nil();
}))));
    }
    _fl_result;
}));
    (fl_truthy(fl_lt(__fl_kw_last, fl_int(0))) ? fl_str_val("") : substring(path, fl_int(0), fl_add(__fl_kw_last, fl_int(1))));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue append_all(FLValue acc, FLValue items) {
    fl_push_frame_ln("append_all", __LINE__);
    __fl_tco_append_all:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = fl_int(0);
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = acc;
    FLValue a = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(items))) ? a : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_vec_push(a, get(items, i));
    i = _fl_t0;
    a = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue expand_loads(FLValue nodes, FLValue base_dir) {
    fl_push_frame_ln("expand_loads", __LINE__);
    __fl_tco_expand_loads:;
    { FLValue __fl_ret__ = (__extension__ ({
    FLValue __fl_loop_tmp_0 = fl_int(0);
    FLValue i = __fl_loop_tmp_0;
    FLValue __fl_loop_tmp_2 = fl_vec_new();
    FLValue acc = __fl_loop_tmp_2;
    int _fl_looping = 1; FLValue _fl_result = fl_nil();
    while (_fl_looping) { _fl_looping = 0;
    _fl_result = (fl_truthy(fl_gte(i, length(nodes))) ? acc : ((__extension__ ({
    FLValue node = get(nodes, i);
    (fl_truthy((fl_truthy(fl_eq(get(node, fl_str_val("kind")), fl_str_val("sexpr"))) ? fl_eq(get(node, fl_str_val("op")), fl_str_val("load")) : fl_bool(false))) ? ((__extension__ ({
    FLValue path_node = get(get(node, fl_str_val("args")), fl_int(0));
    FLValue rel_path = get(path_node, fl_str_val("value"));
    FLValue try1 = (fl_truthy(fl_str_starts_with(rel_path, fl_str_val("/"))) ? rel_path : fl_str_n(2, base_dir, rel_path));
    FLValue src1 = fl_file_read(try1);
    FLValue abs_path = (fl_truthy(null_p(src1)) ? rel_path : try1);
    (fl_truthy(fl_includes_item(fl_deref(loaded_paths_atom), abs_path)) ? (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = acc;
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})) : ((__extension__ ({
    FLValue src = (fl_truthy(null_p(src1)) ? fl_file_read(rel_path) : src1);
    (fl_truthy(null_p(src)) ? (__extension__ ({ fl_println(fl_str_n(2, fl_str_val("[load] 파일 없음: "), rel_path)); (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = acc;
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}));  })) : (__extension__ ({ swap_bang(loaded_paths_atom, (__extension__ ({ FLValue __env_16[1] = {abs_path}; fl_fn_new(__fl_anon_16, 1, __env_16); }))); ((__extension__ ({
    FLValue sub_dir = path_dir(abs_path);
    FLValue sub_raw = parse(lex(src));
    FLValue expanded = expand_loads(sub_raw, sub_dir);
    (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = append_all(acc, expanded);
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
}));
})));  })));
}))));
}))) : (__extension__ ({
    FLValue _fl_t0 = fl_add(i, fl_int(1));
    FLValue _fl_t1 = fl_vec_push(acc, node);
    i = _fl_t0;
    acc = _fl_t1;
    _fl_looping = 1; fl_nil();
})));
}))));
    }
    _fl_result;
})); fl_pop_frame(); return __fl_ret__; }
}

FLValue ast_json_str(FLValue s) {
    fl_push_frame_ln("ast_json_str", __LINE__);
    { FLValue __fl_ret__ = fl_str_n(3, fl_str_val("\""), str_replace(str_replace(s, fl_str_val("\\"), fl_str_val("\\\\")), fl_str_val("\""), fl_str_val("\\\"")), fl_str_val("\"")); fl_pop_frame(); return __fl_ret__; }
}

FLValue ast_node_to_json(FLValue node) {
    fl_push_frame_ln("ast_node_to_json", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(null_p(node)) ? fl_str_val("null") : ((__extension__ ({
    FLValue k = get(node, fl_str_val("kind"));
    (fl_truthy(fl_eq(k, fl_str_val("sexpr"))) ? fl_str_n(8, fl_str_val("{\"type\":\"call\",\"fn\":"), ast_json_str(get(node, fl_str_val("op"))), fl_str_val(",\"args\":["), join(fl_map_fn(fl_fn_new(__fl_wrap_ast_node_to_json, 0, NULL), get(node, fl_str_val("args"))), fl_str_val(",")), fl_str_val("]"), fl_str_val(",\"line\":"), get(node, fl_str_val("line")), fl_str_val("}")) : (fl_truthy(fl_eq(k, fl_str_val("literal"))) ? ((__extension__ ({
    FLValue t = get(node, fl_str_val("type"));
    FLValue v = get(node, fl_str_val("value"));
    (fl_truthy(fl_eq(t, fl_str_val("number"))) ? fl_str_n(3, fl_str_val("{\"type\":\"number\",\"value\":"), v, fl_str_val("}")) : (fl_truthy(fl_eq(t, fl_str_val("string"))) ? fl_str_n(3, fl_str_val("{\"type\":\"string\",\"value\":"), ast_json_str(v), fl_str_val("}")) : (fl_truthy(fl_eq(t, fl_str_val("symbol"))) ? fl_str_n(3, fl_str_val("{\"type\":\"symbol\",\"value\":"), ast_json_str(v), fl_str_val("}")) : (fl_truthy(fl_eq(t, fl_str_val("bool"))) ? fl_str_n(3, fl_str_val("{\"type\":\"bool\",\"value\":"), (fl_truthy(v) ? fl_str_val("true") : fl_str_val("false")), fl_str_val("}")) : fl_str_n(1, fl_str_val("{\"type\":\"null\"}"))))));
}))) : (fl_truthy(fl_eq(k, fl_str_val("variable"))) ? fl_str_n(3, fl_str_val("{\"type\":\"var\",\"name\":"), ast_json_str(get(node, fl_str_val("name"))), fl_str_val("}")) : (fl_truthy(fl_eq(k, fl_str_val("array-block"))) ? fl_str_n(3, fl_str_val("{\"type\":\"array\",\"items\":["), join(fl_map_fn(fl_fn_new(__fl_wrap_ast_node_to_json, 0, NULL), get(node, fl_str_val("items"))), fl_str_val(",")), fl_str_val("]}")) : fl_str_n(3, fl_str_val("{\"type\":\"unknown\",\"kind\":"), ast_json_str(k), fl_str_val("}"))))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue ast_tree_lines(FLValue node, FLValue prefix, FLValue is_last) {
    fl_push_frame_ln("ast_tree_lines", __LINE__);
    { FLValue __fl_ret__ = (fl_truthy(null_p(node)) ? fl_vec_new() : ((__extension__ ({
    FLValue k = get(node, fl_str_val("kind"));
    FLValue conn = (fl_truthy(is_last) ? fl_str_val("└─ ") : fl_str_val("├─ "));
    FLValue next = (fl_truthy(is_last) ? fl_str_val("   ") : fl_str_val("│  "));
    (fl_truthy(fl_eq(k, fl_str_val("sexpr"))) ? ((__extension__ ({
    FLValue head = fl_str_n(3, prefix, conn, get(node, fl_str_val("op")));
    FLValue args = get(node, fl_str_val("args"));
    FLValue n = length(args);
    FLValue child_lines = fl_reduce_fn((__extension__ ({ FLValue __env_18[4] = {args, prefix, next, n}; fl_fn_new(__fl_anon_18, 4, __env_18); })), fl_vec_new(), range(n));
    fl_vec_push(child_lines, head);
}))) : (fl_truthy(fl_eq(k, fl_str_val("literal"))) ? ((__extension__ ({
    FLValue v = get(node, fl_str_val("value"));
    FLValue t = get(node, fl_str_val("type"));
    (__extension__ ({ FLValue __fl_arr[1] = {fl_str_n(3, prefix, conn, (fl_truthy(fl_eq(t, fl_str_val("string"))) ? fl_str_n(3, fl_str_val("\""), v, fl_str_val("\"")) : fl_str_n(1, v)))}; fl_vec_from(__fl_arr, 1); }));
}))) : (fl_truthy(fl_eq(k, fl_str_val("variable"))) ? (__extension__ ({ FLValue __fl_arr[1] = {fl_str_n(4, prefix, conn, fl_str_val("$"), get(node, fl_str_val("name")))}; fl_vec_from(__fl_arr, 1); })) : (fl_truthy(fl_eq(k, fl_str_val("array-block"))) ? ((__extension__ ({
    FLValue head = fl_str_n(3, prefix, conn, fl_str_val("[…]"));
    FLValue items = get(node, fl_str_val("items"));
    FLValue n = length(items);
    FLValue child_lines = fl_reduce_fn((__extension__ ({ FLValue __env_17[4] = {items, prefix, next, n}; fl_fn_new(__fl_anon_17, 4, __env_17); })), fl_vec_new(), range(n));
    fl_vec_push(child_lines, head);
}))) : (__extension__ ({ FLValue __fl_arr[1] = {fl_str_n(3, prefix, conn, fl_str_val("?"))}; fl_vec_from(__fl_arr, 1); }))))));
})))); fl_pop_frame(); return __fl_ret__; }
}

FLValue ast_emit(FLValue input) {
    fl_push_frame_ln("ast_emit", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue _reset __attribute__((unused)) = swap_bang(loaded_paths_atom, fl_fn_new(__fl_anon_19, 0, NULL));
    FLValue src = fl_file_read(input);
    FLValue base_dir = path_dir(input);
    FLValue raw = parse(lex(src));
    FLValue nodes = expand_loads(raw, base_dir);
    FLValue json = fl_str_n(3, fl_str_val("["), join(fl_map_fn(fl_fn_new(__fl_wrap_ast_node_to_json, 0, NULL), nodes), fl_str_val(",")), fl_str_val("]"));
    FLValue tree_lines = fl_reduce_fn(fl_fn_new(__fl_anon_20, 0, NULL), fl_vec_new(), nodes);
    FLValue tree = join(tree_lines, fl_str_val("\n"));
    fl_println(fl_str_n(2, fl_str_val("AST_JSON:"), json));
    fl_println(fl_str_n(2, fl_str_val("AST_TREE:\n"), tree));
}))); fl_pop_frame(); return __fl_ret__; }
}

FLValue cgc_run(FLValue argv) {
    fl_push_frame_ln("cgc_run", __LINE__);
    { FLValue __fl_ret__ = ((__extension__ ({
    FLValue input = get(argv, fl_int(0));
    FLValue output = get(argv, fl_int(1));
    (fl_truthy(null_p(input)) ? fl_println(fl_str_val("usage: cgc-bin <input.fl> <output.c>\n       cgc-bin --ast <input.fl>")) : (fl_truthy(fl_eq(input, fl_str_val("--ast"))) ? (fl_truthy(null_p(output)) ? fl_println(fl_str_val("usage: cgc-bin --ast <input.fl>")) : ast_emit(output)) : (fl_truthy(null_p(output)) ? fl_println(fl_str_val("usage: cgc-bin <input.fl> <output.c>")) : ((__extension__ ({
    FLValue _reset __attribute__((unused)) = swap_bang(loaded_paths_atom, fl_fn_new(__fl_anon_21, 0, NULL));
    FLValue src = fl_file_read(input);
    FLValue base_dir = path_dir(input);
    FLValue raw = parse(lex(src));
    FLValue nodes = expand_loads(raw, base_dir);
    FLValue _v __attribute__((unused)) = ir_validate(nodes);
    FLValue c_code = generate_c(nodes);
    fl_file_write(output, c_code);
    fl_println(fl_str_n(4, fl_str_val("Compiled "), input, fl_str_val(" -> "), output));
}))))));
}))); fl_pop_frame(); return __fl_ret__; }
}

int main(int argc, char** argv) {
    fl_init_argv(argc, argv);
    #line 717 "<fl>"
    lambda_id_atom = fl_atom_new(fl_int(0));;
    #line 718 "<fl>"
    lambda_defs_atom = fl_atom_new(fl_vec_new());;
    #line 719 "<fl>"
    outer_params_atom = fl_atom_new(fl_vec_new());;
    #line 721 "<fl>"
    known_fncall_targets_atom = fl_atom_new(fl_vec_new());;
    #line 723 "<fl>"
    known_defns_atom = fl_atom_new(fl_vec_new());;
    #line 724 "<fl>"
    wrapper_defs_atom = fl_atom_new(fl_vec_new());;
    #line 726 "<fl>"
    global_decls_atom = fl_atom_new(fl_vec_new());;
    #line 728 "<fl>"
    defn_arity_atom = fl_atom_new(fl_map_new());;
    #line 730 "<fl>"
    cgc_defn_depth_atom = fl_atom_new(fl_int(0));;
    #line 731 "<fl>"
    cgc_hoisted_fns_atom = fl_atom_new(fl_str_val(""));;
    #line 2412 "<fl>"
    ir_kind_set = (__extension__ ({ FLValue __fl_arr[9] = {fl_str_val("literal"), fl_str_val("variable"), fl_str_val("keyword"), fl_str_val("sexpr"), fl_str_val("block"), fl_str_val("and"), fl_str_val("or"), fl_str_val("try"), fl_str_val("throw")}; fl_vec_from(__fl_arr, 9); }));;
    #line 2413 "<fl>"
    ir_lit_types = (__extension__ ({ FLValue __fl_arr[5] = {fl_str_val("number"), fl_str_val("string"), fl_str_val("boolean"), fl_str_val("nil"), fl_str_val("symbol")}; fl_vec_from(__fl_arr, 5); }));;
    #line 2494 "<fl>"
    loaded_paths_atom = fl_atom_new(fl_vec_new());;
    #line 2638 "<fl>"
    cgc_run(fl_get_argv());
    return 0;
}
