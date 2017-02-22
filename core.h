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

#ifndef CORE_H
#define CORE_H

#include "php_astx.h"
#include "zend_ast.h"

void(*prev_ast_process)(zend_ast *);
extern ZEND_API zend_ast_process_t zend_ast_process;
void astx_ast_process(zend_ast *);

uint32_t astx_ast_get_num_children(zend_ast *);
zend_string *astx_ast_kind_to_string(zend_ast_kind);
zend_ast *astx_get_ast_at_line(zend_ast *, uint32_t);
zend_ast *astx_get_ast_at_line_alt(zend_ast *, uint32_t);
zend_ast *astx_get_ast_by_node_type(zend_ast *, uint32_t);
zend_object *astx_parse_code(zend_string *, zend_long);
zend_object *astx_parse_file(zend_string *, zend_long);
zend_object *astx_create_ast_node(zend_ast *, ast_tree *);

#define ASTX_OBJ_PROP(prop) prop, sizeof(prop)-1

#endif  /* CORE_H */

/*
* Local variables:
* tab-width: 4
* c-basic-offset: 4
* End:
* vim600: noet sw=4 ts=4 fdm=marker
* vim<600: noet sw=4 ts=4
*/
