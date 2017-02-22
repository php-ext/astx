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

///////////////////////////////////////////////////////////
// AstNode CLASS DEFINITION

zend_class_entry *astx_ce_ast_node = NULL;
zend_object_handlers ast_node_object_handlers;

static zend_object *ast_node_create_object(zend_class_entry *ce) {
	ast_node *node;

	node = ecalloc(1, sizeof(ast_node) + zend_object_properties_size(ce));
	zend_object_std_init(&node->std, ce);
	object_properties_init(&node->std, ce);
	node->std.handlers = &ast_node_object_handlers;
	return &node->std;
}

static void ast_node_free(zend_object *zobj) {
	ast_node *node;
	zval obj, *children;

	node = AST_NODE_P(zobj);
	ZVAL_OBJ(&obj, zobj);
	children = zend_read_property(zobj->ce, &obj, ASTX_OBJ_PROP("children"), 1, NULL);
	zval_ptr_dtor(children);

	if (node->tree) {
		if (--node->tree->refcount <= 0) {
			zend_ast_destroy(node->tree->root);
			zend_arena_destroy(node->tree->arena);
			efree(node->tree);
		}
	}
	zend_get_std_object_handlers()->free_obj(zobj);
}

static HashTable *ast_node_get_properties(zval *obj) {
	HashTable *ht;
	ast_node *node;
	zend_string *prop_val;


	node = Z_AST_NODE_P(obj);

	prop_val = zend_ast_export("", node->ast, "");
	zend_update_property_string(Z_OBJCE_P(obj), obj, ASTX_OBJ_PROP("code"), ZSTR_VAL(prop_val));
	zend_string_release(prop_val);
	zend_update_property_long(Z_OBJCE_P(obj), obj, ASTX_OBJ_PROP("attr"), node->ast->attr);
	zend_update_property_long(Z_OBJCE_P(obj), obj, ASTX_OBJ_PROP("line"), node->ast->lineno);
	zend_update_property_long(Z_OBJCE_P(obj), obj, ASTX_OBJ_PROP("kind"), node->ast->kind);

	prop_val = astx_ast_kind_to_string(node->ast->kind);
	zend_update_property_string(Z_OBJCE_P(obj), obj, ASTX_OBJ_PROP("kind(constant name)"), ZSTR_VAL(prop_val));
	zend_string_release(prop_val);

	ht = zend_get_std_object_handlers()->get_properties(obj);
	return ht;
}

/* {{{ AstNode::accept(AstNodeVisitor $visitor) : void*/
ZEND_BEGIN_ARG_INFO(astx_arg_info_ast_node_accept, 0)
ZEND_ARG_OBJ_INFO(0, visitor, AstNodeVisitor, 0)
ZEND_END_ARG_INFO()
static ZEND_METHOD(AstNode, accept) {
	zval *visitor;
	ast_node *node;
	zval retval, child;
	uint32_t i, cc;
	zend_object *zobj;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_EX(visitor, 1, 0)
		ZEND_PARSE_PARAMETERS_END();

	node = Z_AST_NODE_P(getThis());

	zend_call_method(visitor, Z_OBJ_P(visitor)->ce, NULL, "visit", sizeof("visit") - 1, &retval, 1, getThis(), NULL);

	if (Z_TYPE_INFO_P(&retval) == IS_FALSE) {
		return;
	}
	if (node->ast->kind == ZEND_AST_ZVAL) {
		return;
	}

	if (zend_ast_is_list(node->ast)) {
		cc = ((zend_ast_list*)node->ast)->children;
		for (i = 0; i < cc; i++) {
			zobj = astx_create_ast_node(((zend_ast_list*)node->ast)->child[i], node->tree);
			if (zobj) {
				ZVAL_OBJ(&child, zobj);
				zend_call_method(&child, astx_ce_ast_node, NULL, "accept", sizeof("accept") - 1, NULL, 1, visitor, NULL);
				ZVAL_PTR_DTOR(&child);
			}
		}
		return;
	}

	if (node->ast->kind > (1 << ZEND_AST_SPECIAL_SHIFT) && node->ast->kind < (1 << ZEND_AST_IS_LIST_SHIFT)) {
		for (i = 0; i < 4; i++) {
			zobj = astx_create_ast_node(((zend_ast_decl*)node->ast)->child[i], node->tree);
			if (zobj) {
				ZVAL_OBJ(&child, zobj);
				zend_call_method(&child, astx_ce_ast_node, NULL, "accept", sizeof("accept") - 1, NULL, 1, visitor, NULL);
				ZVAL_PTR_DTOR(&child);
			}
		}
		return;
	}

	cc = zend_ast_get_num_children(node->ast);
	for (i = 0; i < cc; i++) {
		zobj = astx_create_ast_node(node->ast->child[i], node->tree);
		if (zobj) {
			ZVAL_OBJ(&child, zobj);
			zend_call_method(&child, astx_ce_ast_node, NULL, "accept", sizeof("accept") - 1, NULL, 1, visitor, NULL);
			ZVAL_PTR_DTOR(&child);
		}
	}
}/* }}} */

 /* {{{ proto AstNode::export($prefix='', $suffix='') : string */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(astx_arg_info_ast_node_export, IS_STRING, NULL, 0)
