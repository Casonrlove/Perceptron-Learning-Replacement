#include <algorithm>
#include <cassert>
#include <map>
#include <vector>

#include "cache.h"

/*************************************************************************/
/******************************** CLASS **********************************/

namespace {
    //debug stuff
    size_t lru_counter;
    size_t reuse_counter;
    size_t sample_count;
    
    /* ATTRIBUTES */

    //Vars for keeping track of previous PCs
    uint64_t pc_0 = 0;
    uint64_t pc_1 = 0;
    uint64_t pc_2 = 0;
    uint64_t pc_3 = 0;

    //thresholds
    constexpr int32_t bypass_threshold = 3;
    constexpr int32_t replace_threshold = 124;
    constexpr int32_t sampler_threshold = 67;

    //sampler entry struct -
    struct SamplerEntry {
        uint64_t address; //address of our stored cache block. Only use 15 bits to check tags (see sampler for details)
        uint64_t lru_bits; //sampler LRU. For simulation purposes we use current clock cycle, in actuality number of bits used would be NUM_WAYS
        int32_t yout; //most recent prediction calculation value, 9 bits used
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

    // 6 feature tables (from the paper), 256 entries of 6 bits saturating counters [-32, +31]
    // index via -> (hashed feature value) XOR (PC)
    std::vector<int32_t> pc_0_feature(256); // 1)PCi shifted right by 2, for i = 0.
    std::vector<int32_t> pc_1_feature(256); // 2)The three values of PCi shifted right by i, for 1 ≤ i ≤ 3.
    std::vector<int32_t> pc_2_feature(256);
    std::vector<int32_t> pc_3_feature(256);
    std::vector<int32_t> tag_1_feature(256); // 3)The tag of the current block shifted right by 4,
    std::vector<int32_t> tag_2_feature(256); // 4)The tag of the current block shifted right by 7.

    //vectors representing physical tables (vectors are mapped to their corresponding cache via its address)

    std::map<CACHE*, std::vector<SamplerEntry>> sampler_table; //sampler vector table - indexed via (set_id * sampler_num_ways + way_id)
    std::map<CACHE*, std::vector<uint64_t>> sampler_cache_sets; //value V is cache set id, index I is sampler set ID.

