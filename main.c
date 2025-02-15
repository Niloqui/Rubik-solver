/*
Compile with the following parameters:

-std=c23 -O3 -mavx2 -fshort-enums
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h> // struct timespec
// #include <windows.h> // Sleep()

// https://github.com/jean553/c-simd-avx2-example
#include <immintrin.h>

#include "utilities.h"
#include "face.h"
#include "move.h"
#include "cube.h"
#include "cube3.h"

// Kociemba algorithm
// https://kociemba.org/cube.htm




int main(int argc, char **argv) {
    struct timespec start, end, diff;
    timespec_get(&start, TIME_UTC);
    
    //char scramble[] = "R U2 R' U2 R U2 L' U R' U' L";
    char *scramble, *str;
    //cube_t cube = create_cube(3);
    
    
    scramble = argv[1];
    moves_seq_t seq = get_move_seq_from_str(3, scramble);
    str = get_str_from_move_seq(3, seq);
    
    printf_s("scramble = %s\n", scramble);
    printf_s("str      = %s\n", str);
    printf_s("%i\n", seq.len);
    
    /* 
    print_separator();
    
    solution_t sol;
    
    timespec_get(&start, TIME_UTC);
    //sol = solve_cube(cube);
    timespec_get(&end, TIME_UTC);
    
    diff = compute_timespec_difference(end, start);
    printf_s("\nIt took %u.%09u seconds to solve the %ix%i.\n",
            diff.tv_sec, diff.tv_nsec, cube.num_layers, cube.num_layers
    );
     */
    
    
    
    print_separator();
    
    cube3_t cube3;
    coord_cube_t coords;
    
    cube3 = create_solved_cube3();
    
    str = get_one_line_str_from_cube3(cube3, 0);
    printf_s("cube3 = %s\n", str);
    free(str);
    coords = get_coordinates_from_cube3(cube3);
    printf_s("cor_o = %i\n", coords.cor_o);
    printf_s("cor_p = %i\n", coords.cor_p);
    printf_s("edge_o = %i\n", coords.edge_o);
    printf_s("edge_p = %i\n\n", coords.edge_p);
    
    
    cube3 = apply_seq_to_cube3(cube3, seq);
    
    str = get_one_line_str_from_cube3(cube3, 0);
    printf_s("cube3 = %s\n", str);
    free(str);
    coords = get_coordinates_from_cube3(cube3);
    printf_s("cor_o = %i\n", coords.cor_o);
    printf_s("cor_p = %i\n", coords.cor_p);
    printf_s("edge_o = %i\n", coords.edge_o);
    printf_s("edge_p = %i\n\n", coords.edge_p);
    
    
    for(edges_e i=0; i<EDGE_LAST; i++){
        cube3.edges[EDGE_LAST-1-i].piece = i;
        cube3.edges[EDGE_LAST-1-i].orientation = 1;
    }
    for(corners_e i=0; i<CORNER_LAST; i++){
        cube3.corners[CORNER_LAST-1-i].piece = i;
        cube3.corners[CORNER_LAST-1-i].orientation = 2;
    }
    
    str = get_one_line_str_from_cube3(cube3, 0);
    printf_s("cube3 = %s\n", str);
    free(str);
    coords = get_coordinates_from_cube3(cube3);
    printf_s("cor_o = %i\n", coords.cor_o);
    printf_s("cor_p = %i\n", coords.cor_p);
    printf_s("edge_o = %i\n", coords.edge_o);
    printf_s("edge_p = %i\n\n", coords.edge_p);
    
    
    
    print_separator();
    print_separator();
    
    
    
    return 0;
}
































