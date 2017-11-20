/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2017  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.40 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The  software has  been licensed  to STMicroelectronics International
N.V. a Dutch company with a Swiss branch and its headquarters in Plan-
les-Ouates, Geneva, 39 Chemin du Champ des Filles, Switzerland for the
purposes of creating libraries for ARM Cortex-M-based 32-bit microcon_
troller products commercialized by Licensee only, sublicensed and dis_
tributed under the terms and conditions of the End User License Agree_
ment supplied by STMicroelectronics International N.V.
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : LCDConf_Lin_Template.c
Purpose     : Display controller configuration (single layer)
---------------------------END-OF-HEADER------------------------------
*/

/**
  ******************************************************************************
  * @attention
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

#include "GUI.h"
#include "GUIDRV_Lin.h"
#include "stm32f767xx.h"
#include "stm32f7xx_hal.h"
/*********************************************************************
*
*       Layer configuration (to be modified)
*
**********************************************************************
*/
//
// Physical display size
//
#define XSIZE_PHYS 1024
#define YSIZE_PHYS 600

static uint32_t buffer_index;
//
// Color conversion
//
#define COLOR_CONVERSION GUICC_M8888I

//
// Display driver
//
#define DISPLAY_DRIVER GUIDRV_LIN_32

//
// Buffers / VScreens
//
#define NUM_BUFFERS  1 // Number of multiple buffers to be used
#define NUM_VSCREENS 1 // Number of virtual screens to be used

/*********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef VRAM_ADDR
  #define VRAM_ADDR 0xD0000000 // TBD by customer: This has to be the frame buffer start address
#endif
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   COLOR_CONVERSION
  #error Color conversion not defined!
#endif
#ifndef   DISPLAY_DRIVER
  #error No display driver defined!
#endif
#ifndef   NUM_VSCREENS
  #define NUM_VSCREENS 1
#else
  #if (NUM_VSCREENS <= 0)
    #error At least one screeen needs to be defined!
  #endif
#endif
#if (NUM_VSCREENS > 1) && (NUM_BUFFERS > 1)
  #error Virtual screens and multiple buffers are not allowed!
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_X_Config
*
* Purpose:
*   Called during the initialization process in order to set up the
*   display driver configuration.
*   
*/

