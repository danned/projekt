/*temp_sensor.c - Staffan Piledahl
Driver for LCD Display

Functions to use> void Display_Init(), void Display_Write(string, x,y)

Blocks used: PIOC, PIOD

------- Usage example: -----
Init_Display();

//<---Inside main loop--->
//Writing a custom text at position
Display_Write("",10,15); //Write "hello" at cell x=10, y=15

Display_Write_Header(char warning_status, char *title, char* datetime);
Display_Write_Sidebar();
Display_Write_Temp_screen();

---------------------------
TODO:
- Remove dependencies from this module, for example fetching data. must be done in controller
- Clean display.h and create the defines
- Test module
*/
#include "display.h"
#include "../rtc.h"
#include <math.h>
#include "../includes/at91sam3x8.h"
#include "../includes/common.h"

#define LONG_DELAY 100
#define SHORT_DELAY 20
/* Displays X number of buttons in navigation bar
Takes a pointer to an array containing chars[] (strings)
So we loop thorugh each button,
and loop through each string in every button rendering it onscreen.

Area:
Left side under header all the way down.
TODO do we need params in?
Note: Max 5 chars for every button
 */
//int RTC_Get_Date_String(char* date); //TODO REMOVE
/* ------ PUBLIC functions ------ */


/* Initializes PIO for display */
void Display_Init(void){

  /* TODO INLINE FUNCTION GOES HERE enables clock for PIOC & PIOD */
  *AT91C_PMC_PCER = (3<<13);

  /*enables control of pins*/
  *AT91C_PIOC_PER =  (BIT_2_TO_9);

  /* initializes PIOD pin 2 and 0*/
  *AT91C_PIOD_PER = (1<<2)  | 1;
  *AT91C_PIOD_OER = (1<<2)  | 1;
  *AT91C_PIOD_SODR = (1<<2) | 1;

  /* initializes PIO port 2 3 4 5 as input*/
  *AT91C_PIOC_ODR = BIT_2_3_4_5;

    /* initializes PIO pin 12-17 and 18,19 as output*/
  *AT91C_PIOC_OER = BIT_12_TO_17 | BIT_18_19;

  /* Disable displaycontrol as standard */
  *AT91C_PIOC_SODR = (1<<12);

  /* Set areas etc. */
  Display_Set_Default_State();

}

/*Display has 40 coloumns and 16 rows
void DISPLAY_writeAt(char *text, char coloumn , char row){

    pos = (y-1)*40 + nXPos;

    nXRest = x%nBit;
    nXRest = nBit - nXRest;

  	Display_Write_Data(x);
	Display_Write_Data(y);
	Display_Write_Command(0x24);//Set text coordinates

	 Go thorugh each character and write it with auto increment
	for(int i = 0; *(text + i) != 0x00 ; i++){
		Display_Write_Data( (*(text+i)-0x20) );
		Display_Write_Command(0xC0);
	}
}
*/

/*Writes to a cell, needs rewrite to take correct x and y*/
void Display_Write(char *text, char x , char y){

  Display_Write_Data(x);
  Display_Write_Data(y);
  Display_Write_Command(0x24);//Set text coordinates

  for(int i = 0; *(text + i) != 0x00 ; i++){
    Display_Write_Data( (*(text+i)-0x20) );
    Display_Write_Command(0xC0);
  }

}

