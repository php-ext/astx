/*
+----------------------------------------------------------------------+
| PHP Version 7                                                        |
+----------------------------------------------------------------------+
| Copyright (c) 1997-2016 The PHP Group                                |
+----------------------------------------------------------------------+
| This source file is subject to version 3.01 of the PHP license,      |
| that is bundled with this package in the file LICENSE, and is        |
| available through the world-wide-web at the following url:           |
| http://www.php.net/license/3_01.txt                                  |
| If you did not receive a copy of the PHP license and are unable to   |
| obtain it through the world-wide-web, please send a note to          |
| license@php.net so we can mail you a copy immediately.               |
+----------------------------------------------------------------------+
| Author:                                                              |
+----------------------------------------------------------------------+
*/

/* $Id$ */

#include "php_astx.h"
#include "core.h"

#include "zend_ast.h"
#include "zend_arena.h"
#include "zend_language_scanner.h"
#include "zend_language_parser.h" 
#include "zend_smart_str.h"

BOOL astx_allow_ast_process = FALSE;

static inline size_t ast_size(uint32_t children) {
	return sizeof(zend_ast) - sizeof(zend_ast *) + sizeof(zend_ast *) * children;
}


static inline size_t ast_list_size(uint32_t children) {
	return sizeof(zend_ast_list) - sizeof(zend_ast *) + sizeof(zend_ast *) * children;
}


static zend_ast *ast_copy(zend_ast *ast) {
	if (ast == NULL) {
		return NULL;
	}
	if (ast->kind == ZEND_AST_ZVAL) {
		zend_ast_zval* copy = zend_arena_alloc(&CG(ast_arena), sizeof(zend_ast_zval));
		*copy = *(zend_ast_zval*)ast;
		zval_copy_ctor(&copy->val);
		return (zend_ast*)copy;
	}

	if (zend_ast_is_list(ast)) {
		uint32_t i;
		zend_ast_list* list = zend_ast_get_list(ast);
		zend_ast_list* copy = zend_arena_alloc(&CG(ast_arena), ast_list_size(list->children));

		*copy = *list;
		for (i = 0; i < list->children; i++) {
			copy->child[i] = ast_copy(list->child[i]);
			zend_arena *p = CG(ast_arena);
		}
		return (zend_ast*)copy;
	}

	if (ast->kind >(1 << ZEND_AST_SPECIAL_SHIFT) && ast->kind < (1 << ZEND_AST_IS_LIST_SHIFT)) {
		uint32_t i;
		zend_ast_decl* decl = (zend_ast_decl*)ast;
		zend_ast_decl* copy = zend_arena_alloc(&CG(ast_arena), sizeof(zend_ast_decl));
		*copy = *decl;
		for (i = 0; i < 4; i++) {
			copy->child[i] = ast_copy(decl->child[i]);
			zend_arena *p = CG(ast_arena);
		}
		return (zend_ast*)copy;
	}

	uint32_t i;
	uint32_t cc = zend_ast_get_num_children(ast);
	zend_ast* copy = zend_arena_alloc(&CG(ast_arena), ast_size(cc));
	*copy = *ast;
	for (i = 0; i < cc; i++) {
		copy->child[i] = ast_copy(ast->child[i]);
	}
	return copy;
}


uint32_t astx_ast_get_num_children(zend_ast *ast) {
	ZEND_ASSERT(ast != NULL);
	if (ast->kind == ZEND_AST_ZVAL) {
		return 0;
	}
	if (zend_ast_is_list(ast)) {
		return ((zend_ast_list *)ast)->children;
	}
	if ((ast->kind > (1 << ZEND_AST_SPECIAL_SHIFT) && ast->kind < (1 << ZEND_AST_IS_LIST_SHIFT))) {
		return 3;
	}
	return zend_ast_get_num_children(ast);
}


