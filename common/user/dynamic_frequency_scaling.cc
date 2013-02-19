#include "dynamic_frequency_scaling.h"
#include "simulator.h"
#include "tile_manager.h"
#include "tile.h"
#include "core.h"
#include "core_model.h"
#include "fxsupport.h"

void CarbonGetTileFrequency(volatile float* frequency)
{
   // Floating Point Save/Restore
   FloatingPointHandler floating_point_handler;

   Tile* tile = Sim()->getTileManager()->getCurrentTile();
   *frequency = tile->getFrequency();
}

void CarbonSetTileFrequency(volatile float* frequency)
{
   // Floating Point Save/Restore
   FloatingPointHandler floating_point_handler;

   // Stuff to change
   // 1) Core Model
   // 2) Memory Subsystem Model
   Tile* tile = Sim()->getTileManager()->getCurrentTile();
   tile->acquireLock();
   float old_frequency = tile->getFrequency();
   float new_frequency = *frequency;
   tile->updateInternalVariablesOnFrequencyChange(old_frequency, new_frequency);
   tile->setFrequency(new_frequency);
   tile->releaseLock();
}

void CarbonSetRemoteTileFrequency(tile_id_t tile_id, volatile float* frequency)
{
   // Floating Point Save/Restore
   FloatingPointHandler floating_point_handler;

   Core* core = Sim()->getTileManager()->getCurrentTile()->getCore();
   core->coreSendW(core->getId().tile_id, tile_id, (char*) frequency, sizeof(float), (carbon_network_t) CARBON_NET_USER_2);
}