    //vectors representing extra cache block information (block 10 would have lru bits stored in entry 10 of this vector)
    std::map<CACHE*, std::vector<uint64_t>> lru_bits; //Predictor lru vector
    std::map<CACHE*, std::vector<bool>> reuse_bits; //Predictor reuse vector
}

/*************************************************************************/
/******************************* END CLASS *******************************/





/*************************************************************************/
/************************ INITIALIZE REPLACEMENT *************************/

void CACHE::initialize_replacement() {
    //initialize sampler vector, cache reuse bits and backup LRU bits
    ::sampler_table[this] = std::vector<SamplerEntry>(::sampler_sets * ::sampler_num_ways);
    ::reuse_bits[this] = std::vector<bool>(NUM_SET * NUM_WAY, false);
    ::lru_bits[this] = std::vector<uint64_t>(NUM_SET * NUM_WAY);

    for (size_t i = 0; i < sampler_sets; i++) {
        ::sampler_cache_sets[this].push_back(i * (NUM_SET / sampler_sets));
    }
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
    uint64_t pc_0_hash = ((pc_0 >> 2) ^ pc_0) & champsim::bitmask(8);
    uint64_t pc_1_hash = ((pc_1 >> 1) ^ pc_0) & champsim::bitmask(8);
    uint64_t pc_2_hash = ((pc_2 >> 2) ^ pc_0) & champsim::bitmask(8);
    uint64_t pc_3_hash = ((pc_3 >> 3) ^ pc_0) & champsim::bitmask(8);
    uint64_t tag_1_hash = ((full_addr >> 4) ^ pc_0) & champsim::bitmask(8);
    uint64_t tag_2_hash = ((full_addr >> 7) ^ pc_0) & champsim::bitmask(8);

    int32_t yout = 0;
    yout += ::pc_0_feature[pc_0_hash];
    yout += ::pc_1_feature[pc_1_hash];
    yout += ::pc_2_feature[pc_2_hash];
    yout += ::pc_3_feature[pc_3_hash];
    yout += ::tag_1_feature[tag_1_hash];
    yout += ::tag_2_feature[tag_2_hash];

    if (yout >= bypass_threshold && (access_type{type} != access_type::WRITE) && set != (NUM_SET - 1))
    {
        /* BYPASS */
        // bypass should return this->NUM_WAY, https://champsim.github.io/ChampSim/master/Modules.html#replacement-policies
        return NUM_WAY;
    }
    else
    {
        //check for bit not marked for reuse
        for (uint32_t way = 0; way < NUM_WAY; way++)
        {
            if (::reuse_bits[this].at(set * NUM_WAY + way) == false)
            {
                reuse_counter++;
               // if (reuse_counter % 10000 == 0)
                //    printf("\n REUSE: %d", reuse_counter);
                return static_cast<uint32_t>(way);
            }
        }
    }

    //as a backup we check LRU
    lru_counter++;
  //  if (lru_counter % 10000 == 0)
   //     printf("\n lru: %d", lru_counter);
    auto lru_begin = std::next(std::begin(::lru_bits[this]), set * NUM_WAY);
    auto lru_end = std::next(lru_begin, NUM_WAY);
    auto victim = std::min_element(lru_begin, lru_end);
    return static_cast<uint32_t>(std::distance(lru_begin, victim));
    //}

}

/*************************************************************************/
/*************************** END FIND VICTIM *****************************/





/*************************************************************************/
/************************** UPDATE REPLACMENT ****************************/

void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit)
{

    if (!hit || access_type{ type } != access_type::WRITE) // Skip this for writeback hits
        ::lru_bits[this].at(set * NUM_WAY + way) = current_cycle;

    //no sampler training on writebacks
    if (access_type{ type } == access_type::WRITE) {
        return;

    }

    sample_count++;

    //updating PC values
    //for some reason this function is called many times for the same IP value
    //The if statement is a workaround (not sure if it actually helps, need more testing)
    if (pc_0 != ip) {
        pc_3 = pc_2;
        pc_2 = pc_1;
        pc_1 = pc_0;
        pc_0 = ip;
    }

    ::lru_bits[this].at(set * NUM_WAY + way) = current_cycle;

    //feature hashes
    uint64_t pc_0_hash = ((pc_0 >> 2) ^ pc_0) & champsim::bitmask(8);
    uint64_t pc_1_hash = ((pc_1 >> 1) ^ pc_0) & champsim::bitmask(8);
    uint64_t pc_2_hash = ((pc_2 >> 2) ^ pc_0) & champsim::bitmask(8);
    uint64_t pc_3_hash = ((pc_3 >> 3) ^ pc_0) & champsim::bitmask(8);
    uint64_t tag_1_hash = ((full_addr >> 4) ^ pc_0) & champsim::bitmask(8);
    uint64_t tag_2_hash = ((full_addr >> 7) ^ pc_0) & champsim::bitmask(8);


    //use hashed features as index in feature table to access weights and sum all the weights and calc yout
    //since weights are signed, summation is signed
    int32_t yout = 0;
    yout += ::pc_0_feature[pc_0_hash];
    yout += ::pc_1_feature[pc_1_hash];
    yout += ::pc_2_feature[pc_2_hash];
    yout += ::pc_3_feature[pc_3_hash];
    yout += ::tag_1_feature[tag_1_hash];
    yout += ::tag_2_feature[tag_2_hash];

    //update reuse bit with current yout
    if (yout < replace_threshold) {
        reuse_bits[this].at(set * NUM_WAY + way) = true;
    }
    else
    {
        reuse_bits[this].at(set * NUM_WAY + way) = false;
    }

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
            
            if (old_sample->yout > -sampler_threshold) {
                if (::pc_0_feature[pc_0_hash] > -32) {
                    ::pc_0_feature[pc_0_hash]--;
                }
                if (::pc_1_feature[pc_1_hash] > -32) {
                    ::pc_1_feature[pc_1_hash]--;
                }
                if (::pc_2_feature[pc_2_hash] > -32) {
                    ::pc_2_feature[pc_2_hash]--;
                }
                if (::pc_3_feature[pc_3_hash] > -32) {
                    ::pc_3_feature[pc_3_hash]--;
                }
                if (::tag_1_feature[tag_1_hash] > -32) {
                    ::tag_1_feature[tag_1_hash]--;
                }
                if (::tag_2_feature[tag_2_hash] > -32) {
                    ::tag_2_feature[tag_2_hash]--;
                }
            }
            //update the current sample's hashed feature values, Yout, and LRU value
            *old_sample = { full_addr, current_cycle, yout, pc_0_hash, pc_1_hash, pc_2_hash, pc_3_hash, tag_1_hash, tag_2_hash };
        }
        else {
            //find LRU (lowest clock cycle) sample within a given set
            auto compare_lru = [](const SamplerEntry& a, const SamplerEntry& b) {return a.lru_bits < b.lru_bits; };
            SamplerEntry* throwaway_sample = &(*std::min_element(start, end, compare_lru));

            //check stored yout of that sample, compare to threshold, if below the theta threshold, we increment the weights for the corresponding feature maps
            //if LRU is 0, it means we're evicting nothing, so do not learn from this
            if (throwaway_sample->yout < sampler_threshold && throwaway_sample->lru_bits != 0) {
                if (::pc_0_feature[throwaway_sample->pc_0_hash] < 31) {
                    ::pc_0_feature[throwaway_sample->pc_0_hash]++;
                }
                if (::pc_1_feature[throwaway_sample->pc_1_hash] < 31) {
                    ::pc_1_feature[throwaway_sample->pc_1_hash]++;
                }
                if (::pc_2_feature[throwaway_sample->pc_2_hash] < 31) {
                    ::pc_2_feature[throwaway_sample->pc_2_hash]++;
                }
                if (::pc_3_feature[throwaway_sample->pc_3_hash] < 31) {
                    ::pc_3_feature[throwaway_sample->pc_3_hash]++;
                }
                if (::tag_1_feature[throwaway_sample->tag_shift_1] < 31) {
                    ::tag_1_feature[throwaway_sample->tag_shift_1]++;
                }
                if (::tag_2_feature[throwaway_sample->tag_shift_2] < 31) {
                    ::tag_2_feature[throwaway_sample->tag_shift_2]++;
                }
            }

            //replace sample struct with new struct that has current hashed feature values as well as the current cache block's Yout, tag, and lru (clock cycle)
            *throwaway_sample = { full_addr, current_cycle, yout, pc_0_hash, pc_1_hash, pc_2_hash, pc_3_hash, tag_1_hash, tag_2_hash };
        }
    }

}

/*************************************************************************/
/************************* END UPDATE REPLACMENT *************************/





/*************************************************************************/
/***************************** FINAL STATS *******************************/

void CACHE::replacement_final_stats() {}

/*************************************************************************/
/*************************** END FINAL STATS *****************************/