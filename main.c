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
    cube_t cube = create_cube(3);
    
    
    scramble = argv[1];
    moves_seq_t seq = get_move_seq_from_str(cube.num_layers, scramble);
    str = get_str_from_move_seq(3, seq);
    
    printf_s("scramble = %s\n", scramble);
    printf_s("str      = %s\n", str);
    printf_s("%i\n", seq.len);
    
    
    print_separator();
    
    
    solution_t sol;
    
    timespec_get(&start, TIME_UTC);
    //sol = solve_cube(cube);
    timespec_get(&end, TIME_UTC);
    
    diff = compute_timespec_difference(end, start);
    printf_s("\nIt took %u.%09u seconds to solve the %ix%i.\n",
            diff.tv_sec, diff.tv_nsec, cube.num_layers, cube.num_layers
    );
    
    
    
    print_separator();
    
    printf_s("\nSize of cube3_t = %i\n", sizeof(cube3_t));
    printf_s("\nSize of corner_t = %i\n", sizeof(corner_t));
    printf_s("\nSize of edge_t = %i\n", sizeof(edge_t));
    printf_s("\nSize of face_t = %i\n", sizeof(face_t));
    printf_s("\nSize of move_t = %i\n", sizeof(move_t));
    
    print_separator();
    
    printf_s("\nCorners: ");
    for(corners_e corner=0; corner<CORNER_LAST; corner++){
        printf_s("%i ", corner);
    }
    printf_s("\nEdges: ");
    for(edges_e edge=0; edge<EDGE_LAST; edge++){
        printf_s("%i ", edge);
    }
    
    print_separator();
    
    cube3_t cube3;
    
    cube3 = create_cube3(cube);
    
    str = get_str_from_cube(cube);
    printf_s("%s\n", str);
    free(str);
    str = get_one_line_str_from_cube(cube);
    printf_s("cube  = %s\n", str);
    free(str);
    str = get_one_line_str_from_cube3(cube3, 0);
    printf_s("cube3 = %s\n", str);
    free(str);
    
    printf_s("\n\nApplying inverse sequence on cube.\n\n");
    apply_inverse_seq_to_cube(cube, seq);
    
    cube3 = create_cube3(cube);
    
    str = get_str_from_cube(cube);
    printf_s("%s\n", str);
    free(str);
    str = get_one_line_str_from_cube(cube);
    printf_s("cube  = %s\n", str);
    free(str);
    str = get_one_line_str_from_cube3(cube3, 0);
    printf_s("cube3 = %s\n", str);
    free(str);
    
    
    printf_s("\n\nApplying sequence on cube3.\n\n");
    
    cube3 = apply_seq_to_cube3(cube3, seq);
    
    str = get_one_line_str_from_cube3(cube3, 0);
    printf_s("cube3 = %s\n", str);
    free(str);
    str = get_one_line_str_from_cube3(cube3, 1);
    printf_s("cube3 = %s\n", str);
    free(str);
    
    
    printf_s("\n\nApplying sequence again on cube3.\n\n");
    
    cube3 = apply_seq_to_cube3(cube3, seq);
    
    str = get_one_line_str_from_cube3(cube3, 0);
    printf_s("cube3 = %s\n", str);
    free(str);
    str = get_one_line_str_from_cube3(cube3, 1);
    printf_s("cube3 = %s\n", str);
    free(str);
    
    
    printf_s("\n\nApplying inverse sequence on cube3.\n\n");
    
    cube3 = apply_inverse_seq_to_cube3(cube3, seq);
    
    str = get_one_line_str_from_cube3(cube3, 0);
    printf_s("cube3 = %s\n", str);
    free(str);
    str = get_one_line_str_from_cube3(cube3, 1);
    printf_s("cube3 = %s\n", str);
    free(str);
    
    
    print_separator();
    
    int repetition;
    
    if(argc <= 2){
        repetition = 1000000;
    }
    else{
        repetition = atoi(argv[2]);
    }
    
    stress_test_cube3(cube3, scramble, repetition);
    stress_test_cube(cube, scramble, repetition);
    
    
    
    
    
    
    /* 
    fast_move_t fast_m;
    move_t move, move_2;
    move.layers = 1;
    
    for(int i=0; i<6; i++){
        face_t face = faces[i];
        move.face = face;
        
        for(int rot=1; rot<=3; rot++){
            move.rotation = rot;
            
            fast_m = move.face*3 + move.rotation-1 - (move.face>exchange?3:0);
            move_2 = fast_to_move_table[fast_m];
            
            printf_s("%c -- %3i - %3i (%i)\n", get_char_from_facet(move.face), move.u_val, move_2.u_val, fast_m);
        }
    } */
    
    
    
    
    
    
    
    print_separator();
    print_separator();
    
    
    
    return 0;
}
































