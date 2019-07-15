#!/bin/bash
valgrind --leak-check=full --track-origins=yes bin/rom
