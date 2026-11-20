#ifndef _ADC_H    
#define _ADC_H

#include "definitions.h"  
#include "application.h"  

#undef ext
#undef ext_static

#ifdef _ADC_C
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


typedef struct adc_struct{
    volatile unsigned short rms_ifil; //!< Filament RMS current, mA unit
    volatile unsigned short rms_vfil; //!< Filament RMS Voltage, 0.1V unit
}_ADC_t;

ext volatile _ADC_t ADC_Data;

/**
 * Module Initialization
 */
ext void ADCInit (void);
ext void ADC15msCallback(void);

// End Module Definition
#endif 
    