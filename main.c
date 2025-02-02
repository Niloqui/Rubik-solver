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
#include "solver.h"

// Kociemba algorithm
// https://kociemba.org/cube.htm




int main(int argc, char **argv) {
    struct timespec start, end, diff;
    timespec_get(&start, TIME_UTC);
    
    //char scramble[] = "R U2 R' U2 R U2 L' U R' U' L";
    char *scramble, *str;
    cube_t cube = create_cube(3);
    
    
    if(cube.faces == NULL){
        printf("Error in memory allocation.\n");
        return -1;
    }
    
    scramble = argv[1];
    moves_seq_t seq = get_move_seq_from_str(cube.num_layers, scramble);
    
    str = get_one_line_str_from_cube(cube);
    printf_s("%s\n\n", str);
    free(str);
    str = get_str_from_cube(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    //int repetitions = atoi(argv[2]);
    //int cube_solved = 0;
    
    timespec_get(&end, TIME_UTC);
    
    diff = compute_timespec_difference(end, start);
    printf_s("It took %u.%09u seconds to set things up.\n", diff.tv_sec, diff.tv_nsec);
    
    //////////////////////////////
    print_separator();
    
    
    apply_moves_to_cube(cube, seq);
    
    str = get_one_line_str_from_cube(cube);
    printf_s("%s\n\n", str);
    free(str);
    str = get_str_from_cube(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    
    solution_t sol;
    
    timespec_get(&start, TIME_UTC);
    sol = solve_cube(cube);
    timespec_get(&end, TIME_UTC);
    
    diff = compute_timespec_difference(end, start);
    printf_s("\nIt took %u.%09u seconds to solve the %ix%i.\n",
            diff.tv_sec, diff.tv_nsec, cube.num_layers, cube.num_layers
    );
    
    printf_s("\n\n"
            "Scramble: %s (%2i moves)\n"
            "Solution: ", scramble, seq.len);
    
    str = get_str_from_move_seq(cube.num_layers, sol.initial_rotations);
    printf_s("%s ", str);
    free(str);
    str = get_str_from_move_seq(cube.num_layers, sol.solution);
    printf_s("%s (%2i moves)\n", str, sol.solution.len);
    free(str);
    
    printf_s("\n");
    str = get_one_line_str_from_cube(cube);
    printf_s("%s\n\n", str);
    free(str);
    str = get_str_from_cube(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    
    
    
    
    
    
    
    
    /*
    for(int i=1; i<=repetitions; i++){
        print_separator();
        
        apply_moves_to_cube(cube, seq);
        
        str = cube_to_one_line_string(cube);
        printf_s("%s\n\n", str);
        free(str);
        str = cube_to_formatted_string(cube);
        printf_s("%s\n\n", str);
        free(str);
        
        printf_s("We have applied \"%s\" %d time(s).\n", scramble, i);
        
        timespec_get(&start, TIME_UTC);
        cube_solved = is_cube_solved(cube);
        timespec_get(&end, TIME_UTC);
        
        if(cube_solved){
            printf_s("Cube is SOLVED!\n");
        }
        else{
            printf_s("Cube is not solved.\n");
        }
        diff = compute_timespec_difference(end, start);
        printf_s("It took %u.%09u seconds to check if the cube is solved.\n",
                diff.tv_sec, diff.tv_nsec);
    }
    
    print_separator();
    
    printf_s("\n");
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    stress_test(cube, scramble, atoi(argv[3]));
    
    printf_s("\n");
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    */
    
    
    /*
    char *str;
    cube_t cube = create_cube(3);
    
    if(cube.faces == NULL){
        printf("Error in memory allocation.\n");
        return -1;
    }
    
    printf_s("\n");
    
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    */
    
    
    /*
    cube.faces[R*cube.num_stickers_face + 0] = F;
    cube.faces[R*cube.num_stickers_face + 1] = U;
    cube.faces[R*cube.num_stickers_face + 2] = B;
    
    cube.faces[L*cube.num_stickers_face + 0] = B;
    cube.faces[L*cube.num_stickers_face + 1] = U;
    cube.faces[L*cube.num_stickers_face + 2] = F;
    
    str = cube_to_one_line_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    
    str = cube_to_formatted_string(cube);
    printf_s("%s\n\n", str);
    free(str);
    */
    
    /*
    for(int i=0; i<1; i++){
        printf_s("\n\nRotation = %i\n", rotations[i]);
        
        move.face = L;
        move.layers = 1;
        move.rotation = rotations[i];
        
        turn_layer(cube, move);
        
        str = cube_to_one_line_string(cube);
        printf_s("%s\n\n", str);
        free(str);
        
        str = cube_to_formatted_string(cube);
        printf_s("%s\n\n", str);
        free(str);
    }
    */
    
    /*
    face_t faces_rotation[] =   {R, R, U, R, R, U, R, R, U, R, R, F};
    unsigned char layers[] =    {1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 3};
    unsigned char rotations[] = {2, 2, 1, 2, 2, 2, 2, 2, 1, 2, 2, 1};
    //unsigned char rotations[] = {1, 3, 1, 2};
    //unsigned char layers[] =    {1, 2, 3, 1};
    move_t move;
    
    for(int i=0; i<12; i++){
        move.face = faces_rotation[i];
        move.layers = layers[i];
        move.rotation = rotations[i];
        
        turn_layer(cube, move);
    
        str = cube_to_one_line_string(cube);
        printf_s("%s\n\n", str);
        free(str);
        
        str = cube_to_formatted_string(cube);
        printf_s("%s\n\n", str);
        free(str);
    }
    */
    
    
    free(cube.faces);
    free(seq.moves);
    
    print_separator();
    print_separator();
    return 0;
}
































