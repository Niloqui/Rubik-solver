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
#include "solver.h"

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
    
    printf_s("It took %u.%09u seconds to setup the 3x3x3 movement tables.\n\n",
            diff.tv_sec, diff.tv_nsec);
    
    
    // 3x3x3 symmetries tables
    timespec_get(&start, TIME_UTC);
    
    setup_all_symmetries_table();
    
    // This movement table requires symmetries to work,
    // that's why is not included with the movement tables above
    setup_ud_slice_sym_table();
    
    setup_corner_orientantion_symmetries();
    setup_edge_orientantion_symmetries();
    
    timespec_get(&end, TIME_UTC);
    diff = compute_timespec_difference(end, start);
    
    printf_s("It took %u.%09u seconds to setup the 3x3x3 symmetries tables.\n\n",
            diff.tv_sec, diff.tv_nsec);
    
    
    // 3x3x3 states tables
    setup_double_fast_tables();
    
    timespec_get(&start, TIME_UTC);
    if(setup_states_tables_3x3x3()){
        printf_s("Error in memory allocation. [setup_states_tables_3x3x3()]\n");
    }
    timespec_get(&end, TIME_UTC);
    diff = compute_timespec_difference(end, start);
    
    printf_s("It took %u.%09u seconds to setup the 3x3x3 states tables.\n\n",
            diff.tv_sec, diff.tv_nsec);
    
    
    // 3x3x3 pruning tables
    timespec_get(&start, TIME_UTC);
    //if(setup_pruning_tables_3x3x3()){
    if(1){
        printf_s("Error in memory allocation. [setup_pruning_tables_3x3x3()]\n");
    }
    timespec_get(&end, TIME_UTC);
    diff = compute_timespec_difference(end, start);
    
    printf_s("It took %u.%09u seconds to setup the 3x3x3 pruning tables.\n\n",
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
    char *scramble, *str, *str2;
    cube_t cube = create_cube(3);
    cube3_t cube3 = create_cube3(), new_cube3;
    coord_cube_t coords = create_coord_cube(), new_coords;
    
    
    scramble = argv[1];
    moves_seq_t seq = get_move_seq_from_str(3, scramble);
    str = get_str_from_move_seq(3, seq);
    
    printf_s("scramble = %s\n", scramble);
    printf_s("str      = %s\n", str);
    printf_s("seq.len  = %i\n", seq.len);
    
    free(str);
    
    
    /*
    print_separator();
    
    solution_t sol;
    
    apply_seq_to_cube(cube, seq);
    
    timespec_get(&start, TIME_UTC);
    sol = solve_cube(cube);
    timespec_get(&end, TIME_UTC);
    
    diff = compute_timespec_difference(end, start);
    printf_s("\nIt took %u.%09u seconds to solve the %ix%i.\n",
            diff.tv_sec, diff.tv_nsec, cube.num_layers, cube.num_layers
    );
    
    str = get_str_from_move_seq(3, sol.initial_rotations);
    str2 = get_str_from_move_seq(3, sol.solution);
    printf_s("Solution: %s%s%s\n", str, str[0]=='\0'?"":" ", str2);
    
    free(str);
    free(str2);
     */
    
    print_separator();
    
    cube3 = create_cube3();
    cube3 = apply_seq_to_cube3(cube3, seq);
    
    str = get_one_line_str_from_cube3(cube3, 0);
    printf_s("%s\n", str);
    free(str);
    
    coords = create_coord_cube_from_cube3(cube3);
    
    str = get_str_from_coord_cube(coords);
    printf_s("%s\n", str);
    free(str);
    
    compact_coord_t comp_coords = get_compact_coord_from_cube3(cube3);
    
    printf_s("comp_coords.cor_coord  = %d\n", comp_coords.cor_coord);
    printf_s("comp_coords.edge_coord = %d\n\n", comp_coords.edge_coord);
    
    
    
    print_separator();
    
    
    printf_s("size of triplet_fast_t: %d\n", sizeof(triplet_fast_t));
    printf_s("size of move_t: %d\n", sizeof(move_t));
    printf_s("size of fast_move_t: %d\n", sizeof(fast_move_t));
    
    
    
    
    
    
    
    free(cube.faces);
    
    free(prune_tbs.main_index);
    free(move_tbs.main_index);
    free(states_tbs.main_index);
    return 0;
}
































