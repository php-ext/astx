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

#if PHP_MAJOR_VERSION < 7
# error ASTX requires PHP version 7 or newer
#endif

/* If you declare any globals in php_astx.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(astx)
*/

/* True global resources - no need for thread safety here */
static int le_astx;

/* {{{ PHP_INI
*/
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("astx.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_astx_globals, astx_globals)
STD_PHP_INI_ENTRY("astx.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_astx_globals, astx_globals)
PHP_INI_END()
*/
/* }}} */


/* Remove the following function when you have successfully modified config.m4
so that your module can be compiled into PHP, it exists only for testing
purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_astx_compiled(string arg)
Return a string to confirm that the extension is compiled in */
PHP_FUNCTION(confirm_astx_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Extension %.78s is now compiled into PHP.", "astx", arg);

	RETURN_STR(strg);
}
/* }}} */


/* {{{ php_astx_init_globals
*/
/* Uncomment this function if you have INI entries
static void php_astx_init_globals(zend_astx_globals *astx_globals)
{
astx_globals->global_value = 0;
astx_globals->global_string = NULL;
}
*/
/* }}} */


/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(astx)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(astx)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */


/* Remove if there's nothing to do at request start */
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


/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
*/
PHP_RSHUTDOWN_FUNCTION(astx)
{
	return SUCCESS;
}
/* }}} */


/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(astx)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "astx support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* {{{ astx_functions[]
*
* Every user visible function must have an entry in astx_functions[].
*/
const zend_function_entry astx_functions[] = {
	PHP_FE(confirm_astx_compiled, NULL)           /* For testing, remove later. */
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
	STANDARD_MODULE_PROPERTIES
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