zend_string *astx_ast_kind_to_string(zend_ast_kind kind) {
	smart_str ret = { 0 };

	switch (kind) {
		/*special nodes*/
	case ZEND_AST_ZVAL:
		smart_str_appends(&ret, "AST_ZVAL");
		break;
	case ZEND_AST_ZNODE:
		smart_str_appends(&ret, "AST_ZNODE");
		break;

		/*declaration nodes*/
	case ZEND_AST_FUNC_DECL:
		smart_str_appends(&ret, "AST_FUNC_DECL");
		break;
	case ZEND_AST_CLOSURE:
		smart_str_appends(&ret, "AST_CLOSURE");
		break;
	case ZEND_AST_METHOD:
		smart_str_appends(&ret, "AST_METHOD");
		break;
	case ZEND_AST_CLASS:
		smart_str_appends(&ret, "AST_CLASS");
		break;

		/*list nodes*/
	case ZEND_AST_ARG_LIST:
		smart_str_appends(&ret, "AST_ARG_LIST");
		break;
#if PHP_VERSION_ID < 70100
	case ZEND_AST_LIST:
		smart_str_appends(&ret, "AST_LIST");
		break;
#endif
	case ZEND_AST_ARRAY:
		smart_str_appends(&ret, "AST_ARRAY");
		break;
	case ZEND_AST_ENCAPS_LIST:
		smart_str_appends(&ret, "AST_ENCAPS_LIST");
		break;
	case ZEND_AST_EXPR_LIST:
		smart_str_appends(&ret, "AST_EXPR_LIST");
		break;
	case ZEND_AST_STMT_LIST:
		smart_str_appends(&ret, "AST_STMT_LIST");
		break;
	case ZEND_AST_IF:
		smart_str_appends(&ret, "AST_IF");
		break;
	case ZEND_AST_SWITCH_LIST:
		smart_str_appends(&ret, "AST_SWITCH_LIST");
		break;
	case ZEND_AST_CATCH_LIST:
		smart_str_appends(&ret, "AST_CATCH_LIST");
		break;
	case ZEND_AST_PARAM_LIST:
		smart_str_appends(&ret, "AST_PARAM_LIST");
		break;
	case ZEND_AST_CLOSURE_USES:
		smart_str_appends(&ret, "AST_CLOSURE_USES");
		break;
	case ZEND_AST_PROP_DECL:
		smart_str_appends(&ret, "AST_PROP_DECL");
		break;
	case ZEND_AST_CONST_DECL:
		smart_str_appends(&ret, "AST_CONST_DECL");
		break;
	case ZEND_AST_CLASS_CONST_DECL:
		smart_str_appends(&ret, "AST_CLASS_CONST_DECL");
		break;
	case ZEND_AST_NAME_LIST:
		smart_str_appends(&ret, "AST_NAME_LIST");
		break;
	case ZEND_AST_TRAIT_ADAPTATIONS:
		smart_str_appends(&ret, "AST_TRAIT_ADAPTATIONS");
		break;
	case ZEND_AST_USE:
		smart_str_appends(&ret, "AST_USE");
		break;

		/*0 child nodes*/
	case ZEND_AST_MAGIC_CONST:
		smart_str_appends(&ret, "AST_MAGIC_CONST");
		break;
	case ZEND_AST_TYPE:
		smart_str_appends(&ret, "AST_TYPE");
		break;

		/*1 child nodes*/
	case ZEND_AST_VAR:
		smart_str_appends(&ret, "AST_VAR");
		break;
	case ZEND_AST_CONST:
		smart_str_appends(&ret, "AST_CONST");
		break;
	case ZEND_AST_UNPACK:
		smart_str_appends(&ret, "AST_UNPACK");
		break;
	case ZEND_AST_UNARY_PLUS:
		smart_str_appends(&ret, "AST_UNARY_PLUS");
		break;
	case ZEND_AST_UNARY_MINUS:
		smart_str_appends(&ret, "AST_UNARY_MINUS");
		break;
	case ZEND_AST_CAST:
		smart_str_appends(&ret, "AST_CAST");
		break;
	case ZEND_AST_EMPTY:
		smart_str_appends(&ret, "AST_EMPTY");
		break;
	case ZEND_AST_ISSET:
		smart_str_appends(&ret, "AST_ISSET");
		break;
	case ZEND_AST_SILENCE:
		smart_str_appends(&ret, "AST_SILENCE");
		break;
	case ZEND_AST_SHELL_EXEC:
		smart_str_appends(&ret, "AST_SHELL_EXEC");
		break;
	case ZEND_AST_CLONE:
		smart_str_appends(&ret, "AST_CLONE");
		break;
	case ZEND_AST_EXIT:
		smart_str_appends(&ret, "AST_EXIT");
		break;
	case ZEND_AST_PRINT:
		smart_str_appends(&ret, "AST_PRINT");
		break;
	case ZEND_AST_INCLUDE_OR_EVAL:
		smart_str_appends(&ret, "AST_INCLUDE_OR_EVAL");
		break;
	case ZEND_AST_UNARY_OP:
		smart_str_appends(&ret, "AST_UNARY_OP");
		break;
	case ZEND_AST_PRE_INC:
		smart_str_appends(&ret, "AST_PRE_INC");
		break;
	case ZEND_AST_PRE_DEC:
		smart_str_appends(&ret, "AST_PRE_DEC");
		break;
	case ZEND_AST_POST_INC:
		smart_str_appends(&ret, "AST_POST_INC");
		break;
	case ZEND_AST_POST_DEC:
		smart_str_appends(&ret, "AST_POST_DEC");
		break;
	case ZEND_AST_YIELD_FROM:
		smart_str_appends(&ret, "AST_YIELD_FROM");
		break;

	case ZEND_AST_GLOBAL:
		smart_str_appends(&ret, "AST_GLOBAL");
		break;
	case ZEND_AST_UNSET:
		smart_str_appends(&ret, "AST_UNSET");
		break;
	case ZEND_AST_RETURN:
		smart_str_appends(&ret, "AST_RETURN");
		break;
	case ZEND_AST_LABEL:
		smart_str_appends(&ret, "AST_LABEL");
		break;
	case ZEND_AST_REF:
		smart_str_appends(&ret, "AST_REF");
		break;
	case ZEND_AST_HALT_COMPILER:
		smart_str_appends(&ret, "AST_HALT_COMPILER");
		break;
	case ZEND_AST_ECHO:
		smart_str_appends(&ret, "AST_ECHO");
		break;
	case ZEND_AST_THROW:
		smart_str_appends(&ret, "AST_THROW");
		break;
	case ZEND_AST_GOTO:
		smart_str_appends(&ret, "AST_GOTO");
		break;
	case ZEND_AST_BREAK:
		smart_str_appends(&ret, "AST_BREAK");
		break;
	case ZEND_AST_CONTINUE:
		smart_str_appends(&ret, "AST_CONTINUE");
		break;

		/*2 children nodes*/
	case ZEND_AST_DIM:
		smart_str_appends(&ret, "AST_DIM");
		break;
	case ZEND_AST_PROP:
		smart_str_appends(&ret, "AST_PROP");
		break;
	case ZEND_AST_STATIC_PROP:
		smart_str_appends(&ret, "AST_STATIC_PROP");
		break;
	case ZEND_AST_CALL:
		smart_str_appends(&ret, "AST_CALL");
		break;
	case ZEND_AST_CLASS_CONST:
		smart_str_appends(&ret, "AST_CLASS_CONST");
		break;
	case ZEND_AST_ASSIGN:
		smart_str_appends(&ret, "AST_ASSIGN");
		break;
	case ZEND_AST_ASSIGN_REF:
		smart_str_appends(&ret, "AST_ASSIGN_REF");
		break;
	case ZEND_AST_ASSIGN_OP:
		smart_str_appends(&ret, "AST_ASSIGN_OP");
		break;
	case ZEND_AST_BINARY_OP:
		smart_str_appends(&ret, "AST_BINARY_OP");
		break;
	case ZEND_AST_GREATER:
		smart_str_appends(&ret, "AST_GREATER");
		break;
	case ZEND_AST_GREATER_EQUAL:
		smart_str_appends(&ret, "AST_GREATER_EQUAL");
		break;
	case ZEND_AST_AND:
		smart_str_appends(&ret, "AST_AND");
		break;
	case ZEND_AST_OR:
		smart_str_appends(&ret, "AST_OR");
		break;
	case ZEND_AST_ARRAY_ELEM:
		smart_str_appends(&ret, "AST_ARRAY_ELEM");
		break;
	case ZEND_AST_NEW:
		smart_str_appends(&ret, "AST_NEW");
		break;
	case ZEND_AST_INSTANCEOF:
		smart_str_appends(&ret, "AST_INSTANCEOF");
		break;
	case ZEND_AST_YIELD:
		smart_str_appends(&ret, "AST_YIELD");
		break;
	case ZEND_AST_COALESCE:
		smart_str_appends(&ret, "AST_COALESCE");
		break;

	case ZEND_AST_STATIC:
		smart_str_appends(&ret, "AST_STATIC");
		break;
	case ZEND_AST_WHILE:
		smart_str_appends(&ret, "AST_WHILE");
		break;
	case ZEND_AST_DO_WHILE:
		smart_str_appends(&ret, "AST_DO_WHILE");
		break;
	case ZEND_AST_IF_ELEM:
		smart_str_appends(&ret, "AST_IF_ELEM");
		break;
	case ZEND_AST_SWITCH:
		smart_str_appends(&ret, "AST_SWITCH");
		break;
	case ZEND_AST_SWITCH_CASE:
		smart_str_appends(&ret, "AST_SWITCH_CASE");
		break;

	case ZEND_AST_DECLARE:
		smart_str_appends(&ret, "AST_DECLARE");
		break;
#if PHP_VERSION_ID < 70100
	case ZEND_AST_CONST_ELEM:
		smart_str_appends(&ret, "AST_CONST_ELEM");
		break;
#endif
	case ZEND_AST_USE_TRAIT:
		smart_str_appends(&ret, "AST_USE_TRAIT");
		break;
	case ZEND_AST_TRAIT_PRECEDENCE:
		smart_str_appends(&ret, "AST_TRAIT_PRECEDENCE");
		break;
	case ZEND_AST_METHOD_REFERENCE:
		smart_str_appends(&ret, "AST_METHOD_REFERENCE");
		break;
	case ZEND_AST_NAMESPACE:
		smart_str_appends(&ret, "AST_NAMESPACE");
		break;
	case ZEND_AST_USE_ELEM:
		smart_str_appends(&ret, "AST_USE_ELEM");
		break;
	case ZEND_AST_TRAIT_ALIAS:
		smart_str_appends(&ret, "AST_TRAIT_ALIAS");
		break;
	case ZEND_AST_GROUP_USE:
		smart_str_appends(&ret, "AST_GROUP_USE");
		break;

		/*3 children nodes*/
	case ZEND_AST_METHOD_CALL:
		smart_str_appends(&ret, "AST_METHOD_CALL");
		break;
	case ZEND_AST_STATIC_CALL:
		smart_str_appends(&ret, "AST_STATIC_CALL");
		break;
	case ZEND_AST_CONDITIONAL:
		smart_str_appends(&ret, "AST_CONDITIONAL");
		break;

	case ZEND_AST_TRY:
		smart_str_appends(&ret, "AST_TRY");
		break;
	case ZEND_AST_CATCH:
		smart_str_appends(&ret, "AST_CATCH");
		break;
	case ZEND_AST_PARAM:
		smart_str_appends(&ret, "AST_PARAM");
		break;
	case ZEND_AST_PROP_ELEM:
		smart_str_appends(&ret, "AST_PROP_ELEM");
		break;
#if PHP_VERSION_ID >= 70100
	case ZEND_AST_CONST_ELEM:
		smart_str_appends(&ret, "AST_CONST_ELEM");
		break;
#endif

		/*4 children nodes*/
	case ZEND_AST_FOR:
		smart_str_appends(&ret, "AST_FOR");
		break;
	case ZEND_AST_FOREACH:
		smart_str_appends(&ret, "AST_FOREACH");
		break;

	default:
		smart_str_appends(&ret, "UNKNOWN");
	}

	smart_str_0(&ret);
	return ret.s;
}


