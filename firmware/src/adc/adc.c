#define _ADC_C

#include "application.h"
#include "adc.h"
#include "../filament/filament.h"
#include "Protocol/protocol.h"

#define PIN_IFIL (ADC_POSINPUT_AIN6)
#define PIN_VFIL (ADC_POSINPUT_AIN7)

static void ADC1_VFIL_Callback(ADC_STATUS status, uintptr_t context);
static void ADC1_IFIL_Callback(ADC_STATUS status, uintptr_t context);


void ADC1_VFIL_Callback(ADC_STATUS status, uintptr_t context){
    
    // Vfrms =  VFIL / 0.9 
    // VFIL = N * 1000 * 3.3 / (65536 * 0.9)
    // Vfrms (dV) = N * 1000 * 3.3 / (65536 * 0.9 * 0.9) = N * 0.00062
    unsigned short n = ADC1_ConversionResultGet();
    ADC_Data.rms_vfil = (unsigned short) ((float) n * 0.062);
    FilamentMonitorRegister.rms_voltL = (unsigned char) ADC_Data.rms_vfil;
    FilamentMonitorRegister.rms_voltH = (unsigned char) (ADC_Data.rms_vfil>>8);

    // Selezione del primo canale di conversione nella catena: IFIL
    ADC1_ChannelSelect( PIN_IFIL, ADC_NEGINPUT_GND );
    ADC1_CallbackRegister(ADC1_IFIL_Callback,0);
    ADC1_ConversionStart();
}

void ADC1_IFIL_Callback(ADC_STATUS status, uintptr_t context){
    
    // IFIL * 0.456 = Vmonitor = N*3.3/65536
    // IFIL = N*3.3/(0.456 * 65536)
    // Irms (mA) = IFIL / 0.9 = N*3.3*1000/(0.456 * 65536 * 0.9) = 0.1227 * N 
    unsigned short n = ADC1_ConversionResultGet();
    ADC_Data.rms_ifil =  (unsigned short) ((float) n * 0.1227);
    FilamentMonitorRegister.rms_ifilL = (unsigned char) ADC_Data.rms_ifil;
    FilamentMonitorRegister.rms_ifilH = (unsigned char) (ADC_Data.rms_ifil>>8);
    
    // Cambia canale e va a monitorare la tensione di filamento
    ADC1_ChannelSelect( PIN_VFIL, ADC_NEGINPUT_GND );
    ADC1_CallbackRegister(ADC1_VFIL_Callback,0);
    ADC1_ConversionStart();
    
}


void ADCInit(void){
    
    // INizializzazione conversione canale AD1
    ADC1_Enable();
    
    // Selezione del primo canale di conversione nella catena: IFIL
    ADC1_ChannelSelect( PIN_IFIL, ADC_NEGINPUT_GND );
    ADC1_CallbackRegister(ADC1_IFIL_Callback,0);
    ADC1_ConversionStart();
    
}

void ADC15msCallback(void){
    
}