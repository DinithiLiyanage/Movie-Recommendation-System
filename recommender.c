// Contains all logic and function calls
#include <stdio.h>
#include <stdlib.h>
// #include <conio.h>
#include <time.h>
#include <string.h>

#include "utility_matrix.h"
#include "matrix_normalization.h"
#include "pearsons.h"
#include "kmeans.h"
#include "predictions.h"
#include "sorting.h"

#define No_of_movies 9125

static const char *RATINGS_LEARN_PATH = "Dataset/ratings_learn.csv";
static const char *MOVIES_PATH = "Dataset/movies.csv";
static const char *MOVIES_GENRES_PATH = "Dataset/movies_genres.csv";

int findusers()
{
	char *line, *record;
	char tmp[1024];
	FILE *fstream = fopen(RATINGS_LEARN_PATH, "r");
	if (fstream == NULL)
	{
		perror("Failed to open ratings dataset");
		return 0;
	}
	int j = 0;
	int max = 0;
	while ((line = fgets(tmp, sizeof(tmp), fstream)) != NULL)
	{
		record = strtok(line, ",");
		while (record != NULL)
		{
			if (j == 0)
			{
				int t = atoi(record);
				if (t > max)
					max = t;
			}
			j++;
			record = strtok(NULL, ","); // iterate
		}
		j = 0;
	}
	fclose(fstream);
	return max;
}

void recommender(int uid)
{
	int No_of_users = findusers();
	int i = 0, l = 0;
	int choice;
	double time_taken;
	int userid = uid;
	clock_t t;
	t = clock();
	int k = 16; // number of clusters
	// printf("Choose k: "); scanf("%d",&k); //taking input to test various values for k and deciding which is optimal
	char *movienames = (char *)malloc(sizeof(char) * No_of_movies * 1024);
	char *moviegenres = (char *)malloc(sizeof(char) * No_of_movies * 1024);

	RatingsStore *utility_matrix = load_ratings_optimized((char *)RATINGS_LEARN_PATH, No_of_movies, No_of_users);

	get_movie_names(movienames, (char *)MOVIES_PATH);		   // getting movie names according to movie id(index)
	get_movie_genres(moviegenres, (char *)MOVIES_GENRES_PATH); // getting movie genres according to movie id(index)

	normalize_ratings_optimized(utility_matrix); // normalizing the utility matrix for similarity calculations

	RatingEntry *newuser = utility_matrix->users[userid - 1].entries; // getting the new user's ratings from the utility matrix

	double *similarity = malloc(sizeof(double) * No_of_users);
	calc_similarity_optimized(newuser, utility_matrix->users[userid - 1].size, utility_matrix, similarity); // calculating similarity between new user and users present in dataset

	/*
	 * Existing k-means based similar user selection (kept for reference, intentionally disabled).
	 *
	 * double *centroids = (double *)malloc(sizeof(double) * k);
	 * int *cluster_assignment = (int *)malloc(sizeof(int) * No_of_users);
	 * for (i = 0; i < k; i++) {
	 *     int n = rand() % No_of_users;
	 *     int m = 0, flag = 0;
	 *     for (m = 0; m < i; m++) {
	 *         if (similarity[n] == centroids[m]) {
	 *             flag = 1;
	 *             break;
	 *         }
	 *     }
	 *     if (flag == 1) {
	 *         i--;
	 *         continue;
	 *     }
	 *     centroids[i] = similarity[n];
	 * }
	 * kmeans(1, similarity, No_of_users, k, centroids, cluster_assignment);
	 * int *similar_users = malloc(sizeof(int) * No_of_users);
	 * int no_of_susers = 0;
	 * double max = 0;
	 * int maxid = 0;
	 * for (i = 0; i < k; i++) {
	 *     if (centroids[i] > max && centroids[i] < 0.3) {
	 *         max = centroids[i];
	 *         maxid = i;
	 *     }
	 * }
	 * for (i = 0; i < No_of_users; i++) {
	 *     if (cluster_assignment[i] == maxid) {
	 *         similar_users[no_of_susers] = i;
	 *         no_of_susers++;
	 *     }
	 * }
	 */

	int top_k = k;
	if (top_k > No_of_users - 1)
	{
		top_k = No_of_users - 1;
	}
	if (top_k < 1)
	{
		top_k = 1;
	}

	int *similar_users = malloc(sizeof(int) * top_k);
	double *top_similarities = malloc(sizeof(double) * top_k);
	int no_of_susers = 0;

	for (i = 0; i < top_k; i++)
	{
		similar_users[i] = -1;
		top_similarities[i] = -1.0e18;
	}

	for (i = 0; i < No_of_users; i++)
	{
		if (i == userid - 1)
		{
			continue;
		}
		if (similarity[i] <= 0.0)
		{
			continue;
		}

		int min_idx = 0;
		double min_val = top_similarities[0];
		int t_idx;
		for (t_idx = 1; t_idx < top_k; t_idx++)
		{
			if (top_similarities[t_idx] < min_val)
			{
				min_val = top_similarities[t_idx];
				min_idx = t_idx;
			}
		}

		if (similarity[i] > min_val)
		{
			top_similarities[min_idx] = similarity[i];
			similar_users[min_idx] = i;
		}
	}

	for (i = 0; i < top_k; i++)
	{
		if (similar_users[i] != -1)
		{
			similar_users[no_of_susers] = similar_users[i];
			no_of_susers++;
		}
	}

	free(top_similarities);

	int *recommended_movies = malloc(sizeof(int) * No_of_movies);	   // array containing index of recommended movies(whose ratings were predicted)
	double *predicted_ratings = malloc(sizeof(double) * No_of_movies); // array of ratings of those recommended movies
	int no_of_recommended_movies = 0;

	no_of_recommended_movies = make_prediction_optimized(newuser, utility_matrix->users[userid - 1].size, similar_users, no_of_susers, similarity, utility_matrix, recommended_movies, predicted_ratings); // making predictions and saving them in recommended_movies and predicted_ratings arrays

	for (i = 0; i < no_of_recommended_movies; i++)
	{ // printing the recommended movies to get an idea (movies whose ratings was calculated)
		printf("Rating for movie %d: %.1lf\n", recommended_movies[i] + 1, predicted_ratings[i]);
	}

	sort(recommended_movies, predicted_ratings, no_of_recommended_movies); // sorting the recommended movies in decreasing order according to their predicted ratings

	printf("Top 10 movies recommended for you: \n");

	for (i = 0; i < 10; i++)
	{									   // selecting top 10 movies from the recommended movies
		if (i == no_of_recommended_movies) // if number of recommended movies is less than 10 then we will have to end loop early
		{
			printf("\nSorry these are the only movies that can be recommended based on input.\nPlease enter more input.\n\n");
			break;
		}
		printf("%d. %s %s", i + 1, &movienames[recommended_movies[i] * 1024], &moviegenres[recommended_movies[i] * 1024]);
	}
	t = clock() - t;
	time_taken = ((double)t) / CLOCKS_PER_SEC;
	printf("\nTime taken to process: %.2lf seconds\n", time_taken);

	// freeing the memory
	free_ratings_optimized(utility_matrix);
	free(movienames);
	free(moviegenres);
	free(similarity);
	free(similar_users);
	free(recommended_movies);
	free(predicted_ratings);

	// THE END
}
