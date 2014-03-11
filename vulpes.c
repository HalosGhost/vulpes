/**********************************\
* Status Output Program for Alopex *
* License: GPLv2 Sam Stuewe, 2013  *
\**********************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <curl/curl.h>

/* input files */
static const char *WIFI_FILE  = "/proc/net/wireless";
static const char *WIRD_FILE  = "/sys/class/net/eth0/operstate";
static const char *CPU_FILE	  = "/proc/stat";
static const char *CPU_TEMP	  = "/sys/bus/platform/devices/coretemp.0/temp1_input";
static const char *BATT_NOW	  = "/sys/class/power_supply/BAT0/capacity";
static const char *BATT_STAT  = "/sys/class/power_supply/BAT0/status";

/* variables */
static long	   ln1,ln2,ln3,ln4,j1,j2,j3,j4;
static int	   n, t, loops = 0;
static char	   c, stat, clk[60];
static FILE	   *in;
static time_t  current;

void usage(char *progname) {
	fprintf(stderr,"Usage: %s [-h|--help]\n", progname);
	exit(44);
}

int main(int argc, char** argv) {
   in = fopen(CPU_FILE,"r");
   fscanf(in,"cpu %ld %ld %ld %ld",&j1,&j2,&j3,&j4);
   fclose(in);

   /* main loop */
   if (argv[1]) usage(argv[0]);
   for (;;) {

	   /* Wired Iface Monitor */
	   if ( (in=fopen(WIRD_FILE,"r")) ) {
		   fscanf(in,"%c",&stat); fclose(in);
		   printf("E:");
		   if (stat=='u') printf("CN");
		   else printf("DC");
		   
		   printf(" | ");
	   }
	   
	   /* Wireless Iface Monitor */
	   if ( (in=fopen(WIFI_FILE,"r")) ) {
		   n=0;
		   fscanf(in,"%*[^\n]\n%*[^\n]\nwlan0: %*d %d.",&n); fclose(in);

		   t=15;
		   if ( n > 50 ) printf(".:l");
		   else if ( n > 25 ) printf(".:");
		   else if ( n > 1 ) printf(".");
		   else printf("DC");

		   printf(" | ");
	   }
	   
	   /* CPU Monitor */
	   if ( (in=fopen(CPU_TEMP,"r")) ) {
		   fscanf(in,"%d",&t); fclose(in);

		   printf("CPU:");

		   if (t > 88000) printf("W");

		   in = fopen(CPU_FILE,"r");
		   fscanf(in,"cpu %ld %ld %ld %ld",&ln1,&ln2,&ln3,&ln4); fclose(in);

		   if (ln4>j4) n=(int)100*(ln1-j1+ln2-j2+ln3-j3)/(ln1-j1+ln2-j2+ln3-j3+ln4-j4);
		   else n=0;

		   if (n > 90) printf("l");
		   else if (n > 66) printf("i");
		   else if (n > 33) printf(":");
		   else printf(".");
		   
		   printf(" | ");
	   }
	   
	   /* Volume Monitor */
	   if ( (in=popen("ponymix get-volume","r")) ) {
		   fscanf(in,"%d",&n); pclose(in);
		   printf("a:");
		   if (system("ponymix is-muted")==0) printf("M");
		   else printf("%d", n);
		   printf(" | ");
	   }
	   
	   /* Battery Monitor */
	   if ( (in=fopen(BATT_NOW,"r")) ) {
		   fscanf(in,"%d\n",&n); fclose(in);
		   if ( (in=fopen(BATT_STAT,"r")) ) { fscanf(in,"%c",&c); fclose(in); }
		   printf("B:%d", n);
		   if (c=='C') printf("C");
		   printf(" | ");
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
   return 0;
}
