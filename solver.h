#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// https://github.com/jean553/c-simd-avx2-example
#include <immintrin.h>

#include "face.h"
#include "move.h"
#include "cube.h"
#include "cube3.h"
#include "coord_cubes.h"




solution_t solve_2x2x2(cube_t cube){
    // This function assumes that cube.num_layers == 2
    int i, j, k;
    int solution_found;
    int max_depth;
    
    solution_t sol;
    sol.initial_rotations.len = 0;
    sol.initial_rotations.moves = NULL;
    sol.solution.len = 0;
    sol.solution.moves = NULL;
    
    /* First of all we need to rotate the cube in such a way that
     * the back bottom left corner is solved.
     *    UU              XX
     *    UU              XX
     *  LLFFRRBB        XXXXXXXX
     *  LLFFRRBB        LXXXXXXB
     *    DD              XX
     *    DD              DX
    */
    sol.initial_rotations.moves = (move_t *)malloc(sizeof(move_t) * 3);
    if(sol.initial_rotations.moves == NULL){
        printf_s("Error in memory allocation: sol.initial_rotations.moves\n");
        return sol;
    }
    sol.initial_rotations.len = 3;
    
    for(i=0; i<3; i++){
        sol.initial_rotations.moves[i].face = faces[i];
        sol.initial_rotations.moves[i].layers = 2;
        sol.initial_rotations.moves[i].rotation = 0;
    }
    
    move_t x, y, z;
    x.face = R;
    y.face = U;
    z.face = F;
    x.rotation = y.rotation = z.rotation = 1;
    x.layers = y.layers = z.layers = 2;
    
    solution_found = 0;
    for(i=0; i<4 && !solution_found; i++){
        sol.initial_rotations.moves[0].rotation = i;
        
        for(j=0; j<4 && !solution_found; j++){
            sol.initial_rotations.moves[1].rotation = j;
            
            for(k=0; k<4 && !solution_found; k++){
                sol.initial_rotations.moves[2].rotation = k;
                
                solution_found = cube.faces[L*cube.num_stickers_face + 2] == L &&
                                 cube.faces[B*cube.num_stickers_face + 3] == B &&
                                 cube.faces[D*cube.num_stickers_face + 2] == D;
                
                if(!solution_found){
                    apply_move_to_cube(cube, z);
                }
            }
            
            if(!solution_found){
                apply_move_to_cube(cube, y);
            }
        }
        
        if(!solution_found){
            apply_move_to_cube(cube, x);
        }
    }
    
    if(!solution_found){
        printf_s("Error: it was not possible to bring the back bottom left corner "
                "into its solved position during the first phase of the algorithm.\n");
        free(sol.initial_rotations.moves);
        sol.initial_rotations.moves = NULL;
        sol.initial_rotations.len = 0;
        return sol;
    }
    
    /* The main part of the algorithm is a recursion function
     * TO-DO: create a table with all the cube states and use that to obtain
     * the solution.
    */
    sol.solution.moves = (move_t *)malloc(sizeof(move_t) * 11); // 11 is the 2x2 God's Number
    if(sol.solution.moves == NULL){
        printf_s("Error in memory allocation: sol.initial_rotations.moves\n");
        return sol;
    }
    //sol.solution.len = 11;
    
    /*
    for(i=0; i<11; i++){
        sol.solution.moves[i].face = D;
        sol.solution.moves[i].layers = 7;
        sol.solution.moves[i].rotation = 0;
    }*/
    
    max_depth = 0;
    
    if(is_cube_solved(cube)){
        solution_found = 1;
    }
    else{
        solution_found = 0;
    }
    
    while(!solution_found){
        max_depth++;
        printf_s("Searching solution at depth %i.\n", max_depth);
        
        //solution_found = solve_2x2x2_recursion(cube, &sol.solution,
        //                                    get_axis_from_face(empty), max_depth, 0);
        solution_found = 1;
    }
    sol.solution.len = max_depth;
    
    
    
    return sol;
}

int solve_3x3x3_recursion(coord_cube_t coord_cube, fast_move_t *fast_ms, face_t last_face, int max_depth, int current_depth){
    // End of recursion
    if(current_depth == max_depth){
        return is_coord_cube_solved(coord_cube);
    }
    
    // Pruning --- TO-DO
    
    
    
    // Recursion
    coord_cube_t new_coords;
    int solution_found = 0;
    int last_axis = get_axis_from_face(last_face);
    
    for(fast_move_t fast_m=0; fast_m<LAST_FAST_MOVE && !solution_found; fast_m++){
        face_t face = fast_to_move_table[fast_m].face;
        int axis = get_axis_from_face(face);
        
        if(axis == last_axis && face > exchange){
            continue;
        }
        
        fast_ms[current_depth] = fast_m;
        
        new_coords = apply_fast_move_to_coord_cube(coord_cube, fast_m);
        solution_found = solve_3x3x3_recursion(new_coords, fast_ms, face, max_depth, current_depth+1);
    }
    
    return solution_found;
}





