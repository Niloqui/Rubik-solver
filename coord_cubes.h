#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <direct.h> // _mkdir

// https://github.com/jean553/c-simd-avx2-example
#include <immintrin.h>

#include "utilities.h"
#include "face.h"
#include "move.h"
//#include "cube.h"
#include "cube3.h"


#define CORNER_PERM_N (1*2*3*4*5*6*7*8)             // 40320
#define CORNER_ORI_N (3*3*3*3*3*3*3)                // 2187
#define EDGE_PERM_N (1*2*3*4*5*6*7*8*9*10*11*12)    // 479001600
#define EDGE_ORI_N (2*2*2*2*2*2*2*2*2*2*2)          // 2048

#define UDSLICE_N (9*10*11*12)                      // 11880
#define UDSLICE_EQUI_N 788                          // Equivalence classes

//#define SOLVER_SYMMETRIES_N 4*2*2               // 16
//#define SYMMETRIES_N 3*SOLVER_SYMMETRIES_N      // 48    // Defined in cube3.h (another value)


int n_choose_k_table[NUM_EDGES][4] = {-1};

void setup_n_choose_k_table(){
    for(int n=0; n<NUM_EDGES; n++){
        int n_fat = factorial_table[n];
        
        for(int k=0; k<(n>4?4:n+1); k++){
            int val = n_fat / (factorial_table[k] * factorial_table[n-k]);
            n_choose_k_table[n][k] = val;
        }
    }
}


int ud_slice_sorted_sym_to_raw_table[UDSLICE_EQUI_N];
int ud_slice_sorted_raw_to_sym_table[UDSLICE_N][2];

typedef struct {
    int cor_o; // Corner orientation
    int cor_p; // Corner permutation
    int edge_o; // Edge orientation
    //int edge_p; // Edge permutation
    
    // Edge permutation divided in three indexes
    // Based on UDSliceSorted
    int ud_slice, u_face, d_face;
    
    // Sym coordinates
    int ud_sym_c, ud_sym_s;
} coord_cube_t;

int get_coord_corner_orientation(cube3_t cube3){
    __m256i helper_v, data_v;
    int p[8];
    
    helper_v = _mm256_set_epi32(
        3*3*3*3*3*3, 3*3*3*3*3, 3*3*3*3, 3*3*3, 3*3, 3, 1, 0
    );
    
    data_v = _mm256_set_epi32(
        cube3.corners[UFR].orientation,
        cube3.corners[UFL].orientation,
        cube3.corners[UBR].orientation,
        cube3.corners[UBL].orientation,
        cube3.corners[DFR].orientation,
        cube3.corners[DFL].orientation,
        cube3.corners[DBR].orientation,
        0 //cube3.corners[DBL].orientation
    );
    
    data_v = _mm256_mullo_epi32(data_v, helper_v);
    
    int cor_o = 0;
    _mm256_store_si256((__m256i *)p, data_v);
    //p = (int *)&data_v;
    for(int i=0; i<8; i++){
        cor_o += p[i];
    }
    
    return cor_o;
}

int get_coord_corner_permutation(cube3_t cube3){
    int p[8];
    __m256i helper_v, data_v;
    
    helper_v = _mm256_set_epi32(
        1*2*3*4*5*6*7, 1*2*3*4*5*6, 1*2*3*4*5, 1*2*3*4, 1*2*3, 1*2, 1, 0
        //0, 1, 1*2, 1*2*3, 1*2*3*4, 1*2*3*4*5, 1*2*3*4*5*6, 1*2*3*4*5*6*7
    );
    
    for(int i=0; i<8; i++){
        // Loop starts with i=0 because we need to set p[i]=0
        p[i] = 0;
        for(int j=0; j<i; j++){
            if(cube3.corners[j].piece > cube3.corners[i].piece){
                p[i] += 1;
            }
        }
    }
    
    data_v = _mm256_load_si256((__m256i *)p);
    data_v = _mm256_mullo_epi32(data_v, helper_v);
    
    int cor_p = 0;
    _mm256_store_si256((__m256i *)p, data_v);
    for(int i=0; i<8; i++){
        cor_p += p[i];
    }
    
    return cor_p;
}