zend_ast *astx_get_ast_at_line(zend_ast *ast, uint32_t lineno) {
	zend_ast *retval = NULL;
	uint32_t i, cc;
	if (ast == NULL) {
		return retval;
	}

	if (ast->lineno == lineno) {
		return ast;
	}

	if (zend_ast_is_list(ast)) {
		zend_ast_list* list = zend_ast_get_list(ast);
		for (i = 0; i < list->children; i++) {
			retval = astx_get_ast_at_line(list->child[i], lineno);
			if (retval) {
				return retval;
			}
		}
		return retval;
	}

	if (ast->kind >(1 << ZEND_AST_SPECIAL_SHIFT) && ast->kind < (1 << ZEND_AST_IS_LIST_SHIFT)) {
		zend_ast_decl* decl = (zend_ast_decl*)ast;
		for (i = 1; i < 4; i++) {
			retval = astx_get_ast_at_line(decl->child[i], lineno);
			if (retval) {
				return retval;
			}
		}
		return retval;
	}

	cc = zend_ast_get_num_children(ast);
	for (i = 0; i < cc; i++) {
		retval = astx_get_ast_at_line(ast->child[i], lineno);
		if (retval) {
			return retval;
		}
	}

	return retval;
}


zend_ast* astx_get_closest_node_in_list(zend_ast_list *ast_list, uint32_t lineno) {

	zend_ast* closestChild = NULL;

	for (uint32_t i = 0; i < ast_list->children; i++) {
		if (ast_list->child[i]->lineno <= lineno) {
			closestChild = ast_list->child[i];
		}
		else
			return closestChild;
	}

	return closestChild;
}