/*Home screen that shows current values of everything*/
void Display_Write_Home_Screen(char* temp, char* lux, char* air, char* date){

    Display_Write(date,87,0); //Already includes wrapping string "Date: "

    Display_Write("Temp: ",167,0);
    Display_Write(temp,172,0);
    Display_Write(" Cels",177,0);

    Display_Write("Illu: ",207,0);
    Display_Write(lux,212,0);
    Display_Write(" Lux",218,0);

    Display_Write("Air: ",247,0);
    Display_Write(air,252,0);
    Display_Write(" kPa",0,1);

}
void Display_Write_Light_Screen(void){
  Display_Write("Sun position ",95,0);
  int reading = SERVO_getPos();
  reading = (reading)/44; //Turn into angle
 char *angle = malloc(12*sizeof(char *));
  if(angle == 0){
    //TODO Handle error
  }
sprintf(angle, "%d degrees", reading);

  Display_Write(angle,128,0);
  free(angle);
	//Draw suntracker painting.
	Display_Draw_Arc(145, 135, 70);
	Display_Draw_Sun(80, 110, 10); //TODO make this take the angle from servo
}
void Display_Write_Temp_Screen(char* date){
  Display_Write(date,87,0);
	//Display_Write("Min: ",100,0);
  Display_Draw_Axis();

  Display_Write("Tdy",57,2);
  Display_Write("Ytd",61,2);

  Display_Write("40",167,0);
  Display_Write("20",71,1);
  Display_Write("0",16,2);
  //fetch tinitial data, this weeks
  datestamp_t todays_datestamp = mem.temp->date; //TODO get date from RTC
  temp_t *tmp = mem.temp;
  char count = 0;
  //Get last 7 days worth of data from database
  while(tmp != NULL && count <7 ){
      //Display_Draw_Graph(&temp->, count);
      Display_Draw_Temp_Graph(tmp, count);
      count++;
      tmp = tmp->next;
  }
}
/*Draws the initial set date screen on startup*/
void Display_Write_Date_Screen(void){

  Display_Write("Cent:              ",88,0);
	Display_Write("Year:              ",128,0);
	Display_Write("Month:              ",168,0);
	Display_Write("Date:               ",208,0);
	Display_Write("                    ",248,0);

  unsigned char date_entries_done = 0;
  unsigned char time_entries_done = 0;
  char cent = 0;
  char year = 0;
  char month = 0;
  char date = 0;
  char hr = 0;
  char sec = 0;
  char min = 0;
  Display_Write("_",94+((date_entries_done%2)*40),0);
  unsigned char pressed;
  while(date_entries_done < 8){
	  pressed = Keypad_Read();
    if(pressed != 0){

    	switch(pressed){
        case 1:
          Display_Write("1",94+((date_entries_done%2)*40),0); //write the number at correct place
            date_entries_done++;
		    break;
        case 2:
          Display_Write("2",94+((date_entries_done/2)*40)+(date_entries_done%2),0); //write the number at correct place
            date_entries_done++;
		    break;
        case 3:
          Display_Write("3",94+((date_entries_done/2)*40)+(date_entries_done%2),0); //write the number at correct place
            date_entries_done++;
        break;
        case 4:
			   Display_Write("4",94+((date_entries_done/2)*40)+(date_entries_done%2),0); //write the number at correct place
          date_entries_done++;
        break;
        case 5:
			Display_Write("5",94+((date_entries_done/2)*40)+(date_entries_done%2),0); //write the number at correct place
            date_entries_done++;
        break;
        case 6:
			Display_Write("6",94+((date_entries_done/2)*40)+(date_entries_done%2),0); //write the number at correct place
            date_entries_done++;
        break;
        case 7:
			Display_Write("7",94+((date_entries_done/2)*40)+(date_entries_done%2),0); //write the number at correct place
            date_entries_done++;
        break;
        case 8:
			Display_Write("8",94+((date_entries_done/2)*40)+(date_entries_done%2),0); //write the number at correct place
            date_entries_done++;
        break;
        case 9:
			Display_Write("9",94+((date_entries_done/2)*40)+(date_entries_done%2),0); //write the number at correct place
            date_entries_done++;
        break;
        case 11:
			Display_Write("0",94+((date_entries_done/2)*40)+(date_entries_done%2),0); //write the number at correct place
            date_entries_done++;

        break;
    }

	if(!(pressed == 10 || pressed == 12)){
		if(pressed == 11){pressed = 0;} //Quickfix to make saving easier

		//Find out if its cent year month or date user is entering
		switch((date_entries_done-1)/2){
		  case 0:
			cent = (cent*10)+pressed;
		  break;
		  case 1:
			year = (year*10)+pressed;
		  break;
		  case  2:
			month = (month*10)+pressed;
		  break;
		  case 3:
			date = (date*10)+pressed;
		  break;
		}
	}


    //Press star to move to next item
  while(Keypad_Read() != 10){}
  Display_Write("_",94+((date_entries_done/2)*40)+(date_entries_done%2),0);
	Delay(2000000);
    }
  }

  //Now let user enter the time
  Display_Write("Hr:                ",88,0);
  Display_Write("Min:               ",128,0);
  Display_Write("Sec:              ",168,0);
  Display_Write("                   ",208,0);
  Display_Write("                    ",248,0);
  Display_Write("_",94+((time_entries_done/2)*40)+(time_entries_done%2),0);
  while(time_entries_done < 6){
    pressed = Keypad_Read();
      if(pressed != 0){
        switch(pressed){
          case 1:
            Display_Write("2",94+((time_entries_done/2)*40)+(time_entries_done%2),0); //write the number at correct place
              time_entries_done++;
        break;
          case 2:
            Display_Write("2",94+((time_entries_done/2)*40)+(time_entries_done%2),0); //write the number at correct place
              time_entries_done++;
        break;
          case 3:
            Display_Write("3",94+((time_entries_done/2)*40)+(time_entries_done%2),0); //write the number at correct place
              time_entries_done++;
          break;
          case 4:
        Display_Write("4",94+((time_entries_done/2)*40)+(time_entries_done%2),0); //write the number at correct place
              time_entries_done++;
          break;
          case 5:
        Display_Write("5",94+((time_entries_done/2)*40)+(time_entries_done%2),0); //write the number at correct place
              time_entries_done++;
          break;
          case 6:
        Display_Write("6",94+((time_entries_done/2)*40)+(time_entries_done%2),0); //write the number at correct place
              time_entries_done++;
          break;
          case 7:
        Display_Write("7",94+((time_entries_done/2)*40)+(time_entries_done%2),0); //write the number at correct place
              time_entries_done++;
          break;
          case 8:
        Display_Write("8",94+((time_entries_done/2)*40)+(time_entries_done%2),0); //write the number at correct place
              time_entries_done++;
          break;
          case 9:
        Display_Write("9",94+((time_entries_done/2)*40)+(time_entries_done%2),0); //write the number at correct place
              time_entries_done++;
          break;
          case 11:
        Display_Write("0",94+((time_entries_done/2)*40)+(time_entries_done%2),0); //write the number at correct place
              time_entries_done++;
          break;
      }

      if(!(pressed == 10 || pressed == 12)){
        if(pressed == 11){pressed = 0;} //Quickfix to make saving easier
        //Find out if its hr min or sec or date user is entering
        switch((time_entries_done-1)/2){
          case 0:
            hr = (hr*10)+pressed;
          break;
          case 1:
            min = (min*10)+pressed;
          break;
          case  2:
            sec = (sec*10)+pressed;
          break;
        }
      }
      //Press star to move to next item
    while(Keypad_Read() != 10){}
    Display_Write("_",94+((time_entries_done/2)*40)+(time_entries_done%2),0);
    Delay(2000000);
    }
  }

  RTC_Init(sec, min,hr, cent, year, month, date, 1); //TODO do day aswell, hardcoded for now

}
void Display_Write_Air_Screen(char* date){
  //int test_fetched_temp = *mem_pr->next->temp;
  Display_Write(date,98,0);

  Display_Draw_Axis();
  Display_Write("Hi",167,0);
  Display_Write("--",71,1);
  Display_Write("Lo",15,2);

  //fetch the last 7 days air pressure
  Display_Draw_Air_Graph();
}
void Display_Write_Settings_Screen(void){
  Display_Write("N= ",88,0);
  char *N = malloc(5*sizeof(char *));
  if(N == 0){
    //TODO Handle error
  }
  sprintf(N, "%d", sta.n_avg);
  Display_Write(N,90,0);
  free(N);
  Display_Write("Fast: ",168,0);
  if(sta.mode > 0){
    Display_Write("ENABLED",174,0);
  }else{
    Display_Write("DISABLED",174,0);
  }

  Display_Write("Alarm L:  ",248,0);
  char *alm_l = malloc(5*sizeof(char *));
  if(alm_l == 0){
    //TODO Handle error
  }
  sprintf(alm_l, "%d", sta.alm_l);
  Display_Write(alm_l,1,1);
  free(alm_l);  
  //Settings for temp alarm upper and lower limits
  Display_Write("Alarm H:  ",32,1);
  char *alm_h = malloc(5*sizeof(char *));
  if(alm_h == 0){
    //TODO Handle error
  }
  sprintf(alm_h, "%d", sta.alm_h);
  Display_Write(alm_h,41,1);
  free(alm_h);

}
void Display_Write_Testing_Screen(char temp_pass,char air_pass,char light_pass,char mem_pass){

  Display_Write("Temp module: ",81,0);
	if(temp_pass ==1){
		Display_Write("PASSED!",96,0); //TODO Implement
	}else{
	  Display_Write("FAILED!",96,0);
	}

	Display_Write("Air module: ",121,0);
	if(air_pass ==1){
    	Display_Write("PASSED!",136,0); //TODO Implement
	}else{
    	Display_Write("FAILED!",136,0); //TODO Implement
	}

	Display_Write("Light module: ",161,0);
	if(light_pass ==1){
    	Display_Write("PASSED!",176,0); //TODO Implement
	}else{
    	Display_Write("FAILED!",176,0); //TODO Implement
	}

	Display_Write("Mem check: ",201,0);
	if(mem_pass == 1){
    	Display_Write("PASSED!",216,0); //TODO Implement
	}else{
    	Display_Write("FAILED!",216,0); //TODO Implement
	}
}
/*Draws the bar graphs for one week three bars for every day min avg max*/
void Display_Draw_Air_Graph(){

  int min;
  int avg;
  int max;
for (int j = 0; j < 7; j++){
	  min = mem.pres.min[j];
	  avg = mem.pres.avg[j];
	  max = mem.pres.max[j];

	  //Normalize
	  if(min>120000){
		 min = 120000;
	  }
	   if(avg>120000){
		 avg = 120000;
	  }
	   if(max>120000){
		 max = 120000;
	  }
	  if(min<90000){
		 min = 90000;
	  }
	   if(avg<90000){
		 avg = 90000;
	  }
	   if(max<90000){
		 max = 90000;
	  }

	  //Draw min bar, origin is at (62,100)
	  int start_pos = 61+(j*11)+0;
	  for(int i =0;i< ((min/1000)-90)*3;i++ ){
		Display_Draw_Pixel(start_pos,110-i);
		Display_Draw_Pixel(start_pos+1,110-i);
		Display_Draw_Pixel(start_pos+2,110-i);
	  }
	  //Draw avg bar
	  start_pos = 61+(j*11)+3;
	  for(int i =0;i<((avg/1000)-90)*3;i++ ){
		Display_Draw_Pixel(start_pos,110-i);
		Display_Draw_Pixel(start_pos+1,110-i);
		Display_Draw_Pixel(start_pos+2,110-i);
	  }
	  //Draw max bar
	  start_pos = 61+(j*11)+7;
	  /*Draw vertical line*/
	  for(int i =0;i<((max/1000-90))*3;i++ ){
		Display_Draw_Pixel(start_pos,110-i);
		Display_Draw_Pixel(start_pos+1,110-i);
		Display_Draw_Pixel(start_pos+2,110-i);
	  }
	}
}
/*Draws the bar graphs for one week three bars for every day min avg max*/
void Display_Draw_Temp_Graph(temp_t* temp, char count){
 //TODO assign from temperature struct
 signed char min = temp->min;
 signed char avg = temp->avg;
 signed char max = temp->max;
//Normalize values to spec 0-40 degrees celsius
  if(min>40){
     min = 40;
  }
   if(avg>40){
     avg = 40;
  }
   if(max>40){
     max = 40;
  }
  if(min<-10){
     min = 0;
  }
   if(avg<-10){
     avg = 0;
  }
   if(max<-10){
     max = 0;
  }
  //For testing graph drawing
  //min = 10;
  //avg = 20;
  //max = 30;

  //Draw min bar, origin is at (62,100)
  int start_pos = 61+(count*11)+0;
  for(int i =0;i< min*2;i++ ){
    Display_Draw_Pixel(start_pos,110-i);
    Display_Draw_Pixel(start_pos+1,110-i);
    Display_Draw_Pixel(start_pos+2,110-i);
  }
  //Draw avg bar
  start_pos = 61+(count*11)+3;
  for(int i =0;i<avg*2;i++ ){
    Display_Draw_Pixel(start_pos,110-i);
    Display_Draw_Pixel(start_pos+1,110-i);
    Display_Draw_Pixel(start_pos+2,110-i);
  }
  //Draw max bar
  start_pos = 61+(count*11)+7;
  /*Draw vertical line*/
  for(int i =0;i<max*2;i++ ){
    Display_Draw_Pixel(start_pos,110-i);
    Display_Draw_Pixel(start_pos+1,110-i);
    Display_Draw_Pixel(start_pos+2,110-i);
  }

}
/* ------ PRIVATE helpers ------ */
//Takes X amount of input and graphs it in the content area.