ZEND_END_ARG_INFO()
static ZEND_METHOD(AstNode, export) {
	ast_node *node;
	zend_string *ret;
	//TODO add prefix suffix
	node = Z_AST_NODE_P(getThis());

	ret = zend_ast_export("", node->ast, "");

	if (!ret) {
		RETURN_EMPTY_STRING();
	}
	RETURN_STR(ret);
}/* }}} */

 /* {{{ AstNode::getChildren() : AstNodes */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(astx_arg_info_ast_node_getchildren, IS_OBJECT, "AstNodes", 1)
ZEND_END_ARG_INFO()
static ZEND_METHOD(AstNode, getChildren) {
	ast_node *node;
	zval *children;

	ZEND_PARSE_PARAMETERS_START(0, 0)
		ZEND_PARSE_PARAMETERS_END_EX(NULL);

	node = Z_AST_NODE_P(getThis());
	children = zend_read_property(node->std.ce, getThis(), ASTX_OBJ_PROP("children"), 1, NULL);

	RETURN_ZVAL(children, 1, NULL);
}/* }}} */

 /* {{{ proto AstNode::getKind() : int */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(astx_arg_info_ast_node_getkind, IS_LONG, NULL, 0)
ZEND_END_ARG_INFO()
static ZEND_METHOD(AstNode, getKind) {
	ast_node *node;

	ZEND_PARSE_PARAMETERS_START(0, 0)
		ZEND_PARSE_PARAMETERS_END_EX(NULL);

	node = Z_AST_NODE_P(getThis());
	RETURN_LONG(node->ast->kind);
}/* }}} */

 /* {{{ proto AstNode::isKind() : bool */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(astx_arg_info_ast_node_iskind, _IS_BOOL, NULL, 0)
ZEND_ARG_TYPE_INFO(0, kind, IS_LONG, 0)
ZEND_END_ARG_INFO()
static ZEND_METHOD(AstNode, isKind) {
	ast_node *node;
	zend_long kind;

	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_LONG(kind);
	ZEND_PARSE_PARAMETERS_END_EX(NULL);

	node = Z_AST_NODE_P(getThis());
	if (node->ast->kind == kind) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}/* }}} */

 /* {{{ proto AstNode::getValue() : mixed */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(astx_arg_info_ast_node_getvalue, _IS_BOOL, NULL, 1)
ZEND_END_ARG_INFO()
static ZEND_METHOD(AstNode, getValue) {
	ast_node *node;
	zval *retval;

	ZEND_PARSE_PARAMETERS_START(0, 0)
		ZEND_PARSE_PARAMETERS_END_EX(NULL);

	node = Z_AST_NODE_P(getThis());
	if (node->ast->kind != ZEND_AST_ZVAL) {
		RETURN_NULL();
	}
	retval = zend_ast_get_zval(node->ast);
	RETURN_ZVAL(retval, 1, 0);


}/* }}} */

zend_function_entry astx_me_ast_node[] = {
	ZEND_ME(AstNode, accept, astx_arg_info_ast_node_accept, ZEND_ACC_PUBLIC)
	ZEND_ME(AstNode, export, astx_arg_info_ast_node_export, ZEND_ACC_PUBLIC)
	ZEND_ME(AstNode, getChildren, astx_arg_info_ast_node_getchildren, ZEND_ACC_PUBLIC)
	ZEND_ME(AstNode, getKind, astx_arg_info_ast_node_getkind, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	ZEND_ME(AstNode, isKind, astx_arg_info_ast_node_iskind, ZEND_ACC_PUBLIC | ZEND_ACC_FINAL)
	ZEND_ME(AstNode, getValue, astx_arg_info_ast_node_getvalue, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

int astx_register_class_ast_node() {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "AstNode", astx_me_ast_node);
	astx_ce_ast_node = zend_register_internal_class(&ce);
	astx_ce_ast_node->create_object = ast_node_create_object;
	memcpy(&ast_node_object_handlers, zend_get_std_object_handlers(), sizeof(ast_node_object_handlers));
	ast_node_object_handlers.free_obj = ast_node_free;
	ast_node_object_handlers.get_properties = ast_node_get_properties;
	ast_node_object_handlers.offset = XtOffsetOf(ast_node, std);

	zend_declare_property_string(astx_ce_ast_node, ASTX_OBJ_PROP("code"), "", ZEND_ACC_PRIVATE);
	zend_declare_property_long(astx_ce_ast_node, ASTX_OBJ_PROP("attr"), 0, ZEND_ACC_PUBLIC);
	zend_declare_property_long(astx_ce_ast_node, ASTX_OBJ_PROP("kind"), 0, ZEND_ACC_PUBLIC);
	zend_declare_property_string(astx_ce_ast_node, ASTX_OBJ_PROP("kind(constant name)"), "", ZEND_ACC_PRIVATE);
	zend_declare_property_long(astx_ce_ast_node, ASTX_OBJ_PROP("line"), 0, ZEND_ACC_PUBLIC);
	zend_declare_property_null(astx_ce_ast_node, ASTX_OBJ_PROP("children"), ZEND_ACC_PUBLIC);
	zend_declare_property_null(astx_ce_ast_node, ASTX_OBJ_PROP("value"), ZEND_ACC_PUBLIC);

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