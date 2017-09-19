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

#include "zend_interfaces.h"
#include "ext/spl/spl_iterators.h"

///////////////////////////////////////////////////////////
// AstNodes CLASS DEFINITION

zend_class_entry *astx_ce_ast_nodes = NULL;

zend_object_handlers ast_nodes_object_handlers;

typedef struct _ast_nodes_iterator {
	zend_object_iterator	std;
	ast_nodes				*nodes;
	uint32_t				offset;
	zval					current;
}ast_nodes_iterator;

zend_object_iterator_funcs ast_nodes_iterator_funcs;

///////////////////////////////////////////////////////////
// AstNodes iterator

static void ast_nodes_iterator_dtor(zend_object_iterator *zobjit) {
	ast_nodes_iterator *it;
	zend_object *nodes;

	it = (ast_nodes_iterator*)zobjit;
	nodes = Z_OBJ_P(&it->std.data);
	// why do we need here zval_dtor_func_for_ptr and in move next we need zval_dtor
	zval_ptr_dtor(&it->current);
	GC_REFCOUNT(nodes)--;
}

static int ast_nodes_iterator_valid(zend_object_iterator *zobjit) {
	ast_nodes_iterator *it;
	uint32_t cc;

	it = (ast_nodes_iterator*)zobjit;
	cc = astx_ast_get_num_children(it->nodes->parent->ast);
	return it->offset < cc ? SUCCESS : FAILURE;
}

static zval *ast_nodes_iterator_get_current_data(zend_object_iterator *zobjit) {
	return &((ast_nodes_iterator*)zobjit)->current;
}

static void ast_nodes_iterator_get_current_key(zend_object_iterator *zobjit, zval *key) {
	ZVAL_LONG(key, ((ast_nodes_iterator*)zobjit)->offset);
}

static void ast_nodes_iterator_move_forward(zend_object_iterator *zobjit) {
	ast_nodes_iterator *it;
	ast_nodes *nodes;
	zend_ast *ast;
	zend_object *zobj;
	uint32_t cc;

	it = (ast_nodes_iterator*)zobjit;
	nodes = it->nodes;
	ast = nodes->parent->ast;
	cc = astx_ast_get_num_children(ast);

	zval_dtor(&it->current);
	it->offset++;

	if (it->offset == cc) {
		return;
	}

	if (zend_ast_is_list(ast)) {
		zobj = astx_create_ast_node(((zend_ast_list*)ast)->child[it->offset], nodes->parent->tree);
	}
	else if (ast->kind > (1 << ZEND_AST_SPECIAL_SHIFT) && ast->kind < (1 << ZEND_AST_IS_LIST_SHIFT)) {
		zobj = astx_create_ast_node(((zend_ast_decl*)ast)->child[it->offset], nodes->parent->tree);
	}
	else {
		zobj = astx_create_ast_node(ast->child[it->offset], nodes->parent->tree);
	}
	if (zobj) {
		ZVAL_OBJ(&it->current, zobj);
	}
	else {
		ZVAL_NULL(&it->current);
	}

}

static void ast_nodes_iterator_rewind(zend_object_iterator *zobjit) {
	ast_nodes_iterator *it;

	it = (ast_nodes_iterator*)zobjit;
	it->offset = -1;
	ast_nodes_iterator_move_forward(zobjit);
}

zend_object_iterator_funcs ast_nodes_iterator_funcs = {
	ast_nodes_iterator_dtor,
	ast_nodes_iterator_valid,
	ast_nodes_iterator_get_current_data,
	ast_nodes_iterator_get_current_key,
	ast_nodes_iterator_move_forward,
	ast_nodes_iterator_rewind,
	NULL
};

///////////////////////////////////////////////////////////
// AstNodes class handlers

static zend_object *ast_nodes_create_object(zend_class_entry *ce) {
	ast_nodes *nodes;

	nodes = ecalloc(1, sizeof(ast_nodes) + zend_object_properties_size(ce));
	zend_object_std_init(&nodes->std, ce);
	object_properties_init(&nodes->std, ce);
	nodes->std.handlers = &ast_nodes_object_handlers;
	return &nodes->std;
}

static zend_object_iterator *ast_nodes_get_iterator(zend_class_entry *ce, zval *obj, int by_ref) {
	ast_nodes_iterator *it;
	if (by_ref) {
		zend_error(E_ERROR, "An iterator cannot be used with foreach by reference");
	}
	it = ecalloc(1, sizeof(ast_nodes_iterator));
	zend_iterator_init(&it->std);
	ZVAL_COPY(&it->std.data, obj);
	it->std.funcs = &ast_nodes_iterator_funcs;
	it->nodes = Z_AST_NODES_P(obj);
	it->offset = -1;
	return &it->std;
}


///////////////////////////////////////////////////////////
// AstNode object handlers

static void ast_nodes_free(zend_object *zobj) {
	ast_nodes *nodes;

	nodes = AST_NODES_P(zobj);

#if _DEBUG

	//zend_string *kind = astx_ast_kind_to_string(nodes->parent->ast->kind);
	//ASTX_TRACE("freeing %s#%d's children", ZSTR_VAL(kind), nodes->parent->ast->lineno);
	//zend_string_release(kind);
#endif

	nodes->parent = NULL;
	zend_get_std_object_handlers()->free_obj(zobj);
}



