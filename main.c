#include<LPC21XX.H>
#include<string.h>
#include "lcd_header.h"
#define VERIFY_MODE 0 
#define VOTE_MODE   1
#define CONFIRM_MODE 2
#define RESULT_MODE 3

char Voter_ID[5][14] = {"123456780","123456781","123456782","123456783","123456784"};
int voted[5] = {0,0,0,0,0};
int check_voter(char *);
int read_rfid(char *);
void EINT_config(void);
void UART1_CONFIG(void);
int UART1_RX(void);
void LCD_STRING(unsigned char *);

volatile char system_state = VERIFY_MODE;
volatile char selected_party = 0;

char verified_id[15];
char user[15];

int current_voter_index = -1;

static int DMK = 0;
static int ADMK = 0;
static int TVK = 0;

static char verify_shown = 0;
static char vote_shown = 0;

void EINT0_isr(void) __irq
{
    EXTINT = 0x01;     
    system_state = RESULT_MODE;      // ISR for Selecting RESULT MODE
    VICVectAddr = 0;
}
void EINT1_isr(void) __irq
{
    EXTINT = 0x02;

    if(system_state == VOTE_MODE)
    {
        selected_party = 1;
        vote_shown = 0;
        system_state = CONFIRM_MODE;    // ISR for Selecting 1st Party
    }

    VICVectAddr = 0;
}
void EINT2_isr(void) __irq
{
    EXTINT = 0x04;

    if(system_state == VOTE_MODE)
    {
        selected_party = 2;
        vote_shown = 0;
        system_state = CONFIRM_MODE;    // ISR for Selecting 2nd Party
    }

    VICVectAddr = 0;
}
void EINT3_isr(void) __irq
{
    EXTINT = 0x08;

    if(system_state == VOTE_MODE)
    {
        selected_party = 3;
        vote_shown = 0;
        system_state = CONFIRM_MODE;    // ISR for Selecting 3rd Party
    }

    VICVectAddr = 0;
}
int main()
{
	int check,i;
	LCD_INIT();
	UART1_CONFIG();
	EINT_config();
	
	LCD_COMMAND(0x80);
	LCD_STRING("NEXT GEN-VOTING");
	LCD_COMMAND(0xC0);
	LCD_STRING("ARM BASED RFID AUTHENTICATION SYSTEM");
	//delay_seconds(1);
	for(i=0;i<37;i++){
	LCD_COMMAND(0X18);
	delay_ms(80);
	} 
	
	
while(1)
{
    if(system_state == VERIFY_MODE)         //Candidate in VERIFY MODE
    {
        if(!verify_shown)
				{
					LCD_COMMAND(0x01);
					LCD_STRING("SCAN FOR VERIFY");
					verify_shown = 1;
				}
				
				if(read_rfid(user))          // Reading Candidate ID
				{
					verify_shown = 0;
					check = check_voter(user);        // Checking ID is Valid or Not
				
					if(check == 1)           // Valid Case
					{
						LCD_COMMAND(0x01);
						LCD_STRING("VERIFIED");
						delay_ms(500);
						LCD_COMMAND(0x01);
						strcpy(verified_id, user);
						system_state = VOTE_MODE;
					}
					else if(check == 2)       //Already Voted
					{
						LCD_COMMAND(0x01);
						LCD_STRING("ALREADY VOTED");
						delay_ms(500);
					}
					else                // Not Valid
					{
						LCD_COMMAND(0x01);
						LCD_STRING("INVALID ID");
						delay_ms(500);
						verify_shown = 0;
					}
			  }
    }

    if(system_state == VOTE_MODE)            // Candidate in Voting Mode
    {
         if(!vote_shown)
				 {
						LCD_COMMAND(0x01);
						LCD_STRING("1.DMK 2.ADMK");         // Displaying the Parties Names
						LCD_COMMAND(0xC0);
						LCD_STRING("3.TVK 4.RESULTS");
						vote_shown = 1;
					}
    }
		else
		{
			vote_shown = 0;
		}

    if(system_state == CONFIRM_MODE)             // Candidate in Confirm Mode After Selecting Party
    {
        LCD_COMMAND(0x01);
        LCD_STRING("SCAN AGAIN");
			
        if(!read_rfid(user))
					continue;

        if(strcmp(user, verified_id) == 0)
        {
            if(selected_party == 1) DMK++;
            if(selected_party == 2) ADMK++;         // Selected party is counted
            if(selected_party == 3) TVK++;
					 
					  voted[current_voter_index] = 1;         // Updating the Candidate ID after Voting

            LCD_COMMAND(0x01);
            LCD_STRING("VOTE SUCCESS");
        }
        else
        {
            LCD_COMMAND(0x01);
            LCD_STRING("MISMATCH");
        }

        delay_ms(500);

        // Reset System
        selected_party = 0;
        vote_shown = 0;
				verify_shown = 0;
				system_state = VERIFY_MODE;    
    }
		if(system_state == RESULT_MODE)       // Result Mode
		{

			LCD_COMMAND(0x01);
      LCD_STRING("RESULTS");
      LCD_COMMAND(0xC0);

        if((DMK == ADMK) && (DMK == TVK))           // Winning Party is Displayed
            LCD_STRING("DRAW");
        else if((DMK > ADMK) && (DMK > TVK))
            LCD_STRING("DMK WINS");
        else if((ADMK > DMK) && (ADMK > TVK))
            LCD_STRING("ADMK WINS");
        else
            LCD_STRING("TVK WINS");
			while(1);
		}
}
}
int read_rfid(char *id)          //RFID reader
{
    char i = 0;
    int data;

    while(i < 13)
    {
        if(system_state == RESULT_MODE)
            return 0;  

        data = UART1_RX();         // ID is collected from UART1

        if(data != -1)
        {
            id[i++] = data;  
        }
    }

    id[i] = '\0';
    return 1;
}
int check_voter(char *id)          // Checking Candidate ID
{
    int i;

    for(i=0; i<5; i++)
    {
        if(strcmp(id, Voter_ID[i]) == 0)
        {
					 current_voter_index = i; 
			
            if(voted[i] == 1)
                return 2;  
            else
                return 1;  
        }
    }
    return 0;
}
void LCD_STRING(unsigned char *s)      // Displaying the String on LCD
{
	while(*s)
	{
		LCD_DATA(*s++);
	}
}
void UART1_CONFIG(void)    // Configuration of UART1
{
	PINSEL0 |= 0X50000;     // P0.8 & P0.9 as TXD1 & RXD1
	U1LCR = 0X83; 
	U1DLL = 97;           // Baud rate as 9600
	U1DLM = 0;
	U1LCR = 0X03;
}
int UART1_RX(void)         // RX for UART1
{
	if(U1LSR & 1)
        return U1RBR;
    else
        return -1;
}
void EINT_config(void)          // EXTERNAL INTERRUPT Configuration
{
	PINSEL0 |= 0x5C0CC;           // P0.1 - EINT0, P0.3 - EINT1, P0.7 - EINT2
  PINSEL1 = 0x300;             // P0.20 - EINT3
  VICIntSelect = 0;
	VICVectCntl0 = (0x20)|15;
  VICVectAddr0 = (unsigned long) EINT1_isr;
	VICVectCntl1 = (0x20)|16;
  VICVectAddr1 = (unsigned long) EINT2_isr;
	VICVectCntl2 = (0x20)|17;
  VICVectAddr2 = (unsigned long) EINT3_isr;
	VICVectCntl3 = (0x20)|14;
	VICVectAddr3 = (unsigned long) EINT0_isr;
	
	EXTMODE = 0xF;            // EDGE MODE 
	EXTPOLAR = 0x00;         // Falling Edge
	VICIntEnable = 1<<14|1<<15|1<<16|1<<17;         // Enable Intrrupts
}
