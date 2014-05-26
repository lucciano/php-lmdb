--TEST--
lmdb - info functions
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$functions = get_defined_functions();
var_dump(in_array("mdb_version", $functions["internal"]));
var_dump(in_array("mdb_strerror", $functions["internal"]));
var_dump(count(mdb_version()));
var_dump(mdb_strerror(0));
--EXPECTF--
bool(true)
bool(true)
int(4)
%s