static HashTable *ast_nodes_get_properties(zval *obj) {
	HashTable *ht;
	ast_nodes *nodes;
	zend_ast *ast;
	zend_object *zobj;
	uint32_t i, cc;
	//zval zv;

	ht = zend_get_std_object_handlers()->get_properties(obj);
	nodes = Z_AST_NODES_P(obj);
	ast = nodes->parent->ast;

	if (ast->kind == ZEND_AST_ZVAL) {
		return ht;
	}
	if (zend_ast_is_list(ast)) {
		for (i = 0; i < ((zend_ast_list*)ast)->children; i++) {
			zval zv;
			ZVAL_NULL(&zv);
			zobj = astx_create_ast_node(((zend_ast_list*)ast)->child[i], nodes->parent->tree);
			if (zobj) {
				ZVAL_OBJ(&zv, zobj);
			}
			zend_hash_index_add(ht, i, &zv);
		}
		return ht;
	}

	if (ast->kind > (1 << ZEND_AST_SPECIAL_SHIFT) && ast->kind < (1 << ZEND_AST_IS_LIST_SHIFT)) {
		for (i = 0; i < 4; i++) {
			zval zv;
			ZVAL_NULL(&zv);
			zobj = astx_create_ast_node(((zend_ast_decl*)ast)->child[i], nodes->parent->tree);
			if (zobj) {
				ZVAL_OBJ(&zv, zobj);
			}
			zend_hash_index_add(ht, i, &zv);
		}
		return ht;
	}

	cc = zend_ast_get_num_children(ast);
	for (i = 0; i < cc; i++) {
		zval zv;
		ZVAL_NULL(&zv);
		zobj = astx_create_ast_node(ast->child[i], nodes->parent->tree);
		if (zobj) {
			ZVAL_OBJ(&zv, zobj);
		}
		zend_hash_index_add(ht, i, &zv);
	}

	return ht;
}

static zval *ast_nodes_read_dimension(zval *obj, zval *dimension, int type, zval *retval) {
	ast_nodes *nodes;
	uint32_t idx;
	zend_ast *ast;
	zend_object *zobj;

	nodes = Z_AST_NODES_P(obj);
	ast = nodes->parent->ast;
	idx = (uint32_t)Z_LVAL_P(dimension);
	ZVAL_NULL(retval);
	if (ast->kind == ZEND_AST_ZVAL) {
		return NULL;
	}

	if (zend_ast_is_list(ast)) {
		if (idx >= ((zend_ast_list*)ast)->children) {
			//FIXME
			//zend_throw_exception(NULL, "Index outside of range",0);
		}
		zobj = astx_create_ast_node(((zend_ast_list*)ast)->child[idx], nodes->parent->tree);
		if (zobj) {
			ZVAL_OBJ(retval, zobj);
		}
		return retval;
	}
	if (ast->kind > (1 << ZEND_AST_SPECIAL_SHIFT) && ast->kind < (1 << ZEND_AST_IS_LIST_SHIFT)) {
		if (idx >= 4) {
			//FIXME
			//zend_throw_exception("NULL", "Index outside of range",0);
		}
		zobj = astx_create_ast_node(((zend_ast_decl*)ast)->child[idx], nodes->parent->tree);
		if (zobj) {
			ZVAL_OBJ(retval, zobj);
		}
		return retval;
	}
	if (idx >= zend_ast_get_num_children(ast)) {
		//FIXME
		//zend_throw_exception(NULL, "Index outside of range",0);
	}
	zobj = astx_create_ast_node(ast->child[idx], nodes->parent->tree);
	if (zobj) {
		ZVAL_OBJ(retval, zobj);
	}
	return retval;
}

///////////////////////////////////////////////////////////
// AstNodes class declaration

/* {{{ AstNodes::count() */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(astx_arg_info_ast_nodes_count, IS_LONG, NULL, 0)
ZEND_END_ARG_INFO()
ZEND_METHOD(AstNodes, count) {
	ast_nodes *nodes;
	zend_long count;

	ZEND_PARSE_PARAMETERS_START(0, 0)
		ZEND_PARSE_PARAMETERS_END_EX(NULL);

	nodes = Z_AST_NODES_P(getThis());
	count = astx_ast_get_num_children(nodes->parent->ast);
	RETURN_LONG(count);
}/* }}} */

zend_function_entry astx_me_ast_nodes[] = {
	ZEND_ME(AstNodes, count, astx_arg_info_ast_nodes_count, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

int astx_register_class_ast_nodes() {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "AstNodes", astx_me_ast_nodes);
	astx_ce_ast_nodes = zend_register_internal_class(&ce);

	astx_ce_ast_nodes->create_object = ast_nodes_create_object;
	astx_ce_ast_nodes->get_iterator = ast_nodes_get_iterator;
	astx_ce_ast_nodes->iterator_funcs.funcs = &ast_nodes_iterator_funcs;
	zend_class_implements(astx_ce_ast_nodes, 2, spl_ce_Countable, zend_ce_traversable);

	memcpy(&ast_nodes_object_handlers, zend_get_std_object_handlers(), sizeof(ast_nodes_object_handlers));
	ast_nodes_object_handlers.free_obj = ast_nodes_free;
	ast_nodes_object_handlers.get_properties = ast_nodes_get_properties;
	ast_nodes_object_handlers.read_dimension = ast_nodes_read_dimension;
	ast_nodes_object_handlers.offset = XtOffsetOf(ast_nodes, std);
	return SUCCESS; 
}

/*
* Local variables:
* tab-width: 4
* c-basic-offset: 4
* End:
* vim600: noet sw=4 ts=4 fdm=marker
* vim<600: noet sw=4 ts=4
*/