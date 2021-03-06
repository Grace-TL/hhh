/*
This is a test main function for the one-dimensional output HHH algorithms
-Thomas Steinke (tsteinke@seas.harvard.edu) 2010-11
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#if 1 /* kjc */
#include <sys/time.h>
#else
#include <sys/timeb.h>
#endif

#ifdef PARALLEL
#include <omp.h>
#endif

#include "alloc.h"

#ifndef CLK_PER_SEC
#ifdef CLOCKS_PER_SEC
#define CLK_PER_SEC CLOCKS_PER_SEC
#endif
#endif

#ifdef HHH
#include "hhh1.h"
#define INCLUDED_SOMETHING
#endif

#ifdef ANCESTRY
#include "ancestry1.h"
#define INCLUDED_SOMETHING
#endif

#ifdef EXACT
#include "exact1.h"
#define INCLUDED_SOMETHING
#endif

#ifndef INCLUDED_SOMETHING
#error Algorithm undefined
#endif


#if NUM_MASKS==5
LCLitem_t masks[NUM_MASKS] = {
	0xFFFFFFFFu,
	0xFFFFFF00u,
	0xFFFF0000u,
	0xFF000000u,
	0x00000000u
};
int leveleps[NUM_MASKS] = {
	32,
	24,
	16,
	8,
	0
};
#endif
#if NUM_MASKS==33
LCLitem_t masks[NUM_MASKS] = {
	0xFFFFFFFFu << 0,
	0xFFFFFFFFu << 1,
	0xFFFFFFFFu << 2,
	0xFFFFFFFFu << 3,
	0xFFFFFFFFu << 4,
	0xFFFFFFFFu << 5,
	0xFFFFFFFFu << 6,
	0xFFFFFFFFu << 7,
	0xFFFFFFFFu << 8,
	0xFFFFFFFFu << 9,
	0xFFFFFFFFu << 10,
	0xFFFFFFFFu << 11,
	0xFFFFFFFFu << 12,
	0xFFFFFFFFu << 13,
	0xFFFFFFFFu << 14,
	0xFFFFFFFFu << 15,
	0xFFFFFFFFu << 16,
	0xFFFFFFFFu << 17,
	0xFFFFFFFFu << 18,
	0xFFFFFFFFu << 19,
	0xFFFFFFFFu << 20,
	0xFFFFFFFFu << 21,
	0xFFFFFFFFu << 22,
	0xFFFFFFFFu << 23,
	0xFFFFFFFFu << 24,
	0xFFFFFFFFu << 25,
	0xFFFFFFFFu << 26,
	0xFFFFFFFFu << 27,
	0xFFFFFFFFu << 28,
	0xFFFFFFFFu << 29,
	0xFFFFFFFFu << 30,
	0xFFFFFFFFu << 31,
	0x00000000u
};
int leveleps[NUM_MASKS] = {
	                     32,31,30,
	29,28,27,26,25,24,23,22,21,20,
	19,18,17,16,15,14,13,12,11,10,
	 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};
#endif

/*int _main() {
		int m; //number of heavy hitters in output
		int counters, threshold, n;
		scanf("%d%d%d", &counters, &threshold, &n);
		HeavyHitter * ans;
		int i;
		unsigned int wa, xa, ya, za;
		unsigned int wb, xb, yb, zb;
		unsigned int w, x, y, z;
		unsigned int a, b, ip;

		init((double)1/(double)counters);

		for (i = 0; i < n; i++) {
			scanf("%u%u%u%u", &w, &x, &y, &z);
			ip = (unsigned int)256*((unsigned int)256*((unsigned int)256*w + x) + y) + z;
			update(ip, 1);
		}

		ans = output(threshold, &m);

		deinit();

		for (i = 0; i < m; i++) {
			//output ans[i]

			//break up the ip
			a = ans[i].item;
			za = a % 256; a /= 256;
			ya = a % 256; a /= 256;
			xa = a % 256; a /= 256;
			wa = a % 256; a /= 256;

			//break up the mask
			b = masks[ans[i].mask];
			zb = b % 256; b /= 256;
			yb = b % 256; b /= 256;
			xb = b % 256; b /= 256;
			wb = b % 256; b /= 256;

			//output ip&mask
			if (wb != 0) printf("%u.", wa); else printf("*.");
			if (xb != 0) printf("%u.", xa); else printf("*.");
			if (yb != 0) printf("%u.", ya); else printf("*.");
			if (zb != 0) printf("%u", za); else printf("*");

			//output counts
			printf("	[%d, %d]\n",  ans[i].lower, ans[i].upper);
		}

		free(ans);

	return 0;
}*/

