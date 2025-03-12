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
    
    
    
    // 3x3x3 pruning tables
    timespec_get(&start, TIME_UTC);
    if(setup_pruning_tables_3x3x3()){
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
    long long int new_cor_o, new_edge_o;
    int symmetry;
    
    cube3 = create_cube3();
    //coords = create_coord_cube();
    
    // new_coords
    // new_cube3
    
    cube3 = apply_seq_to_cube3(cube3, seq);
    
    symmetry = atoi(argv[2]);
    cube3 = multiply_cube3(symmetries_table[symmetry], cube3);
    cube3 = multiply_cube3(cube3, symmetries_inverse_table[symmetry]);
    
    coords = create_coord_cube_from_cube3(cube3);
    
    new_cor_o = corner_orientantion_symmetries[coords.cor_o][coords.ud_sym_s];
    new_edge_o = edge_orientantion_symmetries[coords.ud_sym_c][coords.edge_o][coords.ud_sym_s];
    
    printf_s("coords.ud_slice  = %i\n", coords.ud_slice);
    printf_s("coords.ud_sym_c  = %i\n", coords.ud_sym_c);
    printf_s("coords.ud_sym_s  = %i\n", coords.ud_sym_s);
    printf_s("coords.cor_o     = %i\n", coords.cor_o);
    printf_s("coords.edge_o    = %i\n", coords.edge_o);
    printf_s("new_cor_o        = %i\n", new_cor_o);
    printf_s("new_edge_o       = %i\n\n", new_edge_o);
    
    printf_s("\nedge_orientantion_symmetries values:\n");
    for(int i=0; i<SYMMETRIES_N; i++){
        printf_s("%i\n", edge_orientantion_symmetries[coords.ud_sym_c][coords.edge_o][i]);
    }
    printf_s("\n\n");
    
    //
    new_cube3 = multiply_cube3(symmetries_table[coords.ud_sym_s], cube3);
    new_cube3 = multiply_cube3(new_cube3, symmetries_inverse_table[coords.ud_sym_s]);
    
    new_coords = create_coord_cube_from_cube3(new_cube3);
    
    new_cor_o = corner_orientantion_symmetries[new_coords.cor_o][new_coords.ud_sym_s];
    new_edge_o = edge_orientantion_symmetries[new_coords.ud_sym_c][new_coords.edge_o][new_coords.ud_sym_s];
    
    printf_s("new_coords.ud_slice  = %i\n", new_coords.ud_slice);
    printf_s("new_coords.ud_sym_c  = %i\n", new_coords.ud_sym_c);
    printf_s("new_coords.ud_sym_s  = %i\n", new_coords.ud_sym_s);
    printf_s("new_coords.cor_o     = %i\n", new_coords.cor_o);
    printf_s("new_coords.edge_o    = %i\n", new_coords.edge_o);
    printf_s("new_cor_o            = %i\n", new_cor_o);
    printf_s("new_edge_o           = %i\n\n", new_edge_o);
    
     */
    
    /* 
    for(int i=0; i<SYMMETRIES_N; i++){
        printf_s("Symmetry %i\n", i);
        new_cube3 = multiply_cube3(symmetries_table[i], cube3);
        new_cube3 = multiply_cube3(new_cube3, symmetries_inverse_table[i]);
        
        new_coords = create_coord_cube_from_cube3(new_cube3);
        
        new_cor_o = corner_orientantion_symmetries[new_coords.cor_o][new_coords.ud_sym_s];
        new_edge_o = edge_orientantion_symmetries[new_coords.ud_sym_c][new_coords.edge_o][new_coords.ud_sym_s];
        
        printf_s("new_coords.ud_slice  = %i\n", new_coords.ud_slice);
        printf_s("new_coords.ud_sym_c  = %i\n", new_coords.ud_sym_c);
        printf_s("new_coords.ud_sym_s  = %i\n", new_coords.ud_sym_s);
        printf_s("new_coords.cor_o     = %i\n", new_coords.cor_o);
        printf_s("new_coords.edge_o    = %i\n", new_coords.edge_o);
        printf_s("new_cor_o            = %i\n", new_cor_o);
        printf_s("new_edge_o           = %i\n\n", new_edge_o);
    }
     */
    
    
    
    /* 
    print_separator();
    
    coords = create_coord_cube();
    
    
    
    new_coords = coords;
    new_cor_o = corner_orientantion_symmetries[new_coords.cor_o][new_coords.ud_sym_s];
    new_edge_o = edge_orientantion_symmetries[new_coords.ud_sym_c][new_coords.edge_o][new_coords.ud_sym_s];
    
    str = get_str_from_coord_cube(coords);
    printf_s("Solved cube state:\n%s", str);
    free(str);
    printf_s("new_cor_o = %i\nnew_edge_o = %i\n\n", new_cor_o, new_edge_o);
    printf_s("Pruning value: %i\n\n\n", get_pruning_huge_value(coords));
    
    
    new_coords = apply_fast_move_to_coord_cube(coords, move_R);
    new_cor_o = corner_orientantion_symmetries[new_coords.cor_o][new_coords.ud_sym_s];
    new_edge_o = edge_orientantion_symmetries[new_coords.ud_sym_c][new_coords.edge_o][new_coords.ud_sym_s];
    
    str = get_str_from_coord_cube(new_coords);
    printf_s("R move applied:\n%s", str);
    free(str);
    printf_s("new_cor_o = %i\nnew_edge_o = %i\n\n", new_cor_o, new_edge_o);
    printf_s("Pruning value: %i\n\n\n", get_pruning_huge_value(new_coords));
    
    
    new_coords = apply_fast_move_to_coord_cube(coords, move_F2);
    new_cor_o = corner_orientantion_symmetries[new_coords.cor_o][new_coords.ud_sym_s];
    new_edge_o = edge_orientantion_symmetries[new_coords.ud_sym_c][new_coords.edge_o][new_coords.ud_sym_s];
    
    str = get_str_from_coord_cube(new_coords);
    printf_s("F2 move applied:\n%s", str);
    free(str);
    printf_s("new_cor_o = %i\nnew_edge_o = %i\n\n", new_cor_o, new_edge_o);
    printf_s("Pruning value: %i\n\n\n", get_pruning_huge_value(new_coords));
    
    
    new_coords = apply_seq_to_coord_cube(coords, seq);
    new_cor_o = corner_orientantion_symmetries[new_coords.cor_o][new_coords.ud_sym_s];
    new_edge_o = edge_orientantion_symmetries[new_coords.ud_sym_c][new_coords.edge_o][new_coords.ud_sym_s];
    
    str = get_str_from_coord_cube(new_coords);
    printf_s("Seq applied:\n%s", str);
    free(str);
    printf_s("new_cor_o = %i\nnew_edge_o = %i\n\n", new_cor_o, new_edge_o);
    printf_s("Pruning value: %i\n\n\n", get_pruning_huge_value(new_coords));
     */
    
    
    print_separator();
    
    /* 
    int error = 0, huge_type;
    long long int huge_dim;
    long long int i=0, j=0;
    
    fast_move_t fast_m;
    
    char current_num_moves, next_num_moves;
    long long int num_computed_total, num_computed_loop, num_visited_total, num_visited_loop;
    long long int ud_slice, udsl_sym_c, udsl_sym_s, cor_o, edge_o;
    long long int new_ud_slice, new_udsl_sym_c, new_udsl_sym_s;
    //long long int new_cor_o, new_edge_o;
    
    
    edge_o = 550;
    cor_o = 1236;
    udsl_sym_c = 228;
    //udsl_sym_s = 0;
    
    ud_slice = ud_slice_sorted_sym_to_raw_table[udsl_sym_c][0];
    
    if(udsl_sym_c == 228 && cor_o == 1236 && edge_o == 550){
        printf_s("Patate\t");
    }
    
    
    for(fast_m=0; fast_m<LAST_FAST_MOVE; fast_m++){
        new_ud_slice = move_tbs.ud_slice[ud_slice*NUM_MOVES + fast_m];
        new_cor_o = move_tbs.cor_o[cor_o*NUM_MOVES + fast_m];
        new_edge_o = move_tbs.edge_o[edge_o*NUM_MOVES + fast_m];
        //new_edge_o = move_tbs.edge_o[edge_o*NUM_MOVES + move_F];
        
        new_udsl_sym_c = ud_slice_sorted_raw_to_sym_table[new_ud_slice][0];
        new_udsl_sym_s = ud_slice_sorted_raw_to_sym_table[new_ud_slice][1];
        
        new_cor_o = corner_orientantion_symmetries[new_cor_o][new_udsl_sym_s];
        new_edge_o = edge_orientantion_symmetries[new_udsl_sym_c][new_edge_o][new_udsl_sym_s];
        
        //new_edge_o = edge_orientantion_symmetries[new_udsl_sym_c][edge_o][new_udsl_sym_s];
        //new_edge_o = move_tbs.edge_o[new_edge_o*NUM_MOVES + fast_m];
        //new_edge_o = move_tbs.edge_o[new_edge_o*NUM_MOVES + move_F];
        
        // huge[UDSLICE_EQUI_N][CORNER_ORI_N][EDGE_ORI_N]
        j = (new_udsl_sym_c * CORNER_ORI_N + new_cor_o) * EDGE_ORI_N + new_edge_o;
        
        
        /* 
        if(prune_tbs.huge[j] < 0){
            prune_tbs.huge[j] = next_num_moves;
            num_computed_total++;
            num_computed_loop++;
        }
         * /
    }
     */
    
    
    //coords.cor_o
    //coords.edge_o
    
    
    
    
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
    
    free(prune_tbs.main_index);
    free(move_tbs.main_index);
    return 0;
}
































