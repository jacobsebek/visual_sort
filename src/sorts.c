#include "sorts.h"
#include "SDL.h"

//#include <time.h>
//#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <stdbool.h>

extern bool pause, reset, quit, norec;
extern bool mode; // MACROS DEFINED IN HEADER

extern uint WIDTH, HEIGHT;

extern SDL_Renderer* ren;
extern SDL_Window* win;

extern int* vals;
extern uint valc;

extern int drawpdate(column_range* marked, uint count);

#define COMP_AG 01 // B is Greater
#define COMP_BS COMP_AG
#define COMP_BG 02 // A is Greater
#define COMP_AS COMP_BG
#define COMP_EQ 04 // Equal

typedef Uint8 comp_op;

static inline bool priv_comp(const register int a, const register int b, register comp_op op)
{
	if (a == b && (op & COMP_EQ)) return 1;

	if (mode == 1) {
		if (a > b  && (op & COMP_AG)) return 1;
		if (a < b  && (op & COMP_BG)) return 1;
	} else {
		if (a > b  && (op & COMP_BG)) return 1;
		if (a < b  && (op & COMP_AG)) return 1;
	}

	return 0;
}

bool sort_check()
{
	printf("running sortcheck\n");
	norec = 1;

	int sum = 0;
	for (int i = 0; i < valc-1; i++) {
		if (priv_comp(vals[i], vals[i+1], COMP_BS | COMP_EQ)) {
			printf("value on %d is greater or equal than on %d\n", i, i+1);
			return 0; // If a is less or equal than b 
		}
		sum += vals[i];

		column_range marked[] = { {0, i+1, COL_3} };
		drawpdate(marked, arrlen(marked));
		if (quit) return 0; // ignore resets (cannot check for abort)
	}

	printf("sortcheck successful. checksum : %d\n", sum+=vals[valc-1]);

	norec = 0;
	return 1;

}

static void priv_merge(const int start, const int end)
{
	if (start >= end) return;

	register const int mid = (start+end)/2;

	column_range marked[] = { {start, end, COL_1} };
	if (drawpdate(marked, arrlen(marked)) == STEP_ABORT) return;

	priv_merge(start, mid);
	priv_merge(mid+1, end);

	//-----MERGE-------

	const int size = end-start+1;

	int tmp[size];
	register int ap = start, bp = mid+1;

	
	for (int i = 0; i < size; i++)
	{
		register const bool bin = bp <= end,
							ain = ap <= mid,
							bless = priv_comp(vals[bp], vals[ap], COMP_AS),
							aless = priv_comp(vals[bp], vals[ap], COMP_BS | COMP_EQ);// A is preffered if equal

		if (!ain | !bin) {
			if 		(ain && !bin) tmp[i] = vals[ap++];
			else if (bin && !ain) tmp[i] = vals[bp++];
			else break;

			continue;
		}

		if 		(bless) tmp[i] = vals[bp++];
		else if (aless) tmp[i] = vals[ap++];

	}
	
	for (int i = 0; i < size; i++)
		vals[start+i] = tmp[i];
}

void sort_merge()
{
	priv_merge(0, valc-1);
}

void sort_selection()
{
	
	for (uint i = 0; i < valc; i++)	{
		uint record = i;
		for (uint j = i+1; j < valc; j++) {
			if (priv_comp(vals[j], vals[record], COMP_AS)) record = j;

			column_range marked[] = { {0, i-1, COL_2}, {i,j, COL_1} };
			if (drawpdate(marked, arrlen(marked)) == STEP_ABORT) return;

		}

		ARRSWAP(vals, record, i);
	}
}

void sort_bubble()
{
	for (uint end = valc; end > 0; end--)
		for (uint i = 1; i < end; i++) {
			if (priv_comp(vals[i], vals[i-1], COMP_AS)) ARRSWAP (vals, i, i-1);

			column_range marked[] = {{0,i, COL_2}, {end,valc-1, COL_1}};
			if (drawpdate(marked, arrlen(marked)) == STEP_ABORT) return;
		}
}

void sort_coctail() //TODO: DONOTREPEAT
{
	int left = 0,
		right = valc-1;

	int dir = 1; // 1 or -1
	while (left < right) {

		int new = (dir == 1) ? right : left;
		for (int i = (dir == 1) ? left+1 : right-1; (dir == 1) ? i <= right : i >= left; i += dir) {
			if (priv_comp(vals[i], vals[i-dir], (dir == 1) ? COMP_BG : COMP_AG)) {
				ARRSWAP (vals, i, i-dir);
				new = i-dir;
			}

			column_range marked[] = { {dir == 1 ? left : i, dir == 1 ? i : right, COL_2}, 
								{right+1, valc-1 , COL_1}, 
								{0, left-1 , COL_1} };
			if (drawpdate(marked, arrlen(marked)) == STEP_ABORT) return;
		}
		
		if (new == (dir == 1 ? right : left))
			break;
		else {
			if (dir == 1) right = new;
			else left = new;
		}

		dir = -dir;

	}
}

