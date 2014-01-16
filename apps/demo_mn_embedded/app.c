/**
********************************************************************************
\file   app.c

\brief  Demo MN application which implements a running light

This file contains a demo application for digital input/output data.
It implements a running light on the digital outputs. The speed of
the running light is controlled by the read digital inputs.

\ingroup module_demo_mn_console
*******************************************************************************/

/*------------------------------------------------------------------------------
Copyright (c) 2013, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
Copyright (c) 2013, SYSTEC electronik GmbH
Copyright (c) 2013, Kalycito Infotech Private Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holders nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
------------------------------------------------------------------------------*/

//------------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------------
#include <Epl.h>
#include <EplTarget.h>
#include "app.h"
#include "xap.h"

//============================================================================//
//            G L O B A L   D E F I N I T I O N S                             //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------
#define DEFAULT_MAX_CYCLE_COUNT 20      // 6 is very fast
#define APP_LED_COUNT_1         8       // number of LEDs for CN1
#define APP_LED_MASK_1          (1 << (APP_LED_COUNT_1 - 1))
#define MAX_NODES               255

//------------------------------------------------------------------------------
// module global vars
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// global function prototypes
//------------------------------------------------------------------------------

//============================================================================//
//            P R I V A T E   D E F I N I T I O N S                           //
//============================================================================//

//------------------------------------------------------------------------------
// const defines
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// local types
//------------------------------------------------------------------------------
typedef struct
{
    UINT            leds;
    UINT            ledsOld;
    UINT            input;
    UINT            inputOld;
    UINT            period;
    int             toggle;
} APP_NODE_VAR_T;

//------------------------------------------------------------------------------
// local vars
//------------------------------------------------------------------------------
static int                  usedNodeIds_l[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0};
static UINT                 cnt_l;
static APP_NODE_VAR_T       nodeVar_l[MAX_NODES];
static PI_IN*               pProcessImageIn_l;
static PI_OUT*              pProcessImageOut_l;

//------------------------------------------------------------------------------
// local function prototypes
//------------------------------------------------------------------------------
static tEplKernel initProcessImage(void);

//============================================================================//
//            P U B L I C   F U N C T I O N S                                 //
//============================================================================//

//------------------------------------------------------------------------------
/**
\brief  Initialize the synchronous data application

The function initializes the synchronous data application

\return The function returns a tEplKernel error code.

\ingroup module_demo_mn_console
*/
//------------------------------------------------------------------------------
tEplKernel initApp(void)
{
    tEplKernel ret = kEplSuccessful;
    int        i;

    cnt_l = 0;

    for (i = 0; (i < MAX_NODES) && (usedNodeIds_l[i] != 0); i++)
    {
        nodeVar_l[i].leds = 0;
        nodeVar_l[i].ledsOld = 0;
        nodeVar_l[i].input = 0;
        nodeVar_l[i].inputOld = 0;
        nodeVar_l[i].toggle = 0;
        nodeVar_l[i].period = 0;
    }

    ret = initProcessImage();

    return ret;
}

//------------------------------------------------------------------------------
/**
\brief  Shutdown the synchronous data application

The function shut's down the synchronous data application

\return The function returns a tEplKernel error code.

\ingroup module_demo_mn_console
*/
//------------------------------------------------------------------------------
void shutdownApp (void)
{
    oplk_freeProcessImage();
}

