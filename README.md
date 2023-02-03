# M031BSP_UART_78M6610_PSD
 M031BSP_UART_78M6610_PSD

update @ 2023/02/03

1. Initial UART1 (PA.2 : RXD , PA.3 : TXD) , to drive meter IC : 78M6610_PSD EVM

2. below is EVM GUI screen capture  

![image](https://github.com/released/M031BSP_UART_78M6610_PSD/blob/main/evm_log.jpg)	

3. to use UART port , need to make sure PIN level at power on  

![image](https://github.com/released/M031BSP_UART_78M6610_PSD/blob/main/uart_config.jpg)	

4. below is LA UART capture , 

AUTO UART report ON: send UART TX : 0xAA , 0x0A , 0xA3 , 0x00 , 0x00 , 0xD3 , 0x01 , 0x00 , 0xAE , 0x27 

AUTO UART report OFF: send UART TX : 0xAA , 0x0A , 0xA3 , 0x00 , 0x00 , 0xD3 , 0x00 , 0x00 , 0xAE , 0x28 

![image](https://github.com/released/M031BSP_UART_78M6610_PSD/blob/main/LA_tx.jpg)	

5. below is log message 

![image](https://github.com/released/M031BSP_UART_78M6610_PSD/blob/main/log1_meter_ic.jpg)	

![image](https://github.com/released/M031BSP_UART_78M6610_PSD/blob/main/log2_meter_ic.jpg)	

