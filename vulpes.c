/***********************************************************\
* Status Output Program for Alopex:                         *
* Originally based on Scroller.c by Jesse McClure (2012)    *
* License: CCPL Sam Stuewe, 2013 (CC-BY-SA)                 *
* Attribution: Must mention Jesse McClure as the originator *
\***********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

/* input files */
static const char *WIFI_FILE  = "/proc/net/wireless";
static const char *WIRD_FILE  = "/sys/class/net/enp2s0f0/operstate";
static const char *CPU_FILE	  = "/proc/stat";
static const char *AUD_FILE	  = "/proc/asound/card0/codec#0";
static const char *BATT_NOW	  = "/sys/class/power_supply/BAT0/charge_now";
static const char *BATT_FULL  = "/sys/class/power_supply/BAT0/charge_full";
static const char *BATT_STAT  = "/sys/class/power_supply/BAT0/status";

/* statis colors				  R G B */
static const long int CONORM  = 0xC6C6C6;
static const long int CO_LOW  = 0xB7C9D6;
static const long int COMID1  = 0x94AEC2;
static const long int COMID2  = 0x7093AD;
static const long int COHIGH  = 0x4C7899;
static const long int COWARN  = 0xff8880;

/* icons */
enum {
   wired_dcn, wired_cn, wifi_full, wifi_high, wifi_mid, wifi_low,
   cpu, volume_high, volume_mid, volume_low, mute,
   batt_ful, batt_875, batt_750, batt_625, batt_500, batt_375, batt_250, batt_125, batt_000,
   batt_fulc, batt_875c, batt_750c, batt_625c, batt_500c, batt_375c, batt_250c, batt_125c, batt_000c
};

/* variables */
static long	   j1,j2,j3,j4,j5,j6,j7,j8,ln1,ln2,ln3,ln4,ln5,ln6,ln7,ln8;
static int	   n, loops = 0;
static char	   c, stat, clk[8], *aud_file;
static FILE	   *in;
static time_t  current;

