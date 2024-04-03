/*
* Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef R_SCI_B_UART_H
#define R_SCI_B_UART_H

/*******************************************************************************************************************//**
 * @addtogroup SCI_B_UART
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_uart_api.h"
#include "r_sci_b_uart_cfg.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Enumeration for SCI clock source */
typedef enum e_sci_b_clk_src
{
    SCI_B_UART_CLOCK_INT,                      ///< Use internal clock for baud generation
    SCI_B_UART_CLOCK_INT_WITH_BAUDRATE_OUTPUT, ///< Use internal clock for baud generation and output on SCK
    SCI_B_UART_CLOCK_EXT8X,                    ///< Use external clock 8x baud rate
    SCI_B_UART_CLOCK_EXT16X                    ///< Use external clock 16x baud rate
} sci_b_clk_src_t;

/** UART flow control mode definition */
typedef enum e_sci_b_uart_flow_control
{
    SCI_B_UART_FLOW_CONTROL_RTS             = 0U, ///< Use CTSn_RTSn pin for RTS
    SCI_B_UART_FLOW_CONTROL_CTS             = 1U, ///< Use CTSn_RTSn pin for CTS
    SCI_B_UART_FLOW_CONTROL_HARDWARE_CTSRTS = 3U, ///< Use CTSn pin for CTS, CTSn_RTSn pin for RTS
    SCI_B_UART_FLOW_CONTROL_CTSRTS          = 5U, ///< Use SCI pin for CTS, external pin for RTS
} sci_b_uart_flow_control_t;

/** UART instance control block. */
typedef struct st_sci_b_uart_instance_ctrl
{
    /* Parameters to control UART peripheral device */
    uint8_t  fifo_depth;               // FIFO depth of the UART channel
    uint8_t  rx_transfer_in_progress;  // Set to 1 if a receive transfer is in progress, 0 otherwise
    uint8_t  data_bytes         : 2;   // 1 byte for 7 or 8 bit data, 2 bytes for 9 bit data
    uint8_t  bitrate_modulation : 1;   // 1 if bit rate modulation is enabled, 0 otherwise
    uint32_t open;                     // Used to determine if the channel is configured

    bsp_io_port_pin_t flow_pin;

    /* Source buffer pointer used to fill hardware FIFO from transmit ISR. */
    uint8_t const * p_tx_src;

    /* Size of source buffer pointer used to fill hardware FIFO from transmit ISR. */
    uint32_t tx_src_bytes;

    /* Destination buffer pointer used for receiving data. */
    uint8_t const * p_rx_dest;

    /* Size of destination buffer pointer used for receiving data. */
    uint32_t rx_dest_bytes;

    /* Pointer to the configuration block. */
    uart_cfg_t const * p_cfg;

    /* Base register for this channel */
    R_SCI_B0_Type * p_reg;

    void (* p_callback)(uart_callback_args_t *); // Pointer to callback that is called when a uart_event_t occurs.
    uart_callback_args_t * p_callback_memory;    // Pointer to non-secure memory that can be used to pass arguments to a callback in non-secure memory.

    /* Pointer to context to be passed into callback function */
    void const * p_context;
} sci_b_uart_instance_ctrl_t;

/** Receive FIFO trigger configuration. */
typedef enum e_sci_b_uart_rx_fifo_trigger
{
    SCI_B_UART_RX_FIFO_TRIGGER_1   = 0x1, ///< Callback after each byte is received without buffering
    SCI_B_UART_RX_FIFO_TRIGGER_MAX = 0xF, ///< Callback when FIFO is full or after 15 bit times with no data (fewer interrupts)
} sci_b_uart_rx_fifo_trigger_t;

/** Asynchronous Start Bit Edge Detection configuration. */
typedef enum e_sci_b_uart_start_bit_detect
{
    SCI_B_UART_START_BIT_LOW_LEVEL    = 0x0, ///< Detect low level on RXDn pin as start bit
    SCI_B_UART_START_BIT_FALLING_EDGE = 0x1, ///< Detect falling level on RXDn pin as start bit
} sci_b_uart_start_bit_detect_t;

/** Noise cancellation configuration. */
typedef enum e_sci_b_uart_noise_cancellation
{
    SCI_B_UART_NOISE_CANCELLATION_DISABLE = 0x0, ///< Disable noise cancellation
    SCI_B_UART_NOISE_CANCELLATION_ENABLE  = 0x1, ///< Enable noise cancellation
} sci_b_uart_noise_cancellation_t;

/** RS-485 Enable/Disable. */
typedef enum e_sci_b_uart_rs485_enable
{
    SCI_B_UART_RS485_DISABLE = 0,      ///< RS-485 disabled.
    SCI_B_UART_RS485_ENABLE  = 1,      ///< RS-485 enabled.
} sci_b_uart_rs485_enable_t;

