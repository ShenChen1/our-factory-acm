#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define infof(fmt, arg...) //printf(fmt, ##arg)
#define debugf(fmt, arg...) //printf(fmt, ##arg)
#define errorf(fmt, arg...) //printf(fmt, ##arg)

#define NoEdge 		(-1)
#define MAX_USER	(10)
#define MAX_LINE	(20)

typedef struct
{
	int num;
	int list[MAX_LINE];
} user_t;

typedef struct
{
	int user_num;
	user_t usrs[MAX_USER];

	int line_num;
	int lines[MAX_LINE];

} testline_t;

static testline_t s_testline = {0};

static int get_users_and_lines(char *str)
{
	int i = 0;
	int ret = 0;

	ret = sscanf(str, "%d, %d", &s_testline.line_num, &s_testline.user_num);
	if (ret < 0)
	{
		errorf("sscanf err\n");
		return -1;
	}
	
	infof("lines:%d users:%d\n", s_testline.line_num, s_testline.user_num);
	
	for (i = 0; i < s_testline.line_num; i++)
	{
		s_testline.lines[i] = i + 1;
	}
	
	for (i = 0; i < s_testline.user_num; i++)
	{
		s_testline.usrs[i].num = 0;
	}

	return 0;
}

static int get_oneuser_list(int user, char *str, int len)
{
	int i = 0;
	int j = 0;
	int tmp = 0;
	
	for (i = 0; i < len; i++)
	{
		if (str[i] == ',')
		{
			s_testline.usrs[user].list[j] = tmp;
			j++;

			tmp = 0;
		}
		
		if (str[i] >= '0' && str[i] <= '9')
		{
			tmp *= 10;
			tmp += (str[i] - '0');
		}
	}

	s_testline.usrs[user].list[j] = tmp;
	j++;
	
	s_testline.usrs[user].num = j;

	return 0;
}

static int prase_file(char *path)
{
	int i = 0;
	int len = 0;
	FILE *file = NULL;
	char buff[512];

	file = fopen(path , "r");
	if (file == NULL)
	{
		errorf("fopen err\n");
		return -1;
	}

	//users_and_lines
	if (NULL != fgets(buff, sizeof(buff), file))
	{
		get_users_and_lines(buff);
	}

	while (NULL != fgets(buff, sizeof(buff), file))
	{
		infof("read:%s", buff);
		len = strnlen(buff, sizeof(buff));
		get_oneuser_list(i, buff, len);
		
		i++;
	}
	assert(i == s_testline.user_num);

	fclose(file);

	return 0;
}


static int do_extract(int user, int *map)
{
	int i = 0;
	int j = 0;
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
	int map[MAX_LINE] = {0};

	printf("%d\n", do_extract(0, map));
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
