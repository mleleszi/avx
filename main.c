#include <stdio.h>
#include <immintrin.h>
#define HEADER_SIZE 54
#define SPACE_1_SIZE 1080054                     // 600x600x3 + 54 bit header
#define SPACE_2_SIZE 4320054                     // 1200x1200x3 + 54 bit header
#define SPACE_2_HORIZONTALLY_SCALED_SIZE 2160054 // 1200x600x3 + 54 bit header

int main()
{
    FILE *space_1_file = fopen("/pictures/space_1.bmp", "rb");
    FILE *space_2_file = fopen("/pictures/space_2.bmp", "rb");

    char *space_2 = (char *)malloc((SPACE_2_SIZE) * sizeof(char));
    char *space_1 = (char *)malloc((SPACE_1_SIZE) * sizeof(char));
    char *space_2_avg = (char *)malloc((SPACE_2_HORIZONTALLY_SCALED_SIZE) * sizeof(char));
    char *space_2_scaled = (char *)malloc((SPACE_1_SIZE) * sizeof(char));

    // A - bitmapek betöltése
    fread(space_1, SPACE_1_SIZE, 1, space_1_file);
    fread(space_2, SPACE_2_SIZE, 1, space_2_file);
    fseek(space_1_file, 0, SEEK_SET);
    fread(space_2_scaled, HEADER_SIZE, 1, space_1_file);
    fclose(space_2_file);
    fclose(space_1_file);

    int i, j;
    int k = HEADER_SIZE;

    /*
     * Scale down to half size vertically by averaging rows
     */

    // read rows, 2 at a time -> 1200x3x2 = 7200
    for (i = HEADER_SIZE; i < SPACE_2_SIZE; i += 7200)
    {
        // read 32 ints in each row
        for (j = i; j < i + 3584; j += 32, k += 32)
        {
            __m256i mm_a = _mm256_loadu_si256((__m256i *)&(space_2[j]));
            __m256i mm_b = _mm256_loadu_si256((__m256i *)&(space_2[j + 3600]));
            __m256i mm_avg = _mm256_avg_epu8(mm_a, mm_b);
            _mm256_storeu_si256((__m256i *)&(space_2_avg[k]), mm_avg);
        }

        // 16 ints left
        __m128i mm_a = _mm_loadu_si128((__m128i *)&(space_2[j]));
        __m128i mm_b = _mm_loadu_si128((__m128i *)&(space_2[j + 3600]));
        __m128i mm_avg = _mm_avg_epu8(mm_a, mm_b);
        _mm_storeu_si128((__m128i *)&(space_2_avg[k]), mm_avg);
        k += 16;
    }

    /*
     * Scale down horizontally by keeping every second pixel.
     */
    for (i = HEADER_SIZE, j = HEADER_SIZE; i < SPACE_2_HORIZONTALLY_SCALED_SIZE; i += 6, j += 3)
    {
        space_2_scaled[j] = space_2_avg[i];
        space_2_scaled[j + 1] = space_2_avg[i + 1];
        space_2_scaled[j + 2] = space_2_avg[i + 2];
    }

    for (i = HEADER_SIZE; i < SPACE_1_SIZE; i += 32)
    {
        __m256i mm_a = _mm256_loadu_si256((__m256i *)&(space_2_scaled[i]));
        __m256i mm_b = _mm256_loadu_si256((__m256i *)&(space_1[i]));
        __m256i mm_avg = _mm256_avg_epu8(mm_a, mm_b);
        _mm256_storeu_si256((__m256i *)&(space_2_scaled[i]), mm_avg);
    }

    FILE *output = fopen("space_new.bmp", "wb+");
    fwrite(space_2_scaled, 1, SPACE_1_SIZE, output);

    fclose(output);
    free(space_2);
    free(space_1);
    free(space_2_avg);
    free(space_2_scaled);
}