static void DMA2D_CopyBuffer(void * pSrc, void * pDst, U32 xSize, U32 ySize, U32 OffLineSrc, U32 OffLineDst)
{
	/* Enable transfer complete interrupt */
	DMA2D->CR = 0x00000000UL | (1 << 9);
			/*
	 *	Look section 9.5 page 266 of document RM0385
	 *	CR register - DMA2D control register
	 *	0x00000000UL | (1 << 9) = 0b0000_0000__0000_0000__0000_0010_0000_0000
	 *	Bit 9 TCIE: Transfer complete interrupt enable
	 *	0: TC interrupt disable
	 *	1: TC interrupt enable
	 *	Bits 17:16 MODE: DMA2D mode
	 *	00: Memory-to-memory
	 */
	/////////////////////
	// Set up pointers
	/////////////////////
	DMA2D->FGMAR = (U32)pSrc;
			/*
	*	Look section 9.5.4 p.269 of document RM0385
	*	DMA2D foreground memory address register
	*	Address of the data used for the foreground image.
	*	In our case it will be:
	*	pSrc = (void *)AddrSrc = (void *)(VRAM_ADDR + BufferSize * IndexSrc) =
	*	= (void *)(VRAM_ADDR + (SIZE_PHYS*YSIZE_PHYS*(LCD_GetBitsPerPixel() >> 3)) * IndexSrc)
	*	= (void *)(0xC0200000 + 480 * 272 * (LCD_GetBitsPerPixel() / 8) * IndexSrc)
	*/
	DMA2D->OMAR = (U32)pDst;
			/*
	*	Look section 9.5.16 p.279 of document RM0385
	*	DMA2D output memory address register
	*	pDst = (void *)AddrDst = (void *)(VRAM_ADDR + BufferSize * IndexDst) =
	*	= (void *)(VRAM_ADDR + (SIZE_PHYS*YSIZE_PHYS*(LCD_GetBitsPerPixel() >> 3)) * IndexDst)
	*	= (void *)(0xC0200000 + 480 * 272 * (LCD_GetBitsPerPixel() / 8) * IndexDst)
	*/
	DMA2D->FGOR = OffLineSrc;
			/*
	*	Look section 9.5.4 p.269 of document RM0385
	*	DMA2D foreground offset register
	*	Line offset used for the foreground (expressed in pixel).
	*	This value is used to generate the address.
	*	It is added at the end of each line to determine
	*	the starting address of the next line.
	*/
	DMA2D->OOR = OffLineDst;
			/*
	*	Look section 9.5.17 p.280 of document RM0385
	*	DMA2D output offset register
	*	Line offset used for the output (expressed in pixels).
	*	This value is used for the address generation.
	*	It is added at the end of each line
	*	to determine the starting address of the next line.
	*	In our case OffLineDst = 0;
	*/
	/* Set up pixel format */
	DMA2D->FGPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
			/*
	*	Look section 9.5.8 p.272 of document RM0385
	*	DMA2D foreground PFC control register
	*	In file stm23f7xx_hal_ltdc.h
	*	#define LTDC_PIXEL_FORMAT_ARGB8888 ((uint32_t)0x00000000U)
	*	In CMSIS only bits 3:0 of register can be change.
	*/
	/*  Set up size */
	DMA2D->NLR     = (U32)(xSize << 16) | (U16)ySize;
			/*
	*	Look section 9.5.17 p.280 of document RM0385
	*	Bits 29:16 PL[13:0]: Pixel per lines
	*	Number of pixels per lines of the area to be transferred.
	*	In our case PL[13:0] = 0b1_1110_0000
	*	Bits 15:0 NL[15:0]: Number of lines
	*	Number of lines of the area to be transferred.
	*	In our case NL[15:0] = 0b1_0001_0000
	*/
	DMA2D->CR     |= DMA2D_CR_START;
			/*
	*	In file 'stm32f746xx.h:
	*	#define DMA2D_CR_START 0x00000001U
	*	Bit 0 START: Start
	*	This bit can be used to launch the DMA2D
	*	according to the parameters loaded in the various configuration registers
	*/
	/* Wait until transfer is done */
	while (DMA2D->CR & DMA2D_CR_START)
		{
		}
			/*
	*	Bit 0 START of CR register is automatically reset by the following events:
	*		� At the end of the transfer
	*		� When the data transfer is aborted by the user application
	*		by setting the ABORT bit in DMA2D_CR
	*		� When a data transfer error occurs
	*		� When the data transfer has not started
	*		due to a configuration error or another
	*		transfer operation already ongoing (automatic CLUT loading).
	*/
}
static void CUSTOM_CopyRect(int LayerIndex, int x0, int y0, int x1, int y1, int xSize, int ySize)
	{
		U32 AddrSrc, AddrDst;
		AddrSrc = VRAM_ADDR + (XSIZE_PHYS * YSIZE_PHYS * (LCD_GetBitsPerPixel() >> 3) * buffer_index) + (y0 * XSIZE_PHYS + x0) * (LCD_GetBitsPerPixel() >> 3);
		AddrDst = VRAM_ADDR + (XSIZE_PHYS * YSIZE_PHYS * (LCD_GetBitsPerPixel() >> 3) * buffer_index) + (y1 * XSIZE_PHYS + x1) * (LCD_GetBitsPerPixel() >> 3);
		DMA2D_CopyBuffer((void *) AddrSrc, (void *) AddrDst, xSize, ySize, (XSIZE_PHYS - xSize), (XSIZE_PHYS - xSize));
	}
static void CUSTOM_DrawBitmap32bpp(int LayerIndex, int x, int y, U8 const * p, int xSize, int ySize, int BytesPerLine)
{
	U32 AddrDst;
	int OffLineSrc, OffLineDst;
	AddrDst = VRAM_ADDR + (XSIZE_PHYS * YSIZE_PHYS * (LCD_GetBitsPerPixel() >> 3) * buffer_index) + (y * XSIZE_PHYS + x) * (LCD_GetBitsPerPixel() >> 3);
	OffLineSrc = (BytesPerLine / 4) - xSize;
	OffLineDst = XSIZE_PHYS - xSize;
	DMA2D_CopyBuffer((void *) p, (void *) AddrDst, xSize, ySize, OffLineSrc, OffLineDst);
}

static void CUSTOM_CopyBuffer(int LayerIndex, int IndexSrc, int IndexDst)
	{
		U32 BufferSize, AddrSrc, AddrDst;
		BufferSize = XSIZE_PHYS * YSIZE_PHYS * (LCD_GetBitsPerPixel() >> 3); //in bytes
		AddrSrc    = VRAM_ADDR + BufferSize * IndexSrc;
		AddrDst    = VRAM_ADDR + BufferSize * IndexDst;
		DMA2D_CopyBuffer((void *)AddrSrc, (void *)AddrDst, XSIZE_PHYS, YSIZE_PHYS, 0, 0);
		buffer_index = IndexDst;
	}

