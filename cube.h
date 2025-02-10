#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// https://github.com/jean553/c-simd-avx2-example
#include <immintrin.h>


#include "face.h"
#include "move.h"



typedef struct {
    int num_layers;
    int num_stickers_face;
    int faces_len;
    face_t *faces;
} cube_t;

cube_t create_cube(int num_layers){
    cube_t cube;
    cube.num_layers = num_layers;
    cube.num_stickers_face = num_layers*num_layers;
    cube.faces_len = cube.num_stickers_face * (6+1); // 6 faces plus 1 to apply moves
    
    cube.faces = (face_t*)malloc(cube.faces_len * sizeof(face_t));
    if(cube.faces == NULL){
        printf_s("Error in memory allocation.\n");
        return cube;
    }
    
    for(int i=0; i<7; i++){
        //char face_val = get_faceget_face(i);
        
        for(int j=0; j<cube.num_stickers_face; j++){
            cube.faces[i*cube.num_stickers_face + j] = (face_t)i; // face_val
        }
    }
    
    return cube;
}

char* get_str_from_cube(const cube_t cube){
    char *str;
    face_t layers[] = {empty, U, empty, empty,
                           L, F, R, B,
                       empty, D, empty, empty};
    
    str = (char*)malloc((cube.num_stickers_face*12 + cube.num_layers*3 + 1) * sizeof(char));
    if(str == NULL){
        printf_s("Error in memory allocation.\n");
        return str; // return NULL
    }
    
    int ins=0;
    for(int layer=0; layer<3; layer++){
        for(int row=0; row<cube.num_layers; row++){
            for(int face=0; face<4; face++){
                face_t current_face = layers[layer*4 + face];
                
                for(int col=0; col<cube.num_layers; col++){
                    if(current_face == empty){
                        str[ins] = ' ';
                    }
                    else{
                        str[ins] = get_char_from_facet(
                            cube.faces[current_face*cube.num_stickers_face + row*cube.num_layers + col]
                        );
                    }
                    
                    ins++;
                }
            }
            
            str[ins] = '\n';
            ins++;
        }
    }
    str[ins] = '\0';
    
    return str;
}

char* get_one_line_str_from_cube(const cube_t cube){
    char *str;
    
    str = (char*)malloc(cube.num_stickers_face*6 + 1); // last character must be '\0'
    if(str == NULL){
        printf_s("Error in memory allocation.\n");
        return str; // return NULL
    }
    
    face_t val;
    int ins=0;
    for(int i=0; i<cube.faces_len; i++){
        val = cube.faces[i];
        
        if(val != empty && val != exchange){
            str[ins] = get_char_from_facet(val);
            ins++;
        }
    }
    str[ins] = '\0';
    
    return str;
}


void rotate_single_face(cube_t cube, const move_t move){
    face_t tmp;
    face_t *face_matrix;
    int n_i, n_j;
    int n=cube.num_layers;
    
    face_t face = move.face;
    unsigned char rotation = move.rotation;
    
    face_matrix = cube.faces + face*cube.num_stickers_face;
    rotation = rotation % 4;
    
    switch (rotation){
    case 1:
        n_i = cube.num_layers / 2;
        n_j = (cube.num_layers + 1) / 2;
        
        for(int i=0; i<n_i; i++){
            for(int j=0; j<n_j; j++){
                tmp = face_matrix[(i)*n + (j)];
                face_matrix[(i)*n + (j)] = face_matrix[(n-1-j)*n + (i)];
                face_matrix[(n-1-j)*n + (i)] = face_matrix[(n-1-i)*n + (n-1-j)];
                face_matrix[(n-1-i)*n + (n-1-j)] = face_matrix[(j)*n + (n-1-i)];
                face_matrix[(j)*n + (n-1-i)] = tmp; // face_matrix[(i)*n + (j)];
            }
        }
        break;
    case 2:
        for(int i=0, j=cube.num_stickers_face-1; i<j; i++, j--){
            tmp = face_matrix[i];            
            face_matrix[i] = face_matrix[j];
            face_matrix[j] = tmp;
        }
        break;
    case 3:
        n_i = cube.num_layers / 2;
        n_j = (cube.num_layers + 1) / 2;
        
        for(int i=0; i<n_i; i++){
            for(int j=0; j<n_j; j++){
                tmp = face_matrix[(i)*n + (j)];
                face_matrix[(i)*n + (j)] = face_matrix[(j)*n + (n-1-i)];
                face_matrix[(j)*n + (n-1-i)] = face_matrix[(n-1-i)*n + (n-1-j)];
                face_matrix[(n-1-i)*n + (n-1-j)] = face_matrix[(n-1-j)*n + (i)];
                face_matrix[(n-1-j)*n + (i)] = tmp; // face_matrix[(i)*n + (j)];
            }
        }
        break;
    case 0: // No rotation
        break;
    }
}