int get_coord_edge_orientation(cube3_t cube3){
    short sp[16];
    __m256i helper_v, data_v;
    
    helper_v = _mm256_set_epi16(
        2*2*2*2*2*2*2*2*2*2, 2*2*2*2*2*2*2*2*2,
        2*2*2*2*2*2*2*2, 2*2*2*2*2*2*2,
        2*2*2*2*2*2, 2*2*2*2*2,
        2*2*2*2, 2*2*2,
        2*2, 2, 1, 0, 0, 0, 0, 0
    );
    
    //UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR
    
    data_v = _mm256_set_epi16(
        cube3.edges[UR].orientation,
        cube3.edges[UF].orientation,
        cube3.edges[UL].orientation,
        cube3.edges[UB].orientation,
        cube3.edges[DR].orientation,
        cube3.edges[DF].orientation,
        cube3.edges[DL].orientation,
        cube3.edges[DB].orientation,
        cube3.edges[FR].orientation,
        cube3.edges[FL].orientation,
        cube3.edges[BL].orientation,
        0, //cube3.edges[BR].orientation,
        0, 0, 0, 0
    );
    
    data_v = _mm256_mullo_epi16(data_v, helper_v);
    
    int edge_o = 0;
    _mm256_store_si256((__m256i *)sp, data_v);
    for(int i=0+4; i<12+4; i++){
        edge_o += sp[i];
    }
    
    return edge_o;
}

int get_coord_edge_permutation(cube3_t cube3){
    int p[2][8] = {0};
    __m256i data_v;
    __m256i edge_p_helpers[2] = {
        _mm256_set_epi32(
            //1*2*3*4*5*6*7, 1*2*3*4*5*6, 1*2*3*4*5, 1*2*3*4, 1*2*3, 1*2, 1, 0
            //0, 1, 1*2, 1*2*3, 1*2*3*4, 1*2*3*4*5, 0, 0
            0, 0, 1*2*3*4*5, 1*2*3*4, 1*2*3, 1*2, 1, 0
        ),
        _mm256_set_epi32(
            0, 0, 
            1*2*3*4*5*6*7*8*9*10*11, 1*2*3*4*5*6*7*8*9*10,
            1*2*3*4*5*6*7*8*9, 1*2*3*4*5*6*7*8,
            1*2*3*4*5*6*7, 1*2*3*4*5*6
        )
    };
    
    /* 
    for(int i=0; i<2; i++){
        for(int j=0; j<8; j++){
            p[i][j] = 0;
        }
    }
     */
    
    for(int i=1; i<12; i++){
        for(int j=0; j<i; j++){
            if(cube3.edges[j].piece > cube3.edges[i].piece){
                p[i/6][i%6] += 1;
            }
        }
    }
    
    int edge_p = 0;
    for(int i=0; i<2; i++){
        data_v = _mm256_load_si256((__m256i *)p[i]);
        data_v = _mm256_mullo_epi32(data_v, edge_p_helpers[i]);
        
        _mm256_store_si256((__m256i *)p[i], data_v);
        
        for(int j=0; j<6; j++){
            edge_p += p[i][j];
        }
    }
    
    return edge_p;
}

int get_coord_4_edges_permutation(cube3_t cube3, char offset){
    int coord = 0;
    int k = -1;
    int indexes[4];
    
    for(int n=0; n<NUM_EDGES; n++){
        int i = (n + NUM_EDGES - offset) % NUM_EDGES;
        if((cube3.edges[i].piece + offset) % NUM_EDGES >= FR){
            k++;
            indexes[k] = i;
        }
        else if(k>=0){
            int nck = n_choose_k_table[n][k];
            
            coord += nck;
        }
    }
    coord *= 24;
    
    for(int i=1; i<4; i++){
        int val = 0;
        
        for(int j=0; j<i; j++){
            if((cube3.edges[indexes[j]].piece + offset) % NUM_EDGES > 
                    (cube3.edges[indexes[i]].piece + offset) % NUM_EDGES){
                val++;
            }
        }
        
        coord += val * factorial_table[i];
    }
    
    return coord;
}

