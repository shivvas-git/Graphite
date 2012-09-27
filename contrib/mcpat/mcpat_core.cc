/*****************************************************************************
 * Graphite-McPAT Core Interface
 ***************************************************************************/

// [graphite]

#include <string.h>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include <cmath>
#include <assert.h>
#include <fstream>
#include "parameter.h"
#include "array.h"
#include "const.h"
#include "basic_circuit.h"
#include "XML_Parse.h"
#include "processor.h"
#include "version.h"
#include "mcpat_core.h"

namespace McPAT
{

//---------------------------------------------------------------------------
// [graphite] Create McPATCore
//---------------------------------------------------------------------------
McPATCore::McPATCore(ParseXML *XML_interface)
:XML(XML_interface)//TODO: using one global copy may have problems.
{
  /*
   *  placement and routing overhead is 10%, core scales worse than cache 40% is accumulated from 90 to 22nm
   *  There is no point to have heterogeneous memory controller on chip,
   *  thus McPAT only support homogeneous memory controllers.
   */
  int i;
  double pppm_t[4]    = {1,1,1,1};
  set_proc_param();
  if (procdynp.homoCore)
     numCore = procdynp.numCore==0? 0:1;
  else
     numCore = procdynp.numCore;

  for (i = 0;i < numCore; i++)
  {
        cores.push_back(new Core(XML,i, &interface_ip));
        if (procdynp.homoCore){
           core.area.set_area(core.area.get_area() + cores[i]->area.get_area()*procdynp.numCore);
           area.set_area(area.get_area() + core.area.get_area());//placement and routing overhead is 10%, core scales worse than cache 40% is accumulated from 90 to 22nm
        }
        else{
           core.area.set_area(core.area.get_area() + cores[i]->area.get_area());
           area.set_area(area.get_area() + cores[i]->area.get_area());//placement and routing overhead is 10%, core scales worse than cache 40% is accumulated from 90 to 22nm
        }
  }
}

//---------------------------------------------------------------------------
// [graphite] Compute Energy
//---------------------------------------------------------------------------
void McPATCore::computeEnergy()
{
  /*
   *  placement and routing overhead is 10%, core scales worse than cache 40% is accumulated from 90 to 22nm
   *  There is no point to have heterogeneous memory controller on chip,
   *  thus McPAT only support homogeneous memory controllers.
   */
  int i;
  double pppm_t[4]    = {1,1,1,1};

  //--------------------------------------
  // Set Numbers of Components
  //--------------------------------------
  if (procdynp.homoCore)
     numCore = procdynp.numCore==0? 0:1;
  else
     numCore = procdynp.numCore;

  //--------------------------------------
  // Compute Energy of Components
  //--------------------------------------

  // McPAT Core
  power.reset();
  rt_power.reset();

  // Cores
  core.power.reset();
  core.rt_power.reset();
  for (i = 0;i < numCore; i++)
  {
        cores[i]->power.reset();
        cores[i]->rt_power.reset();
        cores[i]->computeEnergy();
        cores[i]->computeEnergy(false);
        if (procdynp.homoCore){
           set_pppm(pppm_t,cores[i]->clockRate*procdynp.numCore, procdynp.numCore,procdynp.numCore,procdynp.numCore);
           core.power = core.power + cores[i]->power*pppm_t;
           set_pppm(pppm_t,1/cores[i]->executionTime, procdynp.numCore,procdynp.numCore,procdynp.numCore);
           core.rt_power = core.rt_power + cores[i]->rt_power*pppm_t;
           power = power  + core.power;
           rt_power = rt_power  + core.rt_power;
        }
        else{
           set_pppm(pppm_t,cores[i]->clockRate, 1, 1, 1);
           core.power = core.power + cores[i]->power*pppm_t;
           power = power  + cores[i]->power*pppm_t;
           set_pppm(pppm_t,1/cores[i]->executionTime, 1, 1, 1);
           core.rt_power = core.rt_power + cores[i]->rt_power*pppm_t;
           rt_power = rt_power  + cores[i]->rt_power*pppm_t;
        }
  }
}

//---------------------------------------------------------------------------
// Print Device Type
//---------------------------------------------------------------------------
void McPATCore::displayDeviceType(int device_type_, uint32_t indent)
{
   string indent_str(indent, ' ');

   switch ( device_type_ ) {

     case 0 :
        cout <<indent_str<<"Device Type= "<<"ITRS high performance device type"<<endl;
       break;
     case 1 :
        cout <<indent_str<<"Device Type= "<<"ITRS low standby power device type"<<endl;
       break;
     case 2 :
        cout <<indent_str<<"Device Type= "<<"ITRS low operating power device type"<<endl;
       break;
     case 3 :
        cout <<indent_str<<"Device Type= "<<"LP-DRAM device type"<<endl;
       break;
     case 4 :
        cout <<indent_str<<"Device Type= "<<"COMM-DRAM device type"<<endl;
       break;
     default :
        {
           cout <<indent_str<<"Unknown Device Type"<<endl;
           exit(0);
        }
   }
}

//---------------------------------------------------------------------------
// Print Interconnect Type
//---------------------------------------------------------------------------
void McPATCore::displayInterconnectType(int interconnect_type_, uint32_t indent)
{
   string indent_str(indent, ' ');

   switch ( interconnect_type_ ) {

     case 0 :
        cout <<indent_str<<"Interconnect metal projection= "<<"aggressive interconnect technology projection"<<endl;
       break;
     case 1 :
        cout <<indent_str<<"Interconnect metal projection= "<<"conservative interconnect technology projection"<<endl;
       break;
     default :
        {
           cout <<indent_str<<"Unknown Interconnect Projection Type"<<endl;
           exit(0);
        }
   }
}

//---------------------------------------------------------------------------
// Print Energy from McPATCore
//---------------------------------------------------------------------------
void McPATCore::displayEnergy(uint32_t indent, int plevel, bool is_tdp)
{
   int i;
   bool long_channel = XML->sys.longer_channel_device;
   string indent_str(indent, ' ');
   string indent_str_next(indent+2, ' ');
   if (is_tdp)
   {

      if (plevel<5)
      {
         cout<<"\nMcPAT (version "<< VER_MAJOR <<"."<< VER_MINOR
               << " of " << VER_UPDATE << ") results (current print level is "<< plevel
         <<", please increase print level to see the details in components): "<<endl;
      }
      else
      {
         cout<<"\nMcPAT (version "<< VER_MAJOR <<"."<< VER_MINOR
                        << " of " << VER_UPDATE << ") results  (current print level is 5)"<< endl;
      }
      cout <<"*****************************************************************************************"<<endl;
      cout <<indent_str<<"Technology "<<XML->sys.core_tech_node<<" nm"<<endl;
      //cout <<indent_str<<"Device Type= "<<XML->sys.device_type<<endl;
      if (long_channel)
         cout <<indent_str<<"Using Long Channel Devices When Appropriate"<<endl;
      //cout <<indent_str<<"Interconnect metal projection= "<<XML->sys.interconnect_projection_type<<endl;
      displayInterconnectType(XML->sys.interconnect_projection_type, indent);
      cout <<indent_str<<"Core clock Rate(MHz) "<<XML->sys.core[0].clock_rate<<endl;
      cout <<endl;
      cout <<"*****************************************************************************************"<<endl;
      if (numCore >0){
      cout <<indent_str<<"Total Cores: "<<XML->sys.number_of_cores << " cores "<<endl;
      displayDeviceType(XML->sys.device_type,indent);
      cout << indent_str_next << "Area = " << core.area.get_area()*1e-6<< " mm^2" << endl;
      cout << indent_str_next << "Peak Dynamic = " << core.power.readOp.dynamic << " W" << endl;
      cout << indent_str_next << "Subthreshold Leakage = "
         << (long_channel? core.power.readOp.longer_channel_leakage:core.power.readOp.leakage) <<" W" << endl;
      //cout << indent_str_next << "Subthreshold Leakage = " << core.power.readOp.longer_channel_leakage <<" W" << endl;
      cout << indent_str_next << "Gate Leakage = " << core.power.readOp.gate_leakage << " W" << endl;
      cout << indent_str_next << "Runtime Dynamic = " << core.rt_power.readOp.dynamic << " W" << endl;
      cout <<endl;
      }
      cout <<"*****************************************************************************************"<<endl;
      if (plevel >1)
      {
         for (i = 0;i < numCore; i++)
         {
            cores[i]->displayEnergy(indent+4,plevel,is_tdp);
            cout <<"*****************************************************************************************"<<endl;
         }
      }
   }
   else
   {
   }
}

//---------------------------------------------------------------------------
// Set McPATCore Parameters
//---------------------------------------------------------------------------
void McPATCore::set_proc_param()
{
   bool debug = false;

   procdynp.homoCore = bool(debug?1:XML->sys.homogeneous_cores);
   procdynp.numCore = XML->sys.number_of_cores;

   /* Basic parameters*/
   interface_ip.data_arr_ram_cell_tech_type    = debug?0:XML->sys.device_type;
   interface_ip.data_arr_peri_global_tech_type = debug?0:XML->sys.device_type;
   interface_ip.tag_arr_ram_cell_tech_type     = debug?0:XML->sys.device_type;
   interface_ip.tag_arr_peri_global_tech_type  = debug?0:XML->sys.device_type;

   interface_ip.ic_proj_type     = debug?0:XML->sys.interconnect_projection_type;
   interface_ip.delay_wt                = 100;//Fixed number, make sure timing can be satisfied.
   interface_ip.area_wt                 = 0;//Fixed number, This is used to exhaustive search for individual components.
   interface_ip.dynamic_power_wt        = 100;//Fixed number, This is used to exhaustive search for individual components.
   interface_ip.leakage_power_wt        = 0;
   interface_ip.cycle_time_wt           = 0;

   interface_ip.delay_dev                = 10000;//Fixed number, make sure timing can be satisfied.
   interface_ip.area_dev                 = 10000;//Fixed number, This is used to exhaustive search for individual components.
   interface_ip.dynamic_power_dev        = 10000;//Fixed number, This is used to exhaustive search for individual components.
   interface_ip.leakage_power_dev        = 10000;
   interface_ip.cycle_time_dev           = 10000;

   interface_ip.ed                       = 2;
   interface_ip.burst_len      = 1;//parameters are fixed for processor section, since memory is processed separately
   interface_ip.int_prefetch_w = 1;
   interface_ip.page_sz_bits   = 0;
   interface_ip.temp = debug?360: XML->sys.temperature;
   interface_ip.F_sz_nm         = debug?90:XML->sys.core_tech_node;//XML->sys.core_tech_node;
   interface_ip.F_sz_um         = interface_ip.F_sz_nm / 1000;

   //***********This section of code does not have real meaning, they are just to ensure all data will have initial value to prevent errors.
   //They will be overridden  during each components initialization
   interface_ip.cache_sz            =64;
   interface_ip.line_sz             = 1;
   interface_ip.assoc               = 1;
   interface_ip.nbanks              = 1;
   interface_ip.out_w               = interface_ip.line_sz*8;
   interface_ip.specific_tag        = 1;
   interface_ip.tag_w               = 64;
   interface_ip.access_mode         = 2;

   interface_ip.obj_func_dyn_energy = 0;
   interface_ip.obj_func_dyn_power  = 0;
   interface_ip.obj_func_leak_power = 0;
   interface_ip.obj_func_cycle_t    = 1;

   interface_ip.is_main_mem     = false;
   interface_ip.rpters_in_htree = true ;
   interface_ip.ver_htree_wires_over_array = 0;
   interface_ip.broadcast_addr_din_over_ver_htrees = 0;

   interface_ip.num_rw_ports        = 1;
   interface_ip.num_rd_ports        = 0;
   interface_ip.num_wr_ports        = 0;
   interface_ip.num_se_rd_ports     = 0;
   interface_ip.num_search_ports    = 1;
   interface_ip.nuca                = 0;
   interface_ip.nuca_bank_count     = 0;
   interface_ip.is_cache            =true;
   interface_ip.pure_ram            =false;
   interface_ip.pure_cam            =false;
   interface_ip.force_cache_config  =false;
   if (XML->sys.Embedded)
      {
      interface_ip.wt                  =Global_30;
      interface_ip.wire_is_mat_type = 0;
      interface_ip.wire_os_mat_type = 0;
      }
   else
      {
      interface_ip.wt                  =Global;
      interface_ip.wire_is_mat_type = 2;
      interface_ip.wire_os_mat_type = 2;
      }
   interface_ip.force_wiretype      = false;
   interface_ip.print_detail        = 1;
   interface_ip.add_ecc_b_          =true;
}

//---------------------------------------------------------------------------
// Delete McPATCore
//---------------------------------------------------------------------------
McPATCore::~McPATCore(){
   while (!cores.empty())
   {
      delete cores.back();
      cores.pop_back();
   }
};

}
