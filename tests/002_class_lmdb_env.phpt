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
$txn = $v->tnx_begin(NULL, 0);
var_dump($txn);
--EXPECTF--
bool(true)
array(5) {
  [0]=>
  string(11) "__construct"
  [1]=>
  string(4) "open"
  [2]=>
  string(11) "set_mapsize"
  [3]=>
  string(10) "set_maxdbs"
  [4]=>
  string(9) "tnx_begin"
}
string(25) "No such file or directory"
