#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h> 

#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#define	BUTTON_PIN	7
#define INTERVAL 300

static volatile short int i = 0 ;
static volatile sig_atomic_t gotAlarm = 0;


static void sigalrmHandler(int sig) {
	gotAlarm = 1;
}

void myInterrupt (void) {
	i++ ;
	digitalRead (BUTTON_PIN);
}

int main (void){
	struct sigaction sa;
	int status=-1;
	int formerstatus=0;
	int formeri=0;

	 if (wiringPiSetup () < 0) {
		fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno)) ;
		return 1 ;
	}

	pullUpDnControl (BUTTON_PIN, PUD_UP) ;
	
	if (wiringPiISR (BUTTON_PIN, INT_EDGE_FALLING, &myInterrupt) < 0) {
		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno)) ;
		return 1 ;
	}

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sigalrmHandler;
	sigaction(SIGALRM, &sa, NULL);

	alarm(INTERVAL);

while(1){
	delay (100);
	if (gotAlarm){
		gotAlarm=0;
		if ((formeri < 2) && (i < 2)) {
			status=0;
		} else {
			status=1;
		}
		formeri=i;
		if ((formerstatus) & (!status)){
			printf ("feierabend ");
			system("/root/gpio/wama-stop.sh");
		}
		if ((!formerstatus) & (status)){
			printf ("schichtbegin ");
			system("/root/gpio/wama-start.sh");
		}

		formerstatus=status;
		i=0;
		alarm(INTERVAL);
	}

}
return 0 ;
}