zend_ast* astx_get_closest_node(zend_ast *ast, uint32_t lineno) {

	zend_ast* closestChild = NULL;

	for (uint32_t i = 0; i < zend_ast_get_num_children(ast); i++) {
		if (ast->child[i]->lineno <= lineno) {
			closestChild = ast->child[i];
		}
		else
			return closestChild;
	}

	return closestChild;
}


zend_ast *astx_get_ast_at_line_alt(zend_ast *ast, uint32_t lineno) {
	zend_ast *retval = NULL;
	uint32_t i;
	zend_ast* closest;

	if (ast == NULL) {
		return retval;
	}

	if (ast->lineno == lineno) {
		return ast;
	}

	if (zend_ast_is_list(ast)) {
		if ((closest = astx_get_closest_node_in_list(zend_ast_get_list(ast), lineno)) != NULL) {
			if ((retval = astx_get_ast_at_line_alt(closest, lineno)) != NULL) {
				return retval;
			}
		}
	}

	if (ast->kind > (1 << ZEND_AST_SPECIAL_SHIFT) && ast->kind < (1 << ZEND_AST_IS_LIST_SHIFT)) {
		for (i = 1; i < 4; i++) {
			if ((retval = astx_get_ast_at_line_alt(((zend_ast_decl*)ast)->child[i], lineno)) != NULL) {
				return retval;
			}
		}

		return retval;
	}

	if ((closest = astx_get_closest_node(ast, lineno)) != NULL) {
		if ((retval = astx_get_ast_at_line_alt(closest, lineno)) != NULL) {
			return retval;
		}
	}

	return retval;
}


