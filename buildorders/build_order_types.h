#ifndef BUILD_ORDER_TYPES_H
#define BUILD_ORDER_TYPES_H

#define INNER_BASE_LARGE_SPOTS 1
#define INNER_BASE_SMALL_SPOTS 21 + 9*INNER_BASE_LARGE_SPOTS
#define INNER_BASE_REFINERY_SPOTS 0

#define BACK_BASE_LARGE_SPOTS 2
#define BACK_BASE_SMALL_SPOTS 3 + 9*BACK_BASE_LARGE_SPOTS
#define BACK_BASE_REFINERY_SPOTS 1

#define NEAR_FRONT_BASE_LARGE_SPOTS 3
#define NEAR_FRONT_BASE_SMALL_SPOTS 8 + 9*NEAR_FRONT_BASE_LARGE_SPOTS
#define NEAR_FRONT_BASE_REFINERY_SPOTS 1

#define FAR_FRONT_BASE_LARGE_SPOTS 5
#define FAR_FRONT_BASE_SMALL_SPOTS 15 + 9*FAR_FRONT_BASE_LARGE_SPOTS
#define FAR_FRONT_BASE_REFINERY_SPOTS 2

//#define CHEATER_BASE_AVAILABLE

#ifdef CHEATER_BASE_AVAILABLE
#define CHEATER_BASE_LARGE_SPOTS 4
#define CHEATER_BASE_SMALL_SPOTS 29 + 9*CHEATER_BASE_LARGE_SPOTS
#define CHEATER_BASE_REFINERY_SPOTS 0
#else
#define CHEATER_BASE_LARGE_SPOTS 0
#define CHEATER_BASE_SMALL_SPOTS 0
#define CHEATER_BASE_REFINERY_SPOTS 0
#endif

enum BUILD_ACTION_T
{
  BUILD_ACTION_NONE,
  BUILD_ACTION_BUILD_SCV,
  BUILD_ACTION_BUILD_SHEEP_FARM,
  BUILD_ACTION_BUILD_SHEEP_FARM_AND_SHEEP,
  BUILD_ACTION_BUILD_SHEEP,
  BUILD_ACTION_BUILD_COW_FARM,
  BUILD_ACTION_BUILD_COW_FARM_AND_COW,
  BUILD_ACTION_BUILD_COW,
  BUILD_ACTION_BUILD_CONVERTER,
  BUILD_ACTION_BUILD_FRUIT_FARM,
  BUILD_ACTION_BUILD_REFINERY,
  BUILD_ACTION_BUILD_CHEMICAL_PLANT,
  BUILD_ACTION_BUILD_CONSTRUCTION_YARD,
  BUILD_ACTION_BUILD_TECH_LAB,
  BUILD_ACTION_BUILD_ARMORY,
  BUILD_ACTION_UPGRADE_REFINERY,
  BUILD_ACTION_UPGRADE_SHEEP_FARM,
  BUILD_ACTION_UPGRADE_CHEMICAL_PLANT,
  BUILD_ACTION_UPGRADE_ADVANCED_BUILD,
  BUILD_ACTION_UPGRADE_UNIFIED_ARMOR,
  BUILD_ACTION_UPGRADE_IMPROVED_FARMING,
  BUILD_ACTION_UPGRADE_LEVEL_2_WEAPONS,
  BUILD_ACTION_UPGRADE_LEVEL_3_WEAPONS,
  BUILD_ACTION_SET_CONVERTER_NONE,
  BUILD_ACTION_SET_CONVERTER_GAS,
  BUILD_ACTION_SET_CONVERTER_MINERALS,
  BUILD_ACTION_BUILD_REAPER,
  BUILD_ACTION_BUILD_SHOCK_TROOPER,
  BUILD_ACTION_BUILD_MARINE,
  BUILD_ACTION_BUILD_VETERAN_MARINE,
  BUILD_ACTION_BUILD_ELITE_MARINE,
  BUILD_ACTION_UPGRADE_GENERATOR,
  BUILD_ACTION_AUTO_BUILD_REAPER,
  BUILD_ACTION_AUTO_BUILD_SHOCK_TROOPER,
  BUILD_ACTION_AUTO_BUILD_MARINE,
  BUILD_ACTION_AUTO_BUILD_VETERAN_MARINE,
  BUILD_ACTION_AUTO_BUILD_ELITE_MARINE,
  BUILD_ACTION_AUTO_BUILD_BARRACKS_OFF,
  BUILD_ACTION_AUTO_UPGRADE_GENERATOR_ON,
  BUILD_ACTION_AUTO_UPGRADE_GENERATOR_OFF,
  BUILD_ACTION_UPGRADE_OVERCHARGE,
  BUILD_ACTION_UPGRADE_ENERGY_CONSERVATION_1,
  BUILD_ACTION_UPGRADE_ENERGY_CONSERVATION_2,
  BUILD_ACTION_UPGRADE_ENERGY_CONSERVATION_3,
  BUILD_ACTION_UPGRADE_SHATTERING_LASER,
  BUILD_ACTION_UPGRADE_DEFLECTIVE_SHATTER,
  BUILD_ACTION_UPGRADE_PENETRATING_LASER,
  BUILD_ACTION_UPGRADE_ELITE_SCOPE,
  BUILD_ACTION_CHARGE_TECH_LAB,
  BUILD_ACTION_AUTO_CHARGE_TECH_LAB_ON,
  BUILD_ACTION_AUTO_CHARGE_TECH_LAB_OFF,
  BUILD_ACTION_BUILD_ITALIS,
  BUILD_ACTION_ION_CANNON,
  BUILD_ACTION_BUILD_BASIC_SHIELD_BATTERY,
  BUILD_ACTION_BUILD_ADVANCED_SHIELD_BATTERY,
  BUILD_ACTION_BACK_ROCKS_CLIP,
  BUILD_ACTION_CHEATER_BASE_CLIP,
  BUILD_ACTION_TAKE_NEAR_BASE,
  BUILD_ACTION_TAKE_FAR_BASE,
  BUILD_ACTION_BUILD_WALL,
  BUILD_ACTION_BUILD_SENTRY_GUN,
  BUILD_ACTION_UPGRADE_FORTIFICATION,
  BUILD_ACTION_UPGRADE_SENTRY_SUPERCHARGE,
  BUILD_ACTION_END_BUILD,
};