int main(int argc, const char **argv) {
   in = fopen(CPU_FILE,"r");
   fscanf(in,"cpu %ld %ld %ld %ld %ld %ld %ld %ld",&j1,&j2,&j3,&j4,&j5,&j6,&j7,&j8);
   fclose(in);
   
   /* main loop */
   for (;;) {
	   
	   /* Wired Iface Monitor */
	   if ( (in=fopen(WIRD_FILE,"r")) ) {
		   fscanf(in,"%s",&stat);
		   fclose(in);
		   if (stat=='u') printf("{#%06X}{i %d}",COHIGH,wired_cn);
		   else printf("{#%06X}{i %d}",CONORM,wired_dcn);
		   
		   printf("{#%06X} | ",CONORM);
	   }
	   
	   /* Wireless Iface Monitor */
	   if ( (in=fopen(WIFI_FILE,"r")) ) {
		   fscanf(in,"%*[^\n]\n%*[^\n]\n wlp3s0: %*d %d.",&n);
		   fclose(in);
		   if (n > 63) printf("{#%06X}{i %d}",COHIGH,wifi_full);
		   else if (n > 50) printf("{#%06X}{i %d}",COMID2,wifi_high);
		   else if (n > 38) printf("{#%06X}{i %d}",COMID1,wifi_mid);
		   else if (n < 38) printf("{#%06X}{i %d}",CO_LOW,wifi_low);
		   else printf("{#%06X}W",CONORM);
		   
		   printf("{#%06X} | ",CONORM);
	   }
	   
	   /* CPU Monitor */
	   if ( (in=fopen(CPU_FILE,"r")) ) {
		   fscanf(in,"cpu %ld %ld %ld %ld %ld %ld %ld %ld",&ln1,&ln2,&ln3,&ln4,&ln5,&ln6,&ln7,&ln8);
		   fclose(in);
		   if (ln8>j8) n=(int)100*(ln1-j1+ln2-j2+ln3-j3+ln4-j4+ln5-j5+ln6-j6+ln7-j7)/(ln1-j1+ln2-j2+ln3-j3+ln4-j4+ln5-j5+ln6-j6+ln7-j7+ln8-j8);
		   else n=0;
		   j1=ln1; j2=ln2; j3=ln3; j4=ln4; j5=ln5; j6=ln6; j7=ln7; j8=ln8;
		   if (n > 85) printf("{#%6X}{i %d}",COWARN,cpu);
		   else printf("{#%06X}{i %d}",CONORM,cpu);
		   
		   printf("{#%06X} | ",CONORM);
	   }
	   
	   /* Volume Monitor */
	   if ( (in=fopen(AUD_FILE,"r")) ) {
		   while ( fscanf(in," Amp-Out caps: ofs=0x%x",&ln1) !=1 )
			   fscanf(in,"%*[^\n]\n");
		   while ( fscanf(in, "Amp-Out vals: [0x%x",&ln2) !=1 )
			   fscanf(in,"%*[^\n]\n");
		   while ( fscanf(in, "Node 0x14 [%c",&c) !=1 )
			   fscanf(in,"%*[^\n]\n");
		   while ( fscanf(in, "Amp-Out vals: [0x%x",&ln3) !=1 )
			   fscanf(in,"%*[^\n]\n");
		   fclose(in);
		   
		   if (ln3 != 0) printf("{#%06X}{i %d}",CONORM,mute);
		   else {
			   n = 100*ln2/ln1;
			   if (n > 90) printf("{#%06X}{i %d}",COWARN,volume_high);
			   else if (n > 85) printf("{#%06X}{i %d}",CO_LOW,volume_high);
			   else if (n > 65) printf("{#%06X}{i %d}",COMID1,volume_mid);
			   else if (n > 35) printf("{#%06X}{i %d}",COMID2,volume_mid);
			   else if (n < 35) printf("{#%06X}{i %d}",COHIGH,volume_low);
		   }
		   printf("{#%06X} | ",CONORM);
	   }
	   
	   /* Battery Monitor */
	   if ( (in=fopen(BATT_NOW,"r")) ) {
		   fscanf(in,"%ld\n",&ln1); fclose(in);
		   if ( (in=fopen(BATT_FULL,"r")) ) { fscanf(in,"%ld\n",&ln2); fclose(in); }
		   if ( (in=fopen(BATT_STAT,"r")) ) { fscanf(in,"%c",&c); fclose(in); }
		   n = (ln1 ? ln1 * 100 / ln2 : 0);
		   if (c=='C') {
			   if (n > 90) printf("{#%06X}{i %d}",COHIGH,batt_fulc);
			   else if (n >= 87.5) printf("{#%06X}{i %d}",COHIGH,batt_875c);
			   else if (n >= 75) printf("{#%06X}{i %d}",COMID2,batt_750c);
			   else if (n >= 62.5) printf("{#%06X}{i %d}",COMID2,batt_625c);
			   else if (n >= 50) printf("{#%06X}{i %d}",COMID1,batt_500c);
			   else if (n >= 37.5) printf("{#%06X}{i %d}",COMID1,batt_375c);
			   else if (n >= 25) printf("{#%06X}{i %d}",CO_LOW,batt_250c);
			   else if (n >= 12.5) printf("{#%06X}{i %d}",CO_LOW,batt_125c);
			   else printf("{#%06X}{i %d}",COWARN,batt_000c);
		   }
		   else {
			   if (n > 90) printf("{#%06X}{i %d}",COHIGH,batt_ful);
			   else if (n >= 87.5) printf("{#%06X}{i %d}",COHIGH,batt_875);
			   else if (n >= 75) printf("{#%06X}{i %d}",COMID2,batt_750);
			   else if (n >= 62.5) printf("{#%06X}{i %d}",COMID2,batt_625);
			   else if (n >= 50) printf("{#%06X}{i %d}",COMID1,batt_500);
			   else if (n >= 37.5) printf("{#%06X}{i %d}",COMID1,batt_375);
			   else if (n >= 25) printf("{#%06X}{i %d}",CO_LOW,batt_250);
			   else if (n >= 12.5) printf("{#%06X}{i %d}",CO_LOW,batt_125);
			   else printf("{#%06X}{i %d}",COWARN,batt_000c);
		   }
		   printf("{#%06X} | ",CONORM);
	   }
	   
	   /* Clock */
	   if ((loops % 40) == 0) {
		   time(&current);
		   strftime(clk,6,"%H.%M | %A, %d %B %Y",localtime(&current));
	   }
	   printf("{#%06X}%s \n",CONORM,clk);
	   fflush(stdout);
	   sleep(1);
   }
   return 0;
}