int get_coord_ud_slice_permutation(cube3_t cube3){
    return get_coord_4_edges_permutation(cube3, 0);
}
int get_coord_u_face_permutation(cube3_t cube3){
    return get_coord_4_edges_permutation(cube3, 8);
}
int get_coord_d_face_permutation(cube3_t cube3){
    return get_coord_4_edges_permutation(cube3, 4);
}

coord_cube_t create_coord_cube_from_cube3(const cube3_t cube3){
    coord_cube_t coords;
    //int p[2][8];
    //short sp[16];
    //__m256i helper_v, data_v;
    
    coords.cor_o = get_coord_corner_orientation(cube3);
    coords.cor_p = get_coord_corner_permutation(cube3);    
    coords.edge_o = get_coord_edge_orientation(cube3);
    //coords.edge_p = get_coord_edge_permutation(cube3);
    
    coords.ud_slice = get_coord_ud_slice_permutation(cube3);
    coords.u_face = get_coord_u_face_permutation(cube3);
    coords.d_face = get_coord_d_face_permutation(cube3);
    
    coords.ud_sym_c = ud_slice_sorted_raw_to_sym_table[coords.ud_slice][0];
    coords.ud_sym_s = ud_slice_sorted_raw_to_sym_table[coords.ud_slice][1];
    
    return coords;
}

coord_cube_t create_coord_cube(){
    coord_cube_t coords;
    
    coords.cor_o = coords.cor_p = coords.edge_o = 0;
    //coords.edge_p = 0;
    coords.ud_slice = coords.d_face = coords.u_face = 0;
    
    coords.ud_sym_c = coords.ud_sym_s = 0;
    
    return coords;
}

void get_inverse_coord_4_edges_permutation(cube3_t *cube3_p, int coord, int offset){
    //coord = coord_cube.ud_slice;
    int coord_p = coord % 24; // Permutation part of the index
    int coord_c = coord / 24; // Combination part of the index
    
    int indexes[4] = {0,0,0,0};
    int used_pieces[4] = {0,0,0,0};
    
    for(int n=NUM_EDGES-1, k=3; n>=0 && k>=0; n--){
        if(coord_c - n_choose_k_table[n][k] >= 0){
            coord_c -= n_choose_k_table[n][k];
        }
        else{
            // "+ NUM_EDGES" so it doesn't go into the negatives
            indexes[k] = (n + NUM_EDGES - offset) % NUM_EDGES;
            k--;
        }
    }
    
    for(int i=4-1; i>=0; i--){
        int val = (coord_p / factorial_table[i]);
        
        for(edges_e j=4-1; j>=0; j--){
            if(used_pieces[j] == 0){
                if(val == 0){
                    used_pieces[j] = 1;
                    cube3_p->edges[indexes[i]].piece = (j + 8 + NUM_EDGES - offset) % NUM_EDGES;
                    break;
                }
                else{
                    val--;
                }
            }
        }
        
        coord_p = coord_p % factorial_table[i];
    }
}

