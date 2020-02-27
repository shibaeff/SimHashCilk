// #define _CRT_SECURE_NO_WARNINGS
#include <cilk/cilk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <random>
#include <climits>
#include <algorithm>
#include <ctime>
#include <mutex>
#include <stdint.h>



#define SIMHASH_BIT 64

enum { NUMWORDS = 1000000, WORDLENGTH = 16 };



/**
@brief 
Some hash function
*/
unsigned long long base_hash(unsigned long long hash, unsigned long long s) {
    return ((hash << 5) + hash) + s;
}

/**
@brief
Calculating a has for the given token
*/
unsigned long long hash_token(const char* arKey, unsigned int length)
{
    std::mutex base_hash_m;
    register unsigned long long ret = 5381;
#pragma simd
    cilk_for (int key = length; key >= 8; key -= 8) {
        
        for (size_t i = 0; i < key; ++i) {
            base_hash_m.lock();
            ret = base_hash(ret, *arKey++);
            base_hash_m.unlock();
        }
        
    }

    for (int i = 0; i < length % 8; ++i) {
        ret = base_hash(ret, *arKey++);
    }
   
    return ret;
}

/**
@brief 
Calculates a simhash of tokenized sequence
*/
unsigned long long simhash_tokens(char const* const* tokens, unsigned int length)
{

    float hash_vector[SIMHASH_BIT];
    memset(hash_vector, 0, SIMHASH_BIT * sizeof(float));

    std::mutex token_hash_m;
    std::mutex hash_vector_m;
// #pragma simd
    for (unsigned i = 0; i < length; i++) {
        unsigned long long token_hash = 0;
        int current_bit = 0;

        // Calculating some hash value, can be any
        token_hash = hash_token(tokens[i], strlen(tokens[i]));
        // Comprised hashvalue 
        cilk_for (int j = SIMHASH_BIT - 1; j >= 0; j--) {
            current_bit = token_hash & 0x1;

            if (current_bit == 1) {
                hash_vector_m.lock();
                hash_vector[j] += 1;
                hash_vector_m.unlock();
            }
            else {
                hash_vector_m.lock();
                hash_vector[j] -= 1;
                hash_vector_m.unlock();
            }
            token_hash_m.lock();
            token_hash = token_hash >> 1;
            token_hash_m.unlock();
        }
    }

    // Accumulated simhash sum
    unsigned simhash = 0;
    for (int i = 0; i < SIMHASH_BIT; i++) {
        if (hash_vector[i] > 0) {
            simhash = (simhash << 1) + 0x1;
           
        }
        else {
           
            simhash = simhash << 1;
            
        }
    }

    return simhash;
}



/**
@brief
Prints a 2D array for debug
*/
void print2D(char** t, int x, int y) {
    for (int i = 0; i < x; ++i)
    {
        puts(t[i]);
    }
}
/**
@brief
Runs a test case for the specific size sz
*/
void run_case(size_t sz) {
    double a = 0;
    // char* words[WORDLENGTH + 1];
    char** words;
    words = (char**)malloc(sizeof(char*) * NUMWORDS);
    for (char** it = words; it != words + NUMWORDS; ++it)
        *it = (char*)malloc(WORDLENGTH + 1);
    for (size_t i = 0; i < sz; ++i) {
        for (int j = 0; j < NUMWORDS; ++j) {
            for (int k = 0; k < WORDLENGTH; ++k) {
                words[j][k] = rand() % 128;
            }
            words[j][WORDLENGTH] = '\0';
        }
        // print2D(words, NUMWORDS, WORDLENGTH);
        auto start = clock();
        simhash_tokens(words, NUMWORDS);
        // printf("%d\n", h);
        auto stop = clock();
        a = (a + stop - start) / 2;
    } 
    for (char** it = words; it != words + NUMWORDS; ++it)
	free(*it); 
    free(words); 		 
    printf("It took simhash %f ticks to run", a);
}

/**
 @brief
 Runs a bunch of test case on the test cases
*/
void driver()
{
    size_t sizes[] = { 100, }; //1000, 10000, 10000, 100000 };
    for (size_t i = 0; i < sizeof(sizes) / sizeof(size_t); ++i) {
        run_case(sizes[i]);
    }
}
int main() {
    srand(2);
    const char* words[] = { "help", 
    };
    int h1 = simhash_tokens(words, 1);
    // int h2 = sh_simhash(words2, 2);
    // printf("%d", h1);
    return 0;
}
