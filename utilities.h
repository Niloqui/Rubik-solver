#pragma once

#include <stdio.h>
#include <time.h> // struct timespec

#define MAX(a,b) (a>b?a:b)



struct timespec compute_timespec_difference(struct timespec end, struct timespec start){
    struct timespec diff;
    
    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;
    if(diff.tv_nsec < 0){
        diff.tv_nsec += 1000000000; // 999999999 + 1
        diff.tv_sec -= 1;
    }
    
    return diff;
}

/* 
int factorial(int val){
    int result = 1;
    for(int i=2; i<=val; i++){
        result *= i;
    }
    return result;
}
 */

// 13! is too big for a 32-bit integer, so the last value is 12!
int factorial_table[13];

void setup_factorial_table(){
    int val = 1;
    
    factorial_table[0] = 1;
    for(int i=1; i<13; i++){
        val *= i;
        factorial_table[i] = val;
    }
}

int power(int base, int exp){
    int result = 1;
    for(int i=0; i<exp; i++){
        result *= base;
    }
    return result;
}


void print_separator(){
    printf_s("\n------------------------------------------------------------------\n\n");
}



