--TEST--
Check for astx presence
--SKIPIF--
<?php if (!extension_loaded("astx")) print "skip"; ?>
--FILE--
<?php
echo "astx extension is available";
/*
        you can add regression tests for your extension here

  the output of your test code has to be equal to the
  text in the --EXPECT-- section below for the tests
  to pass, differences between the output and the
  expected text are interpreted as failure

  see php7/README.TESTING or https://qa.php.net/ for 
  further information on writing regression tests
*/
?>
--EXPECT--
astx extension is available
