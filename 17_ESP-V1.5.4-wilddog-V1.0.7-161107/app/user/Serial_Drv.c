
#include "Serial_Drv.h"


// test :AT+TEST=1,"abc"<,3>
void ICACHE_FLASH_ATTR
at_setupCmdTest(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    uint8 buffer[32] = {0};
    pPara++; // skip '='

    //get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    if (flag == FALSE) {
        at_response_error();
        return;
    }

    if (*pPara++ != ',') { // skip ','
        at_response_error();
        return;
    }

    os_sprintf(buffer, "the first parameter:%d\r\n", result);
    at_port_print(buffer);

    //get the second parameter
    // string
    at_data_str_copy(buffer, &pPara, 10);
    at_port_print("the second parameter:");
    at_port_print(buffer);
    at_port_print("\r\n");

    if (*pPara == ',') {
        pPara++; // skip ','
        result = 0;
        //there is the third parameter
        // digit
        flag = at_get_next_int_dec(&pPara, &result, &err);
        // we donot care of flag
        os_sprintf(buffer, "the third parameter:%d\r\n", result);
        at_port_print(buffer);
    }

    if (*pPara != '\r') {
        at_response_error();
        return;
    }

    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_testCmdTest(uint8_t id)
{
    uint8 buffer[32] = {0};

    os_sprintf(buffer, "%s\r\n", "at_testCmdTest");
    at_port_print(buffer);
    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_queryCmdTest(uint8_t id)
{
    uint8 buffer[32] = {0};

    os_sprintf(buffer, "%s\r\n", "at_queryCmdTest");
    at_port_print(buffer);
    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_exeCmdTest(uint8_t id)
{
    uint8 buffer[32] = {0};

    os_sprintf(buffer, "%s\r\n", "at_exeCmdTest");
    at_port_print(buffer);
    at_response_ok();
}

//extern void at_exeCmdCiupdate(uint8_t id);
at_funcationType at_custom_cmd[] = {
    {"+TEST", 5, at_testCmdTest, at_queryCmdTest, at_setupCmdTest, at_exeCmdTest},

#if 0
#ifdef AT_UPGRADE_SUPPORT
    {"+CIUPDATE", 9,       NULL,            NULL,            NULL, at_exeCmdCiupdate}
#endif
#endif

};

void FLASH_SAVE SerialDrv_SetRxCompleteBool(E_BOOL isComplete)
{
   isComplete = isComplete;
}

void FLASH_SAVE SerialDrv_Init(ATCMD_RxProcessFunc func)
{
    char buf[64] = {0};

	//UART_SetBaudrate(0, 115200);     // to device
	//UART_SetBaudrate(1, 115200);  // to print debug info
	//UART_SetPrintPort(1);  // uart 1 as print debug info port
    /***********************AT init ************************************/
    at_customLinkMax = 5;
    at_init();
    os_sprintf(buf,"compile time:%s %s",__DATE__,__TIME__);
    APP_DEBUG_PRINT("SDK version:%s\n", system_get_sdk_version());
    
    at_set_custom_info(buf);
    at_port_print("\r\nready\r\n");
    at_cmd_array_regist(&at_custom_cmd[0], sizeof(at_custom_cmd)/sizeof(at_custom_cmd[0]));
    /***********************AT init   end ************************************/

    // 将 UART0 RX 的数据由用户管理, 不由AT 模块处理
	at_register_uart_rx_intr(func);
}