cube3_t create_cube3_from_coord_cube(const coord_cube_t coord_cube){
    cube3_t cube3 = create_cube3();
    int coord;
    char total_orientation;
    char orientation;
    //int used_pieces[MAX(NUM_CORNERS,NUM_EDGES)];
    int used_pieces[NUM_CORNERS];
    
    
    // Corner orientation
    coord = coord_cube.cor_o;
    total_orientation = 0;
    for(int i=NUM_CORNERS-2; i>=0; i--){
        orientation = coord % 3;
        
        cube3.corners[i].orientation = orientation;
        total_orientation += orientation;
        
        coord /= 3;
    }
    cube3.corners[NUM_CORNERS-1].orientation = (total_orientation*2)%3;
    
    
    // Edge orientation
    coord = coord_cube.edge_o;
    total_orientation = 0;
    for(int i=NUM_EDGES-2; i>=0; i--){
        orientation = coord % 2;
        
        cube3.edges[i].orientation = orientation;
        total_orientation += orientation;
        
        coord /= 2;
    }
    cube3.edges[NUM_EDGES-1].orientation = total_orientation%2;
    
    
    // Corner permutation
    coord = coord_cube.cor_p;
    for(int i=0; i<NUM_CORNERS; i++){
        used_pieces[i] = 0;
    }
    
    for(int i=NUM_CORNERS-1; i>=0; i--){
        int val = (coord / factorial_table[i]);
        
        //used_pieces[corner] = 1;
        
        for(corners_e j=NUM_CORNERS-1; j>=0; j--){
            if(used_pieces[j] == 0){
                if(val == 0){
                    used_pieces[j] = 1;
                    cube3.corners[i].piece = j;
                    break;
                }
                else{
                    val--;
                }
            }
            /* 
            if(val == 0 && used_pieces[j] > 0){
                used_pieces[j] = 1;
                cube3.corners[i].piece = j;
                break;
            }
            if(used_pieces[j] == 0){
                val--;
            } */
        }
        
        coord = coord % factorial_table[i];
    }
    
    for(int i=0; i<NUM_EDGES; i++){
        cube3.edges[i].piece = EDGE_LAST;
    }
    
    // UD Slice
    get_inverse_coord_4_edges_permutation(&cube3, coord_cube.ud_slice, 0);
    
    // U Face
    get_inverse_coord_4_edges_permutation(&cube3, coord_cube.u_face, 8);
    
    // D Face
    get_inverse_coord_4_edges_permutation(&cube3, coord_cube.d_face, 4);
    
    return cube3;
}

int is_coord_cube_solved(const coord_cube_t coords){
    return !(coords.cor_o || coords.cor_p || coords.edge_o ||
            coords.ud_slice || coords.u_face || coords.d_face);
}

char* get_str_from_coord_cube(const coord_cube_t coord_cube){
    char *str;
    
    // 1024 characters should be more than enough for this,
    // so no checks on out of bounds writes :)
    str = (char*)malloc(1024);
    if(str == NULL){
        printf_s("Error in memory allocation.\n");
        return str; // return NULL
    }
    
    sprintf_s(str, 1024, 
        "cor_o = %i\n"
        "cor_p = %i\n"
        "edge_o = %i\n"
        "ud_slice = %i\n"
        "u_face = %i\n"
        "d_face = %i\n\n",
        coord_cube.cor_o, coord_cube.cor_p, coord_cube.edge_o,
        coord_cube.ud_slice, coord_cube.u_face, coord_cube.d_face
    );
    
    return str;
}



////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Tables //////////////////////////////////
////////////////////////////////////////////////////////////////////////////

typedef struct{
    //edges_e edges_order[12];
    //corners_e corners_order[8];
    
    int *main_index;
    int *cor_o; // cor_o[CORNER_ORI_N][NUM_MOVES]
    int *cor_p; // cor_p[CORNER_PERM_N][NUM_MOVES]
    int *edge_o; // edge_o[EDGE_ORI_N][NUM_MOVES]
    //int *edge_p;
    int *ud_slice, *u_face, *d_face; // ud_slice[UDSLICE_N][NUM_MOVES]
    
    int *ud_slice_sym; // ud_slice_sym[UDSLICE_EQUI_N][NUM_MOVES][2]
} movement_table_t;

movement_table_t move_tbs;

