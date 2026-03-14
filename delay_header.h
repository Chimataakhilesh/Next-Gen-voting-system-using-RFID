#include<LPC21XX.H>

void delay_seconds(unsigned int);
void delay_ms(unsigned int);
void delay_us(unsigned int);

void delay_seconds(unsigned int seconds)
{
T0PR = 15000000 - 1;
T0TCR = 0X01;
while(T0TC<seconds);
T0TCR = 0X03;
T0TCR = 0X00;
}
void delay_ms(unsigned int ms)
{
T0PR = 15000 - 1;
T0TCR = 0X01;
while(T0TC<ms);
T0TCR = 0X03;
T0TCR = 0X00;
}
void delay_us(unsigned int us)
{
T0PR = 15 - 1;
T0TCR = 0X01;
while(T0TC<us);
T0TCR = 0X03;
T0TCR = 0X00;
}
