#ifndef CONFIG_H
#define CONFIG_H

// --- ВРЕМЯ ---
#define REG_FREQ        1200.0f
#define REG_TS          (1.0f / REG_FREQ) 
#define SIM_DT_FAST     0.0001f          

// --- ОБЪЕКТ  ---
#define K1 12.8f
#define T1 0.003f
#define K3 1.95f
#define T2 0.0523f
#define K4 2.86f
#define T4 0.257f

// --- РЕГУЛЯТОРЫ (a=18) ---
#define KP1 0.3492f
#define KI1 6.677f
#define KP2 2496.0f
#define KI2 23111.0f

// --- УСТАВКИ И ЛИМИТЫ ---
#define U_SET_USER      2.0f              // 
#define U_SET_LIMIT     2.0f              // 
#define Y2_LIMIT        1000.0f           // 
#define SPEED_LIMIT     3050.0f           // 
#define K_OC            (2.0f / 3000.0f)  //

#define I_TERM_LIMIT    15.0f             //
#define GAIN_FIX        21.037f           //

#endif