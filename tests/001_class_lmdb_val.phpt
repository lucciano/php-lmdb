--TEST--
LMDB\\Val class
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
var_dump(class_exists("Lmdb\\Val"));
$a = new Lmdb\Val;
var_dump($a);
--EXPECTF--
bool(true)
object(Lmdb\Val)#1 (2) {
  ["mv_size"]=>
  int(0)
  ["mv_data"]=>
  NULL
}
