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
// Ast CLASS DEFINITION

zend_class_entry *astx_ce_ast = NULL;

/* {{{ Ast::getNode($filename, $lineno=0, $nodeType=0)*/
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(astx_arg_info_ast_getnode, IS_OBJECT, "AstNode", 1)
ZEND_END_ARG_INFO()
static ZEND_METHOD(Ast, getNode) {
	zend_string *file;
	zend_long lineno = 0;
	zend_long nodeType = 0;
	zend_object *zobj;
	ast_node *node;
	zend_ast *ast;

	ZEND_PARSE_PARAMETERS_START(1, 3)
		Z_PARAM_STR(file)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(lineno)
		Z_PARAM_LONG(nodeType);
	ZEND_PARSE_PARAMETERS_END();

	zobj = astx_parse_file(file, 0);
	ZEND_ASSERT(zobj != NULL);

	node = AST_NODE_P(zobj);
	ast = astx_get_ast_at_line_alt(node->ast, (uint32_t)lineno);

	if (nodeType != 0)
		ast = astx_get_ast_by_node_type(ast, (uint32_t)nodeType);

	zobj = astx_create_ast_node(ast, node->tree);

	if (zobj == NULL) {
		RETURN_NULL();
	}

	//TODO deal with failure to create the node

	RETURN_OBJ(zobj);
}/* }}} */


 /* {{{ Ast::parseFile($filename) : AstNode*/
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(astx_arg_info_ast_parsefile, IS_OBJECT, "AstNode", 1)
//TODO
ZEND_END_ARG_INFO()
static ZEND_METHOD(Ast, parseFile) {
	zend_string *file;
	zend_object *zobj;
	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_STR(file)
		ZEND_PARSE_PARAMETERS_END();

	zobj = astx_parse_file(file, 0);
	ZEND_ASSERT(zobj != NULL);

	RETURN_OBJ(zobj);
}/* }}} */

 /* {{{ proto Ast::parseString(string $code, int $opts = 0) : AstNode*/
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(astx_arg_info_ast_parsestring, IS_OBJECT, "AstNode", 1)
ZEND_ARG_TYPE_INFO(0, code, IS_STRING, 0)
ZEND_ARG_TYPE_INFO(0, opts, IS_LONG, 0)
ZEND_END_ARG_INFO()
static ZEND_METHOD(Ast, parseString) {
	zend_string *code;
	zend_long opts = 0;
	zend_object *zobj;

	ZEND_PARSE_PARAMETERS_START(1, 2)
		Z_PARAM_STR(code)
		Z_PARAM_OPTIONAL
		Z_PARAM_LONG(opts)
		ZEND_PARSE_PARAMETERS_END();


	zobj = astx_parse_code(code, opts);
	if (!zobj) {
		RETURN_NULL();
	}
	RETURN_OBJ(zobj);
}/* }}} */

zend_function_entry astx_me_ast[] = {
	ZEND_ME(Ast, getNode, astx_arg_info_ast_getnode, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
	ZEND_ME(Ast, parseFile, astx_arg_info_ast_parsefile, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
	ZEND_ME(Ast, parseString, astx_arg_info_ast_parsestring, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

int astx_register_class_ast() {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "Ast", astx_me_ast);
	astx_ce_ast = zend_register_internal_class(&ce);
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