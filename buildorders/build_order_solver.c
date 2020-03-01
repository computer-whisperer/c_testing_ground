#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "build_order_types.h"
#include "utilities.h"

void print_build_order(enum BUILD_ACTION_T * build_order)
{
  printf("\nBUILD ORDER:\n");
  
  int step_num = 0;
  int same_order_count = 1;
  enum BUILD_ACTION_T last_action = BUILD_ACTION_NONE;
  while (last_action != BUILD_ACTION_END_BUILD)
  {
    if (*build_order == last_action)
    {
      same_order_count++;
    }
    else if (last_action != BUILD_ACTION_NONE)
    {
      printf("%3d: %s (%dx)\n", step_num, build_action_strings[last_action], same_order_count);
      step_num++;
      same_order_count = 1;
    }
    last_action = *build_order;
    build_order++;
  }
}

float estimate_survival_margin(struct GAMESTATE_T * state, int use_defenses);

void schedule_event(struct GAMESTATE_T * state, struct EVENT_T * event)
{
  event->time_us = event->period_us + state->time_us;
  struct EVENT_T * considering_event = state->next_event;
  
  if (!considering_event || considering_event->time_us > event->time_us)
  {
    event->next_event_in_time = considering_event;
    state->next_event = event;
  }
  else
  {
    while(considering_event)
    {
      if (!considering_event->next_event_in_time || considering_event->next_event_in_time->time_us > event->time_us)
      {
        event->next_event_in_time = considering_event->next_event_in_time;
        considering_event->next_event_in_time = event;
        break;
      }
      considering_event = considering_event->next_event_in_time;
    }
  }
}

void unschedule_event(struct GAMESTATE_T * state, struct EVENT_T * event)
{
  struct EVENT_T * considering_event = state->next_event;
  if (considering_event == event)
  {
    state->next_event = considering_event->next_event_in_time;
  }
  else
  {
    while (considering_event)
    {
        if (considering_event->next_event_in_time == event)
        {
          considering_event->next_event_in_time = considering_event->next_event_in_time->next_event_in_time;
          break;
        }
        considering_event = considering_event->next_event_in_time;
    }
  }
  
  event->next_event_in_time = NULL;

}

struct EVENT_T * create_event(struct GAMESTATE_T * state, int period_us, enum EVENT_TYPE_T event_type,  struct STRUCTURE_T * structure, enum BUILD_ACTION_T action)
{
  
  struct EVENT_T * new_event = malloc(sizeof(struct EVENT_T));
  new_event->period_us = period_us;
  new_event->event_type = event_type;
  new_event->structure = structure;
  new_event->action = action;
  
  if (structure)
  {
    new_event->next_event_in_structure = structure->first_event;
    structure->first_event = new_event;
  }
  
  schedule_event(state, new_event);
  
  return new_event;
}

void delete_event(struct GAMESTATE_T * state, struct EVENT_T * event_to_delete)
{
  unschedule_event(state, event_to_delete);
  
  struct EVENT_T * considering_event;
  if (event_to_delete->structure)
  {
    considering_event = event_to_delete->structure->first_event;
    if (considering_event == event_to_delete)
    {
      event_to_delete->structure->first_event = considering_event->next_event_in_structure;
    }
    else
    {
      while (considering_event)
      {
          if (considering_event->next_event_in_structure == event_to_delete)
          {
            considering_event->next_event_in_structure = considering_event->next_event_in_structure->next_event_in_structure;
            break;
          }
          considering_event = considering_event->next_event_in_structure;
      }
    }
  }
  free(event_to_delete);
}

void print_event(struct EVENT_T * event)
{
  int secs = event->time_us / 1000000;
  int mins = secs / 60;
  secs = secs % 60;
  printf("    Event: Time  %d:%02d, Type %s\n", mins, secs, event_type_strings[event->event_type]);
}


struct STRUCTURE_T * create_structure(struct GAMESTATE_T * state, enum STRUCTURE_TYPE_T type, enum STRUCTURE_STATE_T struct_state)
{
  struct STRUCTURE_T * new_structure = malloc(sizeof(struct STRUCTURE_T));
  new_structure->time_us = state->time_us;
  new_structure->type = type;
  new_structure->state = struct_state;
  new_structure->next = state->first_structure;
  new_structure->first_event = NULL;
  state->first_structure = new_structure;
  
  int i;
  for (i = 0; i < STRUCTURE_UPGRADE_NUM; i++)
  {
    new_structure->upgrades[i] = 0;
  }
  
  return new_structure;
}


void print_structure(struct STRUCTURE_T * structure)
{
  int secs = structure->time_us / 1000000ULL;
  int mins = secs / 60;
  secs = secs % 60;
  printf("(%02d:%02d) %s", mins, secs, structure_type_strings[structure->type]);
  switch(structure->state)
  {
    case STRUCTURE_STATE_BUILDING:
      printf(" (Building)");
      break;
    case STRUCTURE_STATE_CONVERT_TO_GAS:
      printf(" (Gas)"); 
      break;
    case STRUCTURE_STATE_CONVERT_TO_MINERALS:
      printf(" (Minerals)"); 
      break;
    case STRUCTURE_STATE_CONVERTER_OFF:
      printf(" (Off)"); 
      break;
    default:
      break;
  }
  if (structure->upgrades[STRUCTURE_UPGRADE_LEVEL])
  {
    printf(" (Level=%d)", structure->upgrades[STRUCTURE_UPGRADE_LEVEL]);
  }  
  if (structure->upgrades[STRUCTURE_UPGRADE_SPENT])
  {
    printf(" (Spent)");
  }
  printf("\n");
}

void print_event_stack(struct EVENT_T * next_event)
{
  while(next_event)
  {
    print_event(next_event);
    next_event = next_event->next_event_in_time;
  }
}

struct STRUCTURE_T * find_structure(struct GAMESTATE_T * state, enum STRUCTURE_TYPE_T type)
{
  struct STRUCTURE_T * next_struct = state->first_structure;
  while (next_struct)
  {
    if (next_struct->type == type)
      return next_struct;
    next_struct = next_struct->next;
  }
  return NULL;
}

struct STRUCTURE_T * find_structure_with_state(struct GAMESTATE_T * state, enum STRUCTURE_TYPE_T type, enum STRUCTURE_STATE_T struct_state)
{
  struct STRUCTURE_T * next_struct = state->first_structure;
  while (next_struct)
  {
    if (next_struct->type == type && next_struct->state == struct_state)
      return next_struct;
    next_struct = next_struct->next;
  }
  return NULL;
}

