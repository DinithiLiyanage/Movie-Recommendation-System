// Centered cosine similarity
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "types.h"

double pearson_correlation(double *A, double *B, unsigned int size)
{
	double dot_p = 0.0;
	double mag_a = 0.0;
	double mag_b = 0.0;
	int i;
	for (i = 0; i < size; i++)
	{
		dot_p += A[i] * B[i];
		mag_a += A[i] * A[i];
		mag_b += B[i] * B[i];
	}
	double denom = sqrt(mag_a) * sqrt(mag_b);
	if (denom <= 1e-12)
	{
		return 0.0;
	}
	return dot_p / denom;
}

void calc_similarity(double *normalizeduser, double *normalized_matrix, double *similarity, int No_of_users, int No_of_movies)
{
	int i = 0, j = 0;
	for (i = 0; i < No_of_users; i++)
	{ // traverse through each user
		double *A;
		A = (double *)malloc(sizeof(double) * No_of_movies);

		// get rating vector for that user
		for (j = 0; j < No_of_movies; j++)
		{
			A[j] = normalized_matrix[i * No_of_movies + j];
		}

		// find similarity between new user and ith user
		similarity[i] = pearson_correlation(normalizeduser, A, No_of_movies);
		free(A);
	}
}

void calc_similarity_optimized(RatingEntry *normalizeduser, int normalizeduser_size, RatingsStore *store, double *similarity)
{
	int i, j, k;
	if (normalizeduser == NULL || store == NULL || store->users == NULL || similarity == NULL)
	{
		return;
	}

	double *new_ratings = (double *)calloc(store->no_of_movies, sizeof(double));
	unsigned char *is_rated = (unsigned char *)calloc(store->no_of_movies, sizeof(unsigned char));
	if (new_ratings == NULL || is_rated == NULL)
	{
		free(new_ratings);
		free(is_rated);
		return;
	}

	for (k = 0; k < normalizeduser_size; k++)
	{
		int m = normalizeduser[k].movie;
		if (m >= 0 && m < store->no_of_movies)
		{
			new_ratings[m] = normalizeduser[k].rating;
			is_rated[m] = 1;
		}
	}

	for (i = 0; i < store->no_of_users; i++)
	{
		UserRatings *user = &store->users[i];
		int max_pairs = user->size < normalizeduser_size ? user->size : normalizeduser_size;

		if (max_pairs <= 1)
		{
			similarity[i] = 0.0;
			continue;
		}

		double a, b;
		double dot = 0.0;
		double mag_a = 0.0;
		double mag_b = 0.0;
		int cnt = 0;

		for (j = 0; j < user->size; j++)
		{
			int m = user->entries[j].movie;
			if (m >= 0 && m < store->no_of_movies && is_rated[m])
			{
				a = new_ratings[m];
				b = user->entries[j].rating;
				dot += a * b;
				mag_a += a * a;
				mag_b += b * b;
				cnt++;
			}
		}
		double denom = sqrt(mag_a) * sqrt(mag_b);
		similarity[i] = (cnt > 1 && denom > 1e-12)? (dot / denom) : 0.0;
	}

	free(new_ratings);
	free(is_rated);
}
