#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define infof(fmt, arg...) //printf(fmt, ##arg)
#define debugf(fmt, arg...) //printf(fmt, ##arg)
#define errorf(fmt, arg...) //printf(fmt, ##arg)

#define MAX		(32)

typedef struct
{
	int num;
	int letters[MAX];
} number_t;

typedef struct
{
	char opt;
	number_t x;
	number_t y;
	number_t z;

} equation_t;

typedef struct
{
	int num;
	char letters[MAX];
	int number[MAX];
	int notzero[MAX];

} letters_t;

static letters_t s_letters = {0};
static equation_t s_equation = {0};

//get number char from string
static int get_number(number_t *number, char *buf, int len)
{
	int i = 0;
	
	number->num = len;
	for (i = 0; i < number->num; i++)
	{
		number->letters[i] = strchr(s_letters.letters, buf[len-i-1]) - s_letters.letters;
		infof("%d:%c\n", i, s_letters.letters[number->letters[i]]);
		//the highest positon is not zero
		if (i == number->num-1)
		{
			s_letters.notzero[number->letters[i]] = 1;
			infof("H:%c\n", s_letters.letters[number->letters[i]]);
		}
	}
	
	return 0;
}

//get equation from string
static int get_equation(char *buf, int len)
{
	int i = 0;
	char *ptr = NULL;

	//init data
	s_letters.num = 0;
	memset(s_letters.letters, 0, sizeof(s_letters.letters));
	memset(s_letters.number, 0, sizeof(s_letters.number));
	memset(s_letters.notzero, 0, sizeof(s_letters.notzero));

	//find all useful letters
	for (i = 0; i < len; i++)
	{
		if (buf[i] < 'A' || buf[i] > 'Z')
		{
			continue;
		}
		
		if (strchr(s_letters.letters, buf[i]) == NULL)
		{
			s_letters.letters[s_letters.num] = buf[i];
			s_letters.num++;
		}
	}
	
	infof("letters:%s\n", s_letters.letters);

	//get x 
	ptr = strchr(buf, ' ');
	get_number(&s_equation.x, buf, ptr - buf);

	//get opt
	s_equation.opt = ptr[1];

	//get y
	buf = ptr + 3;
	ptr = strchr(buf, ' ');
	get_number(&s_equation.y, buf, ptr - buf);
	
	//get z
	buf = strchr(buf, '=') + 2;
	ptr = buf + strlen(buf);
	while (*ptr < 'A' || *ptr > 'Z')
	{
		ptr--;
	}
	get_number(&s_equation.z, buf, ptr+1 - buf);
 
#if 0
	infof("notzero:");
	for (i = 0; i < s_letters.num; i++)
	{
		infof("%d", s_letters.notzero[i]);
	}
	infof("\n");
#endif
	
	return 0;
}

//read and prase file
static int prase_file(char *path)
{
	int ret = 0;
	FILE *fp = NULL;
	char *buff = NULL; 
	int nread = 0;

	fp = fopen(path, "r");
	if (fp == NULL)
	{
		errorf("fopen err\n");
		return -1;
	}
	
	fseek(fp, 0, SEEK_END);
	nread = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	buff = (char *)malloc(nread);
	assert(buff);

	ret = fread(buff, 1, nread, fp);
	if (ret != nread)
	{
		ret = -1;
		errorf("fread err\n");
		goto end;
	}

	infof("buff:%s\n", buff);
	ret = get_equation(buff, nread);
	if (ret < 0)
	{
		errorf("get_equation err\n");
		goto end;
	}

end:
	fclose(fp);
	free(buff);

	return ret;
}

//char to integer
static int generate_number(number_t *number)
{
	int i = 0;
	int num = 0;
	
	for (i = 0; i < number->num; i++)
	{
		num += s_letters.number[number->letters[number->num-1-i]];
		num *= 10;
	}
	
	return num/10;
}

//print the result
static void do_show(long x, long y, long z, char opt)
{
	printf("%ld %c %ld = %ld\n", x, opt, y, z);
}

//calc x opt y == z
static int do_calc()
{
	int x = 0, y = 0, z = 0;

	x = generate_number(&s_equation.x);
	y = generate_number(&s_equation.y);
	z = generate_number(&s_equation.z);

	//infof("%d %c %d = %d\n", x,s_equation.opt,y,z);
	switch (s_equation.opt)
	{
		case '+':
		{
			if (x + y == z)
			{
				do_show(x,y,z,s_equation.opt);
			}
			break;
		}
		case '-':
		{
			if (x - y == z)
			{
				do_show(x,y,z,s_equation.opt);
			}
			break;
		}
		case '*':
		{
			if (x * y == z)
			{
				do_show(x,y,z,s_equation.opt);
			}
			break;
		}
		case '/':
		{
			if (x / y == z)
			{
				do_show(x,y,z,s_equation.opt);
			}
			break;
		}
	}
	
	return 0;
}

static void swap(int *a, int *b)
{
	int tmp = 0;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

//全排列
static void do_permutation(int *a, int k, int m)
{
	int i = 0;

	if (k == m)
	{
		for (i = 0; i < s_letters.num; i++)
		{
			//skip the positon limit
			if (s_letters.notzero[i] == 1 && s_letters.number[i] == 0)
			{
				break;
			}
		}
		
		if (i == s_letters.num)
		{
			do_calc();
		}
	}
	else
	{
		for (i = k; i <= m; i++)
		{
			swap(&a[i], &a[k]);
			do_permutation(a, k+1, m);
			swap(&a[i], &a[k]);
		}
	}
}

static int place(int *x, int k, int total)
{
	int i = 0;
	int sum = 0;

	for (i = 0;i <= k; i++)
	{
		sum += x[i];
	}

	return sum < total;
}

//从a中选取num个elem
static void do_extract(int *a, int total)
{
	int k = 0;
	int x[MAX] = {0};

	x[k] = -1;
	while (k >= 0)
	{
		x[k]++;

		if (place(x, k, total))
		{
			if (k == s_letters.num-1)
			{
				int i, t = 0;
				for (i = 0;i < s_letters.num;i++)
				{
					t += x[i];
					s_letters.number[i] = a[t];
					debugf("%d ", s_letters.number[i]);
				}
				debugf("\n");
				do_permutation(s_letters.number, 0, s_letters.num-1);
			}
			else
			{
				k++;
				x[k] = 0;
			}
		}
		else
		{
			k--;
		}
	}
}

static void do_process()
{
	int a[] = {0,1,2,3,4,5,6,7,8,9};

	do_extract(a, sizeof(a)/sizeof(a[0]));
}

int main(int argc, char **argv)
{
	if (argc != 2 || argv[1] == NULL)
	{
		return -1;
	}

	prase_file(argv[1]);
	
	do_process();

	return 0;
}
