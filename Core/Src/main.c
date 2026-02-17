#include "stm32f103xb.h"
#include "config.h"

void Clock_Config_72MHz(void);
void Timer2_Init_1200Hz(void);

volatile float Y1_st = 0, Y2_st = 0, Y4_st = 0; 
volatile float current_x1 = 0;
volatile uint8_t system_fault = 0;        // Флаг перманентной блокировки

float i_term1 = 0, i_term2 = 0;

void TIM2_IRQHandler(void) {
    if (TIM2->SR & TIM_SR_UIF) {
        TIM2->SR &= ~TIM_SR_UIF;

        // 1. ОГРАНИЧЕНИЕ УСТАВКИ (Защита ввода)
        float safe_target = U_SET_USER;
        if (safe_target > U_SET_LIMIT)  safe_target = U_SET_LIMIT;
        if (safe_target < -U_SET_LIMIT) safe_target = -U_SET_LIMIT;

        // 2. ПЕРМАНЕНТНАЯ БЛОКИРОВКА (Если была авария)
        if (system_fault != 0) {
            safe_target = 0.0f;
        }

        // 3. ВНЕШНИЙ КОНТУР (Скорость)
        float feedback_v4 = Y4_st * K_OC;
        float err2 = safe_target - feedback_v4;
        i_term2 += err2 * REG_TS;
        
        if (i_term2 > I_TERM_LIMIT) i_term2 = I_TERM_LIMIT;
        if (i_term2 < -I_TERM_LIMIT) i_term2 = -I_TERM_LIMIT;

        float x1_ref = (KP2 * err2) + (KI2 * i_term2);
        
        if (x1_ref > 10.0f) x1_ref = 10.0f;
        if (x1_ref < -10.0f) x1_ref = -10.0f;

        // 4. ВНУТРЕННИЙ КОНТУР (Ток)
        float feedback_v2 = Y2_st * (2.0f / 1000.0f);
        float err1 = x1_ref - feedback_v2;
        i_term1 += err1 * REG_TS;
        
        if (i_term1 > I_TERM_LIMIT) i_term1 = I_TERM_LIMIT;
        if (i_term1 < -I_TERM_LIMIT) i_term1 = -I_TERM_LIMIT;

        current_x1 = (KP1 * err1) + (KI1 * i_term1);
        
        if (current_x1 > 2.0f)  current_x1 = 2.0f;
        if (current_x1 < -2.0f) current_x1 = -2.0f;
    }
}

int main(void) {
    Clock_Config_72MHz(); 
    Timer2_Init_1200Hz();

    const float invT1 = 1.0f / T1;
    const float invT2 = 1.0f / T2;
    const float invT4 = 1.0f / T4;
    const float dt = SIM_DT_FAST;

    while(1) {
        float u_in = current_x1 * GAIN_FIX;

        // Модель объекта
        Y1_st += (K1 * u_in - Y1_st) * invT1 * dt;
        Y2_st += (K3 * Y1_st - Y2_st) * invT2 * dt;
        Y4_st += (K4 * Y2_st - Y4_st) * invT4 * dt;

        // ЗАЩИТЫ (Срабатывают при превышении в любую сторону)
        if (Y4_st > SPEED_LIMIT || Y4_st < -SPEED_LIMIT || Y2_st > Y2_LIMIT || Y2_st < -Y2_LIMIT) {
            system_fault = 1; 
        }
    }
}
// --- Конфигурирование МК ---
void Clock_Config_72MHz(void) {
    RCC->CR |= RCC_CR_HSEON;
    while (!(RCC->CR & RCC_CR_HSERDY));
    FLASH->ACR |= FLASH_ACR_LATENCY_2;
    RCC->CFGR |= (RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9);
    RCC->CR |= RCC_CR_PLLON;
    while (!(RCC->CR & RCC_CR_PLLRDY));
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

void Timer2_Init_1200Hz(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = 71;   
    TIM2->ARR = 832;  
    TIM2->DIER |= TIM_DIER_UIE;
    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);
    TIM2->CR1 |= TIM_CR1_CEN;
}