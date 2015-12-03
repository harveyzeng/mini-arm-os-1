#include <stddef.h>
#include <stdint.h>
#include "reg.h"
#include "threads.h"

/* USART TXE Flag
 * This flag is cleared when data is written to USARTx_DR and
 * set when that data is transferred to the TDR
 */
#define USART_FLAG_TXE	((uint16_t) 0x0080)
#define USART_FLAG_RXNE ((uint16_t) 0x0020)
#define MAX_COMMAND_LENGTH (50)
#define MAX_ARGC (5)

void usart_init(void)
{
	*(RCC_APB2ENR) |= (uint32_t) (0x00000001 | 0x00000004);
	*(RCC_APB1ENR) |= (uint32_t) (0x00020000);

	/* USART2 Configuration, Rx->PA3, Tx->PA2 */
	*(GPIOA_CRL) = 0x00004B00;
	*(GPIOA_CRH) = 0x44444444;
	*(GPIOA_ODR) = 0x00000000;
	*(GPIOA_BSRR) = 0x00000000;
	*(GPIOA_BRR) = 0x00000000;

	*(USART2_CR1) = 0x0000000C;
	*(USART2_CR2) = 0x00000000;
	*(USART2_CR3) = 0x00000000;
	*(USART2_CR1) |= 0x2000;
}

void print_str(const char *str)
{
	while (*str) {
		while (!(*(USART2_SR) & USART_FLAG_TXE));
		*(USART2_DR) = (*str & 0xFF);
		str++;
	}
}

char usart2_rx()
{
	while(1)
	{
		if((*USART2_SR) & (USART_FLAG_RXNE))
			return (*USART2_DR) & 0xff;
	}
}

int strcmp(char *str1, char *str2){
	int index = 0;
	while(str1[index] != '\0'){
		if(str1[index] != str2[index])
			return 0;
		index++;
	}
	if(str2[index+1] != '\0')
		return 0;
	else
		return 1;
}

void int2char(int x, char *str){
	int num;
	int tmp = x;
	for(int i = 0; i < 10; i++){
		num = tmp % 10;
		str[10-i-1] = '0' + num;
		tmp = tmp / 10;
	}
}

int str2int(char *str){
	int num = 0;
	int index = 0;
	if(str[0]=='-')
		index++;
	while(str[index] != '\0'){
		if(str[index]>='0' && str[index]<='9'){
			num = num*10 + (str[index]-'0');
			index++;
		}
		else
			return -999;
	}
	if(str[0]=='-')
		return num*(-1);
	else
		return num;
	
}
extern int fibonacci(int x);
void fibonacci_thread(void *parameter)
{			
			char fib_result_char[10];
			print_str("\n");
			print_str("execute fibonacci thread");
			print_str("\n");
			int fib_result = fibonacci(str2int(parameter));
			int2char(fib_result, fib_result_char);
			print_str(fib_result_char);
			print_str("\n");
			print_str("kill fibonacci thread");
			print_str("\n");
			thread_self_terminal();


}
void ps_thread(void *parameter)
{			
			ps_function();
			thread_self_terminal();
}

/*
int fibonacci(int x){
	if(x<=0) return 0;
	if(x==1 || x==2) return 1;
	return fibonacci(x-1)+fibonacci(x-2);
}
*/
void shell(void *userdata)
{
	char ichar;
	char command_buffer[MAX_COMMAND_LENGTH+1];
	char command[MAX_ARGC][MAX_COMMAND_LENGTH+1];
	int argv_index = 0;
	int char_index = 0;
	int index = 0;

	
	while(1){
		while(char_index<=MAX_COMMAND_LENGTH){
			char_index++;
			command_buffer[char_index] = '\0';
		}
		print_str("jjj@mini-arm-os:~$ ");
		char_index = 0;
		
		//command input
		while(1){
			ichar = usart2_rx();
			if(ichar == 13){//enter
				print_str("\n");
				break;
			}
			else if(ichar == 127){//backspace
				if(char_index!=0){
					print_str("\b");
					print_str(" ");
					print_str("\b");
					char_index--;
					command_buffer[char_index] = '\0';
				}
			}
			else{
				if(char_index<MAX_COMMAND_LENGTH){
					command_buffer[char_index] = ichar;
					print_str(&command_buffer[char_index]);
					char_index++;
				}
			}
		}
		
		//command identification
		if(char_index != 0){
			for(argv_index = 0; argv_index<MAX_ARGC; argv_index++)
				for(char_index = 0; char_index<=MAX_COMMAND_LENGTH; char_index++)
					command[argv_index][char_index] = '\0';
			argv_index = 0;
			char_index = 0;
			for(index = 0; index<MAX_COMMAND_LENGTH; index++){
				if(command_buffer[index] == ' '){
					if(char_index!=0){
						char_index = 0;
						argv_index++;
					}
				}
				else if(command_buffer[index] != '\0'){
					command[argv_index][char_index] = command_buffer[index];
					char_index++;
				}
				else {}
			}
			
			if(char_index==0)
				argv_index--;
			
			if(strcmp(command[0], "fibonacci\0")){
				if(argv_index != 1){
					print_str("Command should be: fibonacci+(int)");
					print_str("\n");
				}
				else if(str2int(command[1])==-999){
					print_str("Command should be: fibonacci+(int)");
					print_str("\n");
				}
				else{
					if (thread_create(fibonacci_thread, (void *) command[1],3,"fibonacci") == -1)
					print_str("fibonacci creation failed\r\n");
				}
			}
			else if(strcmp(command[0], "ps\0")){
				if (thread_create(ps_thread, (void *) command[1],7,"ps") == -1)
					print_str("ps creation failed\r\n");

			}
			
		}
		char_index = 0;
		argv_index = 0;
		index = 0;
	}
}

/* 72MHz */
#define CPU_CLOCK_HZ 72000000

/* 100 ms per tick. */
#define TICK_RATE_HZ 10

int main(void)
{
	const char *str1 = "shell";

	usart_init();

	if (thread_create(shell, (void *) str1,4,"shell") == -1)
		print_str("Shell creation failed\r\n");

	/* SysTick configuration */
	*SYSTICK_LOAD = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
	*SYSTICK_VAL = 0;
	*SYSTICK_CTRL = 0x07;

	thread_start();

	return 0;
}
