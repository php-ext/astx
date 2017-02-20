<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('astx')) {
        dl('astx.' . PHP_SHLIB_SUFFIX);
}

$functions = get_extension_funcs('astx');
echo "Functions available in the 'astx' extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";
$function = "confirm_astx_compiled";
if (extension_loaded('astx')) {
        $str = confirm_astx_compiled('astx');
} else {
        $str = "Extension 'astx' is not compiled into PHP";
}
echo "$str\n";
?>
