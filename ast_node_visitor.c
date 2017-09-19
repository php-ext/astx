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

///////////////////////////////////////////////////////////
// AstNodeVisitor CLASS DEFINITION

zend_class_entry *astx_ce_ast_node_visitor = NULL;

/* {{{ AstNodeVisitor::visit(AstNode $node):bool */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(astx_arg_info_ast_node_visitor_visit, _IS_BOOL, NULL, 0)
ZEND_ARG_OBJ_INFO(0, node, AstNode, 0)
ZEND_END_ARG_INFO()
static ZEND_METHOD(AstNodeVisitor, visit) {
	zval *node;
	ZEND_PARSE_PARAMETERS_START(1, 1)
		Z_PARAM_OBJECT_EX(node, 1, 0)
		ZEND_PARSE_PARAMETERS_END();
	RETURN_BOOL(IS_TRUE);
}/* }}} */

zend_function_entry astx_me_ast_node_visitor[] = {
	ZEND_ME(AstNodeVisitor, visit, astx_arg_info_ast_node_visitor_visit, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

int astx_register_class_ast_node_visitor() {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "AstNodeVisitor", astx_me_ast_node_visitor);
	astx_ce_ast_node_visitor = zend_register_internal_class(&ce);
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