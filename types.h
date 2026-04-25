// header for types.h
#ifndef TYPES_H
#define TYPES_H

typedef struct
{
	int movie;	   // 0-based movie index
	double rating; // raw rating
} RatingEntry;

typedef struct
{
	RatingEntry *entries;
	int size;
	int cap;
	double mean; // optional: cache after load
} UserRatings;

typedef struct
{
	UserRatings *users; // users[0] -> userId 1
	int no_of_users;
	int no_of_movies;
} RatingsStore;

#endif