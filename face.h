#pragma once

#define NUM_FACES 6

typedef enum {
    R = 0,
    U = R+1,
    F = R+2,
    exchange = R+3,
    L = R+4,
    D = U+4,
    B = F+4,
    empty = exchange+4,
} face_t;

const face_t faces[] = {R, U, F, L, D, B};

char get_char_from_facet(face_t face){
    switch (face){
    case R:
        return 'R';
    case U:
        return 'U';
    case F:
        return 'F';
    case L:
        return 'L';
    case D:
        return 'D';
    case B:
        return 'B';
    case empty:
        return ' ';
    //case exchange:
    default:
        return '_';
    }
}

face_t get_facet_from_char(char c){
    switch (c){
    case 'R':
        return R;
    case 'U':
        return U;
    case 'F':
        return F;
    case 'L':
        return L;
    case 'D':
        return D;
    case 'B':
        return B;
    case ' ':
        return empty;
    //case '_':
    default:
        return exchange;
    }
}

int get_axis_from_face(face_t face){
    return face % 4;
}