const face_t turn_side_order[7][6] = { //   0, 1, 2, 3, 4, 5,
                            /* R  */ exchange, F, U, B, D, exchange,
                            /* U  */ exchange, F, L, B, R, exchange,
                            /* F  */ exchange, U, R, D, L, exchange,
                            /* ex */ exchange, exchange, exchange, exchange, exchange, exchange,
                            /* L  */ exchange, F, D, B, U, exchange,
                            /* D  */ exchange, F, R, B, L, exchange,
                            /* B  */ exchange, U, L, D, R, exchange,
};
#define tso turn_side_order

/* RRR    000    RR1    RRR    3RR
 * RRR    RRR    RR1    RRR    3RR
 * RRR    RRR    RR1    222    3RR
*/
const unsigned char turn_side_table[7][7] = { //  R,  U,  F, ex,  L,  D,  B,
                                        /* R  */ -1,  1,  1,  0, -1,  1,  3,
                                        /* U  */  0, -1,  0,  0,  0, -1,  0,
                                        /* F  */  3,  2, -1,  0,  1,  0, -1,
                                        /* ex */ -1, -1, -1, -1, -1, -1, -1,
                                        /* L  */ -1,  3,  3,  0, -1,  3,  1,
                                        /* D  */  2, -1,  2,  0,  2, -1,  2,
                                        /* B  */  1,  0, -1,  0,  3,  2, -1,
};
#define tst turn_side_table

const int turn_side_index_modifier[][6] = { //    i↕, i↔, n_i,  j↕, j↔, n_j,
                                        /* 0 */    1,  0,   0,   0,  1,   0,
                                        /* 1 */    0, -1,   1,   1,  0,   0,
                                        /* 2 */   -1,  0,   1,   0, -1,   1,
                                        /* 3 */    0,  1,   0,  -1,  0,   1,
};
#define tsim turn_side_index_modifier

void copy_face_section_to_another_face(cube_t cube, move_t move, 
                                    face_t source_face, face_t dest_face,
                                    int turn_side_index_source, int turn_side_index_dest){
    int i, j;
    int n = cube.num_layers;
    
    // You cannot turn non-existing layers, 
    int layers = move.layers<cube.num_layers?move.layers:cube.num_layers;
    
    face_t *source_face_p, *dest_face_p;
    source_face_p = cube.faces + source_face*cube.num_stickers_face;
    dest_face_p = cube.faces + dest_face*cube.num_stickers_face;
    
    int si_rm, si_cm, sni_m, sj_rm, sj_cm, snj_m;
    int di_rm, di_cm, dni_m, dj_rm, dj_cm, dnj_m;
    { // Initialize the variables above
        si_rm = turn_side_index_modifier[turn_side_index_source][0];
        si_cm = turn_side_index_modifier[turn_side_index_source][1];
        sni_m = turn_side_index_modifier[turn_side_index_source][2];
        
        sj_rm = turn_side_index_modifier[turn_side_index_source][3];
        sj_cm = turn_side_index_modifier[turn_side_index_source][4];
        snj_m = turn_side_index_modifier[turn_side_index_source][5];
        
        di_rm = turn_side_index_modifier[turn_side_index_dest][0];
        di_cm = turn_side_index_modifier[turn_side_index_dest][1];
        dni_m = turn_side_index_modifier[turn_side_index_dest][2];
        
        dj_rm = turn_side_index_modifier[turn_side_index_dest][3];
        dj_cm = turn_side_index_modifier[turn_side_index_dest][4];
        dnj_m = turn_side_index_modifier[turn_side_index_dest][5];
    }
    
    int sc, sr, dc, dr;
    //sc = sr = dc = dr = -666;
    
    //printf_s("si_rm = %i\t si_cm = %i\t sni_m = %i\n", si_rm, si_cm, sni_m);
    //printf_s("sj_rm = %i\t sj_cm = %i\t snj_m = %i\n\n\n", sj_rm, sj_cm, snj_m);
    
    for(i=0; i<layers; i++){
        for(j=0; j<n; j++){
            // These values adjust the matrix indexes
            sr = si_rm*(i -(n-1)*sni_m) + sj_rm*(j -(n-1)*snj_m);
            sc = si_cm*(i -(n-1)*sni_m) + sj_cm*(j -(n-1)*snj_m);
            
            dr = di_rm*(i -(n-1)*dni_m) + dj_rm*(j -(n-1)*dnj_m);
            dc = di_cm*(i -(n-1)*dni_m) + dj_cm*(j -(n-1)*dnj_m);
            
            dest_face_p[dr*cube.num_layers + dc] = source_face_p[sr*cube.num_layers + sc];
        }
    }
}