//------------------------------------------------------------------------------
/**
\brief  Synchronous data handler

The function implements the synchronous data handler.

\return The function returns a tEplKernel error code.

\ingroup module_demo_mn_console
*/
//------------------------------------------------------------------------------
tEplKernel processSync(void)
{
    tEplKernel          ret = kEplSuccessful;
    int                 i;

    ret = oplk_exchangeProcessImageOut();
    if (ret != kEplSuccessful)
        return ret;

    cnt_l++;

    nodeVar_l[0].input = pProcessImageOut_l->CN1_M00_Digital_Input_8_Bit_Byte_1;
    nodeVar_l[1].input = pProcessImageOut_l->CN2_M00_Digital_Input_8_Bit_Byte_1;
    nodeVar_l[2].input = pProcessImageOut_l->CN3_M00_Digital_Input_8_Bit_Byte_1;
    nodeVar_l[3].input = pProcessImageOut_l->CN4_M00_Digital_Input_8_Bit_Byte_1;
    nodeVar_l[4].input = pProcessImageOut_l->CN5_M00_Digital_Input_8_Bit_Byte_1;
    nodeVar_l[5].input = pProcessImageOut_l->CN6_M00_Digital_Input_8_Bit_Byte_1;
    nodeVar_l[6].input = pProcessImageOut_l->CN7_M00_Digital_Input_8_Bit_Byte_1;
    nodeVar_l[7].input = pProcessImageOut_l->CN8_M00_Digital_Input_8_Bit_Byte_1;
    nodeVar_l[8].input = pProcessImageOut_l->CN9_M00_Digital_Input_8_Bit_Byte_1;
    nodeVar_l[9].input = pProcessImageOut_l->CN10_M00_Digital_Input_8_Bit_Byte_1;
    nodeVar_l[10].input = pProcessImageOut_l->CN11_M00_Digital_Input_8_Bit_Byte_1;
    nodeVar_l[11].input = pProcessImageOut_l->CN12_M00_Digital_Input_8_Bit_Byte_1;

    for (i = 0; (i < MAX_NODES) && (usedNodeIds_l[i] != 0); i++)
    {
        /* Running Leds */
        /* period for LED flashing determined by inputs */
        nodeVar_l[i].period = (nodeVar_l[i].input == 0) ? 1 : (nodeVar_l[i].input * 20);
        if (cnt_l % nodeVar_l[i].period == 0)
        {
            if (nodeVar_l[i].leds == 0x00)
            {
                nodeVar_l[i].leds = 0x1;
                nodeVar_l[i].toggle = 1;
            }
            else
            {
                if (nodeVar_l[i].toggle)
                {
                    nodeVar_l[i].leds <<= 1;
                    if (nodeVar_l[i].leds == APP_LED_MASK_1)
                    {
                        nodeVar_l[i].toggle = 0;
                    }
                }
                else
                {
                    nodeVar_l[i].leds >>= 1;
                    if (nodeVar_l[i].leds == 0x01)
                    {
                        nodeVar_l[i].toggle = 1;
                    }
                }
            }
        }

        if (nodeVar_l[i].input != nodeVar_l[i].inputOld)
        {
            nodeVar_l[i].inputOld = nodeVar_l[i].input;
        }

        if (nodeVar_l[i].leds != nodeVar_l[i].ledsOld)
        {
            nodeVar_l[i].ledsOld = nodeVar_l[i].leds;
        }
    }

    pProcessImageIn_l->CN1_M00_Digital_Ouput_8_Bit_Byte_1 = nodeVar_l[0].leds;
    pProcessImageIn_l->CN2_M00_Digital_Ouput_8_Bit_Byte_1 = nodeVar_l[1].leds;
    pProcessImageIn_l->CN3_M00_Digital_Ouput_8_Bit_Byte_1 = nodeVar_l[2].leds;
    pProcessImageIn_l->CN4_M00_Digital_Ouput_8_Bit_Byte_1 = nodeVar_l[3].leds;
    pProcessImageIn_l->CN5_M00_Digital_Ouput_8_Bit_Byte_1 = nodeVar_l[4].leds;
    pProcessImageIn_l->CN6_M00_Digital_Ouput_8_Bit_Byte_1 = nodeVar_l[5].leds;
    pProcessImageIn_l->CN7_M00_Digital_Ouput_8_Bit_Byte_1 = nodeVar_l[6].leds;
    pProcessImageIn_l->CN8_M00_Digital_Ouput_8_Bit_Byte_1 = nodeVar_l[7].leds;
    pProcessImageIn_l->CN9_M00_Digital_Ouput_8_Bit_Byte_1 = nodeVar_l[8].leds;
    pProcessImageIn_l->CN10_M00_Digital_Ouput_8_Bit_Byte_1 = nodeVar_l[9].leds;
    pProcessImageIn_l->CN11_M00_Digital_Ouput_8_Bit_Byte_1 = nodeVar_l[10].leds;
    pProcessImageIn_l->CN12_M00_Digital_Ouput_8_Bit_Byte_1 = nodeVar_l[11].leds;

    ret = oplk_exchangeProcessImageIn();

    return ret;
}

//============================================================================//
//            P R I V A T E   F U N C T I O N S                               //
//============================================================================//
/// \name Private Functions
/// \{

//------------------------------------------------------------------------------
/**
\brief  Initialize process image

The function initializes the process image of the application.

\return The function returns a tEplKernel error code.
*/
//------------------------------------------------------------------------------
static tEplKernel initProcessImage(void)
{
    tEplKernel      ret = kEplSuccessful;

    PRINTF("Initializing process image...\n");
    PRINTF("Size of input process image: %d\n", (UINT32)sizeof(PI_IN));
    PRINTF("Size of output process image: %d\n", (UINT32)sizeof (PI_OUT));
    ret = oplk_allocProcessImage(sizeof(PI_IN), sizeof(PI_OUT));
    if (ret != kEplSuccessful)
    {
        return ret;
    }

    pProcessImageIn_l = oplk_getProcessImageIn();
    pProcessImageOut_l = oplk_getProcessImageOut();

    ret = oplk_setupProcessImage();

    return ret;
}

///\}
