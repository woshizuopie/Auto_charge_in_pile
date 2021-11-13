#ifndef __APP_H_
#define __APP_H_

#include <stdint.h>

#define ADAPTER_LOW_LIMIT   20.0f
#define ADAPTER_UP_LIMIT    31.0f


void Handshake_Light_Indication(uint8_t steps);
void Low_Current_Shutoff(float shutoff_I_samp_vlot,
                          float now_I_samp_vlot);
void Over_Current_Protect(float over_current,
                          float shutoff_I_samp_vlot,
                          float now_I_samp_vlot);
void ASC711_Over_Current_Protect(void);
void Short_circuit_recovery(void);
void Over_Temperature_Protect(void);
void Over_Volt_Under_Volt_Protect(void);
void Short_circuit_Output(void);
void Slave_Reset(void);

#endif /* __APP_H_ */

