#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#define DO_PRINT_BOARD  1

#define QUEEN_COUNT 100

#define FAST_RAND_MAX 32767

unsigned int g_seed = 1215;

// Compute a pseudorandom integer.
// Output value in range [0, 32767]
int fast_rand(void) {
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16)&0x7FFF;
}

double getUnixTime(void)
{
    struct timespec tv;

    if(clock_gettime(CLOCK_REALTIME, &tv) != 0) return 0;

    return (tv.tv_sec + (tv.tv_nsec / 1000000000.0));
}


int get_collisions(int board[QUEEN_COUNT])
{
  int collisions = 0;
  int i, j;
  
  for (i = 0; i < QUEEN_COUNT; i++)
  {
    for (j = 0; j < QUEEN_COUNT; j++)
    {
      if (i != j)
      {
        if (board[i] == board[j] + (j - i))
          collisions++;
        else if (board[i] == board[j] - (j - i))
          collisions++;
      }
    }
  }
  return collisions;
}

int best_score;
void report_progress(int board[QUEEN_COUNT], int current_cycle,  int current_iteration, int total_iterations) {
  int max_x = QUEEN_COUNT;
  int max_y = QUEEN_COUNT;
  if (max_x > 50)
    max_x = 50;
  if (max_y > 30)
    max_y = 25;
  int current_score = -get_collisions(board);
  printf("\e[1;1H\e[2J\n\n"); // Clear screen
  printf("Board: %d X %d\n", QUEEN_COUNT, QUEEN_COUNT);
  if (current_cycle >= 0)
    printf("Cycle: #%i \n" , current_cycle);
  printf("Current Score = %i \n", current_score);
  printf("Best Score = %i \n", best_score);
  printf("Iterations = %'i / %'i\n\n", current_iteration, total_iterations);

  if (DO_PRINT_BOARD)
  {
    unsigned char x, y;
    for (y = 0; y < max_y; y++)
    {
      for (x = 0; x < max_x; x++)
      {
        putchar('-');
        putchar('-');
        putchar('-');
        putchar('-');
      }
      putchar('-');
      putchar('\n');
      for (x = 0; x < max_x; x++)
      {
        putchar('|');
        putchar(' ');
        if (board[x]==y)
        {
          putchar('Q');
        }
        else
        {
          putchar(' ');
        }
        putchar(' ');
      }
      putchar('|');
      putchar('\n');
    }
    for (x = 0; x < max_x; x++)
    {
      putchar('-');
      putchar('-');
      putchar('-');
      putchar('-');
    }
    putchar('-');
    putchar('\n');
  }
}

int main()
{
  double start_time = getUnixTime();
  
  int board[QUEEN_COUNT];
  int board_backup[QUEEN_COUNT];
  unsigned int i;
  
  // Board init
  for (i = 0; i < QUEEN_COUNT; i++)
  {
    board[i] = i;
  }

  int score = -get_collisions(board);
  best_score = score;

  double last_update = getUnixTime();
  
  int batch_num = 0;
  
  long max_iterations = 10000;
  long iteration;
  long total_iterations = 0;
  while (1) 
  {
    long i;
    for (iteration = 0; iteration < max_iterations; iteration++) {
      
      /* Save board incase we revert */
      for (i = 0; i < QUEEN_COUNT; i++)
      {
        board_backup[i] = board[i];
      }
      
      /* Apply random swap. */
      int rand_col_a = fast_rand()%QUEEN_COUNT;
      int rand_col_b = fast_rand()%QUEEN_COUNT;
      int q = board[rand_col_a];
      board[rand_col_a] = board[rand_col_b];
      board[rand_col_b] = q;
      
      // Score run
      int new_score = -get_collisions(board);

      // Simulated annealing to determine if this passes
      int does_pass = 0;
      if (new_score > score)
        does_pass = 1;
      else {
        float p = exp(-(score - new_score)/(((float)(max_iterations - iteration)/(float)(max_iterations))*1));
        does_pass = fast_rand() < 32767.0 * p;
      }
      
      if (does_pass) {
        if (new_score > best_score)
          best_score = new_score;
        // Keep the new code
        score = new_score;
      }
      else {
        for (i = 0; i < QUEEN_COUNT; i++)
        {
          board[i] = board_backup[i];
        }
      }
      if (!(iteration % 10) && (getUnixTime()-last_update > 0.5)) {
        last_update += 0.5;
        report_progress(board, batch_num, iteration, max_iterations);
      }
      total_iterations++;
      
      if (score >= 0)
      {
        break;
      }
    }
    if (score >= 0) {
      report_progress(board, batch_num, iteration, max_iterations);
      printf("Goal achieved after %f seconds, keep going? (Y/N)", (getUnixTime() - start_time));
      char buff[10];
      gets(buff);
      if (buff[0] != 'Y' && buff[0] != 'y')
        break;
      last_update = getUnixTime();
    }
    max_iterations = max_iterations * 1.5;
    batch_num++;
  }
}
