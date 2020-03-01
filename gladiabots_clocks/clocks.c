#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

unsigned int g_seed = 500;

// Compute a pseudorandom integer.
// Output value in range [0, 32767]
int fast_rand(void) {
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16)&0x7FFF;
}

unsigned long factorial(unsigned long x)
{
	unsigned long out = 1;
	while (x > 1)
	{
		out *= x;
		x--;
	}
	return out;
}

unsigned int bitflip(unsigned int x, unsigned int n)
{
	unsigned int r = 0;
	for (int i = 0; i < n; i++)
	{
	  r |= x & 1;
	  x >>= 1;
	  r <<= 1;
	}
	return r >> 1;
}

void gen_bitmap(unsigned int id, unsigned int n, unsigned int * maps)
{
	int m=id;
    int elems[n];
    for(int i=0;i<n;i++) elems[i]=i;
    printf("Bitmap: ");
    for(int i=0;i<n;i++)
    {
           int ind = m % (n - i);
           m = m / (n - i);
           maps[i] = 1 << elems[ind];
           printf("%d, ", maps[i]);
           elems[ind] = elems[n-i-1];
    }
    putchar('\n');
}

unsigned int permute_by_bitmap(unsigned int x, unsigned int n, unsigned int * maps)
{
	unsigned int r = 0;
	for (int i = 0; i < n; i++)
	{
		if (x & (1 << i))
			r |= maps[(n-1)-i];
	}
	return r;
}


void get_rate_and_deviation(unsigned char * values, unsigned int n, double average_rate, double * average_error)
{
	double total = 0;
	double error = 0;
	double expected_total = 0;
	for (int i = 0; i < n; i++)
	{
		total += values[i];
		expected_total += average_rate;
		error += pow(total - expected_total, 2);
	}
	*average_error = error / (double)n;
}

#define SEQUENCE_LEN 64
#define NUM_BITS 6
#define NUM_ITERATIONS 100000

#define CHAIN_MODE

#define MAX_MUTATION 50

#define RAND_SEED 500


#define MAX_NUM (pow(2, NUM_BITS)-1)
struct Thresholds
{
	int threshold_a;
	int threshold_b;
	int threshold_c;
	int threshold_d;
};

unsigned char apply_thresholds(unsigned int value, struct Thresholds * threshold)
{
	return ((value > threshold->threshold_a) && (value < threshold->threshold_b)) || ((value > threshold->threshold_c) && (value < threshold->threshold_d));
}

void print_ineq(int a, int b)
{
	putchar('(');
	if (a > 0)
	{
		printf("%d < ", a);
	}
	putchar('x');
	if (b < MAX_NUM)
	{
		printf(" < %d", b);
	}
	putchar(')');
}



