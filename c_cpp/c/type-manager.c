
#include <string.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
	int a;
	int b;
} str1_t;

typedef struct {
	int a;
	int b;
	int c;
} str2_t;

typedef struct {
	int a;
	int cnt1;
	str1_t *s1;
	int cnt2;
	str2_t *s2;
} str3_t;

typedef struct {
	int a;
	int cnt1;
	str3_t *s1;
} str4_t;

typedef union {
	str1_t str1[1];
	str2_t str2[1];
	str3_t str3[1];
	char buf[65535];
} type_manager_t;

#define tman_next(t,p) ((t*)(((char*)(p))+offset))[0]
#define tman_nextp(t,p) &(((t*)(((char*)(p))+offset))[0])
#define tman_getp(t,b,o) ((t*)(((char*)(b))+o))
#define tman_shift(o,t) o+=sizeof(t)

int main()
{
	type_manager_t tman;
	int offset = 0;

	str1_t *pstr1 = tman_getp(str1_t, &tman, offset);
	for (int i = 0; i < 3; ++i) {
		str1_t *next = tman_nextp(str1_t, tman.str1);
		next->a = 1;
		next->b = 2;
		tman_shift(offset, str1_t);
	}

	printf ( "sizeof(int) = %ld\n", sizeof(int));

	printf ( "offset = %d\n", offset);

	str2_t *pstr2 = tman_getp(str2_t, &tman, offset);
	for (int i = 0; i < 3; ++i) {
		str2_t *next = tman_nextp(str2_t, tman.str2);
		next->a = 3;
		next->b = 4;
		next->c = 5;
		tman_shift(offset, str2_t);
	}

	printf ( "offset = %d\n", offset);

	str3_t *pstr3 = tman_getp(str3_t, &tman, offset);
	tman_shift(offset, str3_t);
	{
		pstr3->a = 6;
		pstr3->cnt1 = 3;
		pstr3->s1 = tman_getp(str1_t, &tman, offset);
		for (int i = 0; i < pstr3->cnt1; ++i) {
			tman_shift(offset, str1_t);
		}
		for (int i = 0; i < pstr3->cnt1; ++i) {
			pstr3->s1[i].a = 7;
			pstr3->s1[i].b = 8;
		}
		pstr3->cnt2 = 3;
		pstr3->s2 = tman_getp(str2_t, &tman, offset);
		for (int i = 0; i < pstr3->cnt2; ++i) {
			tman_shift(offset, str2_t);
		}
		for (int i = 0; i < pstr3->cnt2; ++i) {
			pstr3->s2[i].a = 9;
			pstr3->s2[i].b = 10;
			pstr3->s2[i].c = 11;
		}
	}

	printf ( "offset = %d\n", offset);

	str4_t *pstr4 = tman_getp(str4_t, &tman, offset);
	tman_shift(offset, str4_t);
	{
		pstr4->a = 12;
		pstr4->cnt1 = 3;
		pstr4->s1 = tman_getp(str3_t, &tman, offset);
		for (int i = 0; i < pstr4->cnt1; ++i) {
			tman_shift(offset, str3_t);
		}
		for (int i = 0; i < pstr4->cnt1; ++i) {
			str3_t *pstr3 = &(pstr4->s1[i]);
			{
				pstr3->a = 13;
				pstr3->cnt1 = 3;
				pstr3->s1 = tman_getp(str1_t, &tman, offset);
				for (int j = 0; j < pstr3->cnt1; ++j) {
					tman_shift(offset, str1_t);
					pstr3->s1[j].a = 14;
					pstr3->s1[j].b = 15;
				}
				pstr3->cnt2 = 3;
				pstr3->s2 = tman_getp(str2_t, &tman, offset);
				for (int j = 0; j < pstr3->cnt2; ++j) {
					tman_shift(offset, str2_t);
					pstr3->s2[j].a = 16;
					pstr3->s2[j].b = 17;
					pstr3->s2[j].c = 18;
				}
			}
		}
	}

	printf ( "offset = %d\n", offset);


	for (int i = 0; i < 3; ++i) {
		printf ( "pstr1 a = %d b = %d\n", pstr1[i].a, pstr1[i].b );
	}

	for (int i = 0; i < 3; ++i) {
		printf ( "pstr2 a = %d b = %d, c = %d\n", pstr2[i].a, pstr2[i].b, pstr2[i].c );
	}

	printf ( "pstr3 a = %d pstr1{ ", pstr3->a );
	for (int j = 0; j < pstr3->cnt1; ++j) {
		str1_t *pstr1 = &(pstr3->s1[j]);
		printf ( "a = %d b = %d ", pstr1->a, pstr1->b );
	}
	printf ( "} pstr2{ ");
	for (int j = 0; j < pstr3->cnt2; ++j) {
		str2_t *pstr2 = &(pstr3->s2[j]);
		printf ( "a = %d b = %d c = %d ", pstr2->a, pstr2->b, pstr2->c );
	}
	printf ( "}\n");

	printf ( "pstr4 a = %d pstr3{ ", pstr4->a );
	for (int i = 0; i < pstr4->cnt1; ++i) {
		str3_t *pstr3 = &(pstr4->s1[i]);
		printf ( "a = %d pstr1{ ", pstr3->a );
		for (int j = 0; j < pstr3->cnt1; ++j) {
			str1_t *pstr1 = &(pstr3->s1[j]);
			printf ( "a = %d b = %d ", pstr1->a, pstr1->b );
		}
		printf ( "} pstr2{ ");
		for (int j = 0; j < pstr3->cnt2; ++j) {
			str2_t *pstr2 = &(pstr3->s2[j]);
			printf ( "a = %d b = %d c = %d ", pstr2->a, pstr2->b, pstr2->c );
		}
		printf ( "} ");
	}
	printf ( "}\n");

	return 0;
}