static const char *build_action_strings[] = 
{
  "None",
  "Build SCV",
  "Build Sheep Farm",
  "Build Sheep Farm And Sheep",
  "Build Sheep",
  "Build Cow Farm",
  "Build Cow Farm and Cow",
  "Build Cow",
  "Build Converter",
  "Build Fruit Farm",
  "Build Refinery",
  "Build Chemical Plant",
  "Build Construction Yard",
  "Build Tech Lab",
  "Build Armory",
  "Upgrade Refinery",
  "Upgrade Sheep Farm",
  "Research Chemical Plant",
  "Research Advanced Build",
  "Research Unified Armor",
  "Research Improved Farming",
  "Research Tier 2 Equipment",
  "Research Tier 3 Equipment",
  "Set Converter to None",
  "Set Converter to Gas",
  "Set Converter to Minerals",
  "Build Reaper",
  "Build Shock Trooper",
  "Build Marine",
  "Build Veteran Marine",
  "Build Elite Marine",
  "Upgrade Generator",
  "Auto Build Reaper",
  "Auto Build Shock Trooper",
  "Auto Build Marine",
  "Auto Build Veteran Marine",
  "Auto Build Elite Marine",
  "Auto Build Barracks Off",
  "Generator Auto Upgrade On",
  "Generator Auto Upgrade Off",
  "Research Overcharge",
  "Research Energy Conservation 1",
  "Research Energy Conservation 2",
  "Research Energy Conservation 3",
  "Research Shattering Laser",
  "Research Deflective Shatter",
  "Research Penetrating Laser",
  "Research Elite Scope",
  "Charge Tech Lab",
  "Tech Lab Auto Charge On",
  "Tech Lab Auto Charge Off",
  "Build Italis",
  "Use Ion Cannon",
  "Build Basic Shield Battery",
  "Build Advanced Shield Battery",
  "Perform Back Rocks Clip",
  "Perform Cheater Base Clip",
  "Claim Small Outer Base",
  "Claim Full Outer Base",
  "Build Full Outer Base Wall",
  "Build Sentry Gun",
  "Research Fortification",
  "Research Sentry Supercharge",
  "END_BUILD"
};


enum STRUCTURE_STATE_T
{
  STRUCTURE_STATE_BUILDING,
  STRUCTURE_STATE_NORMAL,
  STRUCTURE_STATE_WORKING,
  STRUCTURE_STATE_CONVERT_TO_GAS,
  STRUCTURE_STATE_CONVERT_TO_MINERALS,
  STRUCTURE_STATE_CONVERTER_OFF,
};

enum STRUCTURE_TYPE_T
{
  STRUCTURE_TYPE_COMMAND_CENTER,
  STRUCTURE_TYPE_REFINERY,
  STRUCTURE_TYPE_CHEMICAL_PLANT,
  STRUCTURE_TYPE_FRUIT_FARM,
  STRUCTURE_TYPE_SHEEP_FARM,
  STRUCTURE_TYPE_COW_FARM,
  STRUCTURE_TYPE_COW,
  STRUCTURE_TYPE_SHEEP,
  STRUCTURE_TYPE_CONSTRUCTION_YARD,
  STRUCTURE_TYPE_TECH_LAB,
  STRUCTURE_TYPE_ARMORY,
  STRUCTURE_TYPE_CONVERTER,
  STRUCTURE_TYPE_GENERATOR,
  STRUCTURE_TYPE_BARRACKS,
  STRUCTURE_TYPE_NUM_STRUCTURE_TYPES,
};

static const char *structure_type_strings[] = 
{
  "Command Center",
  "Refinery",
  "Chemical Plant",
  "Fruit Farm",
  "Sheep Farm",
  "Cow Farm",
  "Cow",
  "Sheep",
  "Construction Yard",
  "Tech Lab",
  "Armory",
  "Converter",
  "Generator",
  "Barracks"
};