/*Writes out standard sidebar nav and then state dependent part*/
void Display_Write_Sidebar(){
  if(sta.state == 0){ //Startup sidebar
    Display_Write("* Next",24+40,1);
    Display_Write("1",40*2,0);
    Display_Write("2",40*3,0);
    Display_Write("3",40*4,0);
    Display_Write("4",40*5,0);
    Display_Write("Etc.",40*6,0);
  }else{
    //Regular sidebar part
    Display_Write("1 Home",40*2,0);
    Display_Write("2 Sun",40*3,0);
    Display_Write("3 Temp",40*4,0);
    Display_Write("4 Air",40*5,0);
    Display_Write("5 Conf",40*6,0);
  }


  //state dependent sidebar part (Offset is y=1 x = 24)
  switch(sta.state){

    case 1: //Home screen

    break;

    case 2: //Light follower
    //TODO write start and stop buttons
      Display_Write("* Strt",24+40,1);
      Display_Write("# Stop",24+40*2,1);
    break;

    case 3: //Temperature history
      Display_Write("* -Wk",24+40,1);
      Display_Write("# +Wk",24+40*2,1);
    break;

    case 4: //Air pressure history
      //Display_Write("* -Day",24+40,1);
      //Display_Write("# +Day",24+40*2,1);
    break;

    case 5: //Conf Settings screen
      //Display_Write("* =10",24+40,1);
      Display_Write("9 SetA",24+40*0,1);
      Display_Write("* Load",24+40*1,1);
      Display_Write("0 SetN",24+40*2,1);
      Display_Write("# Fast",24+40*3,1);
    break;
  }

  /*Draw vertical line*/
  for(int i =0;i<160;i++ ){
  Display_Draw_Pixel(40,i);
  }

  //TODO Check global state for contextual buttons
}
/* Displays the header bar containing warnings, title, clock*/
void Display_Write_Header(char warning_status, char* title, char* time){
  //Warning status dependent header
  switch(warning_status){
    case 1: //Temperature overload
      Display_Write("T!",0,0);
    break;
    case 2: //Memory full - overwriting
      Display_Write("M!",0,0); //nr 14 will be a space
    break;
  case 3:
    Display_Write("T!M!",0,0);
    break;

  }
  //Write title and clock
  Display_Write(title,8,0); // 10 chars nr  will be space
  Display_Write(time,24,0); //5 chars

  /*Draw horizontal line*/
  Display_Write_Data(0x40);
  Display_Write_Data(0x41);
  Display_Write_Command(0x24);//Set coordinates
  for(int i =0;i<40;i++ ){
    Display_Write_Data(0xFF);
    Display_Write_Command(0xC0);
  }
}
/*
    sets display to default state
*/
void Display_Set_Default_State(void){
  *AT91C_PIOD_CODR = 1;         //Clear Reset display
  Delay(LONG_DELAY);            //LONG DELAY <------------------
  *AT91C_PIOD_SODR = 1;         //Set Reset display
  //*AT91C_PIOC_CODR = 5<<14;
  Display_Write_Data(0x00);
  Display_Write_Data(0x00);
  Display_Write_Command(0x40);//Set text home address
  Display_Write_Data(0x00);
  Display_Write_Data(0x40); //(0x40 standard)
  Display_Write_Command(0x42); //Set graphic home address
  Display_Write_Data(0x28); //standard was 1e
  Display_Write_Data(0x00);
  Display_Write_Command(0x41); // Set text area
  Display_Write_Data(0x28);
  Display_Write_Data(0x00);
  Display_Write_Command(0x43); // Set graphic area (same as text)
  Display_Write_Command(0x80); // text mode OR (0x80)
  Display_Write_Command(0x9C); // Enable both text and graphic 0x9c     Text on graphic off(0x94)
  //set cusor to upper left
  Display_Write_Data(0x00);
  Display_Write_Data(0x00);
  Display_Write_Command(0x24);
  //clear screen
  Display_Clear_Text();
  Display_Clear_Graphics();
  //set cusor to upper left
  Display_Write_Data(0x00);
  Display_Write_Data(0x00);
  Display_Write_Command(0x24);

  //clear pin 45 and 44 for font select and fr
  *AT91C_PIOC_CODR = BIT_18_19;
}