double dblmainmax(double a, double b) {return (a >= b ? a : b);}

int main(int argc, char * argv[]) {
		int m; //number of heavy hitters in output
		//scanf("%d%d%d", &counters, &threshold, &n);
		int counters = 100;
		int threshold = 1000;
		int n = 100000;
		int memory;
		double time, walltime;
		double epsil;
		HeavyHitter * ans;
		int i;
		unsigned int w, x, y, z;
		clock_t begint, endt;
#if 1 /* kjc */
		struct timeval begintb, endtb;
#else		
		struct timeb begintb, endtb;
#endif
		unsigned int ip;
		unsigned int * data;
		FILE * fp = NULL; //the file for the output
		FILE * fsummary = NULL;
		if (getenv("SUMMARYFILE") != NULL) fsummary = fopen(getenv("SUMMARYFILE"), "a");

		if (argc > 1) n = atoi(argv[1]);
		if (argc > 2) counters = atoi(argv[2]);
		if (argc > 3) threshold = atoi(argv[3]);
		if (argc > 4) fp = fopen(argv[4], "w");

		if(n/counters >= threshold) {
			printf("Unacceptable parameters: eps*n >= theshold\n");
			return 0;
		}

		data = (unsigned int *) malloc(sizeof(unsigned int) * n);

		init((double)1/(double)counters);

		for (i = 0; i < n; i++) {
			scanf("%d%d%d%d", &w, &x, &y, &z);
			ip = (unsigned int)256*((unsigned int)256*((unsigned int)256*w + x) + y) + z;
			data[i] = ip;
		}

		begint = clock();
#if 1 /* kjc */
		gettimeofday(&begintb, NULL);
#else		
		ftime(&begintb);
#endif		
		#ifndef PARALLEL
		for (i = 0; i < n; i++) update(data[i], 1);
		#else
		update(data, n);
		#endif
		endt = clock();
#if 1 /* kjc */
		gettimeofday(&endtb, NULL);
#else
		ftime(&endtb);
#endif		

		time = ((double)(endt-begint))/CLK_PER_SEC;
#if 1 /* kjc */
		walltime = ((double) (endtb.tv_sec-begintb.tv_sec))+((double)endtb.tv_usec-(double)begintb.tv_usec)/1000000;
#else		
		walltime = ((double) (endtb.time-begintb.time))+((double)endtb.millitm-(double)begintb.millitm)/1000;
#endif
		memory=maxmemusage();

		free(data);

		printf("%d ips took %fs %dB [%d counters] ", n, time, memory, counters);

		ans = output(threshold, &m);

		printf("%d HHHs\n", m);

		deinit();

		if (fp != NULL) {
			fprintf(fp, "%s %d %d %d %d %lf %d\n", argv[0], n, counters, threshold, m, time, memory);
			for (i = 0; i < m; i++) {
				fprintf(fp, "%d %d.%d.%d.%d %d %d\n",
				ans[i].mask,
				(int)((ans[i].item >> 24) & (LCLitem_t)255),
				(int)((ans[i].item >> 16) & (LCLitem_t)255),
				(int)((ans[i].item >> 8) & (LCLitem_t)255),
				(int)((ans[i].item >> 0) & (LCLitem_t)255),
				ans[i].lower, ans[i].upper);
				
			}
			fclose(fp);
		}
		
		epsil = -1;
		for (i = 0; i < m; i++) {
			epsil = dblmainmax(epsil, ((double)(ans[i].upper-ans[i].lower))/n);
		}

		FREE(ans);

		if (fsummary != NULL) {
			fprintf(fsummary, "check=false algorithm=%-14s nitems=%-10d counters=%-5d threshold=%-9d memory=%-7d outputsize=%-3d time=%lf walltime=%lf epsilon=%lf",
			                   argv[0],        n,           counters,     threshold,     memory,     m,              time,  walltime, epsil);
			if (getenv("OMP_NUM_THREADS") != NULL) {
				fprintf(fsummary, " threads=%s", getenv("OMP_NUM_THREADS"));
			}
			fprintf(fsummary, "\n");
			fclose(fsummary);       
		}

	return 0;
}