enum WORLD_UPGRADE_T
{
  WORLD_UPGRADE_ANY_TECH_STRUCTURE,
  WORLD_UPGRADE_CONSTRUCTION_YARD,
  WORLD_UPGRADE_TECH_LAB,
  WORLD_UPGRADE_ARMORY,
  WORLD_UPGRADE_ADVANCED_BUILD,
  WORLD_UPGRADE_UNIFIED_ARMOR,
  WORLD_UPGRADE_CHEMICAL_PLANT,
  WORLD_UPGRADE_LEVEL_2_WEAPONS,
  WORLD_UPGRADE_LEVEL_3_WEAPONS,
  WORLD_UPGRADE_IMPROVED_FARMING,
  WORLD_UPGRADE_OVERCHARGE,
  WORLD_UPGRADE_ENERGY_CONSERVATION_1,
  WORLD_UPGRADE_ENERGY_CONSERVATION_2,
  WORLD_UPGRADE_ENERGY_CONSERVATION_3,
  WORLD_UPGRADE_SHATTERING_LASER,
  WORLD_UPGRADE_DEFLECTIVE_SHATTER,
  WORLD_UPGRADE_PENETRATING_LASER,
  WORLD_UPGRADE_ELITE_SCOPE,
  WORLD_UPGRADE_FORTIFICATION,
  WORLD_UPGRADE_SENTRY_SUPERCHARGE,
  WORLD_UPGRADE_NUM
};

static const char *world_upgrade_strings[] = 
{
  "Any Tech Structure",
  "Construction Yard",
  "Tech Lab",
  "Armory",
  "Advanced Build",
  "Unified Armor",
  "Chemical Plant",
  "Tier 2 Equipment",
  "Tier 3 Equipment",
  "Improved Farming",
  "Overcharge",
  "Energy Conservation 1",
  "Energy Conservation 2",
  "Energy Conservation 3",
  "Shattering Laser",
  "Deflective Shatter",
  "Penetrating Laser",
  "Elite Scope",
  "Fortification",
  "Sentry Supercharge",
  "Invalid Upgrade"
};


enum STRUCTURE_UPGRADE_T
{
  STRUCTURE_UPGRADE_LEVEL,
  STRUCTURE_UPGRADE_SPENT,
  STRUCTURE_UPGRADE_NUM
};

struct STRUCTURE_T
{
  long time_us;
  enum STRUCTURE_STATE_T state;
  struct EVENT_T *first_event;
  enum STRUCTURE_TYPE_T type;
  struct STRUCTURE_T * next;
  unsigned char upgrades[STRUCTURE_UPGRADE_NUM];
};

enum EVENT_TYPE_T
{
  EVENT_TYPE_MINERAL_INCOME,
  EVENT_TYPE_GAS_INCOME,
  EVENT_TYPE_ENERGY_INCOME,
  EVENT_TYPE_UPGRADE_COMPLETE,
  EVENT_TYPE_BUILD_COMPLETE,
  EVENT_TYPE_UNIT_BUILD_COMPLETE,
  EVENT_TYPE_ABILITY_COOLDOWN,
  EVENT_TYPE_AUTO_BUILD_POLL,
  EVENT_TYPE_CHECK_SURVIVAL,
};

static const char *event_type_strings[] = 
{
  "Mineral Income",
  "Gas Income",
  "Energy Income",
  "Upgrade Complete",
  "Build Complete",
  "Unit Build Complete",
  "Ability Cooldown",
  "Auto Build Poll",
  "Check Survival"
};

struct EVENT_T
{
  int period_us;
  long time_us;
  enum EVENT_TYPE_T event_type;
  struct STRUCTURE_T * structure;
  struct EVENT_T * next_event_in_time;
  struct EVENT_T * next_event_in_structure;
  enum BUILD_ACTION_T action;
};


struct GAMESTATE_T
{
  float min_survival_margin;
  
  int gas;
  int minerals;
  
  int free_animal_slot_count;
  long time_us;
  
  int free_scv_count;
  int live_scv_count;
  
  int large_spots_left;
  int in_progress_large_spots_left;
  int small_spots_left;
  int in_progress_small_spots_left;
  int refinery_spots_left;
  int in_progress_refinery_spots_left;
  
  int italis_count;
  int ion_cannon_count;
  int reaper_count;
  int shock_trooper_count;
  int marine_count;
  int veteran_marine_count;
  int elite_marine_count;
  
  int sentry_gun_count;
  
  int front_wall_state;
  int near_front_base_state;
  int far_front_base_state;
  int back_rocks_state;
  int cheater_base_state;
  
  int generator_charge;
  int shield_battery_max_charge;
  int tech_lab_charges;
  
  float charge_used_per_sec_for_fighting;
  
  unsigned char upgrades[WORLD_UPGRADE_NUM];
  
  struct EVENT_T * next_event;
  struct STRUCTURE_T * first_structure;
};

#endif
