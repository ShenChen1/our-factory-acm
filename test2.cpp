#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define NoEdge	(-1)
#define MAX_M	(100)
#define MAX_N	(100)

#define infof(fmt, arg...) //printf(fmt, ##arg)
#define debugf(fmt, arg...) //printf(fmt, ##arg)

typedef enum
{
	MV_RIGHT = 0,
	MV_DOWN,
	MV_MAX,
} MV_ACT;

typedef struct
{
	int index;
	int value;
} node_t;

typedef struct
{
	node_t node[MV_MAX];
	int total;
} elem_t;

typedef struct
{
	int m;
	int n;

	int point[MAX_M*MAX_N];

	elem_t map[MAX_M*MAX_N];

} map_t;


static map_t s_map = {0};

static void do_cleanup()
{
}

static int get_m_and_n(char *str)
{
	int ret = 0;

	ret = sscanf(str, "%d, %d", &s_map.m, &s_map.n);
	if (ret < 0)
	{
		printf("sscanf err\n");
		return -1;
	}
	
	infof("m:%d n:%d\n", s_map.m, s_map.n);

	return 0;
}

static int get_nline_point(int n, char *str)
{
	int i = 0;
	char *token = NULL;
	char *ptr = NULL;
	
	token = strtok_r(str, " ,", &ptr);
	while (token != NULL)
	{
		s_map.point[n*s_map.m + i] = atoi(token);
		token = strtok_r(NULL, " ,", &ptr);
		i++;
	}
	
	assert(i == s_map.m);

	return 0;
}

static int prase_file(char *path)
{
	int i = 0;
	FILE *file;
	char buff[512];

	file = fopen(path , "r");
	if (file == NULL)
	{
		printf("fopen err\n");
		return -1;
	}

	//m and n
	if (NULL != fgets(buff, sizeof(buff), file))
	{
		get_m_and_n(buff);
	}

	while (NULL != fgets(buff, sizeof(buff), file))
	{
		//info("read:%s", buff);
		get_nline_point(i++, buff);
	}
	assert(i == s_map.n);

	fclose(file);

	return 0;
}


static int generate_map()
{
	int i, j;

	for (j = 0; j < s_map.n; j++)
	{
		for (i = 0; i < s_map.m; i++)
		{
			s_map.map[j*s_map.m + i].node[MV_RIGHT].value = NoEdge;
			s_map.map[j*s_map.m + i].node[MV_DOWN].value = NoEdge;
			s_map.map[j*s_map.m + i].total = 0;

			if (i == s_map.m-1 && j == s_map.n-1)
			{
				//nothing to do
			}
			else if (i == s_map.m-1)
			{
				s_map.map[j*s_map.m + i].node[MV_DOWN].index = (j+1)*s_map.m + i;
				s_map.map[j*s_map.m + i].node[MV_DOWN].value = s_map.point[(j+1)*s_map.m + i];
				
			}
			else if (j == s_map.n-1)
			{
				
				s_map.map[j*s_map.m + i].node[MV_RIGHT].index = j*s_map.m + (i+1);
				s_map.map[j*s_map.m + i].node[MV_RIGHT].value = s_map.point[j*s_map.m + (i+1)];
			}
			else
			{
				s_map.map[j*s_map.m + i].node[MV_DOWN].index = (j+1)*s_map.m + i;
				s_map.map[j*s_map.m + i].node[MV_DOWN].value = s_map.point[(j+1)*s_map.m + i];

				s_map.map[j*s_map.m + i].node[MV_RIGHT].index = j*s_map.m + (i+1);
				s_map.map[j*s_map.m + i].node[MV_RIGHT].value = s_map.point[j*s_map.m + (i+1)];
			}
		}
	}

#if 0
	for (i = 0; i < s_map.m * s_map.n; i++)
	{
		debugf("%4d: ", i);
		for (j = 0; j < MV_MAX; j++)
		{
			debugf("%4d:%4d ", s_map.map[i].node[j].index, s_map.map[i].node[j].value);
		}
		debugf("\n");
	}
#endif

	return 0;
}

static int do_calc(int node, int sum)
{
	int i;
	int tmp = sum;
	int max = sum;

	if (node == s_map.m * s_map.n - 1)
	{
		debugf("sum:%d\n", sum);
		return sum;
	}
	
	if (s_map.map[node].total)
	{
		return s_map.map[node].total + sum;
	}

	for (i = 0; i < MV_MAX; i++)
	{
		if (s_map.map[node].node[i].value == NoEdge)
		{
			continue;
		}

		debugf("node:%d index:%d v:%d \n", node, s_map.map[node].node[i].index, s_map.map[node].node[i].value);
		tmp = do_calc(s_map.map[node].node[i].index, sum + s_map.map[node].node[i].value);

		max = tmp > max ? tmp : max;
	}
	
	s_map.map[node].total = max - sum;
	
	return max;
}

static void do_process()
{
	generate_map();

	printf("%d\n", do_calc(0, s_map.point[0]));
}

int main(int argc, char **argv)
{
	if (argc != 2 || argv[1] == NULL)
	{
		return -1;
	}
	
	prase_file(argv[1]);

	do_process();
	do_cleanup();

	return 0;
}
