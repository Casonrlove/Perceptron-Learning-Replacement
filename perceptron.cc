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
    //perceptron history
    //percpetron bits
    //number of perceptrons

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

void CACHE::initialize_replacement() { ::last_used_cycles[this] = std::vector<uint64_t>(NUM_SET * NUM_WAY); }

/*************************************************************************/
/**************************** END REPLACEMENT ****************************/






/*************************************************************************/
/************************** UPDATE REPLACMENT ****************************/
void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
  // Mark the way as being used on the current cycle
  if (!hit || access_type{type} != access_type::WRITE) // Skip this for writeback hits
    ::last_used_cycles[this].at(set * NUM_WAY + way) = current_cycle;
}
/*************************************************************************/
/*************************** END REPLACMENT ******************************/





/*************************************************************************/
/***************************** FINAL STATS *******************************/
void CACHE::replacement_final_stats() {}
/*************************************************************************/
/*************************** END FINAL STATS *****************************/