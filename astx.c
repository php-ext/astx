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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_astx.h"
#include "core.h"

#if PHP_MAJOR_VERSION < 7
# error ASTX requires PHP version 7 or newer
#endif

int astx_register_class_ast();
int astx_register_class_ast_node();
int astx_register_class_ast_nodes();
int astx_register_class_ast_node_visitor();


ZEND_DECLARE_MODULE_GLOBALS(astx)


/* True global resources - no need for thread safety here */
static int le_astx;

/* {{{ PHP_INI */
PHP_INI_BEGIN()
PHP_INI_ENTRY("astx.enable_ast_process",  "0", PHP_INI_ALL, NULL)
PHP_INI_ENTRY("astx.allow_ast_process", "0", PHP_INI_ALL, NULL)
PHP_INI_END()
/* }}} */


/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(astx)
{
	REGISTER_INI_ENTRIES();

	/*special nodes*/
	REGISTER_MAIN_LONG_CONSTANT("AST_ZVAL", ZEND_AST_ZVAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ZNODE", ZEND_AST_ZNODE, CONST_PERSISTENT | CONST_CS);

	/*declaration nodes*/
	REGISTER_MAIN_LONG_CONSTANT("AST_FUNC_DECL", ZEND_AST_FUNC_DECL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CLOSURE", ZEND_AST_CLOSURE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_METHOD", ZEND_AST_METHOD, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CLASS", ZEND_AST_CLASS, CONST_PERSISTENT | CONST_CS);

	/*list nodes*/
	REGISTER_MAIN_LONG_CONSTANT("AST_ARG_LIST", ZEND_AST_ARG_LIST, CONST_PERSISTENT | CONST_CS);
#if PHP_VERSION_ID < 70100
	REGISTER_MAIN_LONG_CONSTANT("AST_LIST", ZEND_AST_LIST, CONST_PERSISTENT | CONST_CS);
#endif
	REGISTER_MAIN_LONG_CONSTANT("AST_ARRAY", ZEND_AST_ARRAY, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ENCAPS_LIST", ZEND_AST_ENCAPS_LIST, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_EXPR_LIST", ZEND_AST_EXPR_LIST, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_STMT_LIST", ZEND_AST_STMT_LIST, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_IF", ZEND_AST_IF, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_SWITCH_LIST", ZEND_AST_SWITCH_LIST, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CATCH_LIST", ZEND_AST_CATCH_LIST, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_PARAM_LIST", ZEND_AST_PARAM_LIST, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CLOSURE_USES", ZEND_AST_CLOSURE_USES, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_PROP_DECL", ZEND_AST_PROP_DECL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CONST_DECL", ZEND_AST_CONST_DECL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CLASS_CONST_DECL", ZEND_AST_CLASS_CONST_DECL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_NAME_LIST", ZEND_AST_NAME_LIST, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_TRAIT_ADAPTATIONS", ZEND_AST_TRAIT_ADAPTATIONS, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_USE", ZEND_AST_USE, CONST_PERSISTENT | CONST_CS);

	/*0 child nodes*/
	REGISTER_MAIN_LONG_CONSTANT("AST_MAGIC_CONST", ZEND_AST_MAGIC_CONST, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_TYPE", ZEND_AST_TYPE, CONST_PERSISTENT | CONST_CS);

	/*1 child nodes*/
	REGISTER_MAIN_LONG_CONSTANT("AST_VAR", ZEND_AST_VAR, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CONST", ZEND_AST_CONST, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_UNPACK", ZEND_AST_UNPACK, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_UNARY_PLUS", ZEND_AST_UNARY_PLUS, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_UNARY_MINUS", ZEND_AST_UNARY_MINUS, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CAST", ZEND_AST_CAST, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_EMPTY", ZEND_AST_EMPTY, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ISSET", ZEND_AST_ISSET, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_SILENCE", ZEND_AST_SILENCE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_SHELL_EXEC", ZEND_AST_SHELL_EXEC, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CLONE", ZEND_AST_CLONE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_EXIT", ZEND_AST_EXIT, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_PRINT", ZEND_AST_PRINT, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_INCLUDE_OR_EVAL", ZEND_AST_INCLUDE_OR_EVAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_UNARY_OP", ZEND_AST_UNARY_OP, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_PRE_INC", ZEND_AST_PRE_INC, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_PRE_DEC", ZEND_AST_PRE_DEC, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_POST_INC", ZEND_AST_POST_INC, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_POST_DEC", ZEND_AST_POST_DEC, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_YIELD_FROM", ZEND_AST_YIELD_FROM, CONST_PERSISTENT | CONST_CS);

	REGISTER_MAIN_LONG_CONSTANT("AST_GLOBAL", ZEND_AST_GLOBAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_UNSET", ZEND_AST_UNSET, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_RETURN", ZEND_AST_RETURN, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_LABEL", ZEND_AST_LABEL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_REF", ZEND_AST_REF, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_HALT_COMPILER", ZEND_AST_HALT_COMPILER, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ECHO", ZEND_AST_ECHO, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_THROW", ZEND_AST_THROW, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_GOTO", ZEND_AST_GOTO, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_BREAK", ZEND_AST_BREAK, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CONTINUE", ZEND_AST_CONTINUE, CONST_PERSISTENT | CONST_CS);

	/*2 children nodes*/
	REGISTER_MAIN_LONG_CONSTANT("AST_DIM", ZEND_AST_DIM, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_PROP", ZEND_AST_PROP, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_STATIC_PROP", ZEND_AST_STATIC_PROP, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CALL", ZEND_AST_CALL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CLASS_CONST", ZEND_AST_CLASS_CONST, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN", ZEND_AST_ASSIGN, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_REF", ZEND_AST_ASSIGN_REF, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_OP", ZEND_AST_ASSIGN_OP, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_BINARY_OP", ZEND_AST_BINARY_OP, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_GREATER", ZEND_AST_GREATER, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_GREATER_EQUAL", ZEND_AST_GREATER_EQUAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_AND", ZEND_AST_AND, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_OR", ZEND_AST_OR, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ARRAY_ELEM", ZEND_AST_ARRAY_ELEM, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_NEW", ZEND_AST_NEW, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_INSTANCEOF", ZEND_AST_INSTANCEOF, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_YIELD", ZEND_AST_YIELD, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_COALESCE", ZEND_AST_COALESCE, CONST_PERSISTENT | CONST_CS);

	REGISTER_MAIN_LONG_CONSTANT("AST_STATIC", ZEND_AST_STATIC, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_WHILE", ZEND_AST_WHILE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_DO_WHILE", ZEND_AST_DO_WHILE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_IF_ELEM", ZEND_AST_IF_ELEM, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_SWITCH", ZEND_AST_SWITCH, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_SWITCH_CASE", ZEND_AST_SWITCH_CASE, CONST_PERSISTENT | CONST_CS);

	REGISTER_MAIN_LONG_CONSTANT("AST_DECLARE", ZEND_AST_DECLARE, CONST_PERSISTENT | CONST_CS);
#if PHP_VERSION_ID < 70100
	REGISTER_MAIN_LONG_CONSTANT("AST_CONST_ELEM", ZEND_AST_CONST_ELEM, CONST_PERSISTENT | CONST_CS);
#endif
	REGISTER_MAIN_LONG_CONSTANT("AST_USE_TRAIT", ZEND_AST_USE_TRAIT, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_TRAIT_PRECEDENCE", ZEND_AST_TRAIT_PRECEDENCE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_METHOD_REFERENCE", ZEND_AST_METHOD_REFERENCE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_NAMESPACE", ZEND_AST_NAMESPACE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_USE_ELEM", ZEND_AST_USE_ELEM, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_TRAIT_ALIAS", ZEND_AST_TRAIT_ALIAS, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_GROUP_USE", ZEND_AST_GROUP_USE, CONST_PERSISTENT | CONST_CS);

	/*3 children nodes*/
	REGISTER_MAIN_LONG_CONSTANT("AST_METHOD_CALL", ZEND_AST_METHOD_CALL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_STATIC_CALL", ZEND_AST_STATIC_CALL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CONDITIONAL", ZEND_AST_CONDITIONAL, CONST_PERSISTENT | CONST_CS);

	REGISTER_MAIN_LONG_CONSTANT("AST_TRY", ZEND_AST_TRY, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_CATCH", ZEND_AST_CATCH, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_PARAM", ZEND_AST_PARAM, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_PROP_ELEM", ZEND_AST_PROP_ELEM, CONST_PERSISTENT | CONST_CS);
#if PHP_VERSION_ID >= 70100
	REGISTER_MAIN_LONG_CONSTANT("AST_CONST_ELEM", ZEND_AST_CONST_ELEM, CONST_PERSISTENT | CONST_CS);
#endif

	/*4 children nodes*/
	REGISTER_MAIN_LONG_CONSTANT("AST_FOR", ZEND_AST_FOR, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_FOREACH", ZEND_AST_FOREACH, CONST_PERSISTENT | CONST_CS);

	/*ast flags*/
	REGISTER_MAIN_LONG_CONSTANT("AST_ACC_PUBLIC", ZEND_ACC_PUBLIC, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ACC_PROTECTED", ZEND_ACC_PROTECTED, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ACC_PRIVATE", ZEND_ACC_PRIVATE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ACC_STATIC", ZEND_ACC_STATIC, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ACC_ABSTRACT", ZEND_ACC_ABSTRACT, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ACC_FINAL", ZEND_ACC_FINAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ACC_RETURN_REFERENCE", ZEND_ACC_RETURN_REFERENCE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ACC_INTERFACE", ZEND_ACC_INTERFACE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ACC_TRAIT", ZEND_ACC_TRAIT, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ACC_EXPLICIT_ABSTRACT_CLASS", ZEND_ACC_EXPLICIT_ABSTRACT_CLASS, CONST_PERSISTENT | CONST_CS);

	/* magic constants*/
	/*REGISTER_MAIN_LONG_CONSTANT("AST_T_LINE", T_LINE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_T_FILE", T_FILE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_T_DIR", T_DIR, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_T_TRAIT_C", T_TRAIT_C, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_T_METHOD_C", T_METHOD_C, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_T_FUNC_C", T_FUNC_C, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_T_NS_C", T_NS_C, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_T_CLASS_C", T_CLASS_C, CONST_PERSISTENT | CONST_CS);*/

	REGISTER_MAIN_LONG_CONSTANT("AST_INCLUDE_ONCE", ZEND_INCLUDE_ONCE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_INCLUDE", ZEND_INCLUDE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_REQUIRE_ONCE", ZEND_REQUIRE_ONCE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_REQUIRE", ZEND_REQUIRE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_EVAL", ZEND_EVAL, CONST_PERSISTENT | CONST_CS);

	/*ast type attributes*/
	REGISTER_MAIN_LONG_CONSTANT("AST_IS_NULL", IS_NULL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_IS_BOOL", _IS_BOOL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_IS_LONG", IS_LONG, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_IS_DOUBLE", IS_DOUBLE, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_IS_STRING", IS_STRING, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_IS_ARRAY", IS_ARRAY, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_IS_OBJECT", IS_OBJECT, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_IS_CALLABLE", IS_CALLABLE, CONST_PERSISTENT | CONST_CS);

	/* ast binary operation assignment attributes*/
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_ADD", ZEND_ASSIGN_ADD, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_SUB", ZEND_ASSIGN_SUB, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_MUL", ZEND_ASSIGN_MUL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_DIV", ZEND_ASSIGN_DIV, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_MOD", ZEND_ASSIGN_MOD, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_SL", ZEND_ASSIGN_SL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_SR", ZEND_ASSIGN_SR, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_CONCAT", ZEND_ASSIGN_CONCAT, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_BW_OR", ZEND_ASSIGN_BW_OR, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_BW_AND", ZEND_ASSIGN_BW_AND, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ASSIGN_BW_XOR", ZEND_ASSIGN_BW_XOR, CONST_PERSISTENT | CONST_CS);

	/* ast binary operation attributes*/
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_ADD", ZEND_ADD, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_SUB", ZEND_SUB, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_MUL", ZEND_MUL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_DIV", ZEND_DIV, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_MOD", ZEND_MOD, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_SL", ZEND_SL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_SR", ZEND_SR, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_CONCAT", ZEND_CONCAT, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_BW_OR", ZEND_BW_OR, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_BW_AND", ZEND_BW_AND, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_BW_XOR", ZEND_BW_XOR, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_IS_IDENTICAL", ZEND_IS_IDENTICAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_IS_NOT_IDENTICAL", ZEND_IS_NOT_IDENTICAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_IS_EQUAL", ZEND_IS_EQUAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_IS_NOT_EQUAL", ZEND_IS_NOT_EQUAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_IS_SMALLER", ZEND_IS_SMALLER, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_IS_SMALLER_OR_EQUAL", ZEND_IS_SMALLER_OR_EQUAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_POW", ZEND_POW, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_BOOL_XOR", ZEND_BOOL_XOR, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("AST_ATTR_SPACESHIP", ZEND_SPACESHIP, CONST_PERSISTENT | CONST_CS);

	if (INI_BOOL("astx.enable_ast_process")) {
		prev_ast_process = zend_ast_process;
		zend_ast_process = astx_ast_process;
	}

//	if (INI_BOOL("astx.allow_ast_process")) {
//		astx_allow_ast_process = TRUE;
//	}

	astx_register_class_ast();
	astx_register_class_ast_node();
	astx_register_class_ast_nodes();
	astx_register_class_ast_node_visitor();
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(astx)
{
	if (prev_ast_process) {
		zend_ast_process = prev_ast_process;
	}
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_RINIT_FUNCTION
*/
PHP_RINIT_FUNCTION(astx)
{
#if defined(COMPILE_DL_astx) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_RSHUTDOWN_FUNCTION
*/
PHP_RSHUTDOWN_FUNCTION(astx)
{
	zend_hash_clean(&ASTX_G(file_ast_nodes));
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(astx)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "ast support", "enabled");
	php_info_print_table_end();
	
	DISPLAY_INI_ENTRIES();
}
/* }}} */


/* {{{ PHP_GINIT_FUNCTION
*/
PHP_GINIT_FUNCTION(astx)
{
	zend_hash_init(&astx_globals->file_ast_nodes, 32, NULL, ZVAL_PTR_DTOR, 1);
}
/* }}} */

/* {{{ astx_functions[]
*
* Every user visible function must have an entry in astx_functions[].
*/
const zend_function_entry astx_functions[] = {
	PHP_FE_END      /* Must be the last line in astx_functions[] */
};
/* }}} */


/* {{{ astx_module_entry
*/
zend_module_entry astx_module_entry = {
	STANDARD_MODULE_HEADER,
	"astx",
	astx_functions,
	PHP_MINIT(astx),
	PHP_MSHUTDOWN(astx),
	PHP_RINIT(astx),      /* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(astx),  /* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(astx),
	PHP_ASTX_VERSION,
	PHP_MODULE_GLOBALS(astx),
	PHP_GINIT(astx),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_ASTX
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(astx)
#endif

/*
* Local variables:
* tab-width: 4
* c-basic-offset: 4
* End:
* vim600: noet sw=4 ts=4 fdm=marker
* vim<600: noet sw=4 ts=4
*/