void turn_face_side(cube_t cube, const move_t move){
    int index_mod_source, index_mod_dest;
    face_t source_face, dest_face;
    
    face_t face = move.face;
    unsigned char rotation = move.rotation;
    
    int case3 = 0; // false
    switch (rotation){
    case 3:
        case3 = 2; // true
    case 1:
        for(int i=(case3?1:4); i>=0 && i<6; i=i-1+case3){
            source_face = turn_side_order[move.face][i];
            dest_face = turn_side_order[move.face][i+1-case3];
            
            index_mod_source = turn_side_table[face][source_face];
            index_mod_dest = turn_side_table[face][dest_face];
            
            copy_face_section_to_another_face(cube, move, source_face, dest_face,
                                    index_mod_source, index_mod_dest);
        }
        break;
    case 2:
        const face_t turn_side_order_opposite[2][4] = {
            exchange, turn_side_order[move.face][1], turn_side_order[move.face][3], exchange,
            exchange, turn_side_order[move.face][2], turn_side_order[move.face][4], exchange,
        };
        
        for(int i=0; i<2; i++){
            for(int j=0; j<3; j++){
                source_face = turn_side_order_opposite[i][j+1];
                dest_face = turn_side_order_opposite[i][j];
                
                index_mod_source = turn_side_table[face][source_face];
                index_mod_dest = turn_side_table[face][dest_face];
                
                copy_face_section_to_another_face(cube, move, source_face, dest_face,
                                        index_mod_source, index_mod_dest);
            }
        }
        break;;
    case 0: // No rotation
        break;
    }
    
    // Assign the exchange value to the exchange face
    for(int i=0; i<cube.num_stickers_face; i++){
        cube.faces[exchange*cube.num_stickers_face + i] = exchange;
    }
    
}

void turn_layer(cube_t cube, const move_t move){
    turn_face_side(cube, move);
    rotate_single_face(cube, move);
    
    if(move.layers == cube.num_layers){
        move_t m = move;
        m.face = (m.face + 4) % 8;
        m.rotation = (m.rotation * 3) % 4;
        
        rotate_single_face(cube, m);
    }
}