typedef struct{
    int coord; // Coordinate
    int num_moves;
    cube3_t cube_state;
} table_entry_t;

int generate_move_table(int *table_p, int table_dim, int (*get_coord_f)(cube3_t)){
    table_entry_t *cube_queue;
    unsigned char *visited_nodes;
    // 0: not visited
    // 1: in list
    // 2: visited
    
    cube_queue = malloc(table_dim * sizeof(table_entry_t) + 
                        table_dim * sizeof(unsigned char));
    
    if(cube_queue == NULL){
        printf_s("Error in memory allocation. [generate_move_table()]\n");
        return 1;
    }
    
    visited_nodes = (unsigned char *)cube_queue + table_dim * sizeof(table_entry_t);
    for(int i=0; i<table_dim; i++){
        visited_nodes[i] = 0;
    }
    
    cube3_t cube3 = create_cube3();
    int coord = (*get_coord_f)(cube3);
    int num_moves = 0;
    
    cube_queue[0].coord = coord;
    cube_queue[0].num_moves = num_moves;
    cube_queue[0].cube_state = cube3;
    visited_nodes[coord] = 1;
    
    int entry=0, num_entries=1;
    while(entry!=num_entries){
        coord = cube_queue[entry].coord;
        num_moves = cube_queue[entry].num_moves;
        cube3 = cube_queue[entry].cube_state;
        
        for(fast_move_t fast_m=0; fast_m<LAST_FAST_MOVE; fast_m++){
            move_t move = fast_to_move_table[fast_m];
                
            cube3_t new_cube3 = apply_move_to_cube3(cube3, move);
            int new_coord = (*get_coord_f)(new_cube3);
            
            table_p[coord*NUM_MOVES + fast_m] = new_coord;
            
            if(visited_nodes[new_coord] == 0){
                visited_nodes[new_coord] = 1;
                cube_queue[num_entries].cube_state = new_cube3;
                cube_queue[num_entries].coord = new_coord;
                cube_queue[num_entries].num_moves = num_moves + 1;
                num_entries++;
            }
        }
        
        visited_nodes[coord] = 2;
        entry++;
    }
    /* 
    printf_s("visited_nodes:\n");
    for(int i=0; i<CORNER_ORI_N * sizeof(unsigned char); i++){
        printf_s("%i - %i\n", i, visited_nodes[i]);
    } */
    
    free(cube_queue);
    return 0;
}

int allocate_move_table(const char *file_name, char *base_check_array, int check_length,
                        int *table_p, int table_dim, int (*get_coord_f)(cube3_t)){
    // If the table file exists, this function reads it
    // If it doesn't, this function generate a new one and saves it in a file
    
    char check_order[MAX(NUM_CORNERS,NUM_EDGES)]; // = {-1};
    
    FILE *f;
    int error;
    
    printf_s("Checking if \"%s\" exists.\n", file_name);
    error = fopen_s(&f, file_name, "rb");
    
    if(error == 0){
        // File exists, now we must check for the validity.
        fread_s(check_order, check_length, sizeof(char), check_length, f);
        
        for(int i=0; i<check_length; i++){
            if(check_order[i] != base_check_array[i]){
                error = 1;
                break;
            }
        }
    }
    
    // error can be modified in the previous section
    if(error){
        // File doesn't exist or it has wrong piece order.
        // The table must be computed here.
        printf_s("File \"%s\" doesn't exist or it is not valid.\n", file_name);
        
        // Closing the file, it must be reopened in writing mode
        fclose(f);
        
        //for(int i=0; i<table_dim*NUM_MOVES; i++){
        //    table_p[i] = -1;
        //}
        
        error = generate_move_table(table_p, table_dim, get_coord_f);
        if(error){
            return 1;
        }
        
        error = fopen_s(&f, file_name, "wb");
        fwrite(base_check_array, sizeof(char), check_length, f);
        fwrite(table_p, sizeof(int), table_dim*NUM_MOVES, f);
    }
    else{
        // File exists and it's ok, now we must read the data.
        printf_s("File \"%s\" is valid.\n", file_name);
        
        // The check_order has been already read
        fread_s(table_p, table_dim*NUM_MOVES*sizeof(int),
                sizeof(int), table_dim*NUM_MOVES, f);
    }
    
    printf_s("\n");
    fclose(f);
    return 0;
}

