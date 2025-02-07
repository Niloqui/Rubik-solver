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



solution_t solve_2x2x2(cube_t cube){
    solution_t sol;
    sol.initial_rotations.len = 0;
    sol.initial_rotations.moves = NULL;
    sol.solution.len = 0;
    sol.solution.moves = NULL;
    
    
    // TO-DO
    printf_s("No solver has been implemented yet.\n");
    
    return sol;
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
        turn_layer(cube, move);
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
        turn_layer(cube, move);
        i++;
    }
    sol.initial_rotations.len = i;
    
    /* This is the main part of the solving algorithm
    */
    sol.solution.moves = (move_t *)malloc(sizeof(move_t) * 20); // 20 is the 3x3 God's Number
    if(sol.solution.moves == NULL){
        printf_s("Error in memory allocation: sol.initial_rotations.moves\n");
        return sol;
    }
    //sol.solution.len = 11;
    
    for(i=0; i<20; i++){
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
    
    while(!solution_found){
        max_depth++;
        printf_s("Searching solution at depth %i.\n", max_depth);
        
        //solution_found = solve_3x3x3_recursion(cube, &sol.solution, empty, max_depth, 0);
        solution_found = 1;
    }
    sol.solution.len = max_depth;
    
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








