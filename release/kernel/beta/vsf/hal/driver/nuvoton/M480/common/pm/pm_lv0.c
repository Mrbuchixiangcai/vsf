/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

/*============================ INCLUDES ======================================*/
#include "../common.h"
#include "./pm.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

pm_power_status_t vsf_pm_power_get_status(pm_power_cfg_no_t index)
{
    return CLK->PWRCTL;
}

vsf_err_t vsf_pm_power_resume(pm_power_cfg_no_t index, pm_power_status_t status)
{
    uint_fast32_t mask = 1 << index;
    if ((CLK->PWRCTL ^ status) & mask) {
        bool state = m480_reg_unlock();
            VSF_HAL_CFG_ATOM_ACCESS() {
                if (status & mask) {
                    CLK->PWRCTL |= mask;
                } else {
                    CLK->PWRCTL &= ~mask;
                }
            }
        m480_reg_lock(state);
    }
    return VSF_ERR_NONE;
}

pm_power_status_t vsf_pm_power_enable(pm_power_cfg_no_t index)
{
    pm_power_status_t orig = vsf_pm_power_get_status(index);
    vsf_pm_power_resume(index, orig | (1 << index));
    return orig;
}

pm_power_status_t vsf_pm_power_disable(pm_power_cfg_no_t index)
{
    pm_power_status_t orig = vsf_pm_power_get_status(index);
    vsf_pm_power_resume(index, orig & ~(1 << index));
    return orig;
}

vsf_err_t vsf_pm_sleep(pm_sleep_cfg_t *pcfg)
{
    uint_fast32_t reg = pcfg->sleep_mode | pcfg->sleep_cfg | pcfg->sleep_walking_cfg | pcfg->wake_cfg;
    bool state = m480_reg_unlock();
        CLK->PMUCTL = reg;
    m480_reg_lock(state);
    return VSF_ERR_NONE;
}

pm_periph_async_clk_status_t vsf_pm_peripheral_get_status(pm_periph_async_clk_no_t index)
{
    return (pm_periph_async_clk_status_t)(
                (m480_bit_field_get((index >> 0) & 0x3FFF, (uint32_t *)&CLK->CLKSEL0) << 8)
            |   (m480_bit_field_get((index >> 14) & 0x3FFF, (uint32_t *)&CLK->CLKDIV0) << 0));
}

vsf_err_t vsf_pm_peripheral_resume(pm_periph_async_clk_no_t index , pm_periph_async_clk_status_t status)
{
    if (status != vsf_pm_peripheral_get_status(index)) {
        m480_bit_field_set((index >> 0) & 0x3FFF, (uint32_t *)&CLK->CLKSEL0, (status >> 8) & 0xFF);
        m480_bit_field_set((index >> 14) & 0x3FFF, (uint32_t *)&CLK->CLKDIV0, (status >> 0) & 0xFF);
    }
    return VSF_ERR_NONE;
}

pm_periph_async_clk_status_t vsf_pm_peripheral_config(pm_periph_async_clk_no_t index,
                                                const pm_periph_asyn_clk_cfg_t *pcfg)
{
    pm_periph_async_clk_status_t orig = vsf_pm_peripheral_get_status(index);
    vsf_pm_peripheral_resume(index, (pm_periph_async_clk_status_t)(pcfg->div | (pcfg->clk_src << 8)));
    return orig;
}

#define __def_vsf_pm_peripheral_clksel(__name)                                  \
            [TPASTE2(__name, _MAP_IDX)] = TPASTE2(__name, _MAP)

static const pm_periph_clksel_t vsf_pm_peripheral_clksel[] = {
    __def_vsf_pm_peripheral_clksel(SDH_CLKSEL),
    __def_vsf_pm_peripheral_clksel(STCLK_CLKSEL),
    __def_vsf_pm_peripheral_clksel(HCLK_CLKSEL),
    __def_vsf_pm_peripheral_clksel(WWDT_CLKSEL),
    __def_vsf_pm_peripheral_clksel(CLKO_CLKSEL),
    __def_vsf_pm_peripheral_clksel(UART_CLKSEL),
    __def_vsf_pm_peripheral_clksel(TMR3_CLKSEL),
    __def_vsf_pm_peripheral_clksel(TMR2_CLKSEL),
    __def_vsf_pm_peripheral_clksel(TMR1_CLKSEL),
    __def_vsf_pm_peripheral_clksel(TMR0_CLKSEL),
    __def_vsf_pm_peripheral_clksel(WDT_CLKSEL),
    __def_vsf_pm_peripheral_clksel(SPI13_CLKSEL),
    __def_vsf_pm_peripheral_clksel(SPI02_CLKSEL),
    __def_vsf_pm_peripheral_clksel(BPWM1_CLKSEL),
    __def_vsf_pm_peripheral_clksel(BPWM0_CLKSEL),
    __def_vsf_pm_peripheral_clksel(RTC_CLKSEL),
};