void setup_ud_slice_sym_table(){
    cube3_t cube3 = create_cube3(), temp_cube3;
    
    int i, j, idx, ud_slice_coord;
    
    // "ud_slice_sorted_raw_to_sym_table" is used as visited array
    for(i=0; i<UDSLICE_N; i++){
        ud_slice_sorted_raw_to_sym_table[i][0] = -1;
        ud_slice_sorted_raw_to_sym_table[i][1] = -1;
    }
    
    //// First phase: initiliaze "ud_slice_sorted_sym_to_raw_table"
    //                     and "ud_slice_sorted_raw_to_sym_table"
    idx = 0;
    for(i=0; i<UDSLICE_N; i++){
        if(ud_slice_sorted_raw_to_sym_table[i][0] >= 0){
            // Already visited state
            continue;
        }
        ud_slice_sorted_sym_to_raw_table[idx] = i;
        
        // Cleaning the cube pieces.
        // This is done because the "get_inverse_coord_4_edges_permutation" function
        // only modifies the values for the UD slice pieces,
        // leaving everything else the same.
        // Normally this function is used also for U face and D face pieces (so all edges),
        // so this problem never happens, but in this case it's only being used for ud slice
        // pieces. If the cube is not cleaned, there could be more UD slice pieces in the
        // edges array, breaking the function.
        for(j=0; j<NUM_EDGES; j++){
            cube3.edges[j].piece = UR;
        }
        
        get_inverse_coord_4_edges_permutation(&cube3, i, 0);
        
        // j goes backwards because, in case the same value for ud_slice_coord
        // appears in more than one symmetry,
        // the symmetry with the lowest value is saved in the table.
        //for(j=0; j<SYMMETRIES_N; j++){
        for(j=SYMMETRIES_N-1; j>=0; j--){
            temp_cube3 = multiply_cube3(symmetries_table[j], cube3);
            temp_cube3 = multiply_cube3(temp_cube3, symmetries_inverse_table[j]);
            
            ud_slice_coord = get_coord_ud_slice_permutation(temp_cube3);
            
            //ud_slice_sorted_raw_to_sym_table[ud_slice_coord] = idx*SYMMETRIES_N + j;
            ud_slice_sorted_raw_to_sym_table[ud_slice_coord][0] = idx;
            ud_slice_sorted_raw_to_sym_table[ud_slice_coord][1] = j;
        }
        
        idx++;
    }
    
    //// Second phase: set up "move_tbs.ud_slice_sym"
    //restore_cube3(&cube3);
    int new_raw_coord, new_sym_coord, new_sym;
    
    for(i=0; i<UDSLICE_EQUI_N; i++){
        ud_slice_coord = ud_slice_sorted_sym_to_raw_table[i];
        
        // Cleaning the cube pieces.
        // Same reasons as in the first phase.
        for(j=0; j<NUM_EDGES; j++){
            cube3.edges[j].piece = UR;
        }
        get_inverse_coord_4_edges_permutation(&cube3, ud_slice_coord, 0);
        
        for(fast_move_t fast_m=0; fast_m<LAST_FAST_MOVE; fast_m++){
            temp_cube3 = apply_move_to_cube3(cube3, fast_to_move_table[fast_m]);
            
            new_raw_coord = get_coord_ud_slice_permutation(temp_cube3);
            
            new_sym_coord = ud_slice_sorted_raw_to_sym_table[new_raw_coord][0];
            new_sym = ud_slice_sorted_raw_to_sym_table[new_raw_coord][1];
            
            // [UDSLICE_EQUI_N][NUM_MOVES][2]
            idx = i * NUM_MOVES * 2 + fast_m * 2;
            
            move_tbs.ud_slice_sym[idx] = new_sym_coord;
            move_tbs.ud_slice_sym[idx + 1] = new_sym;
        }
    }
    
    printf_s("\n\n");
    return;
}