zend_ast *astx_get_ast_by_node_type(zend_ast *ast, uint32_t nodeType) {
	zend_ast *retval = NULL;
	uint32_t i, cc;

	if (ast == NULL) {
		return NULL;
	}

	if (ast->kind == nodeType) {
		return ast;
	}

	if (zend_ast_is_list(ast)) {
		zend_ast_list* list = zend_ast_get_list(ast);
		for (i = 0; i < list->children; i++) {
			retval = astx_get_ast_by_node_type(list->child[i], nodeType);
			if (retval) {
				return retval;
			}
		}
		return retval;
	}

	if (ast->kind >(1 << ZEND_AST_SPECIAL_SHIFT) && ast->kind < (1 << ZEND_AST_IS_LIST_SHIFT)) {
		zend_ast_decl* decl = (zend_ast_decl*)ast;
		for (i = 1; i < 4; i++) {
			retval = astx_get_ast_by_node_type(decl->child[i], nodeType);
			if (retval) {
				return retval;
			}
		}
		return retval;
	}

	cc = zend_ast_get_num_children(ast);
	for (i = 0; i < cc; i++) {
		retval = astx_get_ast_by_node_type(ast->child[i], nodeType);
		if (retval) {
			return retval;
		}
	}


	return retval;
}


zend_object *astx_create_ast_node(zend_ast *ast, ast_tree *tree) {
	zend_class_entry *ce = NULL;
	ast_node *node;
	zend_object *nodes;
	zval obj, children, *value;

	if (ast == NULL) {
		return NULL;
	}

	ZEND_ASSERT(tree != NULL);

	if (ce == NULL) {
		ce = astx_ce_ast_node;
	}
	object_init_ex(&obj, ce);

	node = Z_AST_NODE_P(&obj);
	node->ast = ast;
	node->tree = tree;
	tree->refcount++;

	if (ast->kind == ZEND_AST_ZVAL) {
		value = zend_ast_get_zval(node->ast);
		zend_update_property(Z_OBJCE_P(&obj), &obj, ASTX_OBJ_PROP("value"), value);
	}
	else {
		nodes = astx_ce_ast_nodes->create_object(astx_ce_ast_nodes);
		AST_NODES_P(nodes)->parent = node;

		ZVAL_OBJ(&children, nodes);
		zend_update_property(Z_OBJCE_P(&obj), &obj, ASTX_OBJ_PROP("children"), &children);
		ZVAL_PTR_DTOR(&children);
	}
	zend_update_property_long(Z_OBJCE_P(&obj), &obj, ASTX_OBJ_PROP("attr"), node->ast->attr);
	zend_update_property_long(Z_OBJCE_P(&obj), &obj, ASTX_OBJ_PROP("line"), node->ast->lineno);
	zend_update_property_long(Z_OBJCE_P(&obj), &obj, ASTX_OBJ_PROP("kind"), node->ast->kind);

	return Z_OBJ_P(&obj);
}