// todo: calculate frequency of clocks
static uint_fast32_t vsf_pm_peripheral_get_clksrc(uint_fast8_t index)
{
    switch (index) {
    case CLKSRC_HXT:
    case CLKSRC_HXTD2:
    case CLKSRC_LXT:
    case CLKSRC_HIRC:
    case CLKSRC_HIRCD2:
    case CLKSRC_LIRC:

    case CLKSRC_PLL:
    case CLKSRC_HCLK:
    case CLKSRC_HCLKD2:
    case CLKSRC_HCLKD2K:
    case CLKSRC_PCLK0:
    case CLKSRC_PCLK1:

    case CLKSRC_TM0_PIN:
    case CLKSRC_TM1_PIN:
    case CLKSRC_TM2_PIN:
    case CLKSRC_TM3_PIN:
        return 100;
    }
    return 0;
}

static uint_fast32_t vsf_pm_peripheral_get_clksrc_from_clksel(uint_fast8_t map_idx, uint_fast8_t clksel)
{
    ASSERT((map_idx < dimof(vsf_pm_peripheral_clksel)) && (clksel <= 7));
    uint_fast8_t clksrc = (vsf_pm_peripheral_clksel[map_idx] >> (4 * clksel)) & 0x0F;
    return vsf_pm_peripheral_get_clksrc(clksrc);
}

uint_fast32_t vsf_pm_peripheral_get_clock(pm_periph_async_clk_no_t index)
{
    uint_fast16_t bf_clksel = (index >> 0) & 0x3FFF;

    if (m480_bit_field_get_bitlen(bf_clksel)) {
        pm_periph_async_clk_status_t status = vsf_pm_peripheral_get_status(index);
        uint_fast8_t clksel = (status >> 8) & 0xFF;
        uint_fast16_t clkdiv = (status >> 0) & 0xFFFF;
        uint_fast8_t clksel_map_idx = index >> 28;

        uint_fast32_t clk = vsf_pm_peripheral_get_clksrc_from_clksel(clksel_map_idx, clksel);
        return clk / (clkdiv + 1);
    }
    return 0;
}

pm_ahbclk_status_t vsf_pm_ahbclk_get_status(pm_ahb_clk_no_t index)
{
    uint_fast8_t bus_idx = index >> 5;
    return (pm_ahbclk_status_t)(&CLK->AHBCLK)[bus_idx];
}

vsf_err_t vsf_pm_ahbclk_resume(pm_ahb_clk_no_t index, pm_ahbclk_status_t status)
{
    uint_fast8_t bus_idx = index >> 5;
    uint_fast8_t bit_idx = index & 0x1F;

    reg32_t *reg_enable = &CLK->AHBCLK;
    uint_fast32_t mask = 1 << bit_idx;

    if ((reg_enable[bus_idx] ^ status) & mask) {
        bool state = m480_reg_unlock();
            if (status & mask) {
                reg_enable[bus_idx] |= mask;
            } else {
                reg_enable[bus_idx] &= ~mask;
            }
        m480_reg_lock(state);
    }
    return VSF_ERR_NONE;
}

pm_ahbclk_status_t vsf_pm_ahbclk_enable(pm_ahb_clk_no_t index)
{
    pm_ahbclk_status_t orig = vsf_pm_ahbclk_get_status(index);
    uint_fast8_t bit_idx = index & 0x1F;
    vsf_pm_ahbclk_resume(index, (pm_ahbclk_status_t)(1 << bit_idx));
    return orig;
}

pm_ahbclk_status_t vsf_pm_ahbclk_disable(pm_ahb_clk_no_t index)
{
    pm_ahbclk_status_t orig = vsf_pm_ahbclk_get_status(index);
    vsf_pm_ahbclk_resume(index, (pm_ahbclk_status_t)0);
    return orig;
}

fsm_rt_t vsf_pm_mclk_init(pm_main_clk_cfg_t *pcfg)
{
}

uint_fast32_t vsf_pm_mclk_get(void)
{
}

fsm_rt_t vsf_pm_pll_init(pm_pll_sel_t pll, pm_pll_cfg_t *pcfg)
{
}

bool vsf_pm_pll_is_locked(pm_pll_sel_t pll)
{
}

uint_fast32_t vsf_pm_pll_get_clk_out(pm_pll_sel_t pll)
{
}

uint_fast32_t vsf_pm_pll_get_clk_in(pm_pll_sel_t pll)
{
}