void priv_heap_down(uint scope_start, uint start, uint end) // Global : If "start" should be treated as the root
{
	if (start < scope_start) {
		printf("in function priv_heap_down : Start out of designated scope\n");
		return;
	}

	register int tmp = vals[start];
	for (int j = start; j <= end;) { // j == start ? li = j+1

		register const int li = scope_start+(j-scope_start)*2+1;
		register const int ri = li+1;

		if (ri <= end && priv_comp(vals[ri], tmp, COMP_AG) && priv_comp(vals[li], vals[ri], COMP_AS)) {
			vals[j] = vals[ri];
			j = ri;
		} else if (li <= end && priv_comp(vals[li], tmp, COMP_AG)) {
			vals[j] = vals[li];
			j = li;
		} else {
			vals[j] = tmp;
			break;
		}
	}
}

void priv_heap(int start, int end)  // Global : If "start" should be treated as the root
{
	for (int i = start+(end-start-1)/2; i >= start; i--) { //12345
		priv_heap_down(start, i, end);

		column_range marked[] = { {start, i, COL_1} };
		if (drawpdate(marked, arrlen(marked)) == STEP_ABORT) return;
	}
	//return;
	for (int uns = end; uns > start; uns--) { // uns - unsorted
		ARRSWAP(vals, start, uns);
		priv_heap_down(start, start, uns-1);

		column_range marked[] = { {uns, end, COL_1} };
		if (drawpdate(marked, arrlen(marked)) == STEP_ABORT) return;
	}
}

void sort_heap() // TODO: TAKY
{
	priv_heap(0, valc-1);
}

static void priv_quick(const int left, const int right)
{
	if (left >= right) return;

	//ARRSWAP(vals, left, (left+right)/2); //Significantly optimizes finding
	uint piv = left;
	for (int i = left+1; i <= right; i++) {
		if (priv_comp(vals[i], vals[left], COMP_AS)) 
			ARRSWAP(vals, ++piv, i);

		column_range marked[] = { {piv, i, COL_2}, {left, right, COL_1}};
		if (drawpdate(marked, arrlen(marked)) == STEP_ABORT) return;

		//SDL_Delay(25);
	}
	ARRSWAP(vals, piv, left);

	priv_quick(piv+1, right);
	priv_quick(left, piv-1);
}

void sort_quick()
{
	priv_quick(0, valc-1);
}

void sort_insertion()
{
	for (int end = 0; end < valc-1; end++) {
		int val = vals[end+1];

		int insert = end+1;
		while (--insert >= 0 && priv_comp(val, vals[insert], COMP_AS))
			;
		
		for (int i = end+1; i > insert+1; i--)
			vals[i] = vals[i-1];

		vals[insert+1] = val;

		column_range marked[] = { {0, insert-1, COL_1}, 
							{insert+1, end+1, COL_2}};
		if (drawpdate(marked, arrlen(marked)) == STEP_ABORT) return;

	}

}

static void priv_intro(int left, int right, uint maxdepth)
{
	if (left >= right) return;
	if (maxdepth == 0) {
		printf("calling heapsort\n");
		priv_heap(left, right);
		return;
	}

	uint piv = left;
	for (int i = left+1; i <= right; i++) {
		if (priv_comp(vals[i], vals[left], COMP_AS)) 
			ARRSWAP(vals, ++piv, i);

		column_range marked[] = { {piv, i, COL_2}, {left, right, COL_1}};
		if (drawpdate(marked, arrlen(marked)) == STEP_ABORT) return;
	}
	ARRSWAP(vals, piv, left);

	priv_intro(piv+1, right, maxdepth-1);
	priv_intro(left, piv-1, maxdepth-1);
}

void sort_intro()
{
	priv_intro(0, valc-1, log2(valc)*2);
}

void sort_shell()
{
	for (int gap = valc; gap > 0; gap/=2)
		for (int i = gap; i < valc; i++) {
			int tmp = vals[i];

			int j = -1;
			for (j = i; j >= gap && priv_comp(vals[j-gap], tmp, COMP_AG); j-=gap) {
				vals[j] = vals[j-gap];

				column_range marked[] = { {j-gap, j, COL_1}, {i, i+gap, COL_2}};
				if (drawpdate(marked, arrlen(marked)) == STEP_ABORT) return;
			}	

			vals[j] = tmp;
		}
}

static int priv_pow(int b, int exp) {
	for (int i = 0; i < exp-1; i++, b*=b);
	return b;
}

static void priv_counting(uint digit)
{
	const uint exp = (digit == 1 ? 1 : (uint)priv_pow(10, digit-1));
	int count[10] = { 0 };
	int out[valc];

	for (int i = 0; i < valc; i++)
		count[(vals[i]/exp)%10]++;
	
	for (int i = (mode ? 1 : 8); (mode ? i < 10 : i >= 0); i += (mode ? 1 : -1))
		count[i] += count[i-(mode ? 1 : -1)];

	for (int i = valc-1; i >= 0; i--) {
		uint index = (vals[i]/exp) % 10;
		out[--count[index]] = vals[i];
	}

	for (int i = 0; i < valc; i++) {
		column_range marked[] = {{0, i, COL_1}};
		if (drawpdate(marked, arrlen(marked)) == STEP_ABORT) return;
		
		vals[i] = out[i];
	}

}

void sort_radix() {
	int max = vals[0];
	for (int i = 1; i < valc; i++) // Count the biggest number of digits
		if (vals[i] > max)
			max = vals[i];

	//printf("Max value %d\n", max);

	uint maxdigs = 0;
	while (max > 0) {
		maxdigs++;
		max /= 10;
	}

	//printf("Number of digits %d\n", maxdigs);
	
	for (int i = 1; i <= maxdigs; i++)
		priv_counting(i);
}
