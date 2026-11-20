#define _FILAMENT_C

#include "application.h"
#include "filament.h"
#include "../adc/adc.h"
#include "Protocol/protocol.h"


static bool  FilamentVrefSetG1(unsigned short value );
static bool  FilamentVrefSetG2(unsigned short value );
static bool  FilamentRMSCurrentSet(unsigned short ma );
static void  FilamentSetFocus(FILSEL_t focus);
static void  FilamentSetStatus(FILSTAT_t status);

// Impostazione dei fuoci:
// Il Fuoco A è selezionato con segnale spento
// Il Fuoco G è selezionato con segnale spento
#define SET_FILA uC_FOCUS_BA_Clear();
#define SET_FILB uC_FOCUS_BA_Set();
#define SET_FILG uC_FOCUS_PG_Clear();
#define SET_FILS uC_FOCUS_PG_Set();

#define SEQ_INIT    0
#define SEQ_WARMING 10
#define SEQ_TARGET  20
#define SEQ_RUNNING 30

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
    float val = (unsigned short) ((float) ma * (float) DAC_REFERENCE/ (float) ImA_REFERENCE );
    if(val > 4095) return 4095; 
    return (unsigned short) val;
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
    bool ris = FilamentVrefSetG1(N);
    if(ris) FIL_Data.rms_current = ma;
    return ris;    
}

void FilamentInit(void){
    
    // Azzeramento Vreference
    FilamentVrefSetG1(0);
    FilamentVrefSetG2(0);
    
    // Inizializza in Off e fuoco A grande
    FilamentOff();
    FilamentSetFocus(FILSEL_AL);
  
    
}

void  FilamentSetStatus(FILSTAT_t status){
    FIL_Data.status = status;    
    FilamentStatusRegister.status = (unsigned char) status;
    encodeStatusRegister(&FilamentStatusRegister);
    return;
}

void  FilamentSetFocus(FILSEL_t focus){

    // Seleziona la corretta combinazione di rele
    switch(focus){
        case FILSEL_AS:
            SET_FILA;
            SET_FILS;
            FIL_Data.focus.fildgn_ptr = &FIL_Data.focus.filas;
            break;
        case FILSEL_AL:
            SET_FILA;
            SET_FILG;
            FIL_Data.focus.fildgn_ptr = &FIL_Data.focus.filal;
            break;
        case FILSEL_BS:
            SET_FILB;
            SET_FILS;
            FIL_Data.focus.fildgn_ptr = &FIL_Data.focus.filbs;
            break;
        case FILSEL_BL:
            SET_FILB;
            SET_FILG;
            FIL_Data.focus.fildgn_ptr = &FIL_Data.focus.filbl;
            break;
        default:
            focus = FILSEL_AL;
            SET_FILA;
            SET_FILG;
            FIL_Data.focus.fildgn_ptr = &FIL_Data.focus.filal;
            break;
    }
    
    FIL_Data.focus.code = focus;    
     
    FilamentStatusRegister.focus_selection = (unsigned char) FIL_Data.focus.code;
    encodeStatusRegister(&FilamentStatusRegister);
    return;
}


void FilamentOff(void){
    FilamentSetStatus(FILSTAT_OFF);
    FIL_Data.command = FILCMD_NONE;
    uC_REFDIS_Set();
    uC_FILON_Clear();
    FilamentRMSCurrentSet(0);    
    return;
}

bool FilamentSetKeeping(FILSEL_t focus){    
    
    // Se già in Keeping e riconosciuto allora si ferma qui
    if((focus == FIL_Data.focus.code) && (FIL_Data.focus.fildgn_ptr->detected)&&((FIL_Data.command == FILCMD_KEEPING)||(FIL_Data.status == FILSTAT_KEEPING))){
        if(FIL_Data.rms_current != KEEPING_CURRENT)   FilamentRMSCurrentSet((unsigned short) KEEPING_CURRENT);
        return true;
    }
    
    // Inizia la sequenza di attivazione del keeping.
    FIL_Data.command = FILCMD_KEEPING;
    FIL_Data.sequence.focus = focus;
    FIL_Data.sequence.delay = 0;
    FIL_Data.sequence.target_current = KEEPING_CURRENT;
    
    
    // Se il filamento era spento o non selzionato o non identificato parte da INIT
    if((FIL_Data.status == FILSTAT_OFF) || (FIL_Data.focus.code != focus)|| (!FIL_Data.focus.fildgn_ptr->detected)){
        FIL_Data.sequence.step = SEQ_INIT;   
        FilamentSetStatus(FILSTAT_INIT);
        return true;
    }
    
    // Se la corrente attuale è maggiore di quella richiesta
    // modifica subito la corrente e termina il comando
    if(KEEPING_CURRENT < FIL_Data.rms_current){
        FilamentRMSCurrentSet(KEEPING_CURRENT);
        FilamentSetStatus(FILSTAT_KEEPING);
        FIL_Data.command = FILCMD_NONE;
        return true;
    }
    
    FIL_Data.sequence.step = SEQ_WARMING; 
    FilamentSetStatus(FILSTAT_WARMING);
    return true;
    
}