int try_process_action(struct GAMESTATE_T * state, enum BUILD_ACTION_T build_action)
{
  struct STRUCTURE_T * structure = NULL;
  struct EVENT_T * barracks_event;
  unsigned char completed = 0;
  int has_struct;
  int is_building;
  switch (build_action)
  {
    case BUILD_ACTION_NONE:
      completed = 1;
      break;
    case BUILD_ACTION_UPGRADE_SENTRY_SUPERCHARGE:      
      structure = find_structure(state, STRUCTURE_TYPE_CONSTRUCTION_YARD);
      if (!structure || state->upgrades[WORLD_UPGRADE_SENTRY_SUPERCHARGE] != 0 || (state->upgrades[WORLD_UPGRADE_FORTIFICATION] < 5 && structure->state == STRUCTURE_STATE_NORMAL))
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->gas >= 200 && state->minerals >= 50)
      {
        state->gas -= 200;
        state->minerals -= 50;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_SENTRY_SUPERCHARGE] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_FORTIFICATION:      
      structure = find_structure(state, STRUCTURE_TYPE_CONSTRUCTION_YARD);
      if (!structure || state->upgrades[WORLD_UPGRADE_FORTIFICATION] == 30)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->gas >= 140 + state->upgrades[WORLD_UPGRADE_FORTIFICATION]*10)
      {
        state->gas -= 140 + state->upgrades[WORLD_UPGRADE_FORTIFICATION]*1;
        structure->state = STRUCTURE_STATE_WORKING;
        create_event(state, 5000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_SENTRY_GUN:
      if (state->upgrades[WORLD_UPGRADE_CONSTRUCTION_YARD] == 0 || state->live_scv_count == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (state->upgrades[WORLD_UPGRADE_CONSTRUCTION_YARD] == 2 && state->gas >= 40 &&  state->minerals >= 60 && state->free_scv_count > 0)
      {
        state->gas -= 40;
        state->minerals -= 60;
        state->free_scv_count--;
        create_event(state, 15000000ULL, EVENT_TYPE_BUILD_COMPLETE, NULL, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_TAKE_NEAR_BASE:
      if (state->near_front_base_state > 0)
        completed = 1;
      else
      {
        state->near_front_base_state = 1;
        state->in_progress_refinery_spots_left += NEAR_FRONT_BASE_REFINERY_SPOTS;
        state->in_progress_large_spots_left += NEAR_FRONT_BASE_LARGE_SPOTS;
        state->in_progress_small_spots_left += NEAR_FRONT_BASE_SMALL_SPOTS;
        create_event(state, 3000000, EVENT_TYPE_UPGRADE_COMPLETE, NULL, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_TAKE_FAR_BASE:
      if (state->far_front_base_state > 0)
        completed = 1;
      else if (state->near_front_base_state == 0 || state->near_front_base_state == 2)
      {
        state->far_front_base_state = 1;
        // Take them simultaneously if we havent take near base yet.
        if (state->near_front_base_state == 0)
        {        
          state->near_front_base_state = 1;
          state->in_progress_refinery_spots_left += NEAR_FRONT_BASE_REFINERY_SPOTS;
          state->in_progress_large_spots_left += NEAR_FRONT_BASE_LARGE_SPOTS;
          state->in_progress_small_spots_left += NEAR_FRONT_BASE_SMALL_SPOTS;
        }
        state->in_progress_refinery_spots_left += FAR_FRONT_BASE_REFINERY_SPOTS;
        state->in_progress_large_spots_left += FAR_FRONT_BASE_LARGE_SPOTS;
        state->in_progress_small_spots_left += FAR_FRONT_BASE_SMALL_SPOTS;
        create_event(state, 5000000, EVENT_TYPE_UPGRADE_COMPLETE, NULL, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_WALL:
      if (state->front_wall_state > 0 || state->live_scv_count < 3 || state->far_front_base_state == 0)
        completed = 1;
      else if (state->far_front_base_state == 2 && state->free_scv_count >= 3 && (state->gas >= 120 || state->upgrades[WORLD_UPGRADE_ADVANCED_BUILD] == 2))
      {
        if (state->upgrades[WORLD_UPGRADE_ADVANCED_BUILD] < 2)
        {
          state->gas -= 120;
        }
        state->front_wall_state = 1;
        state->free_scv_count -= 3;
        state->live_scv_count -= 1;
        create_event(state, 30000000, EVENT_TYPE_UPGRADE_COMPLETE, NULL, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BACK_ROCKS_CLIP:
      if (state->back_rocks_state > 0 || state->live_scv_count == 0)
        completed = 1;
      else if (state->free_scv_count > 0)
      {
        state->back_rocks_state = 1;
        state->free_scv_count -= 1;
        state->live_scv_count -= 1;
        state->in_progress_refinery_spots_left += BACK_BASE_REFINERY_SPOTS;
        state->in_progress_large_spots_left += BACK_BASE_LARGE_SPOTS;
        state->in_progress_small_spots_left += BACK_BASE_SMALL_SPOTS;
        create_event(state, 30000000, EVENT_TYPE_UPGRADE_COMPLETE, NULL, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_CHEATER_BASE_CLIP:
      if (state->cheater_base_state > 0 || state->live_scv_count == 0)
        completed = 1;
      else if (state->free_scv_count > 0)
      {
        state->cheater_base_state = 1;
        state->free_scv_count -= 1;
        state->live_scv_count -= 1;
        state->in_progress_large_spots_left += CHEATER_BASE_LARGE_SPOTS;
        state->in_progress_small_spots_left += CHEATER_BASE_SMALL_SPOTS;
        state->in_progress_refinery_spots_left += CHEATER_BASE_REFINERY_SPOTS;
        create_event(state, 60000000, EVENT_TYPE_UPGRADE_COMPLETE, NULL, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_SCV:
      if (state->gas >= 10)
      {
        state->free_scv_count++;
        state->live_scv_count++;
        state->gas -= 10;
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_BASIC_SHIELD_BATTERY:
      if (state->gas >= 25)
      {
        state->shield_battery_max_charge += 700;
        state->gas -= 25;
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_ADVANCED_SHIELD_BATTERY:
      if (state->gas >= 50 && state->minerals >= 30)
      {
        state->shield_battery_max_charge += 2500;
        state->gas -= 50;
        state->minerals -= 30;
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_REFINERY:
      if (state->refinery_spots_left + state->in_progress_large_spots_left == 0 || state->live_scv_count == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (state->refinery_spots_left > 0 && state->gas >= 20 && state->free_scv_count > 0)
      {
        state->refinery_spots_left--;
        state->gas -= 20;
        state->free_scv_count--;
        struct STRUCTURE_T * ref = create_structure(state, STRUCTURE_TYPE_REFINERY, STRUCTURE_STATE_BUILDING);
        create_event(state, 20000000ULL, EVENT_TYPE_BUILD_COMPLETE, ref, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_REFINERY:
      structure = state->first_structure;
      while (structure)
      {
        if (structure->type == STRUCTURE_TYPE_REFINERY && structure->upgrades[STRUCTURE_UPGRADE_LEVEL] == 0)
          break;
        structure = structure->next;
      }
      if (!structure)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->minerals >= 80)
      {
        state->minerals -= 80;
        structure->state = STRUCTURE_STATE_WORKING;
        create_event(state, 45000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_FRUIT_FARM:
      if (state->large_spots_left + state->in_progress_large_spots_left == 0 || state->small_spots_left < 9 || state->live_scv_count == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (state->large_spots_left > 0 && state->gas >= 30 && state->free_scv_count > 0)
      {
        state->large_spots_left -= 1;
        state->small_spots_left -= 9;
        state->gas -= 30;
        state->free_scv_count--;
        struct STRUCTURE_T * ref = create_structure(state, STRUCTURE_TYPE_FRUIT_FARM, STRUCTURE_STATE_BUILDING);
        create_event(state, 20000000ULL, EVENT_TYPE_BUILD_COMPLETE, ref, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_CONSTRUCTION_YARD:
      if (state->upgrades[WORLD_UPGRADE_CONSTRUCTION_YARD] || state->live_scv_count == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (state->gas >= 100 && state->minerals >= 50 && state->free_scv_count > 0)
      {
        state->gas -= 100;
        state->minerals -= 50;
        state->free_scv_count--;
        state->upgrades[WORLD_UPGRADE_CONSTRUCTION_YARD] = 1;
        if (state->upgrades[WORLD_UPGRADE_ANY_TECH_STRUCTURE] == 0)
          state->upgrades[WORLD_UPGRADE_ANY_TECH_STRUCTURE] = 1;
        struct STRUCTURE_T * ref = create_structure(state, STRUCTURE_TYPE_CONSTRUCTION_YARD, STRUCTURE_STATE_BUILDING);
        create_event(state, 30000000ULL, EVENT_TYPE_BUILD_COMPLETE, ref, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_ARMORY:
      if (state->upgrades[WORLD_UPGRADE_ARMORY] || state->live_scv_count == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (state->gas >= 100 && state->minerals >= 50 && state->free_scv_count > 0)
      {
        state->gas -= 100;
        state->minerals -= 50;
        state->free_scv_count--;
        state->upgrades[WORLD_UPGRADE_ARMORY] = 1;
        if (state->upgrades[WORLD_UPGRADE_ANY_TECH_STRUCTURE] == 0)
          state->upgrades[WORLD_UPGRADE_ANY_TECH_STRUCTURE] = 1;
        struct STRUCTURE_T * ref = create_structure(state, STRUCTURE_TYPE_ARMORY, STRUCTURE_STATE_BUILDING);
        create_event(state, 30000000ULL, EVENT_TYPE_BUILD_COMPLETE, ref, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_TECH_LAB:
      if (state->upgrades[WORLD_UPGRADE_TECH_LAB] || state->live_scv_count == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (state->gas >= 100 && state->minerals >= 50 && state->free_scv_count > 0)
      {
        state->gas -= 100;
        state->minerals -= 50;
        state->free_scv_count--;
        state->upgrades[WORLD_UPGRADE_TECH_LAB] = 1;
        if (state->upgrades[WORLD_UPGRADE_ANY_TECH_STRUCTURE] == 0)
          state->upgrades[WORLD_UPGRADE_ANY_TECH_STRUCTURE] = 1;
        struct STRUCTURE_T * ref = create_structure(state, STRUCTURE_TYPE_TECH_LAB, STRUCTURE_STATE_BUILDING);
        create_event(state, 30000000ULL, EVENT_TYPE_BUILD_COMPLETE, ref, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_COW_FARM:
    case BUILD_ACTION_BUILD_COW_FARM_AND_COW:
      if (state->small_spots_left + state->in_progress_small_spots_left == 0  || state->upgrades[WORLD_UPGRADE_CONSTRUCTION_YARD] == 0 || state->live_scv_count == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (state->small_spots_left > 0 && state->upgrades[WORLD_UPGRADE_CONSTRUCTION_YARD] == 2 && state->gas >= 60 &&  state->minerals >= 30 && state->free_scv_count > 0)
      {
        state->small_spots_left -= 1;
        state->gas -= 60;
        state->minerals -= 30;
        state->free_scv_count--;
        struct STRUCTURE_T * ref = create_structure(state, STRUCTURE_TYPE_COW_FARM, STRUCTURE_STATE_BUILDING);
        create_event(state, 30000000ULL, EVENT_TYPE_BUILD_COMPLETE, ref, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_COW:
      // Check for Cow Farm
      structure = state->first_structure;
      has_struct = 0;
      is_building = 1;
      while (structure)
      {
        if (structure->type == STRUCTURE_TYPE_COW_FARM && structure->upgrades[STRUCTURE_UPGRADE_SPENT] == 0)
        {
          has_struct = 1;
          if (structure->state != STRUCTURE_STATE_BUILDING)
            is_building = 0;
        }
        if (has_struct && !is_building)
          break;
        structure = structure->next;
      }
      if (!has_struct)
      {
        // Deadlock, skip.
        completed = 1;
        break;
      }
      if (is_building)
        break;
      if (state->minerals >= 15)
      {
        state->minerals -= 15;
        structure->upgrades[STRUCTURE_UPGRADE_SPENT] = 1;
        struct STRUCTURE_T * ref = create_structure(state, STRUCTURE_TYPE_COW, STRUCTURE_STATE_BUILDING);
        create_event(state, 5000000ULL, EVENT_TYPE_BUILD_COMPLETE, ref, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_SHEEP_FARM:
    case BUILD_ACTION_BUILD_SHEEP_FARM_AND_SHEEP:
      if (state->small_spots_left + state->in_progress_small_spots_left == 0  || state->live_scv_count == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (state->small_spots_left > 0 && state->gas >= 40 && state->free_scv_count > 0)
      {
        state->small_spots_left -= 1;
        state->gas -= 40;
        state->free_scv_count--;
        struct STRUCTURE_T * ref = create_structure(state, STRUCTURE_TYPE_SHEEP_FARM, STRUCTURE_STATE_BUILDING);
        create_event(state, 20000000ULL, EVENT_TYPE_BUILD_COMPLETE, ref, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_SHEEP:
      // Check for Sheep Farm
      structure = state->first_structure;
      has_struct = 0;
      is_building = 1;
      while (structure)
      {
        if (structure->type == STRUCTURE_TYPE_SHEEP_FARM && structure->upgrades[STRUCTURE_UPGRADE_SPENT] == 0)
        {
          has_struct = 1;
          if (structure->state != STRUCTURE_STATE_BUILDING)
            is_building = 0;
        }
        if (has_struct && !is_building)
          break;
        structure = structure->next;
      }
      if (!has_struct)
      {
        // Deadlock, skip.
        completed = 1;
        break;
      }
      if (is_building)
        break;
      if (state->minerals >= 5)
      {
        state->minerals -= 5;
        structure->upgrades[STRUCTURE_UPGRADE_SPENT] = 1;
        struct STRUCTURE_T * ref = create_structure(state, STRUCTURE_TYPE_SHEEP, STRUCTURE_STATE_BUILDING);
        create_event(state, 5000000ULL, EVENT_TYPE_BUILD_COMPLETE, ref, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_SHEEP_FARM:
      // Check for any tech structure
      if (!state->upgrades[WORLD_UPGRADE_ANY_TECH_STRUCTURE])
      {
        // Deadlock, skip.
        completed = 1;
        break;
      }
      
      // We have a finished tech structure, find a sheep farm
      has_struct = 0;
      is_building = 1;
      while (structure)
      {
        if (structure->type == STRUCTURE_TYPE_SHEEP_FARM && structure->upgrades[STRUCTURE_UPGRADE_LEVEL] == 0)
        {
          has_struct = 1;
          if (structure->state == STRUCTURE_STATE_NORMAL)
            is_building = 0;
        }
        if (has_struct && !is_building)
          break;
        structure = structure->next;
      }
      if (!has_struct)
      {
        // Deadlock, skip.
        completed = 1;
        break;
      }
      if (is_building)
        break;
        
      // All clear, purchase upgrade
      if (state->upgrades[WORLD_UPGRADE_ANY_TECH_STRUCTURE] == 2 && state->minerals >= 20 &&  state->gas >= 20)
      {
        state->gas -= 20;
        state->minerals -= 20;
        structure->state = STRUCTURE_STATE_WORKING;
        create_event(state, 0000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_ADVANCED_BUILD:
      structure = find_structure(state, STRUCTURE_TYPE_CONSTRUCTION_YARD);
      if (!structure || state->upgrades[WORLD_UPGRADE_ADVANCED_BUILD] != 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->gas >= 100)
      {
        state->gas -= 100;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_ADVANCED_BUILD] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_UNIFIED_ARMOR:
      structure = find_structure(state, STRUCTURE_TYPE_CONSTRUCTION_YARD);
      if (!structure || state->upgrades[WORLD_UPGRADE_UNIFIED_ARMOR] != 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->gas >= 75)
      {
        state->gas -= 75;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_UNIFIED_ARMOR] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_IMPROVED_FARMING:
      structure = find_structure(state, STRUCTURE_TYPE_CONSTRUCTION_YARD);
      if (!structure || state->upgrades[WORLD_UPGRADE_IMPROVED_FARMING] != 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->gas >= 400)
      {
        state->gas -= 400;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_IMPROVED_FARMING] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_CHEMICAL_PLANT:
      structure = find_structure(state, STRUCTURE_TYPE_CONSTRUCTION_YARD);
      if (!structure || state->upgrades[WORLD_UPGRADE_CHEMICAL_PLANT] != 0 || state->upgrades[WORLD_UPGRADE_ADVANCED_BUILD] == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->gas >= 600)
      {
        state->gas -= 600;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_CHEMICAL_PLANT] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_LEVEL_2_WEAPONS:
      structure = find_structure(state, STRUCTURE_TYPE_TECH_LAB);
      if (!structure || state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] != 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->gas >= 250)
      {
        state->gas -= 250;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_LEVEL_3_WEAPONS:
      structure = find_structure(state, STRUCTURE_TYPE_TECH_LAB);
      if (!structure || state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] == 0 || state->upgrades[WORLD_UPGRADE_LEVEL_3_WEAPONS] != 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] == 2 && state->gas >= 600)
      {
        state->gas -= 600;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_LEVEL_3_WEAPONS] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_CONVERTER:
      if (state->small_spots_left + state->in_progress_small_spots_left == 0 || state->upgrades[WORLD_UPGRADE_ADVANCED_BUILD] == 0  || state->live_scv_count == 0)
      {
        completed = 1;
      }
      else if (state->small_spots_left > 0 && state->upgrades[WORLD_UPGRADE_ADVANCED_BUILD] == 2 && state->minerals >= 100 && state->free_scv_count > 0)
      {
        state->small_spots_left -= 1;
        state->minerals -= 100;
        state->free_scv_count--;
        struct STRUCTURE_T * ref = create_structure(state, STRUCTURE_TYPE_CONVERTER, STRUCTURE_STATE_BUILDING);
        create_event(state, 20000000ULL, EVENT_TYPE_BUILD_COMPLETE, ref, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_SET_CONVERTER_GAS:
      structure = find_structure_with_state(state, STRUCTURE_TYPE_CONVERTER, STRUCTURE_STATE_CONVERTER_OFF);
      if (!structure)
      {
        structure = find_structure_with_state(state, STRUCTURE_TYPE_CONVERTER, STRUCTURE_STATE_CONVERT_TO_MINERALS);
      }
      if (structure)
      {
        structure->state = STRUCTURE_STATE_CONVERT_TO_GAS;
        
      }
      completed = 1;
      break;
    case BUILD_ACTION_SET_CONVERTER_MINERALS:
      structure = find_structure_with_state(state, STRUCTURE_TYPE_CONVERTER, STRUCTURE_STATE_CONVERTER_OFF);
      if (!structure)
      {
        structure = find_structure_with_state(state, STRUCTURE_TYPE_CONVERTER, STRUCTURE_STATE_CONVERT_TO_GAS);
      }
      if (structure)
      {
        structure->state = STRUCTURE_STATE_CONVERT_TO_MINERALS;
      }
      completed = 1;
      break;
    case BUILD_ACTION_SET_CONVERTER_NONE:
      structure = find_structure_with_state(state, STRUCTURE_TYPE_CONVERTER, STRUCTURE_STATE_CONVERT_TO_GAS);
      if (!structure)
      {
        structure = find_structure_with_state(state, STRUCTURE_TYPE_CONVERTER, STRUCTURE_STATE_CONVERT_TO_MINERALS);
      }
      if (structure)
      {
        structure->state = STRUCTURE_STATE_CONVERTER_OFF;
      }
      completed = 1;
      break;
    case BUILD_ACTION_BUILD_CHEMICAL_PLANT:
      if (state->small_spots_left + state->in_progress_small_spots_left == 0 || state->upgrades[WORLD_UPGRADE_CHEMICAL_PLANT] == 0 || state->live_scv_count == 0)
      {
        completed = 1;
      }
      else if (state->small_spots_left > 0 && state->upgrades[WORLD_UPGRADE_CHEMICAL_PLANT] == 2 && state->minerals >= 1000 && state->gas >= 200 && state->free_scv_count > 0)
      {
        state->small_spots_left -= 1;
        state->minerals -= 1000;
        state->gas -= 200;
        state->free_scv_count--;
        struct STRUCTURE_T * ref = create_structure(state, STRUCTURE_TYPE_CHEMICAL_PLANT, STRUCTURE_STATE_BUILDING);
        create_event(state, 30000000ULL, EVENT_TYPE_BUILD_COMPLETE, ref, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_REAPER:
      structure = find_structure(state, STRUCTURE_TYPE_BARRACKS);
      if (structure->state == STRUCTURE_STATE_NORMAL && state->minerals >= 20 && state->gas >= 3)
      {
        state->minerals -= 20;
        state->gas -= 3;
        structure->state = STRUCTURE_STATE_WORKING;
        create_event(state, 2000000ULL, EVENT_TYPE_UNIT_BUILD_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_SHOCK_TROOPER:
      structure = find_structure(state, STRUCTURE_TYPE_BARRACKS);
      if (structure->state == STRUCTURE_STATE_NORMAL  && state->minerals >= 15 && state->gas >= 25)
      {
        state->minerals -= 15;
        state->gas -= 25;
        structure->state = STRUCTURE_STATE_WORKING;
        create_event(state, 2000000ULL, EVENT_TYPE_UNIT_BUILD_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_MARINE:
      structure = find_structure(state, STRUCTURE_TYPE_BARRACKS);
      if (structure->state == STRUCTURE_STATE_NORMAL  && state->minerals >= 15 && state->gas >= 5)
      {
        state->minerals -= 15;
        state->gas -= 5;
        structure->state = STRUCTURE_STATE_WORKING;
        create_event(state, 2000000ULL, EVENT_TYPE_UNIT_BUILD_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_VETERAN_MARINE:
      structure = find_structure(state, STRUCTURE_TYPE_BARRACKS);
      if (state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] == 0)
      {
        completed = 1;
        break;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] == 2 && state->minerals >= 50 && state->gas >= 10)
      {
        state->minerals -= 50;
        state->gas -= 10;
        structure->state = STRUCTURE_STATE_WORKING;
        create_event(state, 2000000ULL, EVENT_TYPE_UNIT_BUILD_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_ELITE_MARINE:
      structure = find_structure(state, STRUCTURE_TYPE_BARRACKS);
      if (state->upgrades[WORLD_UPGRADE_LEVEL_3_WEAPONS] == 0)
      {
        completed = 1;
        break;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->upgrades[WORLD_UPGRADE_LEVEL_3_WEAPONS] == 2 && state->minerals >= 100 && state->gas >= 20)
      {
        state->minerals -= 100;
        state->gas -= 20;
        structure->state = STRUCTURE_STATE_WORKING;
        create_event(state, 2000000ULL, EVENT_TYPE_UNIT_BUILD_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_GENERATOR:
      structure = find_structure(state, STRUCTURE_TYPE_GENERATOR);
      int cost = 32 + 2*structure->upgrades[STRUCTURE_UPGRADE_LEVEL];
      if (structure->upgrades[STRUCTURE_UPGRADE_LEVEL] >= 50)
        completed = 1;
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->minerals >= cost)
      {
        state->minerals -= cost;
        structure->state = STRUCTURE_STATE_WORKING;
        create_event(state, 5000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_AUTO_BUILD_ELITE_MARINE:
      if (state->upgrades[WORLD_UPGRADE_LEVEL_3_WEAPONS] == 0)
      {
        completed = 1;
        break;
      }
    case BUILD_ACTION_AUTO_BUILD_VETERAN_MARINE:
      if (state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] == 0)
      {
        completed = 1;
        break;
      }
    case BUILD_ACTION_AUTO_BUILD_REAPER:
    case BUILD_ACTION_AUTO_BUILD_SHOCK_TROOPER:
    case BUILD_ACTION_AUTO_BUILD_MARINE:
    
      /* Get Barracks */
      structure = find_structure(state, STRUCTURE_TYPE_BARRACKS);
      /* Check for Auto Poll */
      barracks_event = structure->first_event;
      while (barracks_event)
      {
        if (barracks_event->event_type == EVENT_TYPE_AUTO_BUILD_POLL)
        {
          break;
        }
        barracks_event = barracks_event->next_event_in_time;
      }
      if (!barracks_event)
      {
        barracks_event = create_event(state, 1000000, EVENT_TYPE_AUTO_BUILD_POLL, structure, build_action);
      }
      barracks_event->action = build_action;
      completed = 1;
      break;
    case BUILD_ACTION_AUTO_BUILD_BARRACKS_OFF:
      /* Get Barracks */
      structure = find_structure(state, STRUCTURE_TYPE_BARRACKS);
      /* Check for Auto Poll */
      barracks_event = structure->first_event;
      while (barracks_event)
      {
        if (barracks_event->event_type == EVENT_TYPE_AUTO_BUILD_POLL)
        {
          break;
        }
        barracks_event = barracks_event->next_event_in_time;
      }
      if (barracks_event)
      {
        delete_event(state, barracks_event);
      }
      completed = 1;
      break;
    case BUILD_ACTION_AUTO_UPGRADE_GENERATOR_ON:
      /* Get Generator */
      structure = find_structure(state, STRUCTURE_TYPE_GENERATOR);
      /* Check for Auto Poll */
      barracks_event = structure->first_event;
      while (barracks_event)
      {
        if (barracks_event->event_type == EVENT_TYPE_AUTO_BUILD_POLL)
        {
          break;
        }
        barracks_event = barracks_event->next_event_in_time;
      }
      if (!barracks_event)
      {
        barracks_event = create_event(state, 1000000, EVENT_TYPE_AUTO_BUILD_POLL, structure, build_action);
      }
      barracks_event->action = build_action;
      completed = 1;
      break;
    case BUILD_ACTION_AUTO_UPGRADE_GENERATOR_OFF:
      /* Get Generator */
      structure = find_structure(state, STRUCTURE_TYPE_GENERATOR);
      /* Check for Auto Poll */
      barracks_event = structure->first_event;
      while (barracks_event)
      {
        if (barracks_event->event_type == EVENT_TYPE_AUTO_BUILD_POLL)
        {
          break;
        }
        barracks_event = barracks_event->next_event_in_time;
      }
      if (barracks_event)
      {
        delete_event(state, barracks_event);
      }
      completed = 1;
      break;
    case BUILD_ACTION_UPGRADE_OVERCHARGE:
      structure = find_structure(state, STRUCTURE_TYPE_ARMORY);
      if (!structure || state->upgrades[WORLD_UPGRADE_OVERCHARGE] != 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->gas >= 50)
      {
        state->gas -= 50;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_OVERCHARGE] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_ENERGY_CONSERVATION_1:
      structure = find_structure(state, STRUCTURE_TYPE_ARMORY);
      if (!structure || state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_1] != 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->gas >= 300)
      {
        state->gas -= 300;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_1] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_ENERGY_CONSERVATION_2:
      structure = find_structure(state, STRUCTURE_TYPE_ARMORY);
      if (!structure || state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_2] != 0 || state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_1] == 0 || state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_1] == 2 && state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] == 2 && state->gas >= 450)
      {
        state->gas -= 450;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_2] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_ENERGY_CONSERVATION_3:
      structure = find_structure(state, STRUCTURE_TYPE_ARMORY);
      if (!structure || state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_3] != 0 || state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_2] == 0 || state->upgrades[WORLD_UPGRADE_LEVEL_3_WEAPONS] == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_2] == 2 && state->upgrades[WORLD_UPGRADE_LEVEL_3_WEAPONS] == 2 && state->gas >= 750)
      {
        state->gas -= 750;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_3] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_SHATTERING_LASER:
      structure = find_structure(state, STRUCTURE_TYPE_ARMORY);
      if (!structure || state->upgrades[WORLD_UPGRADE_SHATTERING_LASER] != 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->gas >= 200)
      {
        state->gas -= 200;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_SHATTERING_LASER] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_DEFLECTIVE_SHATTER:
      structure = find_structure(state, STRUCTURE_TYPE_ARMORY);
      if (!structure || state->upgrades[WORLD_UPGRADE_DEFLECTIVE_SHATTER] != 0 || state->upgrades[WORLD_UPGRADE_SHATTERING_LASER] == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->upgrades[WORLD_UPGRADE_SHATTERING_LASER] == 2 && state->gas >= 600)
      {
        state->gas -= 600;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_DEFLECTIVE_SHATTER] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_PENETRATING_LASER:
      structure = find_structure(state, STRUCTURE_TYPE_ARMORY);
      if (!structure || state->upgrades[WORLD_UPGRADE_PENETRATING_LASER] != 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->gas >= 250)
      {
        state->gas -= 250;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_PENETRATING_LASER] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_UPGRADE_ELITE_SCOPE:
      structure = find_structure(state, STRUCTURE_TYPE_ARMORY);
      if (!structure || state->upgrades[WORLD_UPGRADE_ELITE_SCOPE] != 0 || state->upgrades[WORLD_UPGRADE_LEVEL_3_WEAPONS] == 0)
      {
        // Deadlock, skip.
        completed = 1;
      }
      else if (structure->state == STRUCTURE_STATE_NORMAL && state->upgrades[WORLD_UPGRADE_LEVEL_3_WEAPONS] == 2 && state->gas >= 700)
      {
        state->gas -= 700;
        structure->state = STRUCTURE_STATE_WORKING;
        state->upgrades[WORLD_UPGRADE_ELITE_SCOPE] = 1;
        create_event(state, 30000000ULL, EVENT_TYPE_UPGRADE_COMPLETE, structure, build_action);
        completed = 1;
      }
      break;
    case BUILD_ACTION_CHARGE_TECH_LAB:
      if (state->upgrades[WORLD_UPGRADE_TECH_LAB] == 0 || state->tech_lab_charges >= 40)
      {
        completed = 1;
      }
      else if (state->upgrades[WORLD_UPGRADE_TECH_LAB] == 2 && state->generator_charge >= 1000)
      {
        state->generator_charge -= 1000;
        state->tech_lab_charges += 1;
        completed = 1;
        break;
      }
      break;
    case BUILD_ACTION_AUTO_CHARGE_TECH_LAB_ON:
      if (state->upgrades[WORLD_UPGRADE_TECH_LAB] == 0)
      {
        completed = 1;
      }
      else if (state->upgrades[WORLD_UPGRADE_TECH_LAB] == 2)
      {
        /* Get Tech Lab */
        structure = find_structure(state, STRUCTURE_TYPE_TECH_LAB);
        /* Check for Auto Poll */
        barracks_event = structure->first_event;
        while (barracks_event)
        {
          if (barracks_event->event_type == EVENT_TYPE_AUTO_BUILD_POLL)
          {
            break;
          }
          barracks_event = barracks_event->next_event_in_time;
        }
        if (!barracks_event)
        {
          barracks_event = create_event(state, 1000000, EVENT_TYPE_AUTO_BUILD_POLL, structure, build_action);
        }
        completed = 1;
      }
      break;
    case BUILD_ACTION_AUTO_CHARGE_TECH_LAB_OFF:
      if (state->upgrades[WORLD_UPGRADE_TECH_LAB] == 0)
      {
        completed = 1;
      }
      else if (state->upgrades[WORLD_UPGRADE_TECH_LAB] == 2)
      {
        /* Get Tech Lab */
        structure = find_structure(state, STRUCTURE_TYPE_TECH_LAB);
        /* Check for Auto Poll */
        barracks_event = structure->first_event;
        while (barracks_event)
        {
          if (barracks_event->event_type == EVENT_TYPE_AUTO_BUILD_POLL)
          {
            break;
          }
          barracks_event = barracks_event->next_event_in_time;
        }
        if (barracks_event)
        {
          delete_event(state, barracks_event);
        }
        completed = 1;
      }
      break;
    case BUILD_ACTION_BUILD_ITALIS:
      if (state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] == 0)
      {
        // Deadlock, skip
        completed = 1;
      }
      else if (state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] == 2 && state->tech_lab_charges >= 25)
      {
        state->tech_lab_charges -= 25;
        state->italis_count += 1;
        completed = 1;
      }
      break;
    case BUILD_ACTION_ION_CANNON:
      if (state->upgrades[WORLD_UPGRADE_TECH_LAB] == 0)
      {
        // Deadlock, skip
        completed = 1;
      }
      else if (state->upgrades[WORLD_UPGRADE_TECH_LAB] == 2 && state->tech_lab_charges >= 40 && state->minerals >= 1000 && state->gas >= 500)
      {
        state->minerals -= 1000;
        state->gas -= 500;
        state->tech_lab_charges -= 40;
        state->ion_cannon_count += 1;
        completed = 1;
      }
      break;
    
    default:
      completed = 1;
      break;
  }
  return completed;
}

void build_sim(struct GAMESTATE_T * state, enum BUILD_ACTION_T * build_action, long end_time_us)
{
  while (state->time_us <= end_time_us)
  {
    struct EVENT_T * event = state->next_event;
    if (!event)
    {
      state->time_us = end_time_us;
      return;
    }
    state->time_us = event->time_us;
    
    unschedule_event(state, event);
    
    if (event->structure)
    {
      
      // Command Center
       
      if (event->structure->type == STRUCTURE_TYPE_COMMAND_CENTER)
      {
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          event->structure->state = STRUCTURE_STATE_NORMAL;
          delete_event(state, event);
          create_event(state, 5000000ULL, EVENT_TYPE_MINERAL_INCOME, event->structure, 0);
          create_event(state, 1000000ULL, EVENT_TYPE_GAS_INCOME, event->structure, 0);
        }
        else if (event->event_type == EVENT_TYPE_GAS_INCOME)
        {
          state->gas += 1;
          schedule_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_MINERAL_INCOME)
        {
          state->minerals += 1;
          schedule_event(state, event);
        }
        else
        {
          delete_event(state, event);
        }
      }
      
      // Refinery
      
      else if (event->structure->type == STRUCTURE_TYPE_REFINERY)
      {
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          state->free_scv_count += 1;
          event->structure->state = STRUCTURE_STATE_NORMAL;
          create_event(state, 3000000ULL, EVENT_TYPE_GAS_INCOME, event->structure, 0);
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_UPGRADE_COMPLETE)
        {
          event->structure->upgrades[STRUCTURE_UPGRADE_LEVEL] = 1;
          event->structure->state = STRUCTURE_STATE_NORMAL;
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_GAS_INCOME)
        {
          if (event->structure->state == STRUCTURE_STATE_NORMAL)
          {
            if (event->structure->upgrades[STRUCTURE_UPGRADE_LEVEL])
            {
              state->gas += 2;
            }
            else
            {
              state->gas += 1;
            }
          }
          schedule_event(state, event);
        }
        else
        {
          delete_event(state, event);
        }
      }

      // Fruit farm
      
      else if (event->structure->type == STRUCTURE_TYPE_FRUIT_FARM)
      {
        
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          state->free_scv_count += 1;
          state->free_animal_slot_count += 3;
          event->structure->state = STRUCTURE_STATE_NORMAL;
          create_event(state, 6000000ULL, EVENT_TYPE_MINERAL_INCOME, event->structure, 0);
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_MINERAL_INCOME)
        {
          state->minerals += 1;
          schedule_event(state, event);
        }
        else
        {
          delete_event(state, event);
        }
      }
      
      
      
      // Construction Yard
      else if (event->structure->type == STRUCTURE_TYPE_CONSTRUCTION_YARD)
      {
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          state->free_scv_count += 1;
          event->structure->state = STRUCTURE_STATE_NORMAL;
          state->upgrades[WORLD_UPGRADE_ANY_TECH_STRUCTURE] = 2;
          state->upgrades[WORLD_UPGRADE_CONSTRUCTION_YARD] = 2;
        }
        else if (event->event_type == EVENT_TYPE_UPGRADE_COMPLETE)
        {
          event->structure->state = STRUCTURE_STATE_NORMAL;
          if (event->action == BUILD_ACTION_UPGRADE_ADVANCED_BUILD)
            state->upgrades[WORLD_UPGRADE_ADVANCED_BUILD] = 2;
          else if (event->action == BUILD_ACTION_UPGRADE_UNIFIED_ARMOR)
            state->upgrades[WORLD_UPGRADE_UNIFIED_ARMOR] = 2;
          else if (event->action == BUILD_ACTION_UPGRADE_CHEMICAL_PLANT)
            state->upgrades[WORLD_UPGRADE_CHEMICAL_PLANT] = 2;
          else if (event->action == BUILD_ACTION_UPGRADE_IMPROVED_FARMING)
            state->upgrades[WORLD_UPGRADE_IMPROVED_FARMING] = 2;
          else if (event->action == BUILD_ACTION_UPGRADE_FORTIFICATION)
            state->upgrades[WORLD_UPGRADE_FORTIFICATION] += 1;
          else if (event->action == BUILD_ACTION_UPGRADE_SENTRY_SUPERCHARGE)
            state->upgrades[WORLD_UPGRADE_SENTRY_SUPERCHARGE] = 2;
        }
        delete_event(state, event);
      }
      
      // Tech Lab
      else if (event->structure->type == STRUCTURE_TYPE_TECH_LAB)
      {
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          state->free_scv_count += 1;
          event->structure->state = STRUCTURE_STATE_NORMAL;
          state->upgrades[WORLD_UPGRADE_ANY_TECH_STRUCTURE] = 2;
          state->upgrades[WORLD_UPGRADE_TECH_LAB] = 2;
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_UPGRADE_COMPLETE)
        {
          event->structure->state = STRUCTURE_STATE_NORMAL;
          if (event->action == BUILD_ACTION_UPGRADE_LEVEL_2_WEAPONS)
            state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] = 2;
          else if (event->action == BUILD_ACTION_UPGRADE_LEVEL_3_WEAPONS)
            state->upgrades[WORLD_UPGRADE_LEVEL_3_WEAPONS] = 2;
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_AUTO_BUILD_POLL)
        {
          try_process_action(state, BUILD_ACTION_CHARGE_TECH_LAB);
          schedule_event(state, event);
        }
        
      }
      
      // Armory
      else if (event->structure->type == STRUCTURE_TYPE_ARMORY)
      {
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          state->free_scv_count += 1;
          event->structure->state = STRUCTURE_STATE_NORMAL;
          state->upgrades[WORLD_UPGRADE_ANY_TECH_STRUCTURE] = 2;
          state->upgrades[WORLD_UPGRADE_ARMORY] = 2;
        }
        else if (event->event_type == EVENT_TYPE_UPGRADE_COMPLETE)
        {
          event->structure->state = STRUCTURE_STATE_NORMAL;
          if (event->action == BUILD_ACTION_UPGRADE_OVERCHARGE)
            state->upgrades[WORLD_UPGRADE_OVERCHARGE] = 2;
          else if (event->action == BUILD_ACTION_UPGRADE_ENERGY_CONSERVATION_1)
            state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_1] = 2;
          else if (event->action == BUILD_ACTION_UPGRADE_ENERGY_CONSERVATION_2)
            state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_2] = 2;
          else if (event->action == BUILD_ACTION_UPGRADE_ENERGY_CONSERVATION_3)
            state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_3] = 2;
          else if (event->action == BUILD_ACTION_UPGRADE_PENETRATING_LASER)
            state->upgrades[WORLD_UPGRADE_PENETRATING_LASER] = 2;
          else if (event->action == BUILD_ACTION_UPGRADE_SHATTERING_LASER)
            state->upgrades[WORLD_UPGRADE_SHATTERING_LASER] = 2;
          else if (event->action == BUILD_ACTION_UPGRADE_DEFLECTIVE_SHATTER)
            state->upgrades[WORLD_UPGRADE_DEFLECTIVE_SHATTER] = 2;
          else if (event->action == BUILD_ACTION_UPGRADE_ELITE_SCOPE)
            state->upgrades[WORLD_UPGRADE_ELITE_SCOPE] = 2;
        }
        delete_event(state, event);
      }
      
      
      // Cow Farm
      else if (event->structure->type == STRUCTURE_TYPE_COW_FARM)
      {
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          state->free_scv_count += 1;
          event->structure->state = STRUCTURE_STATE_NORMAL;
          create_event(state, 10000000ULL, EVENT_TYPE_MINERAL_INCOME, event->structure, 0);
          

          if (event->action == BUILD_ACTION_BUILD_COW_FARM_AND_COW)
          {
            if (!try_process_action(state, BUILD_ACTION_BUILD_COW))
            {
              create_event(state, 1000000ULL, EVENT_TYPE_AUTO_BUILD_POLL, event->structure, event->action);
            }
          }
          
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_AUTO_BUILD_POLL)
        {
          if (try_process_action(state, BUILD_ACTION_BUILD_COW))
          {
            delete_event(state, event);
          }
          else
          {
            schedule_event(state, event);
          }
        }
        else if (event->event_type == EVENT_TYPE_MINERAL_INCOME)
        {
          state->minerals += 2;
          schedule_event(state, event);
        }
        else
        {
          delete_event(state, event);
        }
      }
      
      // Sheep Farm
      else if (event->structure->type == STRUCTURE_TYPE_SHEEP_FARM)
      {
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          state->free_scv_count += 1;
          event->structure->state = STRUCTURE_STATE_NORMAL;
          create_event(state, 10000000ULL, EVENT_TYPE_MINERAL_INCOME, event->structure, 0);
          
          if (event->action == BUILD_ACTION_BUILD_SHEEP_FARM_AND_SHEEP)
          {
            create_event(state, 1000000ULL, EVENT_TYPE_AUTO_BUILD_POLL, event->structure, event->action);
          }
          
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_AUTO_BUILD_POLL)
        {
          if (try_process_action(state, BUILD_ACTION_BUILD_SHEEP))
          {
            delete_event(state, event);
          }
          else
          {
            schedule_event(state, event);
          }
        }
        else if (event->event_type == EVENT_TYPE_UPGRADE_COMPLETE)
        {
          event->structure->state = STRUCTURE_STATE_NORMAL;
          event->structure->upgrades[STRUCTURE_UPGRADE_LEVEL] = 1;
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_MINERAL_INCOME)
        {
          if (event->structure->upgrades[STRUCTURE_UPGRADE_LEVEL])
          {
            state->minerals += 2;
          }
          else
          {
            state->minerals += 1;
          }
          schedule_event(state, event);
        }
        else
        {
          delete_event(state, event);
        }
      }

      // Chemical Plant
      else if (event->structure->type == STRUCTURE_TYPE_CHEMICAL_PLANT)
      {
        
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          state->free_scv_count += 1;
          event->structure->state = STRUCTURE_STATE_NORMAL;
          delete_event(state, event);
          create_event(state, 5000000ULL, EVENT_TYPE_GAS_INCOME, event->structure, 0);
        }
        else if (event->event_type == EVENT_TYPE_GAS_INCOME)
        {
          state->gas += 10;
          schedule_event(state, event);
        }
        else
        {
          delete_event(state, event);
        }
      }
      
      // Cow
      else if (event->structure->type == STRUCTURE_TYPE_COW)
      {
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          // Poll for available slot by restarting build event
          if (state->free_animal_slot_count > 0)
          {
            state->free_animal_slot_count -= 1;
            event->structure->state = STRUCTURE_STATE_NORMAL;
            create_event(state, 18000000ULL, EVENT_TYPE_MINERAL_INCOME, event->structure, 0);
            delete_event(state, event);
          }
          else
          {
            schedule_event(state, event);
          }
        }
        else if (event->event_type == EVENT_TYPE_MINERAL_INCOME)
        {
          if (state->upgrades[WORLD_UPGRADE_IMPROVED_FARMING] == 2)
            state->minerals += 4;
          else
            state->minerals += 3;
          schedule_event(state, event);
        }
        else
        {
          delete_event(state, event);
        }
      }

      // Sheep
      else if (event->structure->type == STRUCTURE_TYPE_SHEEP)
      {
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          // Poll for available slot by restarting build event
          if (state->free_animal_slot_count > 0)
          {
            state->free_animal_slot_count -= 1;
            event->structure->state = STRUCTURE_STATE_NORMAL;
            create_event(state, 18000000ULL, EVENT_TYPE_MINERAL_INCOME, event->structure, 0);
            delete_event(state, event);
          }
          else
          {
            schedule_event(state, event);
          }
        }
        else if (event->event_type == EVENT_TYPE_MINERAL_INCOME)
        {
          if (state->upgrades[WORLD_UPGRADE_IMPROVED_FARMING] == 2)
            state->minerals += 2;
          else
            state->minerals += 1;
          schedule_event(state, event);
        }
        else
        {
          delete_event(state, event);
        }
      }
      
      // Generator
      else if (event->structure->type == STRUCTURE_TYPE_GENERATOR)
      {
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          event->structure->state = STRUCTURE_STATE_NORMAL;
          create_event(state, 5000000ULL, EVENT_TYPE_ENERGY_INCOME, event->structure, 0);
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_ENERGY_INCOME)
        {
          state->generator_charge += 5*(7+3*event->structure->upgrades[STRUCTURE_UPGRADE_LEVEL]);
          if (state->generator_charge > 5000 + state->shield_battery_max_charge)
            state->generator_charge = 5000 + state->shield_battery_max_charge;
          state->generator_charge -= state->charge_used_per_sec_for_fighting*5;
          if (state->generator_charge < 0)
            state->generator_charge = 0;
          schedule_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_UPGRADE_COMPLETE)
        {
          event->structure->state = STRUCTURE_STATE_NORMAL;
          event->structure->upgrades[STRUCTURE_UPGRADE_LEVEL] += 1;
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_AUTO_BUILD_POLL)
        {
          try_process_action(state, BUILD_ACTION_UPGRADE_GENERATOR);
          if (event->structure->upgrades[STRUCTURE_UPGRADE_LEVEL] >= 50)
          {
            delete_event(state, event);
          }
          else
          {
            schedule_event(state, event);
          }
        }
        else
        {
          delete_event(state, event);
        }
      }
      
      

      // Converter
      else if (event->structure->type == STRUCTURE_TYPE_CONVERTER)
      {        
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          state->free_scv_count++;
          event->structure->state = STRUCTURE_STATE_CONVERT_TO_GAS;
          create_event(state, 5000000ULL, EVENT_TYPE_ENERGY_INCOME, event->structure, 0);
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_ENERGY_INCOME)
        {
          if (event->structure->state == STRUCTURE_STATE_CONVERT_TO_GAS)
          {
            if (state->minerals >= 4)
            {
              state->gas += 2;
              state->minerals -= 4;
            }
          }
          else if (event->structure->state == STRUCTURE_STATE_CONVERT_TO_MINERALS)
          {
            if (state->gas >= 4)
            {
              state->minerals += 2;
              state->gas -= 4;
            }
          }
          schedule_event(state, event);
        }
        else
        {
          delete_event(state, event);
        }
      }
      
      // Barracks
      else if (event->structure->type == STRUCTURE_TYPE_BARRACKS)
      {
        if (event->event_type == EVENT_TYPE_BUILD_COMPLETE)
        {
          event->structure->state = STRUCTURE_STATE_NORMAL;
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_UNIT_BUILD_COMPLETE)
        {
          switch(event->action)
          {
            case BUILD_ACTION_BUILD_REAPER:
              state->reaper_count += 1;
              break;
            case BUILD_ACTION_BUILD_SHOCK_TROOPER:
              state->shock_trooper_count += 1;
              break;
            case BUILD_ACTION_BUILD_MARINE:
              state->marine_count += 1;
              break;
            case BUILD_ACTION_BUILD_VETERAN_MARINE:
              state->veteran_marine_count += 1;
              break;
            case BUILD_ACTION_BUILD_ELITE_MARINE:
              state->elite_marine_count += 1;
              break;
            default:
              break;
          }
          event->structure->state = STRUCTURE_STATE_NORMAL;
          delete_event(state, event);
        }
        else if (event->event_type == EVENT_TYPE_AUTO_BUILD_POLL)
        {
          enum BUILD_ACTION_T simulated_action;
          switch(event->action)
          {
            case BUILD_ACTION_AUTO_BUILD_REAPER:
              simulated_action = BUILD_ACTION_BUILD_REAPER;
              break;
            case BUILD_ACTION_AUTO_BUILD_SHOCK_TROOPER:
              simulated_action = BUILD_ACTION_BUILD_SHOCK_TROOPER;
              break;
            case BUILD_ACTION_AUTO_BUILD_MARINE:
              simulated_action = BUILD_ACTION_BUILD_MARINE;
              break;
            case BUILD_ACTION_AUTO_BUILD_VETERAN_MARINE:
              simulated_action = BUILD_ACTION_BUILD_VETERAN_MARINE;
              break;
            case BUILD_ACTION_AUTO_BUILD_ELITE_MARINE:
            default:
              simulated_action = BUILD_ACTION_BUILD_ELITE_MARINE;
              break;
          }
          try_process_action(state, simulated_action);
          schedule_event(state, event);
        }
        else
        {
          delete_event(state, event);
        }
      }
    }
    
    
    
    // Abilities
    else if (event->event_type == EVENT_TYPE_CHECK_SURVIVAL)
    {
      estimate_survival_margin(state, 1);
      schedule_event(state, event);
    }
    else if (event->action == BUILD_ACTION_BUILD_WALL)
    {
      state->free_scv_count += 2;
      state->front_wall_state = 2;
      delete_event(state, event);
    }
    else if (event->action == BUILD_ACTION_BUILD_SENTRY_GUN)
    {
      state->free_scv_count += 1;
      state->sentry_gun_count += 1;
      delete_event(state, event);
    }
    else if (event->action == BUILD_ACTION_TAKE_NEAR_BASE)
    {
      state->refinery_spots_left            += NEAR_FRONT_BASE_REFINERY_SPOTS;
      state->in_progress_refinery_spots_left -= NEAR_FRONT_BASE_REFINERY_SPOTS;
      state->large_spots_left               += NEAR_FRONT_BASE_LARGE_SPOTS;
      state->in_progress_large_spots_left    -= NEAR_FRONT_BASE_LARGE_SPOTS;
      state->small_spots_left               += NEAR_FRONT_BASE_SMALL_SPOTS;
      state->in_progress_small_spots_left    -= NEAR_FRONT_BASE_SMALL_SPOTS;
      state->near_front_base_state = 2;
      delete_event(state, event);
    }
    else if (event->action == BUILD_ACTION_TAKE_FAR_BASE)
    {
      state->refinery_spots_left            += FAR_FRONT_BASE_REFINERY_SPOTS;
      state->in_progress_refinery_spots_left -= FAR_FRONT_BASE_REFINERY_SPOTS;
      state->large_spots_left               += FAR_FRONT_BASE_LARGE_SPOTS;
      state->in_progress_large_spots_left    -= FAR_FRONT_BASE_LARGE_SPOTS;
      state->small_spots_left               += FAR_FRONT_BASE_SMALL_SPOTS;
      state->in_progress_small_spots_left    -= FAR_FRONT_BASE_SMALL_SPOTS;
      state->far_front_base_state = 2;
      if (state->near_front_base_state < 2)
      {      
        state->refinery_spots_left            += NEAR_FRONT_BASE_REFINERY_SPOTS;
        state->in_progress_refinery_spots_left -= NEAR_FRONT_BASE_REFINERY_SPOTS;
        state->large_spots_left               += NEAR_FRONT_BASE_LARGE_SPOTS;
        state->in_progress_large_spots_left    -= NEAR_FRONT_BASE_LARGE_SPOTS;
        state->small_spots_left               += NEAR_FRONT_BASE_SMALL_SPOTS;
        state->in_progress_small_spots_left    -= NEAR_FRONT_BASE_SMALL_SPOTS;
        state->near_front_base_state = 2;
      }
      delete_event(state, event);
    }
    else if (event->action == BUILD_ACTION_BACK_ROCKS_CLIP)
    {
      state->refinery_spots_left            += BACK_BASE_REFINERY_SPOTS;
      state->in_progress_refinery_spots_left -= BACK_BASE_REFINERY_SPOTS;
      state->large_spots_left               += BACK_BASE_LARGE_SPOTS;
      state->in_progress_large_spots_left    -= BACK_BASE_LARGE_SPOTS;
      state->small_spots_left               += BACK_BASE_SMALL_SPOTS;
      state->in_progress_small_spots_left    -= BACK_BASE_SMALL_SPOTS;
      state->back_rocks_state = 2;
      delete_event(state, event);
    }
    else if (event->action == BUILD_ACTION_CHEATER_BASE_CLIP)
    {
      state->refinery_spots_left            += CHEATER_BASE_REFINERY_SPOTS;
      state->in_progress_refinery_spots_left -= CHEATER_BASE_REFINERY_SPOTS;
      state->large_spots_left               += CHEATER_BASE_LARGE_SPOTS;
      state->in_progress_large_spots_left    -= CHEATER_BASE_LARGE_SPOTS;
      state->small_spots_left               += CHEATER_BASE_SMALL_SPOTS;
      state->in_progress_small_spots_left    -= CHEATER_BASE_SMALL_SPOTS;
      state->cheater_base_state = 2;
      delete_event(state, event);
    }
    else
    {
      delete_event(state, event);
    }
    
    
    
    /* Try to execute next command */
    if (build_action && *build_action != BUILD_ACTION_END_BUILD)
    {
      if (try_process_action(state, *build_action))
      {
        build_action++;
      }
    }
  }
}


void clean_gamestate(struct GAMESTATE_T * state)
{
  /* Free all events */
  struct EVENT_T * to_free;
  struct EVENT_T * next_event;
  next_event = state->next_event;
  while (next_event)
  {
    to_free = next_event;
    next_event = next_event->next_event_in_time;
    free(to_free);
  }
  
  /* Free all structures */
  struct STRUCTURE_T * to_free_s;
  struct STRUCTURE_T * next_struct;
  next_struct = state->first_structure;
  while (next_struct)
  {
    to_free_s = next_struct;
    next_struct = next_struct->next;
    free(to_free_s);
  }
}

static float damage_calc(float damage, float armor)
{
  if (damage-1 < armor)
    return 1;
  else
    return damage - armor;
}

enum UNIT_TYPE
{
  UNIT_TYPE_REAPER,
  UNIT_TYPE_SHOCK_TROOPER,
  UNIT_TYPE_MARINE,
  UNIT_TYPE_VETERAN_MARINE,
  UNIT_TYPE_ELITE_MARINE,
  UNIT_TYPE_ITALIS
};

struct FIGHT_SIM_SCENARIO_T
{
  float terran_spread_factor;
  
  int terran_use_base_defenses;
  
  float zerg_armor;
  float zerg_density;
  float is_zerg_structure;
  float zerg_hp;
  float zerg_structure_fire_rate;
  float zerg_structure_damage;
  float zerg_structure_splash;
  float charge_time;
  float fight_time;
};

struct FIGHT_SIM_RESULTS_T
{
  float energy_used;
  
  float damage_dealt;
  
  float marines_lost;
  float veteran_marines_lost;
  float elite_marines_lost;
  float reapers_lost;
  float shock_troopers_lost;
  float italises_lost;
};



float fight_sim(struct GAMESTATE_T * state, struct FIGHT_SIM_SCENARIO_T * scenario, struct FIGHT_SIM_RESULTS_T * results_out)
{
  float elite_scope_factor = 0;
  if (state->upgrades[WORLD_UPGRADE_ELITE_SCOPE] == 2)
    elite_scope_factor = 1;
    
  float marines_lost = 0;
  float veteran_marines_lost = 0;
  float elite_marines_lost = 0;
  float shock_troopers_lost = 0;
  float reapers_lost = 0;
  float italises_lost = 0;
  
  float time = 0.0;
  
  // Get available energy
  struct STRUCTURE_T * generator = find_structure(state, STRUCTURE_TYPE_GENERATOR);
  float energy_per_sec = (7+3*generator->upgrades[STRUCTURE_UPGRADE_LEVEL]); 
    
  // Get energy available before the conflict
  float energy = energy_per_sec * scenario->charge_time;
  float energy_used = 0;
  
  float damage = 0;
  
  while (time < scenario->fight_time && damage < scenario->zerg_hp)
  {
    
    // Get unit counts in our ball
    float marines = state->marine_count - marines_lost;
    float veteran_marines = state->veteran_marine_count - veteran_marines_lost;
    float elite_marines = state->elite_marine_count - elite_marines_lost;
    float shock_troopers = state->shock_trooper_count - shock_troopers_lost;
    float reapers = state->reaper_count - reapers_lost;
    float italises = state->italis_count - italises_lost;
    
    float sentry_guns = 0;
    if (scenario->terran_use_base_defenses)
    {
      // one in 3 sentry guns will be in range of a single enemy at a time
      sentry_guns = state->sentry_gun_count/3;
    }
    
    float average_unit_range = marines*8.0 + veteran_marines*8 + elite_marines*(8 + elite_scope_factor) + shock_troopers*8 + reapers*5 + italises*14;
    average_unit_range = average_unit_range / (marines + veteran_marines + elite_marines + shock_troopers + reapers + italises);
    
    // This approximation limits usable units to a circle with diameter == average unit range. Also prioritises better units.
    float space_left = (average_unit_range*scenario->terran_spread_factor)*(average_unit_range*scenario->terran_spread_factor)*3.14159;
    
    space_left -= italises;
    if (space_left < 0)
    {
      italises += space_left;
      space_left = 0;
    }
    
    space_left -= elite_marines;
    if (space_left < 0)
    {
      elite_marines += space_left;
      space_left = 0;
    }
      
    space_left -= veteran_marines;
    if (space_left < 0)
    {
      veteran_marines += space_left;
      space_left = 0;
    }
    
    // Shock troopers can avoid zerg structures
    if (!scenario->is_zerg_structure)
    {
      space_left -= shock_troopers;
      if (space_left < 0)
      {
        shock_troopers += space_left;
        space_left = 0;
      }
    }

    space_left -= marines;
    if (space_left < 0)
    {
      marines += space_left;
      space_left = 0;
    }
    
    space_left -= reapers;
    if (space_left < 0)
    {
      reapers += space_left;
      space_left = 0;
    }
    
    // Estimate the time we have until we next take damage
    // we do this by comparing average hp vs zerg damage and damage period
    float hp = 0;
  
    hp += marines * 45;
    hp += reapers * 28;
    hp += shock_troopers * 55;
    hp += veteran_marines * 85;
    hp += elite_marines * 140;
    hp += italises * 220;
    if (hp > 0)
      hp = hp / (marines + reapers + shock_troopers + veteran_marines + elite_marines + italises);
    
    int zerg_hits_required = ceil(hp/scenario->zerg_structure_damage);
    float zerg_cycle_time = zerg_hits_required / scenario->zerg_structure_fire_rate;
    if (zerg_cycle_time == 0 || scenario->is_zerg_structure == 0)
      zerg_cycle_time = 10;
    float cycle_time = zerg_cycle_time;
    if ((cycle_time + time) > scenario->fight_time)
      cycle_time = scenario->fight_time - time;

    // Get energy available for this period of time
    energy += energy_per_sec * cycle_time;
    float energy_available = energy;
    
    // Cap energy available with battery capacity (assume 1 minute charge/discharge cycles, starting with full batteries near army.)
    float battery_max_energy_transfer = state->shield_battery_max_charge;// + state->shield_battery_max_charge*cycle_time/600.0;
    if (energy_available > battery_max_energy_transfer)
    {
      energy_available = battery_max_energy_transfer;
    }
    
    // Estimate energy consumption rate so we can calculate firing time
    float energy_consumption_rate = 0;
    float overcharge_energy_consumption_rate = 0;
    
    float elite_marine_conservation_effect = 1;
    float veteran_marine_conservation_effect = 1;
    float marine_conservation_effect = 1;

    // Get energy conservation effect for all units
    if (state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_1] == 2)
    {
      marine_conservation_effect = 1.75;
      veteran_marine_conservation_effect = 1.25;
    }
    if (state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_2] == 2)
    {
      marine_conservation_effect = 2.00;
      veteran_marine_conservation_effect = 1.75;
      elite_marine_conservation_effect = 1.25;
    }
    if (state->upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_3] == 2)
    {
      marine_conservation_effect = 2.00;
      veteran_marine_conservation_effect = 2.00;
      elite_marine_conservation_effect = 2.00;
    }
      
    // Calculate energy consumption rate
    if (state->upgrades[WORLD_UPGRADE_SENTRY_SUPERCHARGE] == 2)
      energy_consumption_rate += (sentry_guns * 4/0.5);
    else
      energy_consumption_rate += (sentry_guns * 3/0.5);
    
    energy_consumption_rate += (elite_marines * 3.0/0.4)/elite_marine_conservation_effect;
    overcharge_energy_consumption_rate += (elite_marines * 3.0/0.4)/elite_marine_conservation_effect;
    
    energy_consumption_rate += (veteran_marines * 2.0/0.4)/veteran_marine_conservation_effect;
    overcharge_energy_consumption_rate += (veteran_marines * 2.0/0.4)/veteran_marine_conservation_effect;
    
    energy_consumption_rate += (marines * 1.0/0.4)/marine_conservation_effect;
    overcharge_energy_consumption_rate += (marines * 1.0/0.4)/marine_conservation_effect;
    
    float shock_trooper_ability_rate = 0;
    
    if (scenario->is_zerg_structure)
    {
      // Instant Discharge
      shock_trooper_ability_rate = shock_troopers / 30;
      energy_consumption_rate += shock_trooper_ability_rate * 50;
    }
    else
    {
      // Normal
      energy_consumption_rate += shock_troopers * 2.0/1.0;
      // Ground Pulse
      shock_trooper_ability_rate = shock_troopers / 2.0;
      if (shock_trooper_ability_rate > 5.0)
        shock_trooper_ability_rate = 5.0;
      energy_consumption_rate += shock_trooper_ability_rate * 50;
    }
    
    // Calculate full damage time available
    float damage_time = energy_available / energy_consumption_rate;
    if (damage_time > cycle_time || energy_consumption_rate == 0)
      damage_time = cycle_time;
    
    // Calculate overcharge: 5 bonus seconds every 30 seconds
    float overcharge_time = 0;
    if (state->upgrades[WORLD_UPGRADE_OVERCHARGE])
    {
      float overcharge_cycle_start_phase = time - (time/30.0);
      float overcharge_cycle_end_phase = (time + damage_time) - (time + damage_time)/30.0;
      float full_overcharge_cycles = floor(damage_time / 30.0);
      float overcharge_time_available = full_overcharge_cycles * 5.0;
      
      float first_cycle_time_available = 5.0 - overcharge_cycle_start_phase;
      
      if (overcharge_cycle_end_phase < 5.0)
        first_cycle_time_available = overcharge_cycle_end_phase - overcharge_cycle_start_phase;
        
      if (first_cycle_time_available < 0.0)
        first_cycle_time_available = 0.0;
      
      overcharge_time_available += first_cycle_time_available;
      
      float energy_left = damage_time * energy_consumption_rate - energy;
      float overcharge_time = energy_left / overcharge_energy_consumption_rate;
      if (overcharge_time > overcharge_time_available)
        overcharge_time = overcharge_time_available;
    }
    
    // Mark energy as used
    float energy_used_now = overcharge_time*overcharge_energy_consumption_rate + damage_time*energy_consumption_rate;
    if (energy_consumption_rate > 0)
    {
      energy -= energy_used_now;
      energy_used += energy_used_now;
    }
    
    // Calculate damage done with given energy and time
    float dps;
    
    float shattering_laser_factor = 1;
    if (!scenario->is_zerg_structure)
    {
      if (state->upgrades[WORLD_UPGRADE_SHATTERING_LASER] == 2)
        shattering_laser_factor = 2.0/3.0;
      if (state->upgrades[WORLD_UPGRADE_DEFLECTIVE_SHATTER] == 2)
        shattering_laser_factor = 2.0;
    }
    
    float penetrating_laser_factor = 0;
    if (state->upgrades[WORLD_UPGRADE_PENETRATING_LASER] == 2)
      penetrating_laser_factor = 1;
    
    // Sentry gun
    if (state->upgrades[WORLD_UPGRADE_SENTRY_SUPERCHARGE] == 2)
      dps = damage_calc(25.0, scenario->zerg_armor)/0.5;
    else
      dps = damage_calc(16.0, scenario->zerg_armor)/0.5;
    damage += dps*sentry_guns*(damage_time);
    
    // Elite marine
    dps = damage_calc(11.0 + penetrating_laser_factor + elite_scope_factor, scenario->zerg_armor)/0.4;
    dps += shattering_laser_factor*damage_calc(11.0 + penetrating_laser_factor + elite_scope_factor, scenario->zerg_armor)/0.4;
    damage += dps*elite_marines*(damage_time + overcharge_time);
    
    // Veteran marine
    dps = damage_calc(6.0 + penetrating_laser_factor, scenario->zerg_armor)/0.4;
    dps += shattering_laser_factor*damage_calc(6.0 + penetrating_laser_factor, scenario->zerg_armor)/0.4;
    damage += dps*veteran_marines*(damage_time + overcharge_time);

    // Marine
    dps = damage_calc(3.0 + penetrating_laser_factor, scenario->zerg_armor)/0.4;
    dps += shattering_laser_factor*damage_calc(3.0 + penetrating_laser_factor, scenario->zerg_armor)/0.4;
    damage += dps*marines*(damage_time + overcharge_time);
    
    // Shock trooper
    if (scenario->is_zerg_structure)
    {
      // Ground pulse is 80 damage.
      damage += damage_calc(80, scenario->zerg_armor)*shock_trooper_ability_rate*damage_time;
    }
    else
    {
      // Normal shot damages around 7 units with the same effect
      dps = (7*scenario->zerg_density)*damage_calc(4.0, scenario->zerg_armor)/1.0;
      damage += dps*shock_troopers*damage_time;
      // Ground pulse is 20 damage to around 20 units, ignoring armor.
      damage += (20*scenario->zerg_density)*20*shock_trooper_ability_rate*damage_time;
    }
    
    // Reaper
    if (!scenario->is_zerg_structure)
      dps = damage_calc(3.0, scenario->zerg_armor)*2.0/1.05;
    else
      dps = damage_calc(12.0, scenario->zerg_armor)/1.15;
    damage += dps*reapers*damage_time;
    
    // Italis
    if (!scenario->is_zerg_structure)
      dps = damage_calc(32, scenario->zerg_armor)*4.0/1.2;
    else
      dps = damage_calc(14, scenario->zerg_armor)*4.0/1.2;
    damage += dps*italises*damage_time;
    
    // If we have not murdered the zerg structure, we need to take damage
    if (cycle_time == zerg_cycle_time && damage < scenario->zerg_hp && scenario->is_zerg_structure)
    {
      float units_killed = scenario->zerg_structure_splash;
      float units_lost_now = 0;
      
      // Elite marines
      units_lost_now = units_killed;
      if (elite_marines < units_lost_now)
        units_lost_now = elite_marines;
      units_killed -= units_lost_now;
      elite_marines_lost += units_lost_now;
      
      // Veteran marines
      units_lost_now = units_killed;
      if (veteran_marines < units_lost_now)
        units_lost_now = veteran_marines;
      units_killed -= units_lost_now;
      veteran_marines_lost += units_lost_now;
     
      // Marines
      units_lost_now = units_killed;
      if (marines < units_lost_now)
        units_lost_now = marines;
      units_killed -= units_lost_now;
      marines_lost += units_lost_now;
      
      // Reapers
      units_lost_now = units_killed;
      if (reapers < units_lost_now)
        units_lost_now = reapers;
      units_killed -= units_lost_now;
      reapers_lost += units_lost_now;

      // Shock troopers (can avoid structure damage)
      if (!scenario->is_zerg_structure)
      {
        units_lost_now = units_killed;
        if (shock_troopers < units_lost_now)
          units_lost_now = shock_troopers;
        units_killed -= units_lost_now;
        shock_troopers_lost += units_lost_now;
      }

      // Italises
      units_lost_now = units_killed;
      if (italises < units_lost_now)
        units_lost_now = italises;
      units_killed -= units_lost_now;
      italises_lost += units_lost_now;
    }
    
    time += cycle_time;
  }
  
  if (results_out)
  {
    results_out->damage_dealt = damage;
    results_out->energy_used = energy_used;
    results_out->marines_lost = marines_lost;
    results_out->veteran_marines_lost = veteran_marines_lost;
    results_out->elite_marines_lost = elite_marines_lost;
    results_out->reapers_lost = reapers_lost;
    results_out->shock_troopers_lost = shock_troopers_lost;
    results_out->italises_lost = italises_lost;
  }
  
  return damage;
}

float estimate_spinecrawler_effectiveness(struct GAMESTATE_T * state)
{
  /* Big spine crawler */
  struct FIGHT_SIM_SCENARIO_T scenario = 
  {
    .charge_time = 0,
    .fight_time = 2*60,
    
    .terran_spread_factor = 0.8,
    .terran_use_base_defenses = 0,
    
    .zerg_armor = 2,
    .zerg_density = 1/30,
    
    .is_zerg_structure = 1,
    .zerg_hp = 13000000,
    .zerg_structure_damage = 70,
    .zerg_structure_fire_rate = 1.5,
    .zerg_structure_splash = 8.0,
  };
  struct FIGHT_SIM_RESULTS_T results;
  
  // How many large spines can it kill in 2 minutes?
  
  float damage_dealt = fight_sim(state, &scenario, &results);
  
  return damage_dealt/1300.0;
}

#define SURVIVAL_SPREAD_FACTOR 0.6
float estimate_survival_margin(struct GAMESTATE_T * state, int use_defenses)
{
  float zerg_hp_per_s = 4.3*exp((state->time_us/1000000.0)/462.0);
  float zerg_armor = 0.0;
  float zerg_density = 1.0/6.0;
  // Brutalisks are larger and decrease the zerg density
  if (state->time_us > 25*60*1000000)
  {
    zerg_armor = 6.0;
    zerg_density = 1.0/6.0;
  }
  
  // A few time periods to test
  
  // Rated margin is lowest of these
  float min_margin = 100;
  float margin;
  
  struct FIGHT_SIM_SCENARIO_T scenario = 
  {
    .charge_time = 0,
    .fight_time = 0,
    
    .terran_spread_factor = SURVIVAL_SPREAD_FACTOR,
    .terran_use_base_defenses = use_defenses,
    
    .zerg_armor = zerg_armor,
    .zerg_density = zerg_density,
    
    .is_zerg_structure = 0,
  };
  
  struct FIGHT_SIM_RESULTS_T results;
  
  // First get charge consumption rate
  scenario.charge_time = 0;
  scenario.fight_time = 60;
  scenario.zerg_hp = zerg_hp_per_s;
  fight_sim(state, &scenario, &results);
  state->charge_used_per_sec_for_fighting = results.energy_used/60;
  
  scenario.zerg_hp = 10000000.0;
  
  // 10 min at 2.0 zerg, no charge time
  scenario.charge_time = 0;
  scenario.fight_time = 10*60;
  margin = fight_sim(state, &scenario, NULL)/(zerg_hp_per_s*2.0*10*60);
  if (margin < min_margin)
    min_margin = margin;
    
  // 1 min at 3.0 zerg, 30 sec charge time
  scenario.charge_time = 30;
  scenario.fight_time = 1*60;
  margin = fight_sim(state, &scenario, NULL)/(zerg_hp_per_s*3*1*60);
  if (margin < min_margin)
    min_margin = margin;
  
  // 50 sec gap before shock tests
  if (state->time_us > 50*1000000)
  {
    if (state->near_front_base_state == 2 || !use_defenses)
    {
      if (state->front_wall_state == 2 && use_defenses)
      {
        if (state->upgrades[WORLD_UPGRADE_UNIFIED_ARMOR] < 2)
        {
          // 30 sec fight, 10 second charge, 4.0 zerg
          scenario.charge_time = 10;
          scenario.fight_time = 30;
          margin = fight_sim(state, &scenario, NULL)/(zerg_hp_per_s*4*40);
        }
        else
        {
          // Full wall, can hold shocks.
        }
      }
      else
      {
          // 10 sec fight, 30 second charge, 4.0 zerg
          scenario.charge_time = 30;
          scenario.fight_time = 10;
          margin = fight_sim(state, &scenario, NULL)/(zerg_hp_per_s*4*40);
      }
    }
  }
  if (margin < min_margin)
     min_margin = margin;
  
  if (margin < state->min_survival_margin && use_defenses)
  {
    state->min_survival_margin = margin;
  }
  return margin;
}

float estimate_hp(struct GAMESTATE_T * state)
{
  float hp = 0;
  
  hp += state->marine_count * 45;
  hp += state->reaper_count * 28;
  hp += state->shock_trooper_count * 55;
  hp += state->veteran_marine_count * 85;
  hp += state->elite_marine_count * 140;
  return hp;
}

void print_gamestate(struct GAMESTATE_T * state)
{
  printf("\nGAMESTATE\n");
  printf("Gas: %d\n", state->gas);
  printf("Minerals: %d\n", state->minerals);
  printf("Refinery Spots Left: %d\n", state->refinery_spots_left);
  printf("Large Spots Left: %d\n", state->large_spots_left);
  printf("Small Spots Left: %d\n", state->small_spots_left);
  printf("Free SCVs: %d\n", state->free_scv_count);
  printf("Free Animal Slots: %d\n", state->free_animal_slot_count);
  printf("Italis: %d\n", state->italis_count);
  printf("Reapers: %d\n", state->reaper_count);
  printf("Shock Troopers: %d\n", state->shock_trooper_count);
  printf("Marines: %d\n", state->marine_count);
  printf("Veteran Marines: %d\n", state->veteran_marine_count);
  printf("Elite Marines: %d\n", state->elite_marine_count);
  printf("Generator Charge: %d\n", state->generator_charge);
  printf("Shield Battery Max Charge: %d\n", state->shield_battery_max_charge);
  printf("Tech Lab Charges: %d\n", state->tech_lab_charges);
  printf("Sentry Guns: %d\n", state->sentry_gun_count);
  printf("Ion Cannons: %d\n", state->ion_cannon_count);
  struct FIGHT_SIM_SCENARIO_T scenario = 
  {
    .charge_time = 0,
    .fight_time = 0,
    
    .terran_spread_factor = 0.8,
    .terran_use_base_defenses = 1,
    
    .zerg_armor = 0,
    .zerg_density = 1,
    .zerg_hp = 10000000000.0,
    
    .is_zerg_structure = 0,
  };
  
  scenario.charge_time = 30;
  scenario.fight_time = 10;
  scenario.zerg_armor = 0;
  scenario.zerg_density = 1.0;
  printf("Burst DPS: %f\n", fight_sim(state, &scenario, NULL)/(10));
  scenario.zerg_armor = 7;
  scenario.zerg_density = 1.0/8.0;
  printf("Burst DPS (Brutalisk): %f\n", fight_sim(state, &scenario, NULL)/(10));
  scenario.charge_time = 0;
  scenario.fight_time = 60;
  scenario.zerg_armor = 0;
  scenario.zerg_density = 1.0;
  printf("Sustained DPS: %f\n", fight_sim(state, &scenario, NULL)/(10*60));
  scenario.zerg_armor = 7;
  scenario.zerg_density = 1.0/8.0;
  printf("Sustained DPS (Brutalisk): %f\n", fight_sim(state, &scenario, NULL)/(10*60));
  printf("End Survival Margin (in base): %f\n", estimate_survival_margin(state, 1));
  printf("End Survival Margin (outside base): %f\n", estimate_survival_margin(state, 0));
  printf("Large Spine Crawlers Killed: %f\n", estimate_spinecrawler_effectiveness(state));
  printf("Minimum survival margin: %f\n", state->min_survival_margin);
  printf("Total HP: %f\n", estimate_hp(state));
  int secs = state->time_us / 1000000ULL;
  int mins = secs / 60;
  secs = secs % 60;
  printf("Time: %d:%02d\n", mins, secs);
  /* Print Upgrades */
  int i;
  printf("\nUPGRADES:\n");
  for (i = 0; i < WORLD_UPGRADE_NUM; i++)
  {
    if (state->upgrades[i] == 1)
      printf("%s (Researching)\n", world_upgrade_strings[i]);
    else if (state->upgrades[i] == 2)
      printf("%s\n", world_upgrade_strings[i]);
  }
  /* Print Structures */
  printf("\nSTRUCTURES:\n");
  struct STRUCTURE_T * next_struct = state->first_structure;
  while (next_struct)
  {
    print_structure(next_struct);
    next_struct = next_struct->next;
  }
}

// 40 min #define TIME_HORIZON 2400000000ULL
#define TIME_HORIZON 16*60000000ULL
#define BUILD_ORDER_MAX_LEN 100000
#define SIMULATED_ANNEALING_ITERATIONS 1000000

#define MINIMUM_SURVIVAL_MARGIN 1.3

void init_gamestate(struct GAMESTATE_T * state)
{
  state->gas = 70;
  state->minerals = 10;
  state->live_scv_count = 2;
  state->free_scv_count = 2;
  state->large_spots_left = 1;
  state->small_spots_left = 18;
  state->refinery_spots_left = 0;
  state->free_animal_slot_count = 0;
  state->time_us = 0;
  
  state->cheater_base_state = 0;
  state->back_rocks_state = 0;
  
  state->front_wall_state = 0;
  state->near_front_base_state = 0;
  state->far_front_base_state = 0;
  
  state->min_survival_margin = 100;
  
  state->in_progress_small_spots_left = 0;
  state->in_progress_large_spots_left = 0;
  state->in_progress_refinery_spots_left = 0;
  

  state->small_spots_left += 9*state->large_spots_left;

  state->sentry_gun_count = 0;
  state->reaper_count = 0;
  state->shock_trooper_count = 0;
  state->marine_count = 2;
  state->veteran_marine_count = 0;
  state->elite_marine_count = 0;
  
  state->shield_battery_max_charge = 700;
  state->generator_charge = 0;
  state->tech_lab_charges = 0;
  
  state->italis_count = 0;
  state->ion_cannon_count = 0;
  
  state->next_event = NULL;
  state->first_structure = NULL;
  
  int i;
  for (i = 0; i < WORLD_UPGRADE_NUM; i++)
  {
    state->upgrades[i] = 0;
  }
  
  struct STRUCTURE_T * command = create_structure(state, STRUCTURE_TYPE_COMMAND_CENTER, STRUCTURE_STATE_BUILDING);
  create_event(state, 0, EVENT_TYPE_BUILD_COMPLETE, command, 0);

  struct STRUCTURE_T * generator = create_structure(state, STRUCTURE_TYPE_GENERATOR, STRUCTURE_STATE_BUILDING);
  create_event(state, 0, EVENT_TYPE_BUILD_COMPLETE, generator, 0);

  struct STRUCTURE_T * barracks = create_structure(state, STRUCTURE_TYPE_BARRACKS, STRUCTURE_STATE_BUILDING);
  create_event(state, 0, EVENT_TYPE_BUILD_COMPLETE, barracks, 0);
  
  create_event(state, 10000000, EVENT_TYPE_CHECK_SURVIVAL, NULL, 0);
}

long score_game(struct GAMESTATE_T * state, enum BUILD_ACTION_T * build_order)
{
  int build_score = 0;
  int i;
  enum BUILD_ACTION_T last_action = BUILD_ACTION_END_BUILD;
  for (i = 0; i < BUILD_ORDER_MAX_LEN; i++)
  {
    build_score -= 0;
    if (build_order[i] == BUILD_ACTION_END_BUILD)
    {
      break;
    }
    // Human Usability
    if (build_order[i] != last_action)
    {
      build_score -= 10;
      last_action = build_order[i];
    }
  }
  
  int struct_score = 0;
  struct STRUCTURE_T * structure = state->first_structure;
  while (structure)
  {
    if (structure->type == STRUCTURE_TYPE_CHEMICAL_PLANT)
    {
      //struct_score += 1000;
    }
    else if (structure->type == STRUCTURE_TYPE_TECH_LAB)
    {
      struct_score += 10000;
    }
    else if (structure->type == STRUCTURE_TYPE_GENERATOR)
    {
      //struct_score += 100 * structure->upgrades[STRUCTURE_UPGRADE_LEVEL];
    }
    structure = structure->next;
  }
  struct_score += state->far_front_base_state * 1000000;
  
  int research_score = 0;
  //research_score += state->upgrades[WORLD_UPGRADE_LEVEL_2_WEAPONS] * 3000;
  //research_score += state->upgrades[WORLD_UPGRADE_LEVEL_3_WEAPONS] * 10000;
  
  float survival = MINIMUM_SURVIVAL_MARGIN - state->min_survival_margin;
  if (survival < 0)
    survival = 0;
  survival = exp(survival);
  
  float end_survival = estimate_survival_margin(state, 0);
  //if (end_survival < 0)
  //  end_survival = 0;
  //end_survival = exp(end_survival);
  
  float shock_troopers = 20 - state->shock_trooper_count;
  if (shock_troopers < 0)
    shock_troopers = 0;
  shock_troopers = exp(shock_troopers/2);
  
  float spinecrawler_effectiveness = estimate_spinecrawler_effectiveness(state);
  
  return (8-survival)*100000 + end_survival*10000 + (100 - shock_troopers)*100 + state->minerals/1 + state->gas/1 + spinecrawler_effectiveness*000 + build_score + struct_score + research_score;
}


int best_score;

void report_progress(enum BUILD_ACTION_T * build_order, int current_iteration, int total_iterations) {
  struct GAMESTATE_T state;
  // Score run
  init_gamestate(&state);
  build_sim(&state, build_order, TIME_HORIZON);
  
   // Trivial fitness for now
  int current_score = score_game(&state, build_order);
  
  printf("\e[1;1H\e[2J\n\n"); // Clear screen
  print_build_order(build_order);
  print_gamestate(&state);
  printf("Current Score = %i \n", current_score);
  printf("Best Score = %i \n", best_score);
  printf("Iterations = %'i / %'i\n\n", current_iteration, total_iterations);
  clean_gamestate(&state);
}

void do_simulated_annealing()
{
  struct GAMESTATE_T state;
  
  enum BUILD_ACTION_T buffer_a[BUILD_ORDER_MAX_LEN] = {BUILD_ACTION_END_BUILD};
  enum BUILD_ACTION_T buffer_b[BUILD_ORDER_MAX_LEN];
  
  enum BUILD_ACTION_T * build_order = buffer_a;
  enum BUILD_ACTION_T * build_order_backup = buffer_b;
  
  best_score = 0;
  int score = 0;
  
  double last_update = getUnixTime();
  
  long max_iterations = SIMULATED_ANNEALING_ITERATIONS;
  long total_iterations = 0;

  long i;
  int j;
  for (i = 0; i < max_iterations; i++) {
    // Make backup
    for (j = 0; j < BUILD_ORDER_MAX_LEN; j++)
    {
      build_order_backup[j] = build_order[j];
      if (build_order[j] == BUILD_ACTION_END_BUILD)
        break;
    }
    
    // Modify build order
    int len = j;
    int idx = fast_rand()%(len+1);
    int inst = fast_rand()%4;
    
    /* Don't Overflow */
    if (len == BUILD_ORDER_MAX_LEN-1)
    {
      inst = 1;
      idx = BUILD_ORDER_MAX_LEN-2;
    }
    
    if (inst == 0 && idx < len)
    {
      // Delete at idx
      for (;idx < len; idx++)
      {
        build_order[idx] = build_order[idx + 1];
      }
    }
    else if (inst == 1 && idx < len)
    {
      // Swap idx and idx+1
      enum BUILD_ACTION_T swap;
      swap = build_order[idx];
      build_order[idx] = build_order[idx + 1];
      build_order[idx + 1] = swap;
    }
    else if (inst == 2 && idx < len)
    {
      // Rewrite at idx

      // Select action at random
      build_order[idx] = fast_rand()%BUILD_ACTION_END_BUILD;
    }
    else
    {
      // Insert at idx
      for (;len >= idx; len--)
      {
        build_order[len + 1] = build_order[len];
      }
      // Select action at random
      build_order[idx] = fast_rand()%BUILD_ACTION_END_BUILD;
    }

    
    // Check sanity
    int k;
    for (k = 0; k < BUILD_ORDER_MAX_LEN; k++)
    {
      if (build_order[k] == BUILD_ACTION_END_BUILD)
        break;
      if (build_order[k] > BUILD_ACTION_END_BUILD)
        printf("INVALID OP!!!\n");
      if (k == BUILD_ORDER_MAX_LEN-1)
        printf("We have lost END_BUILD.\n");
    }
    
    // Score run
    init_gamestate(&state);
    build_sim(&state, build_order, TIME_HORIZON);
    clean_gamestate(&state);
    
    int new_score = score_game(&state, build_order);

    // Simulated annealing to determine if this passes
    int does_pass = 0;
    if (new_score > score)
      does_pass = 1;
    else {
      float p = exp(-(score - new_score)/(((float)(max_iterations - i)/(float)(max_iterations))*100));
      does_pass = fast_rand() < 32767.0 * p;
    }
      
    if (does_pass) {
      if (new_score > best_score)
        best_score = new_score;
      // Keep the new code
      score = new_score;
      
    }
    else {
      // Revert to the old code
      enum BUILD_ACTION_T * swap;
      swap = build_order;
      build_order = build_order_backup;
      build_order_backup = swap;
    }
    if ((!(i % 100) && getUnixTime()-last_update > 0.5)) {
      last_update += 0.5;
      report_progress(build_order, i, max_iterations);
    }
    total_iterations++;
  }
  
  
  report_progress(build_order, i, max_iterations);
}

//#define BUILD_SIM_TEST
//#define FIGHT_SIM_TEST

int main()
{
#if defined(BUILD_SIM_TEST)

  struct GAMESTATE_T state;
  enum BUILD_ACTION_T build_order[] =
  {
    BUILD_ACTION_TAKE_FAR_BASE,
    BUILD_ACTION_BUILD_SCV,
    BUILD_ACTION_BUILD_SCV,
    BUILD_ACTION_BUILD_SCV,
    BUILD_ACTION_BACK_ROCKS_CLIP,
    BUILD_ACTION_BUILD_REFINERY,
    BUILD_ACTION_BUILD_REFINERY,
    //BUILD_ACTION_BUILD_SHOCK_TROOPER,
    BUILD_ACTION_BUILD_REFINERY,
    BUILD_ACTION_BUILD_REFINERY,
    BUILD_ACTION_BUILD_REFINERY,
    BUILD_ACTION_CHEATER_BASE_CLIP,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_UPGRADE_REFINERY,
    BUILD_ACTION_UPGRADE_REFINERY,
    BUILD_ACTION_UPGRADE_REFINERY,
    BUILD_ACTION_UPGRADE_REFINERY,
    //BUILD_ACTION_BUILD_SHOCK_TROOPER,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    //BUILD_ACTION_BUILD_SHOCK_TROOPER,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_BUILD_FRUIT_FARM,
    BUILD_ACTION_BUILD_MARINE,
    BUILD_ACTION_BUILD_MARINE,
    BUILD_ACTION_BUILD_MARINE,
    BUILD_ACTION_BUILD_WALL,
    BUILD_ACTION_BUILD_CONSTRUCTION_YARD,
    BUILD_ACTION_BUILD_COW_FARM_AND_COW,
    BUILD_ACTION_BUILD_COW_FARM_AND_COW,
    BUILD_ACTION_BUILD_COW_FARM_AND_COW,
    BUILD_ACTION_BUILD_COW_FARM_AND_COW,
    BUILD_ACTION_BUILD_COW_FARM_AND_COW,
    BUILD_ACTION_BUILD_COW_FARM_AND_COW,
    BUILD_ACTION_BUILD_COW_FARM_AND_COW,
    BUILD_ACTION_BUILD_COW_FARM_AND_COW,
    BUILD_ACTION_BUILD_COW_FARM_AND_COW,
    BUILD_ACTION_AUTO_UPGRADE_GENERATOR_ON,
    BUILD_ACTION_BUILD_SENTRY_GUN,
    BUILD_ACTION_BUILD_SENTRY_GUN,
    BUILD_ACTION_BUILD_SENTRY_GUN,
    BUILD_ACTION_BUILD_SENTRY_GUN,
    BUILD_ACTION_BUILD_SENTRY_GUN,
    BUILD_ACTION_BUILD_SENTRY_GUN,
    BUILD_ACTION_BUILD_SENTRY_GUN,
    BUILD_ACTION_BUILD_SENTRY_GUN,
    BUILD_ACTION_BUILD_SENTRY_GUN,
    BUILD_ACTION_BUILD_SENTRY_GUN,
    BUILD_ACTION_BUILD_SENTRY_GUN,
    BUILD_ACTION_BUILD_SENTRY_GUN,
    BUILD_ACTION_AUTO_UPGRADE_GENERATOR_OFF,
    BUILD_ACTION_AUTO_BUILD_SHOCK_TROOPER,
    BUILD_ACTION_BUILD_WALL,
    BUILD_ACTION_END_BUILD,
  };
  init_gamestate(&state);
  build_sim(&state, build_order, 15UL*60UL*1000000UL);
    
  print_build_order(build_order);
  print_gamestate(&state);

  //clean_gamestate(&state);
//#elif defined(FIGHT_SIM_TEST)
  //struct GAMESTATE_T state;
  //init_gamestate(&state);
  
  //state.elite_marine_count = 100;
  //state.shield_battery_max_charge = 10000;
  //state.upgrades[WORLD_UPGRADE_ENERGY_CONSERVATION_3] = 2;
  //state.generator_charge = 5000;
  
  /*
  struct FIGHT_SIM_SCENARIO_T scenario = 
  {
    .charge_time = 0,
    .fight_time = 10,
    
    .terran_spread_factor = 0.8,
    
    .zerg_armor = 0,
    .zerg_density = 1/30,
    
    .is_zerg_structure = 1,
    .zerg_structure_hp = 250,
    .zerg_structure_damage = 30,
    .zerg_structure_fire_rate = 1.85,
    .zerg_structure_splash = 1.0,
  };
  */
  struct FIGHT_SIM_SCENARIO_T scenario = 
  {
    .charge_time = 0,
    .fight_time = 10,
    
    .terran_spread_factor = 0.8,
    .terran_use_base_defenses = 0,
    
    .zerg_armor = 2,
    .zerg_density = 1/30,
    
    .is_zerg_structure = 1,
    .zerg_hp = 1300,
    .zerg_structure_damage = 70,
    .zerg_structure_fire_rate = 1.5,
    .zerg_structure_splash = 8.0,
  };
  
  struct FIGHT_SIM_RESULTS_T results;
  fight_sim(&state, &scenario, &results);
  
  printf("Damage Dealt: %f\n", results.damage_dealt);
  printf("Energy Used: %f\n", results.energy_used);
  
  printf("Elite Marines Lost: %f\n", results.elite_marines_lost);
  printf("Veteran Marines Lost: %f\n", results.veteran_marines_lost);
  printf("Marines Lost: %f\n", results.marines_lost);
  printf("Reapers Lost: %f\n", results.reapers_lost);
  printf("Shock Troopers Lost: %f\n", results.shock_troopers_lost);
  printf("Italises Lost: %f\n", results.italises_lost);
  
  estimate_survival_margin(&state, 1);
  
#else
  do_simulated_annealing();
#endif
}