void turn_layer_3x3x3_opt(cube_t cube, const move_t move){
    // This function assumes that the cube is a 3x3x3 and that the rotation
    // is 90° clockwise
    
    //turn_face_side(cube, move);
    face_t face = move.face, tmp;
    face_t f1, f2, f3, f4;
    int *f1im, *f2im, *f3im, *f4im;
    
    int n = cube.num_layers;
    int one_m_n = 1 - cube.num_layers;
    int n2 = cube.num_stickers_face;
    
    f1 = tso[face][1];
    f2 = tso[face][2];
    f3 = tso[face][3];
    f4 = tso[face][4];
    
    // warning: assignment discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
    #pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    f1im = tsim[tst[face][f1]];
    f2im = tsim[tst[face][f2]];
    f3im = tsim[tst[face][f3]];
    f4im = tsim[tst[face][f4]];
    #pragma GCC diagnostic warning "-Wdiscarded-qualifiers"
    
    // i↕, i↔, n_i,  j↕, j↔, n_j,
    //  0,  1,   2,   3,  4,   5
    
    /*
    sr = si_rm*(i -(n-1)*sni_m) + sj_rm*(j -(n-1)*snj_m);
    sc = si_cm*(i -(n-1)*sni_m) + sj_cm*(j -(n-1)*snj_m);
    
    dr = di_rm*(1-n)*dni_m + dj_rm*(j +(1-n)*dnj_m);
    dc = di_cm*(1-n)*dni_m + dj_cm*(j +(1-n)*dnj_m);
    */
    
    __m256i row_col_m_v = _mm256_set_epi32(n, 1, n, 1, n, 1, n, 1);
    __m256i one_m_n_v = _mm256_set_epi32(one_m_n, one_m_n, one_m_n, one_m_n,
                                         one_m_n, one_m_n, one_m_n, one_m_n);
    
    __m256i i_rcm_v = _mm256_set_epi32(f1im[0], f1im[1], f2im[0], f2im[1],
                                       f3im[0], f3im[1], f4im[0], f4im[1]);
    __m256i ni_m_v = _mm256_set_epi32(f1im[2], f1im[2], f2im[2], f2im[2],
                                      f3im[2], f3im[2], f4im[2], f4im[2]);
    
    __m256i j_rcm_v = _mm256_set_epi32(f1im[3], f1im[4], f2im[3], f2im[4],
                                       f3im[3], f3im[4], f4im[3], f4im[4]);
    __m256i nj_m_v = _mm256_set_epi32(f1im[5], f1im[5], f2im[5], f2im[5],
                                      f3im[5], f3im[5], f4im[5], f4im[5]);
    
    __m256i first_part_v = _mm256_mullo_epi32(i_rcm_v, _mm256_mullo_epi32(one_m_n_v, ni_m_v));
    __m256i second_part_pre_v = _mm256_mullo_epi32(one_m_n_v, nj_m_v);
    __m256i second_part_middle_v, second_part_v, indexes_v;
    int* indexes = (int*)&indexes_v;
    
    __m256i face_v = _mm256_set_epi32(f1, f1, f2, f2, f3, f3, f4, f4);
    __m256i n2_v = _mm256_set_epi32(0, n2, 0, n2, 0, n2, 0, n2);
    __m256i face_m_v = _mm256_mullo_epi32(face_v, n2_v);
    
    
    for(int j=0; j<n; j++){
        __m256i j_v = _mm256_set_epi32(j, j, j, j, j, j, j, j);
        
        second_part_middle_v = _mm256_add_epi32(j_v, second_part_pre_v);
        second_part_v = _mm256_mullo_epi32(j_rcm_v, second_part_middle_v);
        
        indexes_v = _mm256_add_epi32(first_part_v, second_part_v);
        indexes_v = _mm256_mullo_epi32(row_col_m_v, indexes_v);
        indexes_v = _mm256_add_epi32(face_m_v, indexes_v);
        
        for(int i=0; i<8; i+=2){
            indexes[i] = indexes[i] + indexes[i+1];
        }
        
        tmp = cube.faces[indexes[0]];
        cube.faces[indexes[0]] = cube.faces[indexes[2]];
        cube.faces[indexes[2]] = cube.faces[indexes[4]];
        cube.faces[indexes[4]] = cube.faces[indexes[6]];
        cube.faces[indexes[6]] = tmp;
    }
    
    
    //rotate_single_face(cube, move);
    face_t *face_matrix = cube.faces + face*cube.num_stickers_face;
    face_t tmp2;
    
    tmp = face_matrix[0];
    tmp2 = face_matrix[1];
    
    face_matrix[0] = face_matrix[6];
    face_matrix[1] = face_matrix[3];
    
    face_matrix[6] = face_matrix[8];
    face_matrix[3] = face_matrix[7];
    
    face_matrix[8] = face_matrix[2];
    face_matrix[7] = face_matrix[5];
    
    face_matrix[2] = tmp;
    face_matrix[5] = tmp2;
}

void turn_layer_2x2x2_opt(cube_t cube, const move_t move){
    // This function assumes that the cube is a 2x2x2 and that the rotation
    // is 90° clockwise
    
    //turn_face_side(cube, move);
    face_t face = move.face, tmp;
    face_t f1, f2, f3, f4;
    int *f1im, *f2im, *f3im, *f4im;
    
    int n = cube.num_layers;
    int one_m_n = 1 - cube.num_layers;
    int n2 = cube.num_stickers_face;
    
    f1 = tso[face][1];
    f2 = tso[face][2];
    f3 = tso[face][3];
    f4 = tso[face][4];
    
    // warning: assignment discards 'const' qualifier from pointer target type [-Wdiscarded-qualifiers]
    #pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    f1im = tsim[tst[face][f1]];
    f2im = tsim[tst[face][f2]];
    f3im = tsim[tst[face][f3]];
    f4im = tsim[tst[face][f4]];
    #pragma GCC diagnostic warning "-Wdiscarded-qualifiers"
    
    // i↕, i↔, n_i,  j↕, j↔, n_j,
    //  0,  1,   2,   3,  4,   5
    
    /*
    sr = si_rm*(i -(n-1)*sni_m) + sj_rm*(j -(n-1)*snj_m);
    sc = si_cm*(i -(n-1)*sni_m) + sj_cm*(j -(n-1)*snj_m);
    
    dr = di_rm*(1-n)*dni_m + dj_rm*(j +(1-n)*dnj_m);
    dc = di_cm*(1-n)*dni_m + dj_cm*(j +(1-n)*dnj_m);
    */
    
    __m256i row_col_m_v = _mm256_set_epi32(n, 1, n, 1, n, 1, n, 1);
    __m256i one_m_n_v = _mm256_set_epi32(one_m_n, one_m_n, one_m_n, one_m_n,
                                         one_m_n, one_m_n, one_m_n, one_m_n);
    
    __m256i i_rcm_v = _mm256_set_epi32(f1im[0], f1im[1], f2im[0], f2im[1],
                                       f3im[0], f3im[1], f4im[0], f4im[1]);
    __m256i ni_m_v = _mm256_set_epi32(f1im[2], f1im[2], f2im[2], f2im[2],
                                      f3im[2], f3im[2], f4im[2], f4im[2]);
    
    __m256i j_rcm_v = _mm256_set_epi32(f1im[3], f1im[4], f2im[3], f2im[4],
                                       f3im[3], f3im[4], f4im[3], f4im[4]);
    __m256i nj_m_v = _mm256_set_epi32(f1im[5], f1im[5], f2im[5], f2im[5],
                                      f3im[5], f3im[5], f4im[5], f4im[5]);
    
    __m256i first_part_v = _mm256_mullo_epi32(i_rcm_v, _mm256_mullo_epi32(one_m_n_v, ni_m_v));
    __m256i second_part_pre_v = _mm256_mullo_epi32(one_m_n_v, nj_m_v);
    __m256i second_part_middle_v, second_part_v, indexes_v;
    int* indexes = (int*)&indexes_v;
    
    __m256i face_v = _mm256_set_epi32(f1, f1, f2, f2, f3, f3, f4, f4);
    __m256i n2_v = _mm256_set_epi32(0, n2, 0, n2, 0, n2, 0, n2);
    __m256i face_m_v = _mm256_mullo_epi32(face_v, n2_v);
    
    
    for(int j=0; j<n; j++){
        __m256i j_v = _mm256_set_epi32(j, j, j, j, j, j, j, j);
        
        second_part_middle_v = _mm256_add_epi32(j_v, second_part_pre_v);
        second_part_v = _mm256_mullo_epi32(j_rcm_v, second_part_middle_v);
        
        indexes_v = _mm256_add_epi32(first_part_v, second_part_v);
        indexes_v = _mm256_mullo_epi32(row_col_m_v, indexes_v);
        indexes_v = _mm256_add_epi32(face_m_v, indexes_v);
        
        for(int i=0; i<8; i+=2){
            indexes[i] = indexes[i] + indexes[i+1];
        }
        
        tmp = cube.faces[indexes[0]];
        cube.faces[indexes[0]] = cube.faces[indexes[2]];
        cube.faces[indexes[2]] = cube.faces[indexes[4]];
        cube.faces[indexes[4]] = cube.faces[indexes[6]];
        cube.faces[indexes[6]] = tmp;
    }
    
    
    //rotate_single_face(cube, move);
    face_t *face_matrix = cube.faces + face*cube.num_stickers_face;
    
    tmp = face_matrix[0];
    face_matrix[0] = face_matrix[2];
    face_matrix[2] = face_matrix[3];
    face_matrix[3] = face_matrix[1];
    face_matrix[1] = tmp;
}


void apply_seq_to_cube(cube_t cube, const moves_seq_t seq){
    for(int i=0; i<seq.len; i++){
        turn_layer(cube, seq.moves[i]);
    }
}

void apply_inverse_seq_to_cube(cube_t cube, const moves_seq_t seq){
    for(int i=seq.len-1; i>=0; i--){
        move_t move = seq.moves[i];
        move.rotation = (move.rotation * 3) % 4;
        turn_layer(cube, move);
    }
}


void stress_test_cube(cube_t cube, const char *str, int repetition){
    struct timespec start, end, diff;
    
    moves_seq_t seq = get_move_seq_from_str(cube.num_layers, str);
    
    timespec_get(&start, TIME_UTC);
    for(int i=0; i<repetition; i++){
        apply_seq_to_cube(cube, seq);
    }
    timespec_get(&end, TIME_UTC);
    
    diff = compute_timespec_difference(end, start);
    printf_s("\nIt took %u.%09u seconds to apply \"%s\" %d times (cube_t).\n",
            diff.tv_sec, diff.tv_nsec, str, repetition
    );
    
    free(seq.moves);
}

int is_cube_solved(cube_t cube){
    /* The cube must be in the standard position (F in F, U in U, etc.) 
     * to be considered solved.
     * If the cube is off by a rotation (e.g. "x"), the cube is not considered solved.
    */
    face_t face;
    
    for(int f=0; f<6; f++){
        face = faces[f];
        
        for(int i=0; i<cube.num_stickers_face; i++){
            if(cube.faces[face*cube.num_stickers_face + i] != face){
                return 0;
            }
        }
    }
    return 1;
}





