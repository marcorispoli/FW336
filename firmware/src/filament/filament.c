#define _FILAMENT_C

#include "application.h"
#include "filament.h"


static bool  FilamentVrefSetG1(unsigned short value );
static bool  FilamentVrefSetG2(unsigned short value );

// Impostazione dei fuoci:
// Il Fuoco A è selezionato con segnale spento
// Il Fuoco G è selezionato con segnale spento
#define SET_FILA uC_FOCUS_BA_Clear();
#define SET_FILB uC_FOCUS_BA_Set();
#define SET_FILG uC_FOCUS_PG_Clear();
#define SET_FILS uC_FOCUS_PG_Set();

void FilamentInit(void){
    
    // Disabilita il reference
    uC_REFDIS_Set();
    uC_FILON_Clear();
    FIL_Data.status = FILSTAT_OFF;
    
    uC_FOCUS_BA_Set();
    uC_FILON_Clear();
    
    // Azzeramento Vreference
    FilamentVrefSetG1(0);
}

/**
 * This function set the externa dac 4821 to the 
 * requested value with Gain = 1.
 * 
 * The Maximum output voltage will be 2.048V
 * 
 * @param value = digital value [0:4095]
 * 
 * @return 
 */
bool  FilamentVrefSetG1(unsigned short value ){
    unsigned char buffer[2];
    bool result = false;
    
    buffer[0] = VREF_GAINx1|VREF_RUN;
    buffer[0] |= (unsigned char)  ((value & 0x0F00) >> 8);
    buffer[1] =  (unsigned char)  (value & 0x00FF);
 
    uC_CS_Clear();
    result = SERCOM4_SPI_Write((void*) buffer, 2);
    uC_CS_Set();  
    
    return result;
}

/**
 * This function set the externa dac 4821 to the 
 * requested value with Gain = 2.
 * 
 * The Maximum output voltage will be 4.096V
 * 
 * @param value = digital value [0:4095]
 * @return 
 */
bool  FilamentVrefSetG2(unsigned short value ){
    unsigned char buffer[2];
    bool result = false;
    
    buffer[0] = VREF_GAINx2|VREF_RUN;
    buffer[0] |= (unsigned char)  ((value & 0x0F00) >> 8);
    buffer[1] =  (unsigned char)  (value & 0x00FF);
 
    uC_CS_Clear();
    result = SERCOM4_SPI_Write((void*) buffer, 2);
    uC_CS_Set();  
    
    return result;
}


float FilamentCorrectionFactor(float kv ){
    
    // To Be Done
    return 1;
}

/**
 * This function set the Vreference of the UC3841 
 * to the value corrispondent to the desidered output HV voltage.
 * 
 * 
 * + Nsec/Nsens = 87
 * + Vreference_3841 = (Vfeedback - 1.4) * 8.32 / 1000 (Input voltage divisor network)
 * + Vreference_3841 = [(Vo/87) - 1.4] * [8.32 / 1000] 
 * + Vdac = N/1000 (Gain = 2)
 * + Vreference_3841 = 2 * Vdac = (2*N/1000);
 * 
 * N = Vreference_3841 * 1000 / 2 = (Vo/87 - 1.4) * 8.32 / 2 
 * N = (Vo / 20,91 - 5,82)
 * 
 * @param kv: target output voltage (kV)
 * @return 
 */
bool FilamentCurrentSet(float ma ){
    unsigned short N;
    
    
    // Calcolo N con Gain = 2 e con eventuale correzione introdotta da calibrazione;
    //N = (unsigned short) (GeneratorHVCorrectionFactor(kv) * ((kv * 1000 / 20.91) - 5.82));
    N= 2048;
    return FilamentVrefSetG2(N);
    
}