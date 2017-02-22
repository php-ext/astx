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