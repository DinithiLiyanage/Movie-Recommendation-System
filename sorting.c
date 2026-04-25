#include <stdlib.h>

#include "types.h"

// Insertion sort approach
static void sort_desc_small(RatingEntry *arr, int n)
{
    int i, j;
    for (i = 1; i < n; i++)
    {
        RatingEntry key = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j].rating < key.rating)
        {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

void sort(int *recommended_movies, double *predicted_ratings, int no_of_recommended_movies)
{
    const int TOP_K = 10;
    int i, j;
    int k;
    RatingEntry top[TOP_K];
    if (recommended_movies == NULL || predicted_ratings == NULL || no_of_recommended_movies <= 1)
    {
        return;
    }

    k = no_of_recommended_movies < TOP_K ? no_of_recommended_movies : TOP_K;

    for (i = 0; i < k; i++)
    {
        top[i].movie = recommended_movies[i];
        top[i].rating = predicted_ratings[i];
    }

    sort_desc_small(top, k);

    for (i = k; i < no_of_recommended_movies; i++)
    {
        RatingEntry cand;
        cand.movie = recommended_movies[i];
        cand.rating = predicted_ratings[i];

        if (cand.rating <= top[k - 1].rating)
        {
            continue;
        }

        j = k - 1;
        while (j > 0 && top[j - 1].rating < cand.rating)
        {
            top[j] = top[j - 1];
            j--;
        }
        top[j] = cand;
    }

    for (i = 0; i < k; i++)
    {
        recommended_movies[i] = top[i].movie;
        predicted_ratings[i] = top[i].rating;
    }
}
