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
#include <curl/curl.h>

/* input files */
static const char *WIFI_FILE  = "/proc/net/wireless";
static const char *WIRD_FILE  = "/sys/class/net/enp2s0f0/operstate";
static const char *CPU_FILE	  = "/proc/stat";
static const char *CPU_TEMP	  = "/sys/bus/platform/devices/coretemp.0/temp1_input";
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
   sunny, partly_sunny, overcast, partly_cloudy, rain, thunderstorms, snow, misty,
};

/* variables */
static long	   ln1,ln2,ln3,ln4,j1,j2,j3,j4;
static int	   n, t, wthrloops, loops = 0;
static char	   c, stat, clk[60], condition[60];
static FILE	   *in;
static time_t  current;

void usage(char *progname) {
	fprintf(stderr,"Usage: %s [-h|--help]\n", progname);
	exit(44);
}

int main(int argc, char** argv) {
   CURL *handle;
   CURLcode res;
   curl_global_init(CURL_GLOBAL_ALL);
   handle = curl_easy_init();
   FILE *suppressOutput = fopen("/dev/null", "wb");

   curl_easy_setopt(handle, CURLOPT_WRITEDATA, suppressOutput);
   curl_easy_setopt(handle, CURLOPT_URL, "http://icanhazip.com");

   in = fopen(CPU_FILE,"r");
   fscanf(in,"cpu %ld %ld %ld %ld",&j1,&j2,&j3,&j4);
   fclose(in);

   /* main loop */
   if (argv[1]) usage(argv[0]);
   for (;;) {

	   // Set Default Color
	   printf("{#%06X}",CONORM);

	   /* Weather Update */
	   /*if ( (wthrloops % 300)==0) {
		   if ( (in=popen("shaman -c 55105","r")) ) {
			   fscanf(in,"Condition: %s",condition);pclose(in);
			   if ( strstr(condition,"Overcast") ) printf("{i %d}",overcast);
			   else if ( strstr(condition,"Partly") ) {
				   if ( strstr(condition,"Cloudy") ) printf("{i %d}",partly_cloudy);
				   else if ( strstr(condition,"Sunny") ) printf("{i %d}",partly_sunny);
				   else printf("{i %d}",sunny);
			   }
			   else if ( strstr(condition,"Mostly") ) {
				   if ( strstr(condition,"Cloudy") ) printf("{i %d}",partly_cloudy);
				   else if ( strstr(condition,"Sunny") ) printf("{i %d}",partly_sunny);
				   else printf("{i %d}",sunny);
			   }
			   else if ( strstr(condition,"Thunderstorms") ) printf("{i %d}",thunderstorms);
			   else if ( strstr(condition,"Showers") ) printf("{i %d}",rain);
			   else printf("{i %d}",sunny);
			   printf(" | ");
		   }
	   }*/

	   /* Wired Iface Monitor */
	   if ( (in=fopen(WIRD_FILE,"r")) ) {
		   fscanf(in,"%c",&stat); fclose(in);
		   if (stat=='u') printf("{#%06X}{i %d}",COHIGH,wired_cn);
		   else printf("{i %d}",wired_dc);
		   
		   printf("{#%06X} | ",CONORM);
	   }
	   
	   /* Wireless Iface Monitor */
	   if ( (in=fopen(WIFI_FILE,"r")) ) {
		   n=0;
		   fscanf(in,"%*[^\n]\n%*[^\n]\n wlp3s0: %*d %d.",&n); fclose(in);

		   t=15;
		   if ( (loops % t)==0 ) {
			   res = curl_easy_perform(handle);
			   if ( res == CURLE_OK ) {
				   printf("{#%06X}",COHIGH);
				   t=600;
			   }
		   }

		   if (n > 63) printf("{i %d}",wifi_full);
		   else if (n > 50) printf("{i %d}",wifi_high);
		   else if (n > 38) printf("{i %d}",wifi_mid);
		   else if (n > 1) printf("{i %d}",wifi_low);
		   else printf("W");
		   
		   printf("{#%06X} | ",CONORM);
	   }
	   
	   /* CPU Monitor */
	   if ( (in=fopen(CPU_TEMP,"r")) ) {
		   fscanf(in,"%d",&t); fclose(in);

		   if (t > 88000) printf("{#%06X}{i %d}",COWARN,cpu);
		   else printf("{#%06X}{i %d}",CONORM,cpu);

		   in = fopen(CPU_FILE,"r");
		   fscanf(in,"cpu %ld %ld %ld %ld",&ln1,&ln2,&ln3,&ln4); fclose(in);

		   if (ln4>j4) n=(int)100*(ln1-j1+ln2-j2+ln3-j3)/(ln1-j1+ln2-j2+ln3-j3+ln4-j4);
		   else n=0;

		   if (n > 90) printf("{#%06X}l",COWARN);
		   else if (n > 66) printf("{#%06X}i",COMID2);
		   else if (n > 33) printf("{#%06X}:",COMID1);
		   else printf("{#%06X}.",CO_LOW);
		   
		   printf("{#%06X} | ",CONORM);
	   }
	   
	   /* Volume Monitor */
	   if ( (in=popen("ponymix get-volume","r")) ) {
		   fscanf(in,"%d",&n); pclose(in);
		   if (system("ponymix is-muted")==0) printf("{i %d}",mute);
		   else {
			   if (n >= 85) printf("{#%06X}{i %d}",COWARN,volume_high);
			   else if (n >= 75) printf("{#%06X}{i %d}",CO_LOW,volume_high);
			   else if (n >= 50) printf("{#%06X}{i %d}",COMID1,volume_mid);
			   else if (n >= 25) printf("{#%06X}{i %d}",COMID2,volume_mid);
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
		   printf("%s \n",clk);
	   }
	   fflush(stdout);
	   sleep(1);
   }
   fclose(suppressOutput);
   curl_easy_cleanup(handle);
   curl_global_cleanup();
   return 0;
}
