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
#define DAC_REFERENCE               1165 //!< DAC value corresponding to 2.5A RMS
#define ImA_REFERENCE               2000 //!< Current Reference in mA
#define KEEPING_CURRENT             2000 //!< The RMS current for the Keeping mode
#define INITIAL_WARMUP_CURRENT_ma   500  //!< The initial value of the ramp current if the filament is off
#define CURRENT_RAMP                30   //!< 2000 mA in 1 second
#define ABSOLUTE_RMS_CURRENT        5000 //!< This is the absolute maximum current allowable

typedef enum filstat{
    FILSTAT_OFF = 0,		//!< The filament is off
    FILSTAT_INIT,           //!< The filament is switching and selcting the focus 
    FILSTAT_WARMING,		//!< The filament is warming up to a target or keeping current
    FILSTAT_RUNNING,        //!< The filament is at the target current (with timeout))
    FILSTAT_KEEPING         //!< The filament is in the keeping mode (take the filament warmer)
}FILSTAT_t;

typedef enum filcmd{    
    FILCMD_NONE = 0,        //!< No pending command sequence
    FILCMD_RUNNING,         //!< The filament is executing a Target Current sequence
    FILCMD_KEEPING,         //!< The filament is executing a KEEPING sequence            
            
}FILCMD_t;

typedef enum filsel{
    FILSEL_AS=0, //!< Filament A Small selected
    FILSEL_AL, //!< Filament A Large selected       
    FILSEL_BS, //!< Filament B Small selected
    FILSEL_BL  //!< Filament B Large selected       
}FILSEL_t;


typedef struct fildgn{    
    unsigned char  detected:1;
    unsigned char  calibrated:1;    
    unsigned char  low_current:1;
    unsigned char  high_current:1;    
    unsigned char  max_current:1;    
    unsigned char  filament_open:1;
    unsigned char  spare:1;
    unsigned char  fault:1;
}FILDGN_t;

typedef struct focus_struct{    
   FILSEL_t     code;
   FILDGN_t*    fildgn_ptr;
   FILDGN_t     filas;
   FILDGN_t     filal;
   FILDGN_t     filbs;
   FILDGN_t     filbl;
}FOCUS_t;

typedef struct filseq{
    FILSEL_t       focus;    
    unsigned char  step;
    unsigned char  delay;
    unsigned short target_current; 
    unsigned short timeout_ms;
}FILSEQ_t;


typedef struct filament_struct{
    FILSTAT_t status;
    FOCUS_t   focus;
    FILCMD_t  command;
    FILSEQ_t  sequence;
    unsigned short rms_current; 
}_FILAMENT_t;

ext  _FILAMENT_t FIL_Data;

/**
 * Module Initialization
 */
ext void FilamentInit (void);

#define VREF_SHTD     0x00
#define VREF_RUN      0x10
#define VREF_GAINx1   0x20
#define VREF_GAINx2   0x00

ext void Filament15msCallback(void);

ext bool FilamentSetCurrent(FILSEL_t focus, unsigned short target_current, unsigned short timeout_ms);
ext bool FilamentSetKeeping(FILSEL_t focus);
ext void FilamentOff(void);
ext bool FilamentInitializationSequence(bool start);

// End Module Definition
#endif 
    