static void DMA2D_FillBuffer(void * pDst, U32 xSize, U32 ySize, U32 OffLine, U32 ColorIndex)
	{
		/* Enable transfer complete interrupt */
	 	DMA2D->CR = 0x00030000UL | (1 << 9);
			/*
		* Look section 9.5 page 266 of document RM0385
		* CR register - DMA2D control register
		* 0x00000000UL | (1 << 9) = 0b0000_0000__0000_0011__0000_0010_0000_0000
		* Bit 9 TCIE: Transfer complete interrupt enable
		* 0: TC interrupt disable
		* 1: TC interrupt enable
		* Bits 17:16 MODE: DMA2D mode
		* 11: Register-to-memory (OCOLR -> OMAR)
		*/
		DMA2D->OCOLR = ColorIndex;
			/*
		* Look section 9.5.15 page 278
		* DMA2D output color register
		* Bits 31:24 ALPHA[7: 0]: Alpha Channel Value
		* Bits 23:16 RED[7: 0]: Red Value
		* Bits 15:8 GREEN[7: 0]: Green Value
		* Bits 7:0 BLUE[7: 0]: Blue Value
		*/
		/* Set up pointers */
		DMA2D->OMAR = (U32) pDst;
			/*
		*	Look section 9.5.16 p.279 of document RM0385
		*	DMA2D output memory address register
		*	pDst = (void *)AddrDst = (void *)(VRAM_ADDR + BufferSize * IndexDst) =
		*	= (void *)(VRAM_ADDR + (SIZE_PHYS*YSIZE_PHYS*(LCD_GetBitsPerPixel() >> 3)) * IndexDst)
		*	= (void *)(0xC0200000 + 480 * 272 * (LCD_GetBitsPerPixel() / 8) * IndexDst)
		*/
		/* Set up offsets */
		DMA2D->OOR = OffLine;
			/*
		*	Look section 9.5.17 p.280 of document RM0385
		*	DMA2D output offset register
		*	Line offset used for the output (expressed in pixels).
		*	This value is used for the address generation.
		*	It is added at the end of each line
		*	to determine the starting address of the next line.
		*	In our case OffLineDst = 0;
		*/
		/* Set up pixel format */
		DMA2D->OPFCCR = LTDC_PIXEL_FORMAT_ARGB8888;
			/*
		* Look section 9.5.14 p.277 of document RM0385
		* DMA2D output PFC control register
		* In file stm23f7xx_hal_ltdc.h
		* #define LTDC_PIXEL_FORMAT_ARGB8888 ((uint32_t)0x00000000U)
		* In CMSIS only bits 3:0 of register can be change.
		*/
		/*  Set up size */
		DMA2D->NLR = (U32) (xSize << 16) | (U16) ySize;
			/*
		*	Look section 9.5.17 p.280 of document RM0385
		*	Bits 29:16 PL[13:0]: Pixel per lines
		*	Number of pixels per lines of the area to be transferred.
		*	In our case PL[13:0] = 0b1_1110_0000
		*	Bits 15:0 NL[15:0]: Number of lines
		*	Number of lines of the area to be transferred.
		*	In our case NL[15:0] = 0b1_0001_0000
		*/
		DMA2D->CR |= DMA2D_CR_START;
			/*
		*	In file 'stm32f746xx.h:
		*	#define DMA2D_CR_START 0x00000001U
		*	Bit 0 START: Start
		*	This bit can be used to launch the DMA2D
		*	according to the parameters loaded in the various configuration registers
		*/
		/* Wait until transfer is done */
		while (DMA2D->CR & DMA2D_CR_START)
			{
			}
			/*
		*	Bit 0 START of CR register is automatically reset by the following events:
		*		� At the end of the transfer
		*		� When the data transfer is aborted by the user application
		*		by setting the ABORT bit in DMA2D_CR
		*		� When a data transfer error occurs
		*		� When the data transfer has not started
		*		due to a configuration error or another
		*		transfer operation already ongoing (automatic CLUT loading).
		*/
	}