ext bool FilamentSetCurrent(FILSEL_t focus, unsigned short target_current, unsigned short timeout_ms){
    
    // Non accende filamenti non identificati
    if((focus == FILSEL_AS) && (!FIL_Data.focus.filas.detected)) return false;
    if((focus == FILSEL_AL) && (!FIL_Data.focus.filal.detected)) return false;
    if((focus == FILSEL_BS) && (!FIL_Data.focus.filbs.detected)) return false;
    if((focus == FILSEL_BL) && (!FIL_Data.focus.filbl.detected)) return false;
    
    // Se già c'è un comando di running in corso, effettua solo un update del target di corrente
    // e del timeout
    if((focus == FIL_Data.focus.code) && (FIL_Data.command == FILCMD_RUNNING)){
        FIL_Data.sequence.target_current = target_current;
        FIL_Data.sequence.timeout_ms = timeout_ms;
        
        // Aggiorna immediatamente il livello di corrente
        if(FIL_Data.rms_current > target_current){
            FilamentRMSCurrentSet(target_current);
        }else{
            FIL_Data.sequence.step = SEQ_WARMING;     
            FilamentSetStatus(FILSTAT_WARMING);
        }
        
        return true;
    }
    
    // Inizia la sequenza di attivazione del keeping.
    FIL_Data.command = FILCMD_RUNNING;
    FIL_Data.sequence.focus = focus;
    FIL_Data.sequence.delay = 0;
    FIL_Data.sequence.target_current = target_current;
    FIL_Data.sequence.timeout_ms = timeout_ms;
    
    // Se il filamento era spento o non selzionato, parte da zero
    if((FIL_Data.status == FILSTAT_OFF) || (FIL_Data.focus.code != focus)){
        FIL_Data.sequence.step = SEQ_INIT;      
        FilamentSetStatus(FILSTAT_INIT);
        return true;
    }
    
    FIL_Data.sequence.step = SEQ_WARMING;       
    FilamentSetStatus(FILSTAT_WARMING);
    return true;
   
}

bool FilamentInitializationSequence(bool start){
    static int initseq = 0;
    static int delay = 0;
    if(start){
        if(FIL_Data.command != FILCMD_NONE) return false;
        
        initseq = 1;
        delay = 0;
        // Reset di tutti i fault e dell'identificativo
        ((unsigned char*) &FIL_Data.focus.filas)[0] = 0;
        ((unsigned char*) &FIL_Data.focus.filal)[0] = 0;
        ((unsigned char*) &FIL_Data.focus.filbs)[0] = 0;
        ((unsigned char*) &FIL_Data.focus.filbl)[0] = 0;
        
        return true;
    }
    
    if(!initseq) return false;
    
    if(delay){
        delay--;
        return true;
    }
    
    switch(initseq){
        case 1:
            FilamentSetKeeping(FILSEL_AS);
            initseq++;
            break;
        case 2:
            if(FIL_Data.command != FILCMD_NONE) return true;
            if(FIL_Data.focus.fildgn_ptr->detected) delay = 7;
            else delay = 70;
            initseq++;        
            break;
        case 3:
            FilamentSetKeeping(FILSEL_AL);
            initseq++;        
            break;
        case 4:
            if(FIL_Data.command != FILCMD_NONE) return true;
            if(FIL_Data.focus.fildgn_ptr->detected) delay = 7;
            else delay = 70;
            initseq++;
            break;
        case 5:
            FilamentSetKeeping(FILSEL_BS);
            initseq++;
            break;
        case 6:
            if(FIL_Data.command != FILCMD_NONE) return true;
            if(FIL_Data.focus.fildgn_ptr->detected) delay = 7;
            else delay = 70;
            initseq++;
            break;
        case 7:
            FilamentSetKeeping(FILSEL_BL);
            initseq++;
            break;
        case 8:
            if(FIL_Data.command != FILCMD_NONE) return true;
            if(FIL_Data.focus.fildgn_ptr->detected) delay = 7;
            else delay = 70;
            initseq++;
            break;
        case 9:
            FilamentOff();
            initseq=0;
        break;
    }
    
    return true;
}