double find_thresholds(double target_rate, unsigned int * nums, unsigned char print_results, char * description)
{
	if (print_results)
	{
		printf("%s\n", description);
	}
	unsigned char values[SEQUENCE_LEN];
	struct Thresholds current_thresholds;
	struct Thresholds best_thresholds = {MAX_NUM/2, MAX_NUM/2, MAX_NUM/2, MAX_NUM/2};
	double best_cost = 1000.0;
	double average_rate;
	double average_error;
	for (long j = 0; j < NUM_ITERATIONS; j++)
	{
		memcpy(&current_thresholds, &best_thresholds, sizeof(struct Thresholds));

		current_thresholds.threshold_a += (fast_rand()%MAX_MUTATION - MAX_MUTATION/2);
		current_thresholds.threshold_b += (fast_rand()%MAX_MUTATION - MAX_MUTATION/2);
		//current_thresholds.threshold_c += (fast_rand()%MAX_MUTATION - MAX_MUTATION/2);
		//current_thresholds.threshold_d += (fast_rand()%MAX_MUTATION - MAX_MUTATION/2);
		if (current_thresholds.threshold_a > MAX_NUM)
		{
			current_thresholds.threshold_a = MAX_NUM;
		}
		if (current_thresholds.threshold_a < 0)
		{
			current_thresholds.threshold_a = 0;
		}
		if (current_thresholds.threshold_b > MAX_NUM)
		{
			current_thresholds.threshold_b = MAX_NUM;
		}
		if (current_thresholds.threshold_b < 0)
		{
			current_thresholds.threshold_b = 0;
		}
		if (current_thresholds.threshold_c > MAX_NUM)
		{
			current_thresholds.threshold_c = MAX_NUM;
		}
		if (current_thresholds.threshold_c < 0)
		{
			current_thresholds.threshold_c = 0;
		}
		if (current_thresholds.threshold_d > MAX_NUM)
		{
			current_thresholds.threshold_d = MAX_NUM;
		}
		if (current_thresholds.threshold_d < 0)
		{
			current_thresholds.threshold_d = 0;
		}
		if (current_thresholds.threshold_a > current_thresholds.threshold_b)
		{
			current_thresholds.threshold_a = current_thresholds.threshold_b;
		}
		if (current_thresholds.threshold_c > current_thresholds.threshold_d)
		{
			current_thresholds.threshold_c = current_thresholds.threshold_d;
		}

		for (int i = 0; i < SEQUENCE_LEN; i++)
		{
			values[i] = apply_thresholds(nums[i], &current_thresholds);
		}
		get_rate_and_deviation(values, SEQUENCE_LEN, target_rate, &average_error);
		double cost = average_error*10.0;

	    // Simulated annealing to determine if this passes
	    int does_pass = 0;
	    if (cost < best_cost)
	        does_pass = 1;
	    else {
	        float p = exp(-(cost - best_cost)/(((float)(NUM_ITERATIONS - j)/(float)(NUM_ITERATIONS))*0.01));
	        does_pass = fast_rand() < 32767.0 * p;
	    }

	    if (does_pass)
	    {
			best_cost = cost;
			memcpy(&best_thresholds, &current_thresholds, sizeof(struct Thresholds));
		}
	}
	for (int i = 0; i < SEQUENCE_LEN; i++)
	{
		values[i] = apply_thresholds(nums[i], &best_thresholds);
		if (print_results)
		{
			printf("%d", values[i]);
		}
	}
	if (print_results)
	{
		putchar('\n');
	}
	// Merge them?
	if (((best_thresholds.threshold_a <= best_thresholds.threshold_c) && (best_thresholds.threshold_c <= best_thresholds.threshold_b + 1)) ||
		((best_thresholds.threshold_c <= best_thresholds.threshold_a) && (best_thresholds.threshold_a <= best_thresholds.threshold_d + 1)))
	{
		if (best_thresholds.threshold_c < best_thresholds.threshold_a)
		{
			best_thresholds.threshold_a = best_thresholds.threshold_c;
		}
		if (best_thresholds.threshold_d > best_thresholds.threshold_b)
		{
			best_thresholds.threshold_b = best_thresholds.threshold_d;
		}
		best_thresholds.threshold_c = 0;
		best_thresholds.threshold_d = 0;
	}
	if (best_thresholds.threshold_a == best_thresholds.threshold_b)
	{
		best_thresholds.threshold_a = best_thresholds.threshold_c;
		best_thresholds.threshold_b = best_thresholds.threshold_d;
		best_thresholds.threshold_c = 0;
		best_thresholds.threshold_d = 0;
	}
	if (best_thresholds.threshold_c == best_thresholds.threshold_d)
	{
		best_thresholds.threshold_c = 0;
		best_thresholds.threshold_d = 0;
	}
	if (print_results)
	{
		print_ineq(best_thresholds.threshold_a, best_thresholds.threshold_b);
		if (best_thresholds.threshold_c != best_thresholds.threshold_d)
		{
			printf(" || ");
			print_ineq(best_thresholds.threshold_c, best_thresholds.threshold_d);
		}
		putchar('\n');
	}
	get_rate_and_deviation(values, SEQUENCE_LEN, target_rate, &average_error);
	if (print_results)
	{
		printf("Target Rate: %f, Mean Error: %f\n", target_rate, average_error);
		printf("----\n");
	}
	return best_cost;
}

double gen_clocks(unsigned int * nums, unsigned char print_results)
{
	double cost = 0;
	cost += find_thresholds(0.48, nums, print_results, "Assault at mid range");
	cost += find_thresholds(0.28, nums, print_results, "Machine Gun at mid range");
	cost += find_thresholds(0.8, nums, print_results, "Shotgun at mid range");
	cost += find_thresholds(0.4, nums, print_results, "Sniper at mid range");
	cost += find_thresholds(0.343, nums, print_results, "Assault at long range");
	cost += find_thresholds(0.2, nums, print_results, "Machine Gun at long range");
	cost += find_thresholds(0.5714, nums, print_results, "Shotgun at long range");
	cost += find_thresholds(0.2857, nums, print_results, "Sniper at long range");
	cost += find_thresholds(0.3, nums, print_results, "Shield decrement");
	return cost;
}

int main()
{
	unsigned int bitmap[NUM_BITS];
	unsigned int nums[SEQUENCE_LEN];
	double best_cost = 100;
	unsigned int best_permutation_id = 0;


	unsigned long num_perms = factorial(NUM_BITS);
	for (int j = 0; j < num_perms; j++)
	{
		gen_bitmap(j, NUM_BITS, bitmap);
		g_seed = RAND_SEED;
		unsigned int last_num = 0;
		for (int i = 0; i < SEQUENCE_LEN; i++)
		{
#ifdef CHAIN_MODE
			nums[i] = permute_by_bitmap(last_num+1, NUM_BITS, bitmap);
#else
			nums[i] = permute_by_bitmap(i, NUM_BITS, bitmap);
#endif
			last_num = nums[i];
		}
		double cost = gen_clocks(nums, 0);
		printf("Cost: %f\n---\n", cost);

		if (cost < best_cost)
		{
			best_permutation_id = j;
			best_cost = cost;
		}
	}

	gen_bitmap(best_permutation_id, NUM_BITS, bitmap);
	g_seed = RAND_SEED;
	unsigned int last_num = 0;
	printf("Source sequence:\n");
	for (int i = 0; i < SEQUENCE_LEN; i++)
	{
		if (i > 0)
		{
			printf(", ");
		}

#ifdef CHAIN_MODE
			nums[i] = permute_by_bitmap(last_num+1, NUM_BITS, bitmap);
#else
			nums[i] = permute_by_bitmap(i, NUM_BITS, bitmap);
#endif
		last_num = nums[i];
		printf("%d", nums[i]);
	}
	printf("\n----\n");

	double cost = gen_clocks(nums, 1);
	printf("Resulting cost: %f\n", cost);
}