/* Read display status*/
unsigned char Display_Read_Status(void){
  unsigned char temp;

   *AT91C_PIOC_ODR =  BIT_2_TO_9; //set databus as input
   *AT91C_PIOC_SODR =  BIT_13 | BIT_12;    //Set direction as input  and activate chip
   //*AT91C_PIOC_CODR =  BIT_12;

   /* clear read */
   *AT91C_PIOC_SODR = (1<<17);     // set data
   *AT91C_PIOC_CODR  =3<<15;       // clear CS and rd
   //Delay
    Delay(SHORT_DELAY);
   //Read and save
    temp = (*AT91C_PIOC_PDSR);
    temp &= (3<<1);
    *AT91C_PIOC_SODR  =3<<15;        // set CS and rd
  //deactivate
   *AT91C_PIOC_SODR =  BIT_12;
   //Set Direction output
   *AT91C_PIOC_CODR =  BIT_13;
   return (temp>>1);
}

/* sends data to display */
void Display_Write_Data(unsigned char Data){
  while(Display_Read_Status() != 3){
    Delay(SHORT_DELAY);
  }
  /* clear pins for output*/
  *AT91C_PIOC_CODR = BIT_2_TO_9;
  /* set databus to Data */
  *AT91C_PIOC_SODR = Data<<2;

  /* set direction on chip to output */
  *AT91C_PIOC_CODR =  BIT_13 | BIT_12;
  //*AT91C_PIOC_CODR =  BIT_12;    // enabling 74 chip

  *AT91C_PIOC_OER =  BIT_2_TO_9; // enable output on pins

  *AT91C_PIOC_CODR = 0xD<<14;
 /* *AT91C_PIOC_CODR = (1<<17);     // clear data
  *AT91C_PIOC_CODR  = 1<<16;        // clear CS
  *AT91C_PIOC_CODR = 1<<14;       // and WR*/
  Delay(SHORT_DELAY);             //SHORT DELAY
 // printf("datainput: %d", Data);
  *AT91C_PIOC_SODR = 0x5<<14;      // set CS  and wr

  *AT91C_PIOC_SODR =  BIT_12;     // disable 74 chip

  *AT91C_PIOC_ODR =  BIT_2_TO_9;  // enable input on pins
}