/** The polarity of the RS-485 DE signal. */
typedef enum e_sci_b_uart_rs485_de_polarity
{
    SCI_B_UART_RS485_DE_POLARITY_HIGH = 0, ///< The DE signal is high when a write transfer is in progress.
    SCI_B_UART_RS485_DE_POLARITY_LOW  = 1, ///< The DE signal is low when a write transfer is in progress.
} sci_b_uart_rs485_de_polarity_t;

/** Register settings to acheive a desired baud rate and modulation duty. */
typedef struct st_sci_b_baud_setting_t
{
    union
    {
        uint32_t baudrate_bits;

        struct
        {
            uint32_t       : 3;
            uint32_t       : 1;
            uint32_t bgdm  : 1;        ///< Baud Rate Generator Double-Speed Mode Select
            uint32_t abcs  : 1;        ///< Asynchronous Mode Base Clock Select
            uint32_t abcse : 1;        ///< Asynchronous Mode Extended Base Clock Select 1
            uint32_t       : 1;
            uint32_t brr   : 8;        ///< Bit Rate Register setting
            uint32_t brme  : 1;        ///< Bit Rate Modulation Enable
            uint32_t       : 3;
            uint32_t cks   : 2;        ///< CKS  value to get divisor (CKS = N)
            uint32_t       : 2;
            uint32_t mddr  : 8;        ///< Modulation Duty Register setting
        } baudrate_bits_b;
    };
} sci_b_baud_setting_t;

/** Configuration settings for controlling the DE signal for RS-485. */
typedef struct st_sci_b_uart_rs485_setting
{
    sci_b_uart_rs485_enable_t      enable;   ///< Enable the DE signal.
    sci_b_uart_rs485_de_polarity_t polarity; ///< DE signal polarity.
    uint8_t assertion_time : 5;              ///< Time in baseclock units after assertion of the DE signal and before the start of the write transfer.
    uint8_t negation_time  : 5;              ///< Time in baseclock units after the end of a write transfer and before the DE signal is negated.
} sci_b_uart_rs485_setting_t;

/** UART on SCI device Configuration */
typedef struct st_sci_b_uart_extended_cfg
{
    sci_b_clk_src_t                 clock;            ///< The source clock for the baud-rate generator. If internal optionally output baud rate on SCK
    sci_b_uart_start_bit_detect_t   rx_edge_start;    ///< Start reception on falling edge
    sci_b_uart_noise_cancellation_t noise_cancel;     ///< Noise cancellation setting
    sci_b_baud_setting_t          * p_baud_setting;   ///< Register settings for a desired baud rate.
    sci_b_uart_rx_fifo_trigger_t    rx_fifo_trigger;  ///< Receive FIFO trigger level, unused if channel has no FIFO or if DTC is used.
    bsp_io_port_pin_t               flow_control_pin; ///< UART Driver Enable pin
    sci_b_uart_flow_control_t       flow_control;     ///< CTS/RTS function of the SSn pin
    sci_b_uart_rs485_setting_t      rs485_setting;    ///< RS-485 settings.
} sci_b_uart_extended_cfg_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const uart_api_t g_uart_on_sci_b;

/** @endcond */

fsp_err_t R_SCI_B_UART_Open(uart_ctrl_t * const p_api_ctrl, uart_cfg_t const * const p_cfg);
fsp_err_t R_SCI_B_UART_Read(uart_ctrl_t * const p_api_ctrl, uint8_t * const p_dest, uint32_t const bytes);
fsp_err_t R_SCI_B_UART_Write(uart_ctrl_t * const p_api_ctrl, uint8_t const * const p_src, uint32_t const bytes);
fsp_err_t R_SCI_B_UART_BaudSet(uart_ctrl_t * const p_api_ctrl, void const * const p_baud_setting);
fsp_err_t R_SCI_B_UART_InfoGet(uart_ctrl_t * const p_api_ctrl, uart_info_t * const p_info);
fsp_err_t R_SCI_B_UART_Close(uart_ctrl_t * const p_api_ctrl);
fsp_err_t R_SCI_B_UART_Abort(uart_ctrl_t * const p_api_ctrl, uart_dir_t communication_to_abort);
fsp_err_t R_SCI_B_UART_BaudCalculate(uint32_t                     baudrate,
                                     bool                         bitrate_modulation,
                                     uint32_t                     baud_rate_error_x_1000,
                                     sci_b_baud_setting_t * const p_baud_setting);
fsp_err_t R_SCI_B_UART_CallbackSet(uart_ctrl_t * const          p_api_ctrl,
                                   void (                     * p_callback)(uart_callback_args_t *),
                                   void const * const           p_context,
                                   uart_callback_args_t * const p_callback_memory);
fsp_err_t R_SCI_B_UART_ReadStop(uart_ctrl_t * const p_api_ctrl, uint32_t * remaining_bytes);

/*******************************************************************************************************************//**
 * @} (end addtogroup SCI_B_UART)
 **********************************************************************************************************************/

/* Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER

#endif