solution_t solve_3x3x3(cube_t cube){
    // This function assumes that cube.num_layers == 3
    int i, j, k;
    int solution_found;
    int max_depth;
    
    solution_t sol;
    sol.initial_rotations.len = 0;
    sol.initial_rotations.moves = NULL;
    sol.solution.len = 0;
    sol.solution.moves = NULL;
    
    /* First of all we need to rotate the cube in such a way that
     * the U face is in U and the F face is in F.
     *     UUU                 XXX
     *     UUU                 XUX
     *     UUU                 XXX
     *  LLLFFFRRRBBB        XXXXXXXXXXXX
     *  LLLFFFRRRBBB        XLXXFXXRXXBX
     *  LLLFFFRRRBBB        XXXXXXXXXXXX
     *     DDD                 XXX
     *     DDD                 XDX
     *     DDD                 XXX
    */
    sol.initial_rotations.moves = (move_t *)malloc(sizeof(move_t) * 2);
    
    if(sol.initial_rotations.moves == NULL){
        printf_s("Error in memory allocation: sol.initial_rotations.moves\n");
        return sol;
    }
    sol.solution.moves = sol.initial_rotations.moves + 3;
    //sol.initial_rotations.len = 2;
    
    for(i=0; i<2; i++){
        sol.initial_rotations.moves[i].face = faces[i];
        sol.initial_rotations.moves[i].layers = 3;
        sol.initial_rotations.moves[i].rotation = 0;
    }
    
    move_t move;
    i = 0;
    
    // First step is moving U in U
    if(cube.faces[U*cube.num_stickers_face + 4] != U){
        // Do something only if U is not already on top
        move.layers = 3;
        
        if(cube.faces[R*cube.num_stickers_face + 4] == U){ // z'
            move.face = F;
            move.rotation = 3;
        }
        else if(cube.faces[L*cube.num_stickers_face + 4] == U){ // z
            move.face = F;
            move.rotation = 1;
        }
        else if(cube.faces[F*cube.num_stickers_face + 4] == U){ // x
            move.face = R;
            move.rotation = 1;
        }
        else if(cube.faces[B*cube.num_stickers_face + 4] == U){ // x'
            move.face = R;
            move.rotation = 3;
        }
        else if(cube.faces[D*cube.num_stickers_face + 4] == U){
            if(cube.faces[F*cube.num_stickers_face + 4] == F){ // z2
                move.face = F;
                move.rotation = 2;
            }
            else{ // x2
                move.face = R;
                move.rotation = 2;
            }
        }
        
        sol.initial_rotations.moves[i] = move;
        apply_move_to_cube(cube, move);
        i++;
    }
    
    // Second step is moving F in F
    if(cube.faces[F*cube.num_stickers_face + 4] != F){
        // Do something only if U is not already on top
        move.layers = 3;
        move.face = U;
        
        if(cube.faces[R*cube.num_stickers_face + 4] == F){ // y
            move.rotation = 1;
        }
        else if(cube.faces[B*cube.num_stickers_face + 4] == F){ // y2
            move.rotation = 2;
        }
        //else if(cube.faces[L*cube.num_stickers_face + 4] == F){ // y'
        else{ // y'
            move.rotation = 3;
        }
        
        sol.initial_rotations.moves[i] = move;
        apply_move_to_cube(cube, move);
        i++;
    }
    sol.initial_rotations.len = i;
    
    /* This is the main part of the solving algorithm
    */
    sol.solution.moves = (move_t *)malloc(sizeof(move_t) * GOD_N_3X3); // 20 is the 3x3 God's Number
    if(sol.solution.moves == NULL){
        printf_s("Error in memory allocation: sol.initial_rotations.moves\n");
        return sol;
    }
    //sol.solution.len = 11;
    
    for(i=0; i<GOD_N_3X3; i++){
        sol.solution.moves[i].face = D;
        sol.solution.moves[i].layers = 7;
        sol.solution.moves[i].rotation = 0;
    }
    
    max_depth = 0;
    
    if(is_cube_solved(cube)){
        solution_found = 1;
    }
    else{
        solution_found = 0;
    }
    
    cube3_t cube3 = create_cube3_from_cube(cube);
    coord_cube_t coord_cube = create_coord_cube_from_cube3(cube3);
    
    fast_move_t *fast_ms = malloc(sizeof(fast_move_t) * 20);
    if(fast_ms == NULL){
        printf_s("Error in memory allocation: fast_ms array\n");
        return sol;
    }
    for(i=0; i<20; i++){
        fast_ms[i] = LAST_FAST_MOVE;
    }
    
    while(!solution_found){
        max_depth++;
        printf_s("Searching solution at depth %i.\n", max_depth);
        
        solution_found = solve_3x3x3_recursion(coord_cube, fast_ms, empty, max_depth, 0);
    }
    
    sol.solution.len = max_depth;
    for(i=0; i<max_depth; i++){
        sol.solution.moves[i] = fast_to_move_table[fast_ms[i]];
    }
    
    return sol;
}


solution_t solve_cube(cube_t cube){
    solution_t sol;
    sol.initial_rotations.len = 0;
    sol.initial_rotations.moves = NULL;
    sol.solution.len = 0;
    sol.solution.moves = NULL;
    
    if(cube.num_layers < 2){
        printf_s("Cube is too small / Error in the data.\n");
    }
    else if(cube.num_layers >= 4){
        printf_s("Cube is too big (%i number of layers), no solver implemented yet.\n",
                cube.num_layers);
    }
    else if(cube.num_layers == 2){
        sol = solve_2x2x2(cube);
    }
    else if(cube.num_layers == 3){
        sol = solve_3x3x3(cube);
    }
    
    return sol;
}








