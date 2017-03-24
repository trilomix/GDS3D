# Voro++ makefile
#
# Author : Chris H. Rycroft (LBL / UC Berkeley)
# Email  : chr@alum.mit.edu
# Date   : August 30th 2011

# Load the common configuration file
include ../../config.mk

# List of executables
EXECUTABLES=rad_test finite_sys cylinder_inv single_cell_2d period sphere_mesh lloyd_box import_rahman import_nguyen polycrystal_rahman random_points_10 random_points_200 import_freeman

# Makefile rules
all: $(EXECUTABLES)

rad_test: rad_test.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o rad_test rad_test.cc -lvoro++

finite_sys: finite_sys.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o finite_sys finite_sys.cc -lvoro++

cylinder_inv: cylinder_inv.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o cylinder_inv cylinder_inv.cc -lvoro++

single_cell_2d: single_cell_2d.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o single_cell_2d single_cell_2d.cc -lvoro++

period: period.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o period period.cc -lvoro++

sphere_mesh: sphere_mesh.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o sphere_mesh sphere_mesh.cc -lvoro++

lloyd_box: lloyd_box.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o lloyd_box lloyd_box.cc -lvoro++

import_rahman: import_rahman.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o import_rahman import_rahman.cc -lvoro++

import_nguyen: import_nguyen.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o import_nguyen import_nguyen.cc -lvoro++

import_freeman: import_freeman.cc
	$(CXX) $(E_INC) $(E_LIB) -o import_freeman import_freeman.cc

polycrystal_rahman: polycrystal_rahman.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o polycrystal_rahman polycrystal_rahman.cc -lvoro++

random_points_10: random_points_10.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o random_points_10 random_points_10.cc -lvoro++

random_points_200: random_points_200.cc
	$(CXX) $(CFLAGS) $(E_INC) $(E_LIB) -o random_points_200 random_points_200.cc -lvoro++

clean:
	rm -f $(EXECUTABLES)

.PHONY: all clean
