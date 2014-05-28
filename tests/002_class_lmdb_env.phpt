--TEST--
LMDB\\Env class
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
var_dump(class_exists("Lmdb\\Env"));
var_dump(get_class_methods("Lmdb\\Env"));
$v = new Lmdb\Env();
var_dump(mdb_strerror($v->open(__DIR__. '/dbtest',0,0666)));
--EXPECTF--
bool(true)
array(1) {
  [0]=>
  string(11) "__construct"
}
