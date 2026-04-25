// Collaborative Filtering implementation
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#include "types.h"

int make_prediction(double *user, int *similar_users, int no_of_susers, double *similarity, double *utility_matrix, int *recommended_movies, double *predicted_ratings, int No_of_movies)
{
	int i = 0, k = 0;
	int no_of_recommended_movies = 0;
	for (i = 0; i < No_of_movies; i++)
	{ // traverse through each movie
		double sum1 = 0, sum2 = 0;
		int count = 0;
		if (user[i] == 0)
		{ // if not rated by the user
			for (k = 0; k < no_of_susers; k++)
			{ // traverse through similar users
				if (utility_matrix[similar_users[k] * No_of_movies + i] == 0)
					continue; // all similar users who have also rated movie i
				sum1 += similarity[similar_users[k]] * utility_matrix[similar_users[k] * No_of_movies + i];
				sum2 += similarity[similar_users[k]];
				count++;
			}
			if (count > 1)
			{															   // the movie is common between atleast two users otherwise if it only has one user then we will get that same rating
				recommended_movies[no_of_recommended_movies] = i;		   // add movie index to recommended movies
				predicted_ratings[no_of_recommended_movies] = sum1 / sum2; // make prediction
				no_of_recommended_movies++;
			}
		}
	}
	return no_of_recommended_movies;
}

void test_predictions(double *user, int *similar_users, int no_of_susers, double *similarity, double *utility_matrix, double *predicted_ratings, int No_of_movies)
{
	int i = 0, k = 0;
	for (i = 0; i < No_of_movies; i++)
	{
		double sum1 = 0, sum2 = 0;
		int count = 0;
		if (user[i] != 0)
		{
			for (k = 0; k < no_of_susers; k++)
			{
				if (utility_matrix[similar_users[k] * No_of_movies + i] == 0)
					continue;
				sum1 += similarity[similar_users[k]] * utility_matrix[similar_users[k] * No_of_movies + i];
				sum2 += similarity[similar_users[k]];
				count++;
			}
			if (count > 1)
			{
				predicted_ratings[i] = sum1 / sum2;
			}
			else
			{
				predicted_ratings[i] = 0;
			}
		}
		else
		{
			predicted_ratings[i] = 0;
		}
	}
}

int make_prediction_optimized(
    RatingEntry *user,
    int user_size,
    int *similar_users,
    int no_of_susers,
    double *similarity,
    RatingsStore *store,
    int *recommended_movies,
    double *predicted_ratings)
{
    int i, k;
    int no_of_recommended_movies = 0;
    int no_of_movies;

    if (user == NULL || store == NULL || store->users == NULL ||
        similar_users == NULL || similarity == NULL ||
        recommended_movies == NULL || predicted_ratings == NULL ||
        store->no_of_movies <= 0)
    {
        return 0;
    }

    no_of_movies = store->no_of_movies;

    unsigned char *user_rated = (unsigned char *)calloc(no_of_movies, sizeof(unsigned char));
    double *sum1 = (double *)calloc(no_of_movies, sizeof(double));
    double *sum2 = (double *)calloc(no_of_movies, sizeof(double));
    int *count = (int *)calloc(no_of_movies, sizeof(int));

    if (user_rated == NULL || sum1 == NULL || sum2 == NULL || count == NULL)
    {
        free(user_rated);
        free(sum1);
        free(sum2);
        free(count);
        return 0;
    }

    for (i = 0; i < user_size; i++)
    {
        int movie = user[i].movie;
        if (movie >= 0 && movie < no_of_movies)
        {
            user_rated[movie] = 1;
        }
    }

    #pragma omp parallel for schedule(static) default(none) \
        shared(no_of_susers, similar_users, store, similarity, user_rated, no_of_movies) \
        reduction(+:sum1[:no_of_movies], sum2[:no_of_movies], count[:no_of_movies])
    for (k = 0; k < no_of_susers; k++)
    {
        int suid = similar_users[k];
        if (suid < 0 || suid >= store->no_of_users)
        {
            continue;
        }

        double sim = similarity[suid];
        if (sim <= 0.0)
        {
            continue;
        }

        UserRatings *suser = &store->users[suid];
        for (int m = 0; m < suser->size; m++)
        {
            int movie = suser->entries[m].movie;
            if (movie < 0 || movie >= no_of_movies || user_rated[movie])
            {
                continue;
            }

            double rating = suser->entries[m].rating;
            sum1[movie] += sim * rating;
            sum2[movie] += sim;
            count[movie] += 1;
        }
    }

    for (i = 0; i < no_of_movies; i++)
    {
        if (!user_rated[i] && count[i] > 1 && sum2[i] != 0.0)
        {
            recommended_movies[no_of_recommended_movies] = i;
            predicted_ratings[no_of_recommended_movies] = sum1[i] / sum2[i];
            no_of_recommended_movies++;
        }
    }

    free(user_rated);
    free(sum1);
    free(sum2);
    free(count);
    return no_of_recommended_movies;
}