/* sends command to display */
void Display_Write_Command(unsigned char Command){
  while(Display_Read_Status() != 3){
    Delay(SHORT_DELAY);
  }
  /* clear pins for output */
  *AT91C_PIOC_CODR = BIT_2_TO_9;
  /* set databus to command */
  *AT91C_PIOC_SODR = Command<<2;

  /* set direction on chip to output */
  *AT91C_PIOC_CODR =  BIT_13 | BIT_12; // set direction to output and enabling 74 chip
  //*AT91C_PIOC_CODR =  BIT_12;

  *AT91C_PIOC_OER =  BIT_2_TO_9; // enable output on pins

  *AT91C_PIOC_SODR = (1<<17);     // set data
  *AT91C_PIOC_CODR = 0x5<<14;     //clear CS and WR
  Delay(SHORT_DELAY);             //SHORT DELAY
 // printf("Command: %d\n", Command);
  *AT91C_PIOC_SODR = 0x5<<14;     //set CS and WR

  *AT91C_PIOC_SODR =  BIT_12;    // disable 74 chip

  *AT91C_PIOC_ODR =  BIT_2_TO_9; // enable input on pins

}

/*Overwrites standard graphic mem with 0
NOTE: Expensive, see loop
*/
void Display_Clear_Graphics(){
  //Clear graphics
  Display_Write_Data(0x00);
  Display_Write_Data(0x40);
  Display_Write_Command(0x24);//Set coordinates

  for(int i =0;i<6840;i++ ){
    Display_Write_Data(0x00);
    Display_Write_Command(0xC0);
  }

}