void Filament15msCallback(void){
    
    // Aggiorna il registro del can bus con i dati aggiornati)
    encodeStatusRegister(&FilamentStatusRegister);
    encodeStatusRegister(&FilamentMonitorRegister);

    FilamentDgnRegister.filas = ((unsigned char*) &FIL_Data.focus.filas)[0];
    FilamentDgnRegister.filal = ((unsigned char*) &FIL_Data.focus.filal)[0];
    FilamentDgnRegister.filbs = ((unsigned char*) &FIL_Data.focus.filbs)[0];
    FilamentDgnRegister.filbl = ((unsigned char*) &FIL_Data.focus.filbl)[0];
    encodeStatusRegister(&FilamentDgnRegister);
    
    // Led di Fault in caso di filamento con anomalie presenti
    if(FIL_Data.focus.fildgn_ptr->fault){
        LED_FLT_Set();
    }else LED_FLT_Clear();
    
    
    // Gestisce le fasi di inizializzazione se attivata
    FilamentInitializationSequence(false);
                
    
    
    
    // Controlli in stato di KEEPING e RUNNING per i filamenti identificati
    if((FIL_Data.status == FILSTAT_RUNNING) || (FIL_Data.status == FILSTAT_KEEPING)){

        // Diagnostica sulla corrente massima sempre
        if(ADC_Data.rms_ifil > ABSOLUTE_RMS_CURRENT){
            FilamentOff();
            FIL_Data.focus.fildgn_ptr->max_current = 1;
            FIL_Data.focus.fildgn_ptr->fault = 1;
            return;
        }

            
        if(FIL_Data.focus.fildgn_ptr->detected) {
            // Se il filamento risulta aperto deve essere spento
            if(ADC_Data.rms_ifil < 200 ){

                FilamentOff();
                FIL_Data.focus.fildgn_ptr->filament_open = 1;
                FIL_Data.focus.fildgn_ptr->fault = 1;
            }else if(ADC_Data.rms_ifil < FIL_Data.rms_current * 0.85 ) {
                FIL_Data.focus.fildgn_ptr->low_current = 1;
                FIL_Data.focus.fildgn_ptr->fault = 1;
            }else if(ADC_Data.rms_ifil > FIL_Data.rms_current * 1.15 ){ 
                FIL_Data.focus.fildgn_ptr->high_current = 1;
                FIL_Data.focus.fildgn_ptr->fault = 1;
            }           
            
            // Calcola l'impedenza ma solo in KEEPING
            if((FIL_Data.status == FILSTAT_KEEPING)){
                float Z = ADC_Data.rms_vfil / ADC_Data.rms_ifil;
                if(Z > 3) {
                    FIL_Data.focus.fildgn_ptr->filament_open = 1;
                    FIL_Data.focus.fildgn_ptr->fault = 1;
                }
            }
            
            
       }

    }
     
    
    if(FIL_Data.command == FILCMD_NONE) return;
    
    
    // Ritardo programmato dalla seqeunza in corso
    if(FIL_Data.sequence.delay){
        FIL_Data.sequence.delay--;
        return;
    }

    
    switch(FIL_Data.sequence.step){
        case SEQ_INIT: // Spegne il filamento prima di aprire i relay per evitare archi

            FilamentRMSCurrentSet(0);
            uC_REFDIS_Set();
            FIL_Data.sequence.delay = 14; // 200ms di spegnimento
            FIL_Data.sequence.step++;
            break;

        case (SEQ_INIT+1): // Attiva i rele e attende la commutazione prima di accendere la corrente
            FilamentSetFocus(FIL_Data.sequence.focus);                
            uC_FILON_Set();
            uC_REFDIS_Clear();
            FIL_Data.sequence.delay = 14; // 200ms di spegnimento
            FIL_Data.sequence.step++;
            break;

        case (SEQ_INIT+2): // Attiva la rampa di corrente a partire da zero
            FIL_Data.sequence.delay = 1; 
            FIL_Data.sequence.step = SEQ_WARMING;
            break;

        case SEQ_WARMING: // Inizio rampa perraggiungere il target di corrente richiesto
            FilamentSetStatus(FILSTAT_WARMING);
            FIL_Data.sequence.delay = 0; 
            FIL_Data.sequence.step++;                
            break;

        case SEQ_WARMING+1:                

            // Al raggiungimento della corrente di target si va alla sezione di chiusura
            if(FIL_Data.rms_current >= FIL_Data.sequence.target_current){
                FilamentRMSCurrentSet(FIL_Data.sequence.target_current);
                FIL_Data.sequence.step = SEQ_TARGET;
                FIL_Data.sequence.delay = 0; 
                break;
            }

            // Incrementa il valore della corrente
            FilamentRMSCurrentSet(FIL_Data.rms_current + CURRENT_RAMP);
            FIL_Data.sequence.delay = 0; 
            break;

        case SEQ_TARGET:                               
            
            if(ADC_Data.rms_ifil > 300 ) FIL_Data.focus.fildgn_ptr->detected = 1;
             
            // Se il comando era il mantenimento allora finisce qui
            if(FIL_Data.command == FILCMD_KEEPING){
                
                FIL_Data.command = FILCMD_NONE;
                FilamentSetStatus(FILSTAT_KEEPING);
                FIL_Data.sequence.delay = 0;
                return;

            }

            // Se il comando era RUNNING allora va in running con timeout
            FIL_Data.sequence.step++;
            FilamentSetStatus(FILSTAT_RUNNING);
            FIL_Data.sequence.delay = 0; 
            break;

        case SEQ_TARGET+1: 

            // Finisce il running e torna in mantenimento
            if(FIL_Data.sequence.timeout_ms <= (unsigned short) 15) {
                FilamentRMSCurrentSet(KEEPING_CURRENT);
                FIL_Data.command = FILCMD_NONE;
                FilamentSetStatus(FILSTAT_KEEPING);                   
                FIL_Data.sequence.delay = 0; 
                return;                    
            }

            FIL_Data.sequence.timeout_ms = FIL_Data.sequence.timeout_ms - 15;
            break;
    }

    return;
    
}
