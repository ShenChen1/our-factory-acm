#include <fstream>
#include <cstring>
#include <string>
#include <stdlib.h>

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

static int get_number(number_t *number, char *buf, int len)
{
	int i = 0;
	int tmp = 0;

	for (i = 0; i < len; i++)
	{
		tmp = strchr(s_letters.letters, buf[len - i - 1]) - s_letters.letters;

		infof("%d:%c\n", i, s_letters.letters[tmp]);

		if (i == (len - 1))
		{
			s_letters.notzero[tmp] = 1;
			infof("H:%c\n", s_letters.letters[tmp]);
		}

		number->letters[i] = tmp;
	}
	number->num = len;
	
	return 0;
}

static int get_equation(char *buf, int len)
{
	int i = 0;
	char *ptr = NULL;

	for (i = 0; i < len; i++)
	{
		if ((buf[i] >= 'A') && (buf[i] <= 'Z'))
		{
			ptr = strchr(s_letters.letters, buf[i]);
			if (ptr == NULL)
			{
				s_letters.letters[s_letters.num] = buf[i];
				s_letters.num++;
			}
		}
	}
	
	infof("letters:%s\n", s_letters.letters);

	ptr = strchr(buf, ' ');
	get_number(&s_equation.x, buf, ptr - buf);

	s_equation.opt = ptr[1];

	buf = ptr + 3;
	ptr = strchr(buf, ' ');
	get_number(&s_equation.y, buf, ptr - buf);

	buf = strchr(buf, '=') + 2;
	ptr = buf + strlen(buf);
	while (ptr && (*ptr < 'A' || *ptr > 'Z'))
	{
		ptr--;
	}
	get_number(&s_equation.z, buf, ptr + 1 - buf);

	return 0;
}

static int prase_file(char *path)
{
	int ret = 0;
	FILE *fp = NULL;
	char buff[512] = {0}; 
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

	return ret;
}

static int generate_number(number_t *number)
{
	int i = 0;
	int num = 0;
	int tmp = 0;
	
	for (i = 0; i < number->num; i++)
	{
		tmp = number->letters[number->num - 1 - i];
		num += s_letters.number[tmp];
		num *= 10;
	}
	
	return (num/10);
}

static void do_show(int x, int y, int z, char opt)
{
	printf("%d %c %d = %d\n", x, opt, y, z);
}

static int do_calc()
{
	int x = 0;
	int y = 0;
	int z = 0;

	x = generate_number(&s_equation.x);
	y = generate_number(&s_equation.y);
	z = generate_number(&s_equation.z);

	if ((s_equation.opt == '*') && ((x * y) == z))
	{
		do_show(x,y,z,s_equation.opt);
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

static void do_permutation(int *a, int k, int m)
{
	int i = 0;

	if (k == m)
	{
		for (i = 0; i < s_letters.num; i++)
		{
			if ((s_letters.notzero[i] == 1) && 
				(s_letters.number[i] == 0))
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
			do_permutation(a, k + 1, m);
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

static void do_extract(int *array, int total, int select)
{
	int k = 0;
	int i = 0;
	int t = 0;
	int map[MAX] = {0};

	map[k] = -1;
	while (k >= 0)
	{
		map[k]++;

		if (place(map, k, total))
		{
			if (k == (select - 1))
			{
				t = 0; 
				for (i = 0; i < select; i++)
				{
					t += map[i];
					s_letters.number[i] = array[t];
				}

				do_permutation(s_letters.number, 0, select-1);
			}
			else
			{
				k++;
				map[k] = 0;
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

	do_extract(a, 10, s_letters.num);
}

int main(int argc, char **argv)
{
	if ((argc != 2) || (argv[1] == NULL))
	{
		return -1;
	}

	prase_file(argv[1]);
	
	do_process();

	return 0;
}
