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
#include <sys/times.h>

/* input files */
static const char *WIFI_FILE  = "/proc/net/wireless";
static const char *WIRD_FILE  = "/sys/class/net/enp2s0f0/operstate";
static const char *CPU_FILE	  = "/proc/stat";
static const char *CPU_TEMP	  = "/sys/bus/platform/devices/coretemp.0/temp1_input";
static const char *AUD_FILE	  = "/proc/asound/card0/codec#0";
static const char *BATT_NOW	  = "/sys/class/power_supply/BAT0/capacity";
static const char *BATT_STAT  = "/sys/class/power_supply/BAT0/status";

/* statis colors				  R G B */
static const unsigned int CONORM  = 0xC6C6C6;
static const unsigned int CO_LOW  = 0xB7C9D6;
static const unsigned int COMID1  = 0x94AEC2;
static const unsigned int COMID2  = 0x7093AD;
static const unsigned int COHIGH  = 0x4C7899;
static const unsigned int COWARN  = 0xff8880;

/* icons */
enum {
   cpu, volume_high, volume_mid, volume_low, mute, 
   wifi_full, wifi_high, wifi_mid, wifi_low, wired_dc, wired_cn, 
   app_term, app_web, app_docs, app_music,app_video, app_games, app_images, app_etc, 
   batt_fulc, batt_875c, batt_750c, batt_625c, batt_500c, batt_375c, batt_250c, batt_125c, batt_000c,
   batt_ful, batt_875, batt_750, batt_625, batt_500, batt_375, batt_250, batt_125, batt_000,
};

/* variables */
static long	   ln1,ln2,ln3,ln4,j1,j2,j3,j4;
static int	   n, t, loops = 0;
static char	   c, stat, clk[60];
static FILE	   *in;
static time_t  current;

int main(int argc, const char **argv) {
   in = fopen(CPU_FILE,"r");
   fscanf(in,"cpu %ld %ld %ld %ld",&j1,&j2,&j3,&j4);
   fclose(in);

   /* main loop */
   for (;;) {
	   
	   /* Wired Iface Monitor */
	   if ( (in=fopen(WIRD_FILE,"r")) ) {
		   fscanf(in,"%c",&stat);
		   fclose(in);
		   if (stat=='u') printf("{#%06X}{i %d}",COHIGH,wired_cn);
		   else printf("{#%06X}{i %d}",CONORM,wired_dc);
		   
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
	   if ( (in=fopen(CPU_TEMP,"r")) ) {
		   fscanf(in,"%d",&t);
		   fclose(in);

		   if (t > 88000) printf("{#%6X}{i %d}",COWARN,cpu);
		   else printf("{#%06X}{i %d}",CONORM,cpu);

		   in = fopen(CPU_FILE,"r");
		   fscanf(in,"cpu %ld %ld %ld %ld",&ln1,&ln2,&ln3,&ln4);
		   fclose(in);

		   if (ln4>j4) n=(int)100*(ln1-j1+ln2-j2+ln3-j3)/(ln1-j1+ln2-j2+ln3-j3+ln4-j4);
		   else n=0;

		   if (n > 90) printf("{#%06X}l",COWARN);
		   else if (n > 66) printf("{#%06X}i",COMID2);
		   else if (n > 33) printf("{#%06X}:",COMID1);
		   else printf("{#%06X}.",CO_LOW);
		   
		   printf("{#%06X} | ",CONORM);
	   }
	   
	   /* Volume Monitor */
	   if ( (in=fopen(AUD_FILE,"r")) ) {
		   while ( fscanf(in," Amp-Out caps: ofs=0x%ld",&ln1) !=1 )
			   fscanf(in,"%*[^\n]\n");
		   while ( fscanf(in, "Amp-Out vals: [0x%ld",&ln2) !=1 )
			   fscanf(in,"%*[^\n]\n");
		   fclose(in);
		   
		   n = 10*ln2/ln1;
		   if (n > 10) printf("{#%06X}{i %d}",CONORM,mute);
		   else {
			   if (n >= 9) printf("{#%06X}{i %d}",COWARN,volume_high);
			   else if (n >= 7) printf("{#%06X}{i %d}",CO_LOW,volume_high);
			   else if (n >= 5) printf("{#%06X}{i %d}",COMID1,volume_mid);
			   else if (n >= 3) printf("{#%06X}{i %d}",COMID2,volume_mid);
			   else printf("{#%06X}{i %d}",COHIGH,volume_low);
		   }
		   printf("{#%06X} | ",CONORM);
	   }
	   
	   /* Battery Monitor */
	   if ( (in=fopen(BATT_NOW,"r")) ) {
		   fscanf(in,"%d\n",&n); fclose(in);
		   if ( (in=fopen(BATT_STAT,"r")) ) { fscanf(in,"%c",&c); fclose(in); }
		   if (c=='C') {
			   if (n >= 95) printf("{#%06X}{i %d}",COHIGH,batt_fulc);
			   else if (n >= 88) printf("{#%06X}{i %d}",COHIGH,batt_875c);
			   else if (n >= 75) printf("{#%06X}{i %d}",COMID2,batt_750c);
			   else if (n >= 63) printf("{#%06X}{i %d}",COMID2,batt_625c);
			   else if (n >= 50) printf("{#%06X}{i %d}",COMID1,batt_500c);
			   else if (n >= 38) printf("{#%06X}{i %d}",COMID1,batt_375c);
			   else if (n >= 25) printf("{#%06X}{i %d}",CO_LOW,batt_250c);
			   else if (n >= 13) printf("{#%06X}{i %d}",CO_LOW,batt_125c);
			   else printf("{#%06X}{i %d}",COWARN,batt_000c);
		   }
		   else {
			   if (n >= 95) printf("{#%06X}{i %d}",COHIGH,batt_ful);
			   else if (n >= 87) printf("{#%06X}{i %d}",COHIGH,batt_875);
			   else if (n >= 75) printf("{#%06X}{i %d}",COMID2,batt_750);
			   else if (n >= 62) printf("{#%06X}{i %d}",COMID2,batt_625);
			   else if (n >= 50) printf("{#%06X}{i %d}",COMID1,batt_500);
			   else if (n >= 37) printf("{#%06X}{i %d}",COMID1,batt_375);
			   else if (n >= 25) printf("{#%06X}{i %d}",CO_LOW,batt_250);
			   else if (n >= 12) printf("{#%06X}{i %d}",CO_LOW,batt_125);
			   else printf("{#%06X}{i %d}",COWARN,batt_000);
		   }
		   printf("{#%06X} | ",CONORM);
	   }
	   
	   /* Clock */
	   if ((loops % 40) == 0) {
		   time(&current);
		   strftime(clk,38,"%H.%M | %A, %d %B %Y",localtime(&current));
	   }
	   printf("{#%06X}%s \n",CONORM,clk);
	   fflush(stdout);
	   sleep(1);
   }
   return 0;
}
