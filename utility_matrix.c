// creates utility matrix
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void get_movie_names(char *movienames, char *s)
{
	char *line, *record;
	char tmp[1024];
	int i = 0, j = 0;
	FILE *fstream = fopen(s, "r");
	if (fstream == NULL)
	{
		perror("Failed to open movie names file");
		return;
	}
	while ((line = fgets(tmp, sizeof(tmp), fstream)) != NULL)
	{								// traverse till end of file while storing each line
		record = strtok(line, ","); // break line into multiple strings separated by comma
		while (record != NULL)
		{
			if (j == 1)
			{ // second string(i.e. moviename in the csv file)
				strcpy(&movienames[i * 1024], record);
			}
			j++;
			record = strtok(NULL, ","); // iterate
		}
		i++;
		j = 0;
	}
	fclose(fstream);
	// free(line);
	// free(record);
}

void get_movie_genres(char *moviegenres, char *s)
{
	char *line, *record;
	char tmp[1024];
	int i = 0, j = 0;
	FILE *fstream = fopen(s, "r");
	if (fstream == NULL)
	{
		perror("Failed to open movie genres file");
		return;
	}
	while ((line = fgets(tmp, sizeof(tmp), fstream)) != NULL)
	{
		record = strtok(line, ",");
		while (record != NULL)
		{
			if (j == 1)
			{
				strcpy(&moviegenres[i * 1024], record);
			}
			j++;
			record = strtok(NULL, ",");
		}
		i++;
		j = 0;
	}
	fclose(fstream);
}

void get_utility_matrix(double *utility_matrix, char *s, int No_of_movies, int No_of_users, int uid)
{
	char *line, *record;
	char tmp[1024];
	int i = 0, j = 0, k = 0;
	(void)uid;
	FILE *fstream = fopen(s, "r");
	if (fstream == NULL)
	{
		perror("Failed to open Dataset/ratings_learn.csv");
		return;
	}
	memset(utility_matrix, 0, sizeof(double) * No_of_users * No_of_movies);
	while ((line = fgets(tmp, sizeof(tmp), fstream)) != NULL)
	{
		record = strtok(line, ",");
		while (record != NULL)
		{
			if (k == 0)
			{ // first string is user id, which will give our row
				i = atoi(record) - 1;
			}
			else if (k == 1)
			{ // second string is movie id which will give our coloumn
				j = atoi(record) - 1;
			}
			else
			{ // third is the actual rating
				if (i >= 0 && i < No_of_users && j >= 0 && j < No_of_movies)
				{
					utility_matrix[i * No_of_movies + j] = atof(record); // converting string to float/double
				}
			}
			record = strtok(NULL, ",");
			k++;
		}
		k = 0;
	}
	fclose(fstream);
	// free(line);
	// free(record);
}

void new_user_movies(double *newuser, char *s, int uid)
{
	char *line, *record;
	char tmp[1024];
	int i, j, k = 0;
	FILE *fstream = fopen(s, "r");
	if (fstream == NULL)
	{
		perror("Failed to open Dataset/ratings_learn.csv");
		return;
	}
	while ((line = fgets(tmp, sizeof(tmp), fstream)) != NULL)
	{
		record = strtok(line, ",");
		while (record != NULL)
		{
			if (k == 0)
			{
				i = atoi(record) - 1;
			}
			if (k == 1)
			{
				j = atoi(record) - 1;
			}
			if (k == 2)
			{
				if (i + 1 == uid)
				{
					if (j >= 0)
					{
						newuser[j] = atof(record);
					}
				}
			}
			k++;
			record = strtok(NULL, ",");
		}
		k = 0;
	}
	fclose(fstream);
}

RatingsStore *load_ratings_optimized(char *s, int No_of_movies, int No_of_users)
{
	char *line, *record;
	char tmp[1024];
	int i = 0, j = 0, k = 0;
	FILE *fstream = fopen(s, "r");
	if (fstream == NULL)
	{
		perror("Failed to open Dataset/ratings_learn.csv");
		return NULL;
	}

	RatingsStore *store = (RatingsStore *)malloc(sizeof(RatingsStore));
	if (store == NULL)
	{
		fclose(fstream);
		return NULL;
	}

	store->users = (UserRatings *)calloc(No_of_users, sizeof(UserRatings));
	if (store->users == NULL)
	{
		free(store);
		fclose(fstream);
		return NULL;
	}
	store->no_of_users = 0;
	store->no_of_movies = No_of_movies;

	while ((line = fgets(tmp, sizeof(tmp), fstream)) != NULL)
	{
		record = strtok(line, ",");
		while (record != NULL)
		{
			if (k == 0)
			{ // first string is user id, which will give our row
				i = atoi(record) - 1;
				if (i >= 0 && i < No_of_users && i >= store->no_of_users)
				{
					store->no_of_users = i + 1; // update user count
				}
			}
			else if (k == 1)
			{ // second string is movie id which will give our coloumn
				j = atoi(record) - 1;
			}
			else
			{ // third is the actual rating
				if (i >= 0 && i < No_of_users && j >= 0 && j < No_of_movies)
				{
					double rating = atof(record);
					UserRatings *user = &store->users[i];
					if (user->size == user->cap)
					{
						int new_cap = user->cap == 0 ? 10 : user->cap * 2;
						RatingEntry *new_entries = (RatingEntry *)realloc(user->entries, sizeof(RatingEntry) * new_cap);
						if (new_entries == NULL)
						{
							free_ratings_optimized(store);
							fclose(fstream);
							return NULL;
						}
						user->entries = new_entries;
						user->cap = new_cap;
					}
					user->entries[user->size].movie = j;
					user->entries[user->size].rating = rating;
					user->size++;
				}
			}
			record = strtok(NULL, ",");
			k++;
		}
		k = 0;
	}
	fclose(fstream);
	return store;
}

void free_ratings_optimized(RatingsStore *store)
{
	int i;

	if (store == NULL)
	{
		return;
	}

	if (store->users != NULL)
	{
		for (i = 0; i < store->no_of_users; i++)
		{
			free(store->users[i].entries);
			store->users[i].entries = NULL;
			store->users[i].size = 0;
			store->users[i].cap = 0;
		}
		free(store->users);
	}

	free(store);
}
