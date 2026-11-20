#ifndef _PROTOCOL_H    
#define _PROTOCOL_H

#include "definitions.h"  
#include "application.h"  
#include "Shared/CAN/MET_can_protocol.h"

#undef ext
#undef ext_static

#ifdef _PROTOCOL_C
    #define ext
    #define ext_static static 
#else
    #define ext extern
    #define ext_static extern
#endif

/*!
 * \defgroup CANPROT CAN Communication Protocol Module
 * \ingroup applicationModule
 * 
 * This Module implements the functions of the PCB/22-303 Software Communication protocol specifications.
 * 
 * ## Dependencies
 * 
 * This module requires the following library modules:
 * - Shared/CAN/MET_Can_Protocol.c
 * - Shared/CAN/MET_Can_Protocol.h
 * 
 * 
 * ## Protocol Communication setting
 *  
 * The Application implements the communication protocol  
 * described in the PCB/22-303 Software Communication protocol specifications.
 */


/// \ingroup CANPROT 
/// This structure of a generic protocol register 
typedef struct {
    unsigned char idx;
    unsigned char d0;
    unsigned char d1;
    unsigned char d2;
    unsigned char d3;
}REGISTER_STRUCT_t;
    

/**
 * \addtogroup CANPROT 
 * 
 * ## Protocol Setting
 * 
 * Can ID address: \ref MET_CAN_APP_DEVICE_ID  
 * Number of STATUS registers: MET_CAN_STATUS_REGISTERS
 * Number of DATA registers: MET_CAN_DATA_REGISTERS
 * Number of PARAM registers: MET_CAN_PARAM_REGISTERS
 * 
 */ 

/// \ingroup CANPROT 
/// Protocol Definition Data
typedef enum{
    MET_CAN_APP_DEVICE_ID    =  0x18,      //!< Application DEVICE CAN Id address
    MET_CAN_STATUS_REGISTERS =  3,        //!< Defines the total number of implemented STATUS registers 
    MET_CAN_DATA_REGISTERS   =  0,        //!< Defines the total number of implemented Application DATA registers 
    MET_CAN_PARAM_REGISTERS  =  0         //!< Defines the total number of implemented PARAMETER registers 
}PROTOCOL_DEFINITION_DATA_t;

/**
 * \addtogroup CANPROT
 * 
 * ## Application API
 * 
 * + ApplicationProtocolInit() : this is the module initialization;
 * + ApplicationProtocolLoop() : this is the workflow routine to be placed into the application main loop;
 * + encodeStatusRegister() : This is the function to encode a Status register to the Can Register array 
 * + decodeParamRegister() : This is the function to update and decodea Param register with the actual Can Registers content
 * + protocolGet2DFormat() : This is the function to get the 2D format from the Can Parameters
 * + protocolGetFilter() : This is the function to get the Filter positionfrom the Can Parameters
 * + protocolGetMirror() : This is the function to get the Mirror position from the Can Parameters
 * 
 */


/**
 * \ingroup CANPROT 
 * Protocol Initialization routine
 */
ext void ApplicationProtocolInit ( void);

/// \ingroup CANPROT 
/// This is the Main Loop protocol function
ext void  ApplicationProtocolLoop(void);



/// \ingroup CANPROT 
/// This is the function to encode a Status register to the Can Register array
ext void encodeStatusRegister(void* reg);

/// \ingroup CANPROT 
/// This is the function to update and decodea Param register with the actual Can Registers content
ext void decodeParamRegister(void* reg);
ext void encodeParamRegister(void* reg);

/// \ingroup CANPROT 
/// This is the function to update and decode Data register with the actual Can Registers content
ext void decodeDataRegister(void* reg);

//________________________________________ STATUS REGISTER DEFINITION SECTION _

/**
 * \addtogroup CANPROT
 * 
 * ## STATUS register description
 * 
 * There are the following Status registers:\n
 * (See \ref STATUS_INDEX_t enum table)
 * 
 * |IDX|NAME|DESCRIPTION|
 * |:--|:--|:--|
 * |0|SYSTEM|\ref SYSTEM_STATUS_t|
 * |1|TUBE|\ref TUBE_STATUS_t|
 *   
 */