/* Clears display */
void Display_Clear_Text(){
  //clear text
  //Display_Write_Data(text_home_adress);
   Display_Write_Data(0x00);
  Display_Write_Data(0x00);
  Display_Write_Command(0x24);//Set coordinates

  for(int i =0;i<=640;i++ ){
    Display_Write_Data(0x00);
    Display_Write_Command(0xC0);
  }

}

//Fill one pixel
void Display_Draw_Pixel(int x, int y){
    int nXRest;   // rest vid modulo
    int nXPos;
    int nBit = 6;
    int move1;
    int move2;

    //find position in x
    nXPos = x/nBit;
    nXPos = (y-1)*40 + nXPos;

    nXRest = x%nBit;
    nXRest = nBit - nXRest;

    if(nXRest == nBit){
      nXPos--;
      nXRest = 0;
    }

    move1 = nXPos & 0xFF;
    move2 = (nXPos & 0xFF00)>>8;

        Display_Write_Data(move1);
        Display_Write_Data(0x40 + move2); //0x40 is our graphic mem start adress
        Display_Write_Command(0x24); //move cursor

        Display_Write_Command(0xF8 + nXRest);

}

/*Draw a smal blob*/
//TODO Take illu factor and create rays of  appr length
void Display_Draw_Sun(int xw, int yw, int rw){
    int h;
    int x;
    int y;
	//Draw the circle
    for (x = -rw; x < rw ; x++){
        h = (int)sqrt(rw * rw - x * x);
        for (y = -h; y < h; y++){
            Display_Draw_Pixel(x + xw, y + yw);
        }
    }
	//Draw the lines
	    for (y = -h; y < h; y++){
            Display_Draw_Pixel(x + xw, y + yw);
        }
}
/*Draw the arc the sun should follow*/
void Display_Draw_Arc(int xw, int yw, int rw){
  int f = 1 - rw;
  int ddF_x = 1;
  int ddF_y = -2 * rw;
  int x = 0;
  int y = rw;

  Display_Draw_Pixel(xw, yw+rw);
  Display_Draw_Pixel(xw, yw-rw);
  Display_Draw_Pixel(xw+rw, yw);
  Display_Draw_Pixel(xw-rw, yw);

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    Display_Draw_Pixel(xw + x, yw + y);
    Display_Draw_Pixel(xw - x, yw + y);
    Display_Draw_Pixel(xw + x, yw - y);
    Display_Draw_Pixel(xw - x, yw - y);

    Display_Draw_Pixel(xw + y, yw + x);
    Display_Draw_Pixel(xw - y, yw + x);
    Display_Draw_Pixel(xw + y, yw - x);
    Display_Draw_Pixel(xw - y, yw - x);
	}

}
/*Draws the axis for showing graph*/
void Display_Draw_Axis(){
  for (int i = 0; i < 130; i++)
  {
    Display_Draw_Pixel(60+i,110);
	if(i<80){
	    Display_Draw_Pixel(60,110-i);
	}

  }

}
