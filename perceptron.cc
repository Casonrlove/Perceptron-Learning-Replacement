#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

#include "cache.h"

/*************************************************************************/
/******************************** CLASS **********************************/

class perceptron
{
    /* ATTRIBUTES */

    //percpetron bits
    size_t perceptron_bits    = 0;

    //perceptron history
    size_t perceptron_history = 0;

    //number of perceptrons
    size_t num_of_perceptrons = 0;

    //FEATURES (from the paper)
    // 1) PCi shifted right by 2, for i = 0.
    //2) The three values of PCi shifted right by i, for 1 ≤ i ≤ 3.
    //3) The tag of the current block shifted right by 4,
    //4) The tag of the current block shifted right by 7.
    //PC values for 3 most recent LLC accesses (pc_0 is curr access)
    size_t pc_0 = 0;
    size_t pc_1 = 0;
    size_t pc_2 = 0;
    size_t pc_3 = 0;


    //thresholds
    size_t bypass_threshold = 3;
    size_t replace_threshold = 124;
    size_t sampler_threshold = 68;


    //sampler entry struct - 
    struct SamplerEntry {
        uint64_t partial_cache_tag; //ID tag, only use first 15 bits
        uint64_t lru_bits; //sampler LRU, only use first 4 bits, lower = more recently used
        uint64_t yout; //most recent prediction calculation value, 9 bits used
        uint64_t pc_0_hash; //feature 1, 8 bits used
        uint64_t pc_1_hash; //feature 2, 8 bits used
        uint64_t pc_2_hash; //feature 3, 8 bits used
        uint64_t pc_3_hash; //feature 4, 8 bits used
        uint64_t tag_shift_1; //feature 5, 8 bits used
        uint64_t tag_shift_2; //feature 6, 8 bits used
    };
    size_t sampler_sets = 64;
    size_t sampler_num_ways = 16;
    size_t sampler_table_size = sampler_sets * sampler_num_ways;
    //TODO: prediction table struct


    //vectors representing physical tables (vectors are mapped to their corresponding cache via its address)

    std::map<CACHE*, std::vector<SamplerEntry>> sampler_table;   //sampler vector table - indexed via (set_id * sampler_num_ways + way_id)
    //TODO: prediction table vector


    //vectors representing extra cache block information (block 10 has lru bits stored in entry 10 of this vector)
    std::map<CACHE*, std::vector<uint64_t>> lru_bits; //Predictor lru vector
    std::map<CACHE*, std::vector<uint64_t>> reuse_bits; //Predictor reuse vector


    
    public:

        void update(/* HISTORY */)
        {
            
        }

        void predict(/* NEXT REPLACMENT */)
        {

        }
};

/*************************************************************************/
/******************************* END CLASS *******************************/





/*************************************************************************/
/************************ INITIALIZE REPLACEMENT *************************/

void CACHE::initialize_replacement() { 
    //TODO: initialize sampler
    //TODO: initialize extra cache bits and backup LRU bits --> ::last_used_cycles[this] = std::vector<uint64_t>(NUM_SET * NUM_WAY); 
    //TODO: initialize feature values

}

/*************************************************************************/
/********************** END INITIALIZE REPLACEMENT ***********************/





/*************************************************************************/
/***************************** FIND VICTIM *******************************/

// find replacement victim
uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
    //TODO: FIND REPLACEMENT
        //first: check reuse bits
        //second (as backup if all are marked for reuse): check LRU bits
    
    //NOTE: code below is from SHIP, similar to what we need to do except instead of using maxRRPV we use above criteria



  // look for the maxRRPV line
  auto begin = std::next(std::begin(::rrpv_values[this]), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);
  auto victim = std::find(begin, end, ::maxRRPV);
  while (victim == end) {
    for (auto it = begin; it != end; ++it)
      ++(*it);

    victim = std::find(begin, end, ::maxRRPV);
  }

  assert(begin <= victim);
  return static_cast<uint32_t>(std::distance(begin, victim)); // cast pretected by prior assert
}

/*************************************************************************/
/*************************** END FIND VICTIM *****************************/





/*************************************************************************/
/************************** UPDATE REPLACMENT ****************************/

void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
    //TODO: check if hit or miss
    //TODO: update feature values (pc value, cache tag)... dont forget to shift as well as truncate extra bits (see above)
    //TODO: calculate yout (dont forget to update value in the sampler)
    //TODO: if miss, check for bypass or decide to replace
    //TODO: if hit, update reuse bit with current features
    //TODO: do sampler stuff 
    //  (check if sampled set, check if tag in sampler, store yout & hashed features in sampler with block, handle replacement if necessary)

    //NOTE: for saturating counters, dont forget to make sure they saturate at the algorithm's specifcation
    //  for example, a "6 bit saturating counter" that uses int_32t, should saturate between -32 or 32, not 2 billion or whatever
  



  // Mark the way as being used on the current cycle
  if (!hit || access_type{type} != access_type::WRITE) // Skip this for writeback hits
    ::last_used_cycles[this].at(set * NUM_WAY + way) = current_cycle;
}

/*************************************************************************/
/************************* END UPDATE REPLACMENT *************************/





/*************************************************************************/
/***************************** FINAL STATS *******************************/

void CACHE::replacement_final_stats() {}

/*************************************************************************/
/*************************** END FINAL STATS *****************************/