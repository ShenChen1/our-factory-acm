#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define infof(fmt, arg...) //printf(fmt, ##arg)
#define debugf(fmt, arg...) //printf(fmt, ##arg)

#define NoEdge -1

typedef struct
{
	int num;
	int *list;
} user_t;


typedef struct
{
	int user_num;
	user_t *usrs;

	int line_num;
	int *lines;

} testline_t;


static testline_t s_testline = {0};

static void do_cleanup()
{
	int i = 0;
	
	free(s_testline.lines);
	
	for (i = 0; i < s_testline.user_num; i++)
	{
		free(s_testline.usrs[i].list);
	}
}

static int get_users_and_lines(char *str)
{
	int i = 0;
	int ret = 0;
	
	memset(&s_testline, 0, sizeof(s_testline));

	ret = sscanf(str, "%d, %d", &s_testline.line_num, &s_testline.user_num);
	if (ret < 0)
	{
		printf("sscanf err\n");
		return -1;
	}
	
	infof("lines:%d users:%d\n", s_testline.line_num, s_testline.user_num);
	
	s_testline.lines = (int *)malloc(s_testline.line_num * sizeof(int));
	assert(s_testline.lines);
	
	for (i = 0; i < s_testline.line_num; i++)
	{
		s_testline.lines[i] = i + 1;
	}
	
	s_testline.usrs = (user_t *)malloc(s_testline.user_num * sizeof(user_t));
	assert(s_testline.usrs);
	
	for (i = 0; i < s_testline.user_num; i++)
	{
		s_testline.usrs[i].num = 0;
		s_testline.usrs[i].list = (int *)malloc(20 * sizeof(int));
		assert(s_testline.usrs[i].list);
	}

	return 0;
}

static int get_oneuser_list(int user, char *str)
{
	int i = 0;
	char *token = NULL;
	char *ptr = NULL;

	token = strtok_r(str, " ,", &ptr);
	while (token != NULL)
	{
		s_testline.usrs[user].list[i] = atoi(token);
		token = strtok_r(NULL, " ,", &ptr);
		i++;
	}

	s_testline.usrs[user].num = i;
	
#if 0
	debugf("user:%d  ", user);
	for (i = 0; i < s_testline.usrs[user].num; i++)
	{
		debugf("%4d ", s_testline.usrs[user].list[i]);
	}
	debugf("\n");
#endif

	return 0;
}

static int prase_file(char *path)
{
	int i = 0;
	FILE *file = NULL;
	char buff[512];

	file = fopen(path , "r");
	if (file == NULL)
	{
		printf("fopen err\n");
		return -1;
	}

	//users_and_lines
	if (NULL != fgets(buff, sizeof(buff), file))
	{
		get_users_and_lines(buff);
	}

	while (NULL != fgets(buff, sizeof(buff), file))
	{
		//debugf("read:%s", buff);
		get_oneuser_list(i++, buff);
	}
	assert(i == s_testline.user_num);

	fclose(file);

	return 0;
}


static int do_extract(int user, int *map)
{
	int i, j;
	int sum = 0;

	if (user == s_testline.user_num)
	{
		debugf("fuck\n");
		return 1;
	}
	
	for (i = 0; i < s_testline.line_num; i++)
	{
		debugf("user:%d line:%d map:%d\n", user, s_testline.lines[i], map[i]);
		
		if (map[i] == 1)
		{
			//be used
			continue;
		}
			
		for (j = 0; j < s_testline.usrs[user].num; j++)
		{
			debugf("user:%d line:%d list:%d\n", user, s_testline.lines[i], s_testline.usrs[user].list[j]);
			if (s_testline.usrs[user].list[j] == s_testline.lines[i])
			{
				//find line can be used
				break;
			}
		}
		
		if (j == s_testline.usrs[user].num)
		{
			//not invalid
			continue;
		}
		
		debugf("user:%d find line:%d\n", user, s_testline.lines[i]);

		map[i] = 1;
		sum += do_extract(user+1, map);
		map[i] = 0;
	}
	
	return sum;
}


static void do_process()
{
	int *map = (int *)malloc(s_testline.line_num * sizeof(int));
	assert(map);
	memset(map, 0, s_testline.line_num * sizeof(int));
	
	printf("%d\n", do_extract(0, map));
	
	free(map);

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