int setup_movement_tables_3x3x3(){
    ////////////////// DON'T CHANGE THESE
    ////////////////// DON'T CHANGE THESE
    char corners_order_base[NUM_CORNERS] = {UFR, UFL, UBR, UBL, DFR, DFL, DBR, DBL};
    char edges_order_base[NUM_EDGES] = {UR, UF, UL, UB, DR, DF, DL, DB, FR, FL, BL, BR};
    ////////////////// DON'T CHANGE THESE
    ////////////////// DON'T CHANGE THESE
    
    int error = 0;
    
    move_tbs.main_index = malloc(
        sizeof(int) * (
            CORNER_PERM_N * NUM_MOVES +
            CORNER_ORI_N * NUM_MOVES +
            EDGE_ORI_N * NUM_MOVES +
            UDSLICE_N * 3 * NUM_MOVES +
            UDSLICE_EQUI_N * 2 * NUM_MOVES
        )
    );
    
    if(move_tbs.main_index == NULL){
        return -1;
    }
    
    move_tbs.cor_o = move_tbs.main_index;
    move_tbs.cor_p = move_tbs.cor_o + CORNER_ORI_N * NUM_MOVES;
    move_tbs.edge_o = move_tbs.cor_p + CORNER_PERM_N * NUM_MOVES;
    move_tbs.ud_slice = move_tbs.edge_o + EDGE_ORI_N * NUM_MOVES;
    move_tbs.u_face = move_tbs.ud_slice + UDSLICE_N * NUM_MOVES;
    move_tbs.d_face = move_tbs.u_face + UDSLICE_N * NUM_MOVES;
    move_tbs.ud_slice_sym = move_tbs.d_face + UDSLICE_N * NUM_MOVES;
    
    // Creating folders
    _mkdir("tables");
    _mkdir("tables\\3x3x3");
    
    error += allocate_move_table("tables\\3x3x3\\cor_o.movement",
                                corners_order_base, NUM_CORNERS,
                                move_tbs.cor_o, CORNER_ORI_N,
                                get_coord_corner_orientation);
    
    error += allocate_move_table("tables\\3x3x3\\cor_p.movement",
                                corners_order_base, NUM_CORNERS,
                                move_tbs.cor_p, CORNER_PERM_N,
                                get_coord_corner_permutation);
    
    error += allocate_move_table("tables\\3x3x3\\edge_o.movement",
                                edges_order_base, NUM_EDGES,
                                move_tbs.edge_o, EDGE_ORI_N,
                                get_coord_edge_orientation);
    
    error += allocate_move_table("tables\\3x3x3\\ud_slice.movement",
                                edges_order_base, NUM_EDGES,
                                move_tbs.ud_slice, UDSLICE_N,
                                get_coord_ud_slice_permutation);
    
    error += allocate_move_table("tables\\3x3x3\\u_face.movement",
                                edges_order_base, NUM_EDGES,
                                move_tbs.u_face, UDSLICE_N,
                                get_coord_u_face_permutation);
    
    error += allocate_move_table("tables\\3x3x3\\d_face.movement",
                                edges_order_base, NUM_EDGES,
                                move_tbs.d_face, UDSLICE_N,
                                get_coord_d_face_permutation);
    
    setup_ud_slice_sym_table();
    
    printf_s("Number of errors while allocating the movement tables: %i\n\n", error);
    return error;
}






////////////////////////////////////////////////////////////////////////////
////////////////////////////////// Moves ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////

