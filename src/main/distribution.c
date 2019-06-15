#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "distribution.h"
#include "matrix.h"
#include "random.h"
#include "bmp.h"

#define SET 10  // TODO: revise this, set with system clock
#define MAX 50 
#define MOD ((uint8_t) 251) // closes prime
#define MATRIX_X_COLS 1

// seed must be 48 bits;
int64_t seed; 

int distributeSecret(const char * image, uint8_t k, uint8_t n, const char * dir, const char * watermark)
{
    setSeed(SET);
    generateModInverses(MOD);
    matrix_t * S = read_bmp(image, false);
    matrix_t * A  = createAMatrix(k, n);
    matrix_t * Sproj = proj(A, MOD);
    matrix_t * R = substract(S, Sproj, MOD);
    matrix_t ** X = createXMatrices(k, n);
    matrix_t ** V = createVMatrices(A, X, n);
    matrix_t ** G = createGMatrices(R, k, n, n); // m = n
    matrix_t ** Sh = createShMatrices(V, G, n);
    // Generate watermark and make remainder Rw public
    matrix_t * W = read_bmp(image, false);
    matrix_t * Rw = substract(W, Sproj, MOD);

    return 1;
}

matrix_t ** createShMatrices(matrix_t ** V, matrix_t ** G, uint8_t n)
{
    matrix_t ** Sh = calloc(n, sizeof(matrix_t*));
    for (uint8_t i = 0; i < n; i++)
    {
        Sh[i] = augment(V[i], G[i]);
    }
    return Sh;
}

matrix_t ** createGMatrices(matrix_t * R, uint8_t k, uint8_t n, uint8_t m)
{
    int c;
    size_t cols = (size_t) ceil(((double)m)/k);
    matrix_t ** Gs = calloc(n, sizeof(matrix_t*));
    for (uint8_t i = 0; i < n; i++)
    {
        c = i + 1;
        fillGMatrix(Gs[i], R, k, c);
    }
    return Gs;
}

void fillGMatrix(matrix_t * G, matrix_t * R, uint8_t k, int c)
{
    for (uint8_t i = 0; i < G->rows; i++)
    {
        getColumn(R, i);
    }
}

matrix_t ** createVMatrices(matrix_t * A, matrix_t ** X, uint8_t n)
{
    matrix_t ** matrices = calloc(n, sizeof(matrix_t*));
    for (uint8_t i = 0; i < n; i++)
    {
        matrices[i] = multiply(A, X[i], MOD);
    }
    return matrices;
}

matrix_t ** createXMatrices(uint8_t k, uint8_t n)
{
    matrix_t ** matrices = calloc(n, sizeof(matrix_t*));
    uint8_t * aValues = generateAValues(n);
    for (uint8_t i = 0; i < n; i++)
    {
        matrices[i] = create(k, MATRIX_X_COLS);
        fillLinearlyIndependentMatrix(matrices[i], aValues[i]); // TODO: revise this
    }
    free(aValues);
    return matrices;
}

uint8_t * generateAValues(uint8_t n)
{
    uint8_t * aValues = calloc(n, sizeof(uint8_t));
    uint8_t a;
    bool aAlreadyPresent = true;
    for (int i = 0; i < n; i++)
    {
        while (aAlreadyPresent)
        {
            a = nextChar(seed) % MOD; // 0, 1, 2, 3, 4 will be more likely check this
            aAlreadyPresent = isValueInArray(aValues, a, i);
        }
        aValues[i] = a;
    }
    return aValues;
}

bool isValueInArray(uint8_t * array, uint8_t value, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (array[i] == value)
            return true;
    }
    return false;
}

matrix_t * createAMatrix(uint8_t k, uint8_t n)
{
    matrix_t * A = NULL, * At = NULL, * aux = NULL;
    int m = n; // TODO: revise this
    A = create(m, k);
    do 
    {   
        delete(At);
        delete(A);
        delete(aux);
        At = transpose(A);
        aux = multiply(A, At, MOD);
        fillRandomMatrix(A);
    }
    while (getRank(aux, MOD) == n && getRank(A, MOD) != k);
    delete(aux);
    delete(At);
    return A;
}

void fillRandomMatrix(matrix_t * matrix)
{
    for (uint8_t i = 0; i < matrix->rows; i++)
    {
        for (uint8_t j = 0; j < matrix->columns; j++)
        {
            matrix->data[i][j] = nextChar(seed) % MOD;
        }
    }
}

void fillLinearlyIndependentMatrix(matrix_t * matrix, int a)
{
    for (uint8_t i = 0; i < matrix->rows; i++)
    {
        for (uint8_t j = 0; j < matrix->columns; j++)
        {
            matrix->data[i][j] = (uint8_t) pow(a, i);
        }
    }
}

