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
    
    FilamentSelectFocus(FILSTAT_OFF,0);
   
    // Azzeramento Vreference
    FilamentVrefSetG1(0);
    FilamentVrefSetG2(0);
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


unsigned short FilamentCalibratedValue(unsigned short ma){
     float val = (unsigned short) ((float) ma * (float) DAC_2000mA_REFERENCE/ 2000.0 );
    if(val > 4095) return 4095; 
    return (unsigned short) val;
}

bool FilamentSelectFocus(FILSTAT_t focus, unsigned short target_current){
    if(focus == FILSTAT_OFF) {
        FIL_Data.status = FILSTAT_OFF;
        uC_REFDIS_Set();
        uC_FILON_Clear();
        FilamentVrefSetG1(0);        
        FIL_Data.target_current_ma = 0;
        FIL_Data.warmup_current_ma = 0;
        return false;
    }
    
    // Se il fuoco non è cambiato e non sono cambiati i dati di controllo 
    // non fa altro
    if( (focus == FIL_Data.status) && (FIL_Data.target_current_ma == target_current)) return false;
    
    // Azione immediata se la corrente richiesta è inferiore
    if((focus == FIL_Data.status) && (FIL_Data.target_current_ma > target_current)){
        FIL_Data.target_current_ma = target_current;
        FIL_Data.warmup_current_ma = target_current;
        FilamentRMSCurrentSet(target_current);
        return false;
    }
    
    // Attiva la fase di warmup
    FIL_Data.warmup = true;
    
    // Deve impostare la fase di warmup
    // Se il fuoco era spento, il warmup sarà più lungo e graduale mentre se era già
    // riscaldato allora sarà sufficiente portarlo alla corrente target con una rampa veloce
    if(focus != FIL_Data.status){
        FIL_Data.warmup_current_ma = INITIAL_WARMUP_CURRENT_ma;
        
        // Spegne il riferimento
        FilamentVrefSetG1(0);  
        
        // Apre il filamento
        uC_FILON_Clear();
        FIL_Data.switch_filament = true;
    }else{        
        FIL_Data.switch_filament = false;
        
        
        // Riparte da dove stava
        FIL_Data.warmup_current_ma = FIL_Data.target_current_ma;
    }
    
    FIL_Data.target_current_ma = target_current;
    FIL_Data.status = focus;
    return true;
}

/**
 * Questa funzione imposta il valore RMS della corrente di filamento.
 * 
 * Il Valore RMS corrisponde al valore raddrizzato * 0.9
 * 
 * @param ma: è il valore rms richiesto
 * 
 * @return tru: l'impostazione è andata a buon fine
 */
bool FilamentRMSCurrentSet(unsigned short ma){
    
    // Limita il valore massimo selezionabile 
    if(ma > MAX_SELECTABLE_CURRENT_mA) ma = MAX_SELECTABLE_CURRENT_mA;    
    unsigned short N = FilamentCalibratedValue(ma);
    uC_REFDIS_Clear();
    uC_FILON_Set();
    return FilamentVrefSetG1(N);
    
}


void Filament15msCallback(void){
    static int delay = 0;
    
    if(delay){
        delay--;
        return;
    } 
    
    // GEstione della fase di accensione del filamento
    if(FIL_Data.warmup){
        // Esegue lo scambio dei rele del filamento
        if(FIL_Data.switch_filament){
            FIL_Data.switch_filament = false;
            
            if(FIL_Data.status == FILSTAT_AS){
                SET_FILA;
                SET_FILS;
            }else if(FIL_Data.status == FILSTAT_AG){
                SET_FILA;
                SET_FILG;
            }else if(FIL_Data.status == FILSTAT_BS){
                SET_FILB;
                SET_FILS;
            }else if(FIL_Data.status == FILSTAT_BG){
                SET_FILB;
                SET_FILG;
            }
            uC_REFDIS_Clear();
            uC_FILON_Set();
            delay = 20;
        }
        
        if(FIL_Data.warmup_current_ma < WARMUP_CURRENT_ma) delay = 4;
        else delay = 0;
       
        FIL_Data.warmup_current_ma += 15;
        if(FIL_Data.warmup_current_ma >= FIL_Data.target_current_ma){
            FIL_Data.warmup_current_ma = FIL_Data.target_current_ma;
            FIL_Data.warmup = false;
        }
        
        FilamentRMSCurrentSet(FIL_Data.warmup_current_ma);
    }else{
        delay = 0;
    }
}
