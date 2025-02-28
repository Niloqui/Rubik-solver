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
#include "coord_cubes.h"

// Kociemba algorithm
// https://kociemba.org/cube.htm


void initialization(){
    struct timespec end, start, diff;
    
    setup_factorial_table();
    setup_n_choose_k_table();
    
    // 3x3x3 movement tables
    timespec_get(&start, TIME_UTC);
    if(setup_movement_tables_3x3x3()){
        printf_s("Error in memory allocation. [setup_movement_tables()]\n");
    }
    timespec_get(&end, TIME_UTC);
    diff = compute_timespec_difference(end, start);
    
    printf_s("\nIt took %u.%09u seconds to setup the 3x3x3 movement tables.\n",
            diff.tv_sec, diff.tv_nsec);
    
    
    
    
    
    return;
}






int main(int argc, char **argv) {
    ///////////////////////////////////////////
    initialization();
    ///////////////////////////////////////////
    
    struct timespec start, end, diff;
    timespec_get(&start, TIME_UTC);
    
    //char scramble[] = "R U2 R' U2 R U2 L' U R' U' L";
    char *scramble, *str;
    cube_t cube = create_cube(3);
    
    
    scramble = argv[1];
    moves_seq_t seq = get_move_seq_from_str(3, scramble);
    str = get_str_from_move_seq(3, seq);
    
    printf_s("scramble = %s\n", scramble);
    printf_s("str      = %s\n", str);
    printf_s("%i\n", seq.len);
    
    free(str);
    
    
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
    coord_cube_t coord_cube;
    char *str2;
    
    cube3 = create_cube3();
    coord_cube = create_coord_cube_from_cube3(cube3);

    cube3 = apply_seq_to_cube3(cube3, seq);
    str = get_one_line_str_from_cube3(cube3, 0);
    printf_s("Moves applied directly to cube3:\n%s\n\n", str);
    
    coord_cube = apply_seq_to_coord_cube(coord_cube, seq);
    cube3 = create_cube3_from_coord_cube(coord_cube);
    str2 = get_one_line_str_from_cube3(cube3, 0);
    printf_s("Moves applied to coord_cube:\n%s\n\n", str2);
    
    
    printf_s("%s\n%s\n\n", str, str2);
    
    
    printf_s("str1 and str2 are %sequal.", 
        strcmp(str, str2)==0?"":"NOT "
    );
    
    free(str);
    free(str2);
    
    
    /* 
    str = get_str_from_coord_cube(coord_cube);
    printf_s("Solved state:\n%s\n", str);
    free(str);
    
    coord_cube = apply_seq_to_coord_cube(coord_cube, seq);
    str = get_str_from_coord_cube(coord_cube);
    printf_s("Seq applied to coord_cube:\n%s\n", str);
    free(str);
    
    cube3 = apply_seq_to_cube3(cube3, seq);
    coord_cube = create_coord_cube_from_cube3(cube3);
    str = get_str_from_coord_cube(coord_cube);
    printf_s("Seq applied to cube3, then transformed into coord_cube:\n%s\n", str);
    free(str);
     */
    
    
    
    //print_separator();
    //print_separator();
    
    
    free(cube.faces);
    return 0;
}
