void astx_ast_process(zend_ast *ast) {
	zend_lex_state lex_state;
	zend_arena* prev_arena;
	ast_tree* tree;
	zval obj;
	zend_object *zobj;

	ZEND_ASSERT(ast == CG(ast));

	zend_save_lexical_state(&lex_state);
	prev_arena = CG(ast_arena);
	CG(ast_arena) = zend_arena_create(1024 * 32);
	tree = emalloc(sizeof(ast_tree));

	tree->root = ast_copy(CG(ast));
	tree->arena = CG(ast_arena);
	tree->refcount = 0;

	zobj = astx_create_ast_node(tree->root, tree);
	ZEND_ASSERT(zobj != NULL);

	ZVAL_OBJ(&obj, zobj);
	zend_hash_index_add(&ASTX_G(file_ast_nodes), ZSTR_HASH(lex_state.filename), &obj);

	CG(ast_arena) = prev_arena;

	zend_restore_lexical_state(&lex_state);

	if (prev_ast_process &&	astx_allow_ast_process) {
		prev_ast_process(ast);
	}
}

zend_object *astx_parse_file(zend_string *file, zend_long opts) {
	zend_file_handle file_handle;
	zend_lex_state lex_state;
	ast_tree *tree;
	zval obj;
	zend_object *zobj;

	ZEND_ASSERT(file != NULL);

	if (zend_hash_index_exists(&ASTX_G(file_ast_nodes), ZSTR_HASH(file))) {		
		zobj = Z_OBJ_P(zend_hash_index_find(&ASTX_G(file_ast_nodes), ZSTR_HASH(file)));
		return zobj;
	}

	if (zend_stream_open(ZSTR_VAL(file), &file_handle) == FAILURE) {
		return NULL;
	}
	if (open_file_for_scanning(&file_handle) == FAILURE) {
		zend_destroy_file_handle(&file_handle);
		return NULL;
	}

	zend_save_lexical_state(&lex_state);

	CG(ast) = NULL;
	CG(ast_arena) = zend_arena_create(1024 * 32);

	if (zendparse() != 0) {
		php_error_docref(NULL, E_WARNING, "Failed to parse file %s", ZSTR_VAL(file));
		zend_ast_destroy(CG(ast));
		zend_arena_destroy(CG(ast_arena));
		CG(ast) = NULL;
		CG(ast_arena) = NULL;
		zend_restore_lexical_state(&lex_state);
		zend_destroy_file_handle(&file_handle);
		return NULL;
	}

	//	if (zend_ast_process && !(opts&AST_NO_PROCESS)) {
	//		(prev_ast_process) ? prev_ast_process(CG(ast)) : zend_ast_process(CG(ast));
	//	}

	tree = emalloc(sizeof(ast_tree));
	tree->arena = CG(ast_arena);
	tree->root = CG(ast);
	tree->refcount = 0;

	CG(ast) = NULL;
	CG(ast_arena) = NULL;
	zend_restore_lexical_state(&lex_state);
	zend_destroy_file_handle(&file_handle);

	zobj = astx_create_ast_node(tree->root, tree);

	ZEND_ASSERT(zobj != NULL);
	ZVAL_OBJ(&obj, zobj);
	zend_hash_index_add(&ASTX_G(file_ast_nodes), ZSTR_HASH(file), &obj);

	return zobj;
}

