#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

#include "cache.h"

/*************************************************************************/
/******************************** CLASS **********************************/

namespace{
    /* ATTRIBUTES */

    //perceptron bits
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

    //Vars for keeping track of previous PCs
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
        uint64_t address; //address of our stored cache block. Only use 15 bits to check tags (see sampler for details)
        uint64_t lru_bits; //sampler LRU. For simulation purposes we use current clock cycle, in actuality number of bits used would be NUM_WAYS
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
    struct PredictorTable
    {
        /* data */
    };
    

    //vectors representing physical tables (vectors are mapped to their corresponding cache via its address)

    std::map<CACHE*, std::vector<SamplerEntry>> sampler_table;   //sampler vector table - indexed via (set_id * sampler_num_ways + way_id)
    std::map<CACHE*, std::vector<uint64_t>> sampler_cache_sets; //value V is cache set id, index I is sampler set ID.

    //TODO: prediction table vector
    std::map<CACHE*, std::vector<PredictorTable>> predictor_table;
    

    //vectors representing extra cache block information (block 10 would have lru bits stored in entry 10 of this vector)
    std::map<CACHE*, std::vector<uint64_t>> lru_bits; //Predictor lru vector
    std::map<CACHE*, std::vector<uint64_t>> reuse_bits; //Predictor reuse vector


}

/*************************************************************************/
/******************************* END CLASS *******************************/





/*************************************************************************/
/************************ INITIALIZE REPLACEMENT *************************/

void CACHE::initialize_replacement() {
    //initialize sampler vector
    ::sampler_table[this] = std::vector<SamplerEntry>(::sampler_sets * ::sampler_num_ways);

    //random number (borrowing this idea from other champsim replacement code because I don't know whether I'm allowed to include new libs like <time> and <cstdlib>
    uint64_t random_number = 122053342;
    uint64_t random_offset = 1146;
    for (int i = 0; i < sampler_sets; i++) {
        ::sampler_cache_sets[this].push_back(random_number % sampler_sets);
        random_number = random_number * random_number + random_offset;
    }


    //TODO: initialize extra cache reuse bits and backup LRU bits --> ::last_used_cycles[this] = std::vector<uint64_t>(NUM_SET * NUM_WAY); 
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
        //first: check reuse bits for a 0.
        //second (as backup if all are marked for reuse): check backup LRU bits
    



  //NOTE: code below is from SHIP, similar to what we need to do except instead of using maxRRPV we use above criteria
  //
  //
  // look for the maxRRPV line
  //auto begin = std::next(std::begin(::rrpv_values[this]), set * NUM_WAY);
  //auto end = std::next(begin, NUM_WAY);
  //auto victim = std::find(begin, end, ::maxRRPV);
  //while (victim == end) {
  //  for (auto it = begin; it != end; ++it)
  //    ++(*it);

  //  victim = std::find(begin, end, ::maxRRPV);
  //}

  //assert(begin <= victim);
  //return static_cast<uint32_t>(std::distance(begin, victim)); // cast pretected by prior assert
}

/*************************************************************************/
/*************************** END FIND VICTIM *****************************/





/*************************************************************************/
/************************** UPDATE REPLACMENT ****************************/

void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit)
{
    //NOTE: using bitmask to get rid of extra bits because our theoretical cache only stores 8 bits per feature
    //feature values
    pc_3 = pc_2;
    pc_2 = pc_1;
    pc_1 = pc_0;
    pc_0 = ip;

    //feature hashes
    uint64_t pc_3_hash = ((pc_3 >> 3) ^ pc_0) & champsim::bitmask(8);
    uint64_t pc_2_hash = ((pc_2 >> 2) ^ pc_0) & champsim::bitmask(8);
    uint64_t pc_1_hash = ((pc_1 >> 1) ^ pc_0) & champsim::bitmask(8);
    uint64_t pc_0_hash = ((pc_0 >> 2) ^ pc_0) & champsim::bitmask(8);
    uint64_t tag_1_hash = ((full_addr >> 4) ^ pc_0) & champsim::bitmask(8);
    uint64_t tag_2_hash = ((full_addr >> 7) ^ pc_0) & champsim::bitmask(8);

    //TODO: calculate yout (dont forget to update value in the sampler)
    uint64_t yout;
        //use hashed features as index in feature table to access weights
        //sum all the weights and calc yout
        
    //TODO: update reuse bit with current yout
    if (yout < replace_threshold) {
        //set re-use bit of current cache block to 1
    }
    



    //TODO: do sampler stuff
    /* SAMPLER STUFF */
    //check if sampled set, and if we find one then do training
    auto sample = std::find(std::begin(::sampler_cache_sets[this]), std::end(::sampler_cache_sets[this]), set);
    if (sample != std::end(::sampler_cache_sets[this])) {

        //start and end of our sample set
        auto start = std::begin(::sampler_table[this]) + sampler_num_ways * (sample - std::begin(::sampler_cache_sets[this]));
        auto end = start + ::sampler_num_ways;

        //check for tag match in the set
        //very fun lambda function that compares 15 tag bits of sampler address with 15 tag bits of incoming block address.
        auto compare_addr = [cache_address = full_addr, mask = 15, shift = (OFFSET_BITS + champsim::lg2(::sampler_sets))](auto sampler)
            {return ((cache_address >> shift) & champsim::bitmask(15)) == ((sampler.address >> shift) & champsim::bitmask(15)); };
        auto tag_found = std::find_if(start, end, compare_addr);

        //if tag in sampler, we update yout, update input hash values, update LRU value, and then update weights based on prediction
        if (tag_found != end) {
            //decrement weights of feature table for current features if Yout is above the theta threshold
            SamplerEntry* old_sample = &(*tag_found);
            if (old_sample->yout > sampler_threshold)
                //TODO: decrement weights
            
            //update the current sample's hashed feature values, Yout, and LRU value
            *old_sample = { full_addr, current_cycle, yout, pc_0_hash, pc_1_hash, pc_2_hash, pc_3_hash, tag_1_hash, tag_2_hash };
        }
        else {
            //find LRU (lowest clock cycle) sample within a given set
            auto compare_lru = [](const SamplerEntry& a, const SamplerEntry& b) {return a.lru_bits < b.lru_bits; };
            SamplerEntry* throwaway_sample = &(*std::min_element(start, end, compare_lru));

            //check stored yout of that sample, compare to threshold, if below the theta threshold, we increment the weights for the corresponding feature maps
            if (throwaway_sample->yout < sampler_threshold) {
                //TODO: increment weights
            }
                
            //replace sample struct with new struct that has current hashed feature values as well as the current cache block's Yout, tag, and lru (clock cycle)
            *throwaway_sample = { full_addr, current_cycle, yout, pc_0_hash, pc_1_hash, pc_2_hash, pc_3_hash, tag_1_hash, tag_2_hash };
        }
    }

    //NOTE: for saturating counters, dont forget to make sure they saturate at the algorithm's specifcation
    //  for example, a "6 bit saturating counter" that uses int_32t, should saturate between -32 or 32, not 2 billion or whatever


  //LRU stuff (backup)
  // Mark the way as being used on the current cycle
  //
  //if (!hit || access_type{type} != access_type::WRITE) // Skip this for writeback hits
   // ::last_used_cycles[this].at(set * NUM_WAY + way) = current_cycle;
}

/*************************************************************************/
/************************* END UPDATE REPLACMENT *************************/





/*************************************************************************/
/***************************** FINAL STATS *******************************/

void CACHE::replacement_final_stats() {}

/*************************************************************************/
/*************************** END FINAL STATS *****************************/