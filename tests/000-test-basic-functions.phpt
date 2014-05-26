--TEST--
lmdb - info functions
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$functions = get_defined_functions();
var_dump(in_array("mdb_version", $functions["internal"]));
var_dump(mdb_version());
--EXPECTF--
bool(true)
array(4) {
  ["version"]=>
  string(30) "MDB 0.9.11: (January 15, 2014)"
  ["major"]=>
  int(0)
  ["minor"]=>
  int(9)
  ["patch"]=>
  int(11)
}