/*
* Fill rectangle
* LayerIndex : Layer Index
* x0:          Leftmost coordinate to be filled in screen coordinates
* y0:          Topmost coordinate to be filled in screen coordinates
* x1:          Rightmost coordinate to be filled in screen coordinates
* y1:          Bottommost coordinate to be filled in screen coordinates
* PixelIndex:  Color index to be used to fill the specified area
*/
static void CUSTOM_FillRect(int LayerIndex, int x0, int y0, int x1, int y1, U32 PixelIndex)
	{
		U32 AddrDst;
		int xSize, ySize;
		xSize = x1 - x0 + 1; //1 is added because x1 and x0 are both coordinates of a filling rectangle
		ySize = y1 - y0 + 1;
	 	AddrDst = VRAM_ADDR + (XSIZE_PHYS * YSIZE_PHYS * (LCD_GetBitsPerPixel() >> 3) * buffer_index) + (y0 * XSIZE_PHYS + x0) * (LCD_GetBitsPerPixel() >> 3);
		DMA2D_FillBuffer((void *) AddrDst, xSize, ySize, XSIZE_PHYS - xSize, PixelIndex);
	}

void LCD_X_Config(void) {
  //
  // At first initialize use of multiple buffers on demand
  //
  #if (NUM_BUFFERS > 1)
    GUI_MULTIBUF_Config(NUM_BUFFERS);
  #endif
  //
  // Set display driver and color conversion for 1st layer
  //
  GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);
  //
  // Display driver configuration, required for Lin-driver
  //
  if (LCD_GetSwapXY()) {
    LCD_SetSizeEx (0, YSIZE_PHYS, XSIZE_PHYS);
    LCD_SetVSizeEx(0, YSIZE_PHYS * NUM_VSCREENS, XSIZE_PHYS);
  } else {
    LCD_SetSizeEx (0, XSIZE_PHYS, YSIZE_PHYS);
    LCD_SetVSizeEx(0, XSIZE_PHYS, YSIZE_PHYS * NUM_VSCREENS);
  }
  LCD_SetVRAMAddrEx(0, (void *)VRAM_ADDR);
  //
  // Set user palette data (only required if no fixed palette is used)
  //
  #if defined(PALETTE)
    LCD_SetLUTEx(0, PALETTE);
  #endif
	LCD_SetDevFunc(0, LCD_DEVFUNC_COPYBUFFER,(void (*)(void)) CUSTOM_CopyBuffer);
	LCD_SetDevFunc(0, LCD_DEVFUNC_COPYRECT,(void (*)(void)) CUSTOM_CopyRect);
	LCD_SetDevFunc(0, LCD_DEVFUNC_FILLRECT,(void (*)(void)) CUSTOM_FillRect);
	LCD_SetDevFunc(0, LCD_DEVFUNC_DRAWBMP_32BPP,(void (*)(void)) CUSTOM_DrawBitmap32bpp);
  //
  // Set custom functions for several operations to optimize native processes
  //
}


/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Purpose:
*   This function is called by the display driver for several purposes.
*   To support the according task the routine needs to be adapted to
*   the display controller. Please note that the commands marked with
*   'optional' are not cogently required and should only be adapted if 
*   the display controller supports these features.
*
* Parameter:
*   LayerIndex - Index of layer to be configured
*   Cmd        - Please refer to the details in the switch statement below
*   pData      - Pointer to a LCD_X_DATA structure
*
* Return Value:
*   < -1 - Error
*     -1 - Command not handled
*      0 - Ok
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;

  switch (Cmd) {
  case LCD_X_INITCONTROLLER: {
    //
    // Called during the initialization process in order to set up the
    // display controller and put it into operation. If the display
    // controller is not initialized by any external routine this needs
    // to be adapted by the customer...
    //
    // ...
    return 0;
  }
  case LCD_X_SETVRAMADDR: {
    //
    // Required for setting the address of the video RAM for drivers
    // with memory mapped video RAM which is passed in the 'pVRAM' element of p
    //
    LCD_X_SETVRAMADDR_INFO * p;
    p = (LCD_X_SETVRAMADDR_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_SETORG: {
    //
    // Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
    //
    LCD_X_SETORG_INFO * p;
    p = (LCD_X_SETORG_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_SHOWBUFFER: {
    //
    // Required if multiple buffers are used. The 'Index' element of p contains the buffer index.
    //
    LCD_X_SHOWBUFFER_INFO * p;
    p = (LCD_X_SHOWBUFFER_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_SETLUTENTRY: {
    //
    // Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
    //
    LCD_X_SETLUTENTRY_INFO * p;
    p = (LCD_X_SETLUTENTRY_INFO *)pData;
    //...
    return 0;
  }
  case LCD_X_ON: {
    //
    // Required if the display controller should support switching on and off
    //
    return 0;
  }
  case LCD_X_OFF: {
    //
    // Required if the display controller should support switching on and off
    //
    // ...
    return 0;
  }
  default:
    r = -1;
  }
  return r;
}

/*************************** End of file ****************************/