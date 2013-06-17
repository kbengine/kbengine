<?php
$sigar = new Sigar();

$mem = $sigar->mem();

$swap = $sigar->swap();

echo "\tTotal\tUsed\tFree\n";

echo "Mem:    " .
      ($mem->total() / 1024) . "\t" .
      ($mem->used() / 1024) . "\t" .
      ($mem->free() / 1024) . "\n";

echo "Swap:   " .
      ($swap->total() / 1024) . "\t" .
      ($swap->used() / 1024) . "\t" .
      ($swap->free() / 1024) . "\n";

echo "RAM:    " . $mem->ram() . "MB\n";
