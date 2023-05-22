/*
 * The program does an `AND <character>,15` for each character,
 * and using the result as an index, it saves the corresping element from
 * the array `"isrveawhobpnutfg"` in a local variable.
 *
 * This program checks for the index of the desired element and returns the
 * a byte that will result an that index when passed to the 'AND' instruction.
 */

#include <stdio.h>
#include <string.h>

int main(void) {
	char expect[6] = "giants";
	char table[] = "isrveawhobpnutfg";
	char result[7];
	int index;

	for (int i = 0; i < sizeof(expect) / sizeof(*expect); ++i) {
		index = strchr(table, expect[i]) - table;
		result[i] = (char)index | 0b01100000; // To make it printable
	}
	result[6] = '\0';

	printf("res: %s\n", result);
	return 0;
}
