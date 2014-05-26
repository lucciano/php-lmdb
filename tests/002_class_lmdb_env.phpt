--TEST--
LMDB\\Env class
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
var_dump(class_exists("Lmdb\\Env"));
--EXPECTF--
bool(true)

