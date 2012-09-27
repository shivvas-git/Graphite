/*------------------------------------------------------------
 *                              CACTI 6.5
 *         Copyright 2008 Hewlett-Packard Development Corporation
 *                         All Rights Reserved
 *
 * Permission to use, copy, and modify this software and its documentation is
 * hereby granted only under the following terms and conditions.  Both the
 * above copyright notice and this permission notice must appear in all copies
 * of the software, derivative works or modified versions, and any portions
 * thereof, and both notices must appear in supporting documentation.
 *
 * Users of this software agree to the terms and conditions set forth herein, and
 * hereby grant back to Hewlett-Packard Company and its affiliated companies ("HP")
 * a non-exclusive, unrestricted, royalty-free right and license under any changes,
 * enhancements or extensions  made to the core functions of the software, including
 * but not limited to those affording compatibility with other hardware or software
 * environments, but excluding applications which incorporate this software.
 * Users further agree to use their best efforts to return to HP any such changes,
 * enhancements or extensions that they make and inform HP of noteworthy uses of
 * this software.  Correspondence should be provided to HP at:
 *
 *                       Director of Intellectual Property Licensing
 *                       Office of Strategy and Technology
 *                       Hewlett-Packard Company
 *                       1501 Page Mill Road
 *                       Palo Alto, California  94304
 *
 * This software may be distributed (but not offered for sale or transferred
 * for compensation) to third parties, provided such third parties agree to
 * abide by the terms and conditions of this notice.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND HP DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL HP
 * CORPORATION BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *------------------------------------------------------------*/

#ifndef __ARBITER__
#define __ARBITER__

#include <assert.h>
#include <iostream>
#include "basic_circuit.h"
#include "cacti_interface.h"
#include "component.h"
#include "parameter.h"
#include "mat.h"
#include "wire.h"

namespace McPAT
{

class Arbiter : public Component
{
  public:
    Arbiter(
      double Req,
      double flit_sz,
      double output_len,
      TechnologyParameter::DeviceType *dt = &(g_tp.peri_global));
    ~Arbiter();

    void print_arbiter();
    double arb_req();
    double arb_pri();
    double arb_grant();
    double arb_int();
    void compute_power();
    double Cw3(double len);
    double crossbar_ctrline();
    double transmission_buf_ctrcap();



  private:
    double NTn1, PTn1, NTn2, PTn2, R, PTi, NTi;
    double flit_size;
    double NTtr, PTtr;
    double o_len;
    TechnologyParameter::DeviceType *deviceType;
    double TriS1, TriS2;
    double min_w_pmos, Vdd;

};

}

#endif