/// \ingroup CANPROT
/// Defines the address table for the System Registers 
typedef enum{
    FILAMENT_STATUS_IDX = 0,    //!< This is the System general data status register     
    FILAMENT_MONITOR_IDX,       //!< This is the Filament monitor register
    FILAMENT_DGN_IDX,           //!< Filament Diagnosys
}STATUS_INDEX_t;

    typedef struct {
        const unsigned char idx; //!< Address constant 
        
        unsigned char status; 
        unsigned char focus_selection; 
        unsigned char d2;  
        unsigned char d3;     //!< Filament RMS Voltage, 0.1V unit
    }FILAMENT_STATUS_t;
    
    typedef struct {
        const unsigned char idx; //!< Address constant 
        
        unsigned char rms_ifilL; //!< Filament RMS Current, mA unit
        unsigned char rms_ifilH; //!< Filament RMS Current, mA unit
        unsigned char rms_voltL;    //!< Filament RMS Voltage, mV unit
        unsigned char rms_voltH;    //!< Filament RMS Voltage, mV unit
    }FILAMENT_MONITOR_t;
    
     typedef struct {
        const unsigned char idx; //!< Address constant 
        
        unsigned char filas;    //!< Diagnosys bit field
        unsigned char filal;    //!< Diagnosys bit field
        unsigned char filbs;    //!< Diagnosys bit field
        unsigned char filbl;    //!< Diagnosys bit field
    }FILAMENT_DGN_t;
    
     
    #ifdef _PROTOCOL_C       
        FILAMENT_STATUS_t FilamentStatusRegister = {.idx=FILAMENT_STATUS_IDX};
        FILAMENT_MONITOR_t FilamentMonitorRegister = {.idx=FILAMENT_MONITOR_IDX};
        FILAMENT_DGN_t FilamentDgnRegister = {.idx=FILAMENT_DGN_IDX};
        
    #else
        extern FILAMENT_STATUS_t FilamentStatusRegister;
        extern FILAMENT_MONITOR_t FilamentMonitorRegister;
        extern FILAMENT_DGN_t FilamentDgnRegister;
    #endif  
        
        
//________________________________________ DATA REGISTER DEFINITION SECTION _   

/**
* \addtogroup CANPROT
* 
* ## DATA register description
* 
* No Data registers are implemented
*
*/
typedef enum{
    SYSTEM_DATA_IDX = 0,
}DATA_INDEX_t;

    
typedef struct {
    const unsigned char idx; //!< Address constant 

    unsigned char d0;
    unsigned char d1;
    unsigned char d2;
    unsigned char d3;
}SYSTEM_DATA_t;


 #ifdef _PROTOCOL_C
    SYSTEM_DATA_t SystemDataRegister = {.idx=SYSTEM_DATA_IDX};
    
#else
    extern SYSTEM_DATA_t SystemDataRegister;
 
#endif  
    
//________________________________________ PARAM REGISTER DEFINITION SECTION _     
 
 /**
 * \addtogroup CANPROT
 * 
 * ## PARAMETER register description
 * 
 *   
 */
        

typedef enum{
    PARAM_IDX = 0,
}PARAM_INDEX_t;

    
typedef struct {
    const unsigned char idx; //!< Address constant 
    
    unsigned char d0; 
    unsigned char d1; 
    unsigned char d2; 
    unsigned char d3; 
    
}PARAM_t;


 #ifdef _PROTOCOL_C
    PARAM_t ParamRegister = {.idx=PARAM_IDX};
    
#else
    extern PARAM_t MasParamRegister;
 
#endif  

        
//_______________________________________ PROTOCOL COMMANDS DEFINITION SECTION _ 
        
 /**
 * \addtogroup CANPROT
 * 
 * ## PROTOCOL COMMANDS DESCRIPTION
 * 
 */     

/// \ingroup CANPROT
/// This is the list of the implemented COMMANDS
typedef enum{
   	ABORT_COMMAND = 0,			//!< Abort Command (mandatory as for device protocol)
	FILAMENT_OFF,				//!< Switches Off he filament
	ACTIVATE_KEEPING,			//!< Start the filament in keeping mode
	ACTIVATE_RUNNING,			//!< Start the filament with a target current
    FILAMENT_INITIALIZATION,	//!< Initialize the filament detection
}PROTOCOL_COMMANDS_t;

/// \ingroup CANPROT
/// This is the list of the application COMMAND Errors
typedef enum{
   FIRST_APPLICATION_ERROR = MET_CAN_COMMAND_APPLICATION_ERRORS,      //!< An error during the format collimation has been signaled  
}PROTOCOL_COMMANDS_ERRORS_t;

#endif 
    