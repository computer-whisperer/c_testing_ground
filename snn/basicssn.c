#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

float Rm = 10.0;
float Cm = 0.1;
float Vt = 1;

#define SYNAPSES_PER 2
#define NEURONS 5

struct Neuron {
  struct Neuron * pre_neurons[SYNAPSES_PER];
  float pre_weights[SYNAPSES_PER];
  float spike_time;
  float potential;
  float direct_input;
  int force_spike;
};

struct Neuron neurons[NEURONS];

void init_neurons(){
  for (int i = 0; i < NEURONS; i++) {
    neurons[i].spike_time = -100;
    neurons[i].potential = 0.0;
    neurons[i].force_spike = 0;
    for (int k = 0; k < SYNAPSES_PER; k++) {
      neurons[i].pre_weights[k] = (float)rand()/(float)(RAND_MAX/50.0);
      neurons[i].pre_neurons[k] = &(neurons[rand()%NEURONS]);
    }
  }
}

int spikes;
void sim_neuron(struct Neuron * neuron, float t, float dt){
  
  if (neuron->potential > Vt || neuron->force_spike==2) {
    // promote causal spikes
    if (1){
      for (int i = 0; i < SYNAPSES_PER; i++) {
        neuron->pre_weights[i] += 5*exp((t-neuron->pre_neurons[i]->spike_time)/-0.01);
        if (neuron->pre_weights[i] > 50)
          neuron->pre_weights[i] = 50;
      }
    }
    neuron->potential = 0.0;
    neuron->force_spike = 0;
    neuron->spike_time = t;
    spikes++;
  }

  float curr_in = neuron->direct_input;
  neuron->direct_input = 0;
  for (int i = 0; i < SYNAPSES_PER; i++) {
    if (neuron->pre_neurons[i]->potential>Vt || neuron->pre_neurons[i]->force_spike){
      if (neuron->pre_neurons[i]->force_spike)
        neuron->pre_neurons[i]->force_spike = 2;
      if (1) {
        //demote non-causal spikes
        neuron->pre_weights[i] -= 5*exp((t-neuron->spike_time)/-0.01);
        if (neuron->pre_weights[i] < 0)
            neuron->pre_weights[i] = 0;
      }
      curr_in += neuron->pre_weights[i];
    }
  }
  neuron->potential += dt*(curr_in - neuron->potential/Rm)/Cm;
  
}

int main() {
  //srand(time(NULL));
  
  FILE *f = fopen("data.csv", "w");
  init_neurons();

  float dt = 0.001;
  float t = 0.0;
  for (int i = 0; i < 5000; i++){
    t += dt;
    
    for (int j = 0; j < NEURONS; j++)
      sim_neuron(&(neurons[j]), t, dt);

    // CSV update
    if (i%1 == 0) {
      fprintf(f, "%f,", t);
      //for (int j = 0; j < 5; j++){
      //  fprintf(f, "%f,", neurons[j].potential);
      //}
      fprintf(f, "%f,", neurons[0].potential);
      fprintf(f, "%f,", neurons[4].potential);
      fprintf(f, "\n");
    }
    
    // Teaching output
    if (i > 1000 && i < 4000)
      if (i%40 == 0)
        neurons[4].force_spike = 1;

    // Exciting spikes
//    if (i%20 == 0)
//      neurons[0].force_spike = 1;
    neurons[0].direct_input = 4;
  }
  fclose(f);
  printf("Spikes transmitted: %i", spikes);
}
