#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define infof(fmt, arg...) //printf(fmt, ##arg)
#define debugf(fmt, arg...) //printf(fmt, ##arg)
#define errorf(fmt, arg...) //printf(fmt, ##arg)

#define MAX_ITEM	(1000)

typedef struct
{
	int num;
	int depend[MAX_ITEM];

	int total;
} item_t;

typedef struct
{
	int item_num;
	item_t items[MAX_ITEM];

} safe_t;

static safe_t s_safe = {0};

static int get_item_num(char *str)
{
	int ret = 0;

	ret = sscanf(str, "%d", &s_safe.item_num);
	if (ret < 0)
	{
		errorf("sscanf err\n");
		return -1;
	}
	
	infof("item:%d\n", s_safe.item_num);

	return 0;
}

static int get_oneitem_list(int item, char *str, int len)
{
	int i = 0;
	int j = 0;
	int tmp = 0;
	int flag = 0;
	
	for (i = 0; i < len; i++)
	{
		if (str[i] == ' ')
		{
			tmp = flag ? -tmp : tmp;
			s_safe.items[item].depend[j] = tmp;
			j++;

			tmp = 0;
			flag = 0;
		}
		
		if (str[i] == '-')
		{
			flag = 1;
		}
		
		if (str[i] >= '0' && str[i] <= '9')
		{
			tmp *= 10;
			tmp += (str[i] - '0');
		}
	}

	tmp = flag ? -tmp : tmp;
	s_safe.items[item].depend[j] = tmp;
	j++;
	
	s_safe.items[item].num = j;

	debugf("item:%4d total:%4d v:%4d d:", item, s_safe.items[item].num, s_safe.items[item].depend[0]);
	for (i = 1; i < s_safe.items[item].num; i++)
	{
		debugf("%4d ", s_safe.items[item].depend[i]);
	}
	debugf("\n");

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

	//item num
	if (NULL != fgets(buff, sizeof(buff), file))
	{
		get_item_num(buff);
	}

	while (NULL != fgets(buff, sizeof(buff), file))
	{
		infof("read:%s", buff);
		len = strnlen(buff, sizeof(buff));
		get_oneitem_list(i, buff, len);

		i++;
	}
	assert(i == s_safe.item_num);

	fclose(file);

	return 0;
}


static int do_select(int item, int *map, int *len)
{
	int i = 0;
	int tmp = 0;
	
	*len++;
	
	if (map[item] == 1)
	{
		return 0;
	}

	if (s_safe.items[item].total)
	{
		debugf("***item:%d v:%d\n", item+1, s_safe.items[item].total);
		return s_safe.items[item].total;
	}

	for (i = 1; i < s_safe.items[item].num; i++)
	{
		debugf("item:%d d:%d\n", item+1, 
			s_safe.items[item].depend[i]);
		
		map[item] = 1;
		tmp += do_select(s_safe.items[item].depend[i]-1, map, len);
		map[item] = 0;
	}
	
	tmp += s_safe.items[item].depend[0];
	debugf("+++item:%d v:%d\n", item+1, tmp);

	map[item] = 1;
	s_safe.items[item].total = tmp;

	return tmp;
}


static void do_process()
{
	int i = 0;
	int sum = 0;
	int len[MAX_ITEM] = {0};
	int map[MAX_ITEM] = {0};

	for (i = 0; i < s_safe.item_num; i++)
	{
		if (s_safe.items[i].depend[0] <= 0)
		{
			continue;
		}

		sum += do_select(i, map, &len[i]);
	}

	printf("%d\n", sum);
	for (i = 0; i < s_safe.item_num; i++)
	{
		if (map[i])
		{
			printf("%d\n", i+1);
		}
	}

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
