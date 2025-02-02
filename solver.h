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



int solve_2x2x2_recursion(cube_t cube, moves_seq_t *seq, int previous_axis,
                        int max_depth, int current_depth){
    int solution_found = 0;
    
    if(current_depth == max_depth){
        solution_found = is_cube_solved(cube);
    }
    else{
        move_t move_base;
        move_base.layers = 1;
        move_base.rotation = 1;
        
        for(int i=0; i<6 && !solution_found; i++){
            face_t face = faces[i];
            if(get_axis_from_face(face) == previous_axis){
                continue;
            }
            
            move_base.face = face;
            seq->moves[current_depth] = move_base;
            
            for(int rot=1; rot<4 && !solution_found; rot++){
                seq->moves[current_depth].rotation = rot;
                turn_layer_2x2x2_opt(cube, move_base);
                //turn_layer(cube, move_base);
                
                solution_found = solve_2x2x2_recursion(cube, seq, get_axis_from_face(face),
                                                            max_depth, current_depth+1);
            }
            if(!solution_found){
                turn_layer_2x2x2_opt(cube, move_base);
                //turn_layer(cube, move_base);
            }
        }
    }
    
    return solution_found;
}

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
                    turn_layer(cube, z);
                }
            }
            
            if(!solution_found){
                turn_layer(cube, y);
            }
        }
        
        if(!solution_found){
            turn_layer(cube, x);
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
        solution_found = solve_2x2x2_recursion(cube, &sol.solution,
                                            get_axis_from_face(empty), max_depth, 0);
    }
    sol.solution.len = max_depth;
    
    
    
    return sol;
}



int solve_3x3x3_recursion(cube_t cube, moves_seq_t *seq, face_t precious_face,
                        int max_depth, int current_depth){
    int solution_found = 0;
    
    if(current_depth == max_depth){
        solution_found = is_cube_solved(cube);
    }
    else{
        move_t move_base;
        move_base.layers = 1;
        move_base.rotation = 1;
        
        for(int i=0; i<6 && !solution_found; i++){
            face_t face = faces[i];
            
            if(face == precious_face){
                continue;
            }
            if(get_axis_from_face(face) == get_axis_from_face(precious_face) && face < exchange){
                continue;
            }
            
            move_base.face = face;
            seq->moves[current_depth] = move_base;
            
            for(int rot=1; rot<4 && !solution_found; rot++){
                seq->moves[current_depth].rotation = rot;
                turn_layer_3x3x3_opt(cube, move_base);
                //turn_layer(cube, move_base);
                
                solution_found = solve_3x3x3_recursion(cube, seq, face,
                                                        max_depth, current_depth+1);
            }
            if(!solution_found){
                turn_layer_3x3x3_opt(cube, move_base);
                //turn_layer(cube, move_base);
            }
        }
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
    
    /* The main part of the algorithm is a recursion function
     * TO-DO: create a table with some cube states and use that to help find
     * the solution.
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
        
        solution_found = solve_3x3x3_recursion(cube, &sol.solution, empty, max_depth, 0);
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








