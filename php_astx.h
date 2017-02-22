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

#ifndef PHP_ASTX_H
#define PHP_ASTX_H

#include "php.h"

extern zend_module_entry astx_module_entry;
#define phpext_astx_ptr &astx_module_entry

#define PHP_ASTX_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#       define PHP_ASTX_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#       define PHP_ASTX_API __attribute__ ((visibility("default")))
#else
#       define PHP_ASTX_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(astx)
HashTable file_ast_nodes;
ZEND_END_MODULE_GLOBALS(astx)

ZEND_EXTERN_MODULE_GLOBALS(astx);

#define ASTX_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(astx, v)


typedef struct _ast_tree {
	zend_ast	*root;
	zend_arena	*arena;
	int			refcount;
}ast_tree;

typedef struct _ast_node {
	ast_tree	*tree;
	zend_ast	*ast;
	zend_object std;
}ast_node;

typedef struct _ast_nodes {
	ast_node	*parent;
	zend_object std;
}ast_nodes;

static inline ast_node *ast_node_from_zend_object(zend_object *zobj) {
	return (ast_node *)((char *)(zobj)-XtOffsetOf(ast_node, std));
}

static inline ast_nodes *ast_nodes_from_zend_object(zend_object *zobj) {
	return (ast_nodes *)((char *)(zobj)-XtOffsetOf(ast_nodes, std));
}

#define AST_NODE_P(zobj_p)		ast_node_from_zend_object(zobj_p)
#define Z_AST_NODE_P(zval_p)	AST_NODE_P(Z_OBJ_P(zval_p))

#define AST_NODES_P(zobj_p)		ast_nodes_from_zend_object(zobj_p)
#define Z_AST_NODES_P(zval_p)	AST_NODES_P(Z_OBJ_P(zval_p))

BEGIN_EXTERN_C()
zend_class_entry *astx_ce_ast;
zend_class_entry *astx_ce_ast_node;
zend_class_entry *astx_ce_ast_nodes;
zend_class_entry *astx_ce_ast_node_visitor;
END_EXTERN_C()

#if defined(ZTS) && defined(COMPILE_DL_ASTX)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#endif  /* PHP_ASTX_H */

/*
* Local variables:
* tab-width: 4
* c-basic-offset: 4
* End:
* vim600: noet sw=4 ts=4 fdm=marker
* vim<600: noet sw=4 ts=4
*/
