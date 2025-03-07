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
    
    // 3x3x3 symmetries tables
    setup_all_symmetries_table();
    
    
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
    char *scramble, *str, *str2;
    cube_t cube = create_cube(3);
    cube3_t cube3 = create_cube3();
    coord_cube_t coords = create_coord_cube(), coords2;
    
    
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
    
    
    /* 
    print_separator();
    
    cube3_t cube3 = create_cube3();
    int sym_idx = atoi(argv[2]);
    
    cube3 = apply_seq_to_cube3(cube3, seq);
    
    free(cube.faces);
    cube = create_cube_from_cube3(cube3);
    
    str = get_str_from_cube(cube);
    printf_s("Normal cube (is_cube3_solved = %i):\n%s\n", is_cube3_solved(cube3), str);
    free(str);
    
    
    cube3 = apply_symmetry_on_cube3(cube3, sym_idx);
    free(cube.faces);
    cube = create_cube_from_cube3(cube3);
    
    //remap_face_on_cube_no_centers(cube, sym_idx);
    
    str = get_str_from_cube(cube);
    printf_s("Cube after symmetry (is_cube3_solved = %i):\n%s\n", is_cube3_solved(cube3), str);
    free(str);
     */
    
    
    print_separator();
    
    /* 
    int i;
    fast_move_t fast_m, fast_m2;
    for(i=0; i<SYMMETRIES_N; i++){
        printf_s("\n\n---Symmetry %i\n", i);
        for(fast_m=0; fast_m<LAST_FAST_MOVE; fast_m++){
            fast_m2 = fast_m;
            printf_s("%2i -> ", fast_m2);
            
            fast_m2 = symmetries_inv_fast_m_table[i][fast_m2];
            printf_s("%2i -> ", fast_m2);
            
            fast_m2 = symmetries_fast_m_table[i][fast_m2];
            printf_s("%2i --- ", fast_m2);
            
            printf_s("%s\n", fast_m==fast_m2?"equal":"NOT EQUAL");
        }
    }
     */
    
    
    
    print_separator();
    
    /* 
    cube3 = apply_seq_to_cube3(cube3, seq);
    int ud_slice_coord = get_coord_ud_slice_permutation(cube3);
    
    int sym_coord = ud_slice_sorted_raw_to_sym_table[ud_slice_coord][0];
    int sym_sym = ud_slice_sorted_raw_to_sym_table[ud_slice_coord][1];
    
    printf_s("ud_slice_coord  = %i\n\n"
             "sym_coord_total = %i\n"
             "sym_coord       = %i\n"
             "symmetry        = %i\n"
             "sym_to_raw      = %i\n",
             ud_slice_coord,
             sym_coord*SYMMETRIES_N + sym_sym, 
             sym_coord, 
             sym_sym,
             ud_slice_sorted_sym_to_raw_table[sym_coord]);
     */
    
    
    
    
    /* 
    unsigned long long int val = (UDSLICE_N * CORNER_ORI_N * EDGE_ORI_N) / 4;
    char *test = malloc(sizeof(char) * val);
    
    printf_s("val = %llu\ntest = %p\n", val, test);
    
    if(test){
        for(unsigned long long int i=0; i<val; i++){
            test[i] = 'a';
        }
    }
     */
    
    
    
    
    
    
    
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
    
    free(move_tbs.main_index);
    return 0;
}
































