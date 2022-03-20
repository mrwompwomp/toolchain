#!/bin/sh
ez80-clang -O1 test.c -o /dev/null |& grep -m1 getVRegDef