zend_object *astx_parse_code(zend_string *code, zend_long opts) {
	zval str;
	ast_tree* tree;
	zend_lex_state lex_state;

	ZEND_ASSERT(code != NULL);

	ZVAL_STR_COPY(&str, code);
	zend_prepare_string_for_scanning(&str, "ast_parse_string");
	zval_dtor(&str);

	zend_save_lexical_state(&lex_state);
	CG(ast) = NULL;
	CG(ast_arena) = zend_arena_create(1024 * 32);

	if (zendparse() != 0) {
		zend_ast_destroy(CG(ast));
		zend_arena_destroy(CG(ast_arena));
		CG(ast) = NULL;
		CG(ast_arena) = NULL;
		zend_restore_lexical_state(&lex_state);
		return NULL;
	}

	//TODO
	//if (zend_ast_process && !(opts & AST_NO_PROCESS)) {
	//	(prev_ast_process) ? prev_ast_process(CG(ast)) : zend_ast_process(CG(ast));
	//}

	tree = emalloc(sizeof(ast_tree));
	tree->arena = CG(ast_arena);
	tree->refcount = 0;
	tree->root = CG(ast);

	CG(ast) = NULL;
	CG(ast_arena) = NULL;
	zend_restore_lexical_state(&lex_state);

	return astx_create_ast_node(tree->root, tree);
}


/*
* Local variables:
* tab-width: 4
* c-basic-offset: 4
* End:
* vim600: noet sw=4 ts=4 fdm=marker
* vim<600: noet sw=4 ts=4
*/