#ifndef _FILAMENT_H    
#define _FILAMENT_H

#include "definitions.h"  
#include "application.h"  

#undef ext
#undef ext_static

#ifdef _FILAMENT_C
    #define ext
    #define ext_static static 
#else
    #define ext extern
    #define ext_static extern
#endif

/*!
 * \defgroup GENERATOR_MODULE Generator Handling Module
 * \ingroup applicationModule
 * 
 * TBD
 * 
 * ## Dependencies
 * 
 * TBD
 * 
 * 
 * ## Module Description
 * 
 * TBD
 *  
 */


/**
 * \addtogroup GENERATOR_MODULE
 * 
 * ## Application API
 * TBD
 * 
 */

#define MAX_SELECTABLE_CURRENT_mA   5000 //!< Mas RMS selctable current
#define DAC_2000mA_REFERENCE        1600 //!< DAC value corresponding to 2A RMS
#define WARMUP_CURRENT_ma           2000 //!< The warming filament current 
#define INITIAL_WARMUP_CURRENT_ma   500  //!< The initial value of the ramp current if the filament is off

typedef enum filstat{
    FILSTAT_OFF = 0, //!< Filament switched Off
    FILSTAT_AS, //!< Filament A Small selected
    FILSTAT_AG, //!< Filament A Large selected       
    FILSTAT_BS, //!< Filament B Small selected
    FILSTAT_BG, //!< Filament B Large selected       
}FILSTAT_t;

typedef struct filament_struct{
    FILSTAT_t status;
    bool warmup;    //!< The filament is warming up and cannot be activated at the target current
    bool switch_filament ;//!< Set if the filament shall be switched
    unsigned short warmup_current_ma; //!< This is the current at the beginning of the warmup
    unsigned short target_current_ma; //!< This is the requested target current at the end of the warmup
    
}_FILAMENT_t;

ext _FILAMENT_t FIL_Data;

/**
 * Module Initialization
 */
ext void FilamentInit (void);

#define VREF_SHTD     0x00
#define VREF_RUN      0x10
#define VREF_GAINx1   0x20
#define VREF_GAINx2   0x00

ext void Filament15msCallback(void);
ext bool FilamenVrefSet(unsigned char ctrl, unsigned short value );
ext bool FilamentRMSCurrentSet(unsigned short ma );
ext bool FilamentSelectFocus(FILSTAT_t focus, unsigned short target_current);

// End Module Definition
#endif 
    