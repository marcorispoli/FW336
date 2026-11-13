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


typedef enum filstat{
    FILSTAT_OFF = 0, //!< Filament switched Off
    FILSTAT_AS, //!< Filament A Small selected
    FILSTAT_AG, //!< Filament A Large selected       
    FILSTAT_BS, //!< Filament B Small selected
    FILSTAT_BG, //!< Filament B Large selected       
}FILSTAT_t;

typedef struct filament_struct{
    FILSTAT_t status;
    
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
ext bool FilamenVrefSet(unsigned char ctrl, unsigned short value );

ext bool FilamentCurrentSet(float fil_ma );

// End Module Definition
#endif 
    