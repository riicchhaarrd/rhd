#!/bin/bash

gcc -g linked_list_test3.c
valgrind --leak-check=yes ./a.out
gcc -g hash_map_test2.c
valgrind --leak-check=yes ./a.out