coord_cube_t apply_fast_move_to_coord_cube(coord_cube_t coords, const fast_move_t fast_m){
    coords.cor_o = move_tbs.cor_o[coords.cor_o*NUM_MOVES + fast_m];
    coords.cor_p = move_tbs.cor_p[coords.cor_p*NUM_MOVES + fast_m];
    coords.edge_o = move_tbs.edge_o[coords.edge_o*NUM_MOVES + fast_m];
    coords.ud_slice = move_tbs.ud_slice[coords.ud_slice*NUM_MOVES + fast_m];
    coords.u_face = move_tbs.u_face[coords.u_face*NUM_MOVES + fast_m];
    coords.d_face = move_tbs.d_face[coords.d_face*NUM_MOVES + fast_m];
    
    // Sym-coordinates movement
    coords.ud_sym_c = ud_slice_sorted_raw_to_sym_table[coords.ud_slice][0];
    coords.ud_sym_s = ud_slice_sorted_raw_to_sym_table[coords.ud_slice][1];
    
    /* 
    //// Sym-coordinates movement
    // inputs: ud_sym_c, ud_sym_s, fast_m
    
    // SymMove ------ symmetries_fast_m_table
    // fast_m1 = symmetries_fast_m_table[ud_sym_s][fast_m];
    
    // MoveTable ---- move_tbs.ud_slice_sym
    // idx = ud_sym_c * NUM_MOVES * 2 + fast_m1 * 2;
    // new_coord = move_tbs.ud_slice_sym[idx]
    // new_sym = move_tbs.ud_slice_sym[idx + 1]
    
    // SymMult ------ sym_sym_table
    // new_sym = sym_sym_table[new_sym][ud_sym_s]
    
    // output: new_coord, new_sym
    
    fast_move_t fast_m1;
    int idx, new_coord, new_sym;
    
    fast_m1 = symmetries_fast_m_table[coords.ud_sym_s][fast_m];
    
    idx = coords.ud_sym_c * NUM_MOVES * 2 + fast_m1 * 2;
    new_coord = move_tbs.ud_slice_sym[idx];
    new_sym = move_tbs.ud_slice_sym[idx + 1];
    
    new_sym = sym_sym_table[new_sym][coords.ud_sym_s];
    
    coords.ud_sym_c = new_coord;
    coords.ud_sym_s = new_sym;
     */
    
    return coords;
}

coord_cube_t apply_move_to_coord_cube(coord_cube_t coord_cube, const move_t move){
    return apply_fast_move_to_coord_cube(coord_cube, get_fast_from_move(move));
}

coord_cube_t apply_seq_to_coord_cube(coord_cube_t coord_cube, const moves_seq_t seq){
    for(int i=0; i<seq.len; i++){
        coord_cube = apply_move_to_coord_cube(coord_cube, seq.moves[i]);
    }
    
    return coord_cube;
}

coord_cube_t apply_inverse_seq_to_coord_cube(coord_cube_t coord_cube, const moves_seq_t seq){
    for(int i=seq.len-1; i>=0; i--){
        move_t move = get_inverse_move(seq.moves[i]);
        coord_cube = apply_move_to_coord_cube(coord_cube, move);
    }
    
    return coord_cube;
}


void stress_test_coord_cube(coord_cube_t coord_cube, const char *str, int repetition){
    struct timespec start, end, diff;
    
    moves_seq_t seq = get_move_seq_from_str(3, str);
    
    timespec_get(&start, TIME_UTC);
    for(int i=0; i<repetition; i++){
        coord_cube = apply_seq_to_coord_cube(coord_cube, seq);
    }
    timespec_get(&end, TIME_UTC);
    
    diff = compute_timespec_difference(end, start);
    printf_s("\nIt took %u.%09u seconds to apply \"%s\" %d times (coord_cube_t).\n",
            diff.tv_sec, diff.tv_nsec, str, repetition
    );
    
    char *result = get_str_from_coord_cube(coord_cube);
    printf_s("%s\n", result);
    
    free(seq.moves);
}


















