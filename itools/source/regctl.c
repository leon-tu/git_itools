#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <getopt.h>

#include "regctl.h"

#define RW_UNIT     64

static int file_des = 0;
static char *version = "1.0";
#define REGCTL_DEVMEM_BASE		0xf6000000
#define REGCTL_DEVMEM_TAIL		0xf9000000

unsigned int do_devmem_map(off_t offset, int size)
{
	char *src_filename = "/dev/mem";
	unsigned int mem_mapped = 0;

	file_des = open(src_filename, O_RDWR);
	if (file_des) {
		mem_mapped =
		    (unsigned int)mmap(0, size, PROT_READ | PROT_WRITE,
				       MAP_SHARED, file_des, offset);
		if (mem_mapped == 0xffffffff) {
			printf("mmap failed\n");
		} else {
			//printf("mapped memory@:%x\n", mem_mapped);
		}
	} else {
		printf("%s file open failed\n", src_filename);
	}

	return mem_mapped;
}

int do_devmem_unmap(unsigned int mem_mapped, int size)
{
	if (file_des) {
		if (mem_mapped != 0xffffffff)
			munmap((void *)mem_mapped, size);

		close(file_des);
	}

	return 0;
}

void FileRwScan(char *src_filename, unsigned int position, int size,
		char *dst_filename)
{
	FILE *src_fp = NULL, *dst_fp = NULL;
	unsigned int mem_mapped = 0;
	int *ptr = 0;
	char buf[RW_UNIT];
	int read_len = 0;
	int total_read = 0;
	//unsigned long long position = 18597150720-6*1024;
	//unsigned long long position = 0;
	if (!dst_filename) {
		dst_filename = "./devmem_dump.dat";
	}
	if (!size) {
		size = RW_UNIT;
	}

	printf("scan %s from position:0x%x\n", src_filename, position);

	mem_mapped = do_devmem_map(position, size);
	if (mem_mapped == 0xffffffff) {
		goto failed_ret;
	}
	dst_fp = fopen(dst_filename, "wb");
	if (dst_fp == NULL) {
		printf("Open file %s failed\n", dst_filename);
		goto failed_ret;
	}

	while (total_read < size) {
#if 0
		read_len = fread(buf, 1, sizeof(buf), src_fp);
		if (read_len != sizeof(buf)) {
			if (feof(src_fp)) {
				printf("Reach file end.\n");
				goto failed_ret;
			} else {
				perror("fread");
				printf("Read error at position:%x, len:%d\n",
				       position, read_len);
				goto failed_ret;
			}
		} else {
			//fwrite64(buf, 1, sizeof(buf), dst_fp);
			printf("write bytes %llu\n",
			       fwrite(buf, 1, sizeof(buf), dst_fp));

		}
#else
		read_len = sizeof(buf);
		ptr = (int *)(mem_mapped + total_read);
		// save memory
		memcpy(buf, (void *)(mem_mapped + total_read), read_len);
		{
			int i;
			int mask = 0;
			for (i = 0; i < read_len / 4; i++) {
				*ptr = 0xffffffff;

				if (*ptr != 0xffffffff) {
					mask = (*ptr) ^ 0xffffffff;
					printf
					    ("mapped memory phyaddr:%x, unwritable bit mask:%08x\n",
					     position + (int)ptr -
					     (int)mem_mapped, mask);
				}
				ptr++;
			}
		}
		// restore memory
		memcpy((void *)(mem_mapped + total_read), buf, read_len);
		if (memcmp
		    ((void *)mem_mapped + total_read, (void *)buf, read_len)) {
			printf("can't recover mapped memory\n");
		} else
			printf("write bytes %x, offset:%x\n",
			       fwrite((void *)(mem_mapped + total_read), 1,
				      read_len, dst_fp), position + total_read);
#endif
		total_read += read_len;
		fflush(dst_fp);
	}
	printf("finish file scan, see:%s\n", dst_filename);

 failed_ret:
	do_devmem_unmap(mem_mapped, size);
	src_fp ? fclose(src_fp) : 0;
	dst_fp ? fclose(dst_fp) : 0;

	return 0;
}

int devmem_access_test(unsigned int addr, unsigned int size)
{
	unsigned int mmem_addr = 0, test_addr = 0, range_size = 0, scaned = 0;
	char c;
	unsigned char *ptr = 0;

	if (addr == 0) {
		addr = 0xf7cc0000;
		size = 0x40000;
	} else if (addr == 1) {
		//unsigned int test_value = 0, magic = 0xdf;
		fprintf(stderr, "Please enter phy addr to map (HEX)\n");
		scanf("%x", &addr);
		fprintf(stderr, "Please enter memory size (HEX)\n");
		scanf("%x", &size);
	}

	mmem_addr = do_devmem_map(addr, size);
	if (mmem_addr == 0xffffffff)
		return -1;

	fprintf(stderr, "Please enter phy addr to scan (HEX)\n");
	scanf("%x", &test_addr);
	fprintf(stderr, "Please enter phy range to scan (HEX)\n");
	scanf("%x", &range_size);

	if (!(test_addr >= addr && test_addr < (addr + size))) {
		printf("wrong address return\n");
		goto to_ret;
	}

	ptr = (unsigned char *)(mmem_addr + test_addr - addr);
	while (scaned < range_size) {
		unsigned int test_value = 0, magic = 0xdf, rwflag = 0;

		if (scaned % 64 == 0) {
			printf("\n0x%08x: ", test_addr + scaned);
		} else if (scaned % 32 == 0) {
			printf("\t");
		} else if (scaned % 16 == 0) {
			printf(" ");
		}
		test_value = *ptr;
		//printf("readable:%x\t", test_value);
		//if (test_value == magic) magic++;
		*ptr = magic;
		if (*ptr == magic) {
			*ptr = ++magic;
			if (*ptr == magic) {
				rwflag = 1;
				printf("w");
			}
		} else if (*ptr != 0xff) {
			*ptr = 0xff;
			if (*ptr & 0xff) {
				rwflag = 2;
				printf("p");
			}
		} else {
		}

		*ptr = test_value;
		rwflag ? 0 : printf("r");
		scaned++;
		ptr++;

		fflush(stdout);
	}
	printf("\n");

 to_ret:
	do_devmem_unmap(mmem_addr, size);

	return 0;
}

int devmem_access_get_value(unsigned int addr, unsigned int size)
{
	unsigned int mmem_addr = 0, test_addr = addr, range_size =
	    size, scaned = 0;
	char c;
	unsigned int *ptr = 0;

	if (addr == 0) {
		addr = 0xf7cc0000;
		size = 0x4000;
	} else if (addr == 1) {
		//unsigned int test_value = 0, magic = 0xdf;
		fprintf(stderr, "Please enter phy addr to map (HEX)\n");
		scanf("%x", &addr);
		fprintf(stderr, "Please enter memory size (HEX)\n");
		scanf("%x", &size);
	}

	mmem_addr = do_devmem_map(addr, size);
	if (mmem_addr == 0xffffffff) {

		addr = REGCTL_DEVMEM_BASE;
		size += test_addr - REGCTL_DEVMEM_BASE;
		printf("mmap try again @%x \n", addr);
		mmem_addr = do_devmem_map(addr, size);
		if (mmem_addr == 0xffffffff) {
			return -1;
		}
	}

	if (!(test_addr >= addr && test_addr < (addr + size))) {
		printf("wrong address return\n");
		goto to_ret;
	}

	ptr = (unsigned int *)(mmem_addr + test_addr - addr);
	while (scaned < range_size) {
		unsigned int test_value = 0, magic = 0xdf, rwflag = 0;

		if (scaned % 16 == 0) {
			printf("\n0x%08x(%p): ", test_addr + scaned, ptr);
		}

		printf(" %08x ", *ptr);

		scaned += 4;
		ptr++;

		fflush(stdout);
	}
	printf("\n");

 to_ret:
	do_devmem_unmap(mmem_addr, size);

	return 0;
}

int devmem_access_set_value(unsigned int addr, unsigned int value)
{
	unsigned int mmem_addr = 0, test_addr = addr, size = 64;
	char c;
	unsigned int *ptr = 0;

	if (addr == 0) {
		addr = 0xf7cc0000;
	} else if (addr == 1) {
		//unsigned int test_value = 0, magic = 0xdf;
		fprintf(stderr, "Please enter phy addr to map (HEX)\n");
		scanf("%x", &addr);
	}

	mmem_addr = do_devmem_map(addr, size);
	if (mmem_addr == 0xffffffff) {

		addr = REGCTL_DEVMEM_BASE;
		size += test_addr - REGCTL_DEVMEM_BASE;
		printf("mmap try again @%x \n", addr);
		mmem_addr = do_devmem_map(addr, size);
		if (mmem_addr == 0xffffffff) {
			return -1;
		}
	}

	if (!(test_addr >= addr && test_addr < (addr + size))) {
		printf("wrong address return\n");
		goto to_ret;
	}

	ptr = (unsigned int *)(mmem_addr + test_addr - addr);

	*ptr = value;

 to_ret:
	do_devmem_unmap(mmem_addr, size);

	return -1;
}

int read_hex(char *c)
{
	unsigned int value = 0;
	unsigned char *ptr = 0;
	int i = 0;
	if (c[0] == 0 && (c[1] == 'x' || c[1] == 'X')) {
		int len = strlen(c);
		ptr = &c[len - 1];
		while (ptr[i]) {
			if (ptr[i] > 31) ;
		}
	}
	printf("wrong input hex :%s", c);
	return 0;
}


#define REGCTL_DEFAULT_LOADFILEPATH		"./regctl_data.bin"
static FILE *load_file_ = NULL;

#define REGCTL_O_BINARY					0xc0
#define REGCTL_O_ASCII					0xc1
static int output_type_ = REGCTL_O_ASCII;
static int debug_ = 0;
static int verbose_ = 0;

#define REGCTL_MAN_READ			"   read [ADDR] [SIZE]\tdump registers from address [addr] with [SIZE], default size is 16\n\
   read [ADDR] [SIZE]  [--binary|-b] [--output|-o] [FILE], default binary save in ./regctl_data.bin, please use [--output|-o] to specify output file path\n"

#define REGCTL_MAN_WRITE 		"   write [ADDR] [VAL]\twritw regster at address with value [VAL] aligned"
#define REGCTL_MAN_HELP			"   help [COMMAND]    \tshow this help, and show usage of COMMAND"
#define REGCTL_MAN_TEST		"devmem_assess_test(0,0)"

static const char *optString = "bo:v?";
static const struct option CommandOption[] = {
	{"binary", no_argument, NULL, 'b'},
	{"output", required_argument, NULL, 'o'},
	{"verbose", no_argument, NULL, 'v'},
	{NULL, no_argument, NULL, 0}
};

//static int __regctl_cmd_help(char *cmd);
//static void display_usage(void);

CMD_HANDLER gCmdHandler_regctl[] = {
	{"read", cmd_handler_read_regctl, REGCTL_MAN_READ},
	{"write", cmd_handler_write_regctl, REGCTL_MAN_WRITE},
	{"test", cmd_handler_test_regctl, REGCTL_MAN_TEST},
	{"help", cmd_handler_help_regctl, REGCTL_MAN_HELP},

};

int iNumCmd_regctl = sizeof(gCmdHandler_regctl)/sizeof(CMD_HANDLER);

static int DebugVerboseSet(int argc, char *argv[])
{	
	unsigned int longIndex = 0;
	debug_ = verbose_ = 0;
	
    	while (longIndex < argc)
    	{
        		if (strcasecmp(argv[longIndex], "-v") == 0)
        		{
          		debug_ = verbose_ = 1;
      			break;
        		}
        		longIndex++;
   	 }

	return 0;
}




static long myatol(char *string)
{
	int base = 10;
	char *stop = 0;;
	char *digital = string;
	unsigned long val = 0, len = 0;

	if (digital[0] == '0' && digital[1] == 'x') {
		base = 16;
		//digital += 2;
	}
	val = strtoul(&digital[2], &stop, base);
	len = strlen(stop);

//      printf("digital string:%s %s base:%d len:%d\n", digital, stop, base, len);
	if (len) {
		printf("invalid char:%c, please add 0x before HEX string:%s\n",
		       *stop, digital);
		exit(EXIT_FAILURE);
	}
	return val;
}

static int __regctl_cmd_help(char *cmd)
{
	unsigned int longIndex = 0;

	while (longIndex < iNumCmd_regctl) {
    	if (0 == strncmp(cmd, gCmdHandler_regctl[longIndex].pCmd, 16)) 
	{
    		printf("%s\n", gCmdHandler_regctl[longIndex].pHelp);
    		return 0;
    	}
    	longIndex++;
	}

	printf("\n Invalid command \n");    //added
	return -1;
}



int cmd_handler_read_regctl(int argc, char *argv[])
{

	unsigned int addr = 0, size = 0;
	int longIndex = 0, i;
	int opt = 0;
	char *file_path = "./regctl_data.bin";

	//for (longIndex = 0; longIndex < argc; longIndex++)
	//	printf("argv[%d]:%s\n", longIndex, argv[longIndex]);
	//printf("\n");

	if (argc == 2) {
//      printf("argc[1]:%s\n", argv[1]);
		addr = myatol(argv[1]);
		size = 16;
	} else {
		longIndex = 0;
		opt =
		    getopt_long(argc, argv, optString, CommandOption,
				&longIndex);
		while (opt != -1) {
		    if (debug_)
                printf("\noptarg:%s optind:%d optopt:%d opt:%c\n",
				     optarg, optind, optopt, opt);
			switch (opt) {
			case 'b':
				output_type_ = REGCTL_O_BINARY;
				break;
			case 'o':
				file_path = optarg;
				if (verbose_) printf("open output file: %s\n", file_path);
				load_file_ = fopen(file_path, "wb");
				if (!load_file_) {
					printf("open output file %s fail\n",
					       file_path);
				}
				break;
		    case '?':
		        //display_usage();
		        __regctl_cmd_help(argv[0]);
		        return -1;
			default:
				printf("should not be here:%c\n", opt);
				break;
			}

			opt =
			    getopt_long(argc, argv, optString, CommandOption,
					&longIndex);
		}

		if (output_type_ == REGCTL_O_BINARY && !load_file_) {
			load_file_ = fopen(file_path, "wb");
			if (!load_file_) {
				perror("fail open file\n");
			}
			if (verbose_)printf("open default file ./regctl_data.bin\n");
		}

        if (debug_){
            printf("\noptarg:%s optind:%d optopt:%d opt:%c\n",
                     optarg, optind, optopt, opt);

    		for (longIndex = 0; longIndex < argc; longIndex++)
    			printf("argv[%d]:%s\n", longIndex, argv[longIndex]);
    		printf("\n");
        }
        longIndex = optind;
		addr = myatol(argv[longIndex++]);

		if (longIndex < argc)
			size = myatol(argv[longIndex]);

		if (addr < REGCTL_DEVMEM_BASE || addr > REGCTL_DEVMEM_TAIL) {
			//printf("bad address: 0x%x, out of range\n", addr);
		}

		if (!size)
			size = 16;
		if (size > 1024) {
			printf("read size:%d > 1024, cut", size);
			size = 1024;
		}
	}
	printf("memory/register dump from address:0x%x, size:0x%x\n", addr,
	       size);
	return devmem_access_get_value(addr, size);
}


int cmd_handler_write_regctl(int argc, char *argv[])
{
	unsigned int addr, value;
	int longIndex = 0, i;
	int opt = 0;
	char *file_path = "./regctl_data.bin";

	if (argc == 3) {
        addr = myatol(argv[1]);
		value = myatol(argv[2]);
	} else {
		longIndex = 0;
		opt =
		    getopt_long(argc, argv, optString, CommandOption,
				&longIndex);
		while (opt != -1) {
		    if (debug_)
                printf("\noptarg:%s optind:%d optopt:%d opt:%c\n",
				     optarg, optind, optopt, opt);
			switch (opt) {
			case 'b':
				output_type_ = REGCTL_O_BINARY;
				break;
			case 'o':
				file_path = optarg;
				if (verbose_) printf("open output file: %s\n", file_path);
				load_file_ = fopen(file_path, "wb");
				if (!load_file_) {
					printf("open output file %s fail\n",
					       file_path);
				}
				break;
		    case '?':
		        //display_usage();
		        __regctl_cmd_help(argv[0]);
		        return -1;
			default:
				printf("should not be here:%c\n", opt);
				break;
			}

			opt =
			    getopt_long(argc, argv, optString, CommandOption,
					&longIndex);
		}

		if (output_type_ == REGCTL_O_BINARY && !load_file_) {
			load_file_ = fopen(file_path, "wb");
			if (!load_file_) {
				perror("fail open file\n");
			}
			if (verbose_) printf("open default file ./regctl_data.bin\n");
		}

        if (debug_){
            printf("\noptarg:%s optind:%d optopt:%d opt:%c\n",
                     optarg, optind, optopt, opt);

    		for (longIndex = 0; longIndex < argc; longIndex++)
    			printf("argv[%d]:%s\n", longIndex, argv[longIndex]);
    		printf("\n");
        }
        longIndex = optind;
		addr = myatol(argv[longIndex++]);
        value = myatol(argv[longIndex]);
	}

	if (addr < REGCTL_DEVMEM_BASE || addr > REGCTL_DEVMEM_TAIL)
		printf("bad address: 0x%x, outof range\n", addr);

	printf("set memory/register at address:0x%x with value:0x%x\n orignal value is:", addr,
	       value);
    devmem_access_get_value(addr, 4);

	return devmem_access_set_value(addr, value);
}

int cmd_handler_test_regctl(int argc, char *argv[])
{
	return devmem_access_test(0, 0);
}

/*
static int __regctl_cmd_help(char *cmd)
{
	unsigned int longIndex = 0;

	while (longIndex < iNumCmd_regctl) {
    	if (0 == strncmp(cmd, gCmdHandler_regctl[longIndex].commstr, 16)
    	    && gCmdHandler_regctl[longIndex].cmdfunc) {
    		printf("%s\n", gCmdHandler_regctl[longIndex].helpstring);
    		return 0;
    	}
    	longIndex++;
	}

	printf("\n Invalid command \n");    //added
	return -1;
}

*/
int cmd_handler_help_regctl(int argc, char *argv[])
{	
	printf("Marvell Galois PE Debug Control System\n\n");
   	printf("  Revision: %s\n", version);
    	//printf("  Author: Jun Ma <junma@marvell.com>\n");
    	printf("  Usage: 1. regctl command [arg1] [arg2] ...\n\n");
    	printf("         2. regctl; \n             command [arg1] [arg2] ...\n\n");
    	printf("All supported commands:\n\n");
	int i;
    	for (i = 0; i < iNumCmd_regctl; i++)
    	{
        		printf("         %-12s- %s\n", gCmdHandler_regctl[i].pCmd, gCmdHandler_regctl[i].pHelp);
    	}

	return 0;
//	unsigned int longIndex = 0;
/*
	if (argc == 1) {
		display_usage();
		return 0;
	}
	
    return __regctl_cmd_help(argv[1]);
    */
}

/* Display program usage, and exit.
 */
 /*
static void display_usage(void)
{
        printf("  Berlin Register Control\n\n");
        printf("  Version: %s\n", version);
       // printf("  Author: \n");
        printf("  Usage: 1. regctl command [arg1] [arg2] ...\n\n");
        printf("         2. regctl; \n             command [arg1] [arg2] ...\n\n");
        printf("  All supported commands:\n\n");
        printf("        %-20s- %s\n", "help [COMMAND]", "show this help, and show usage of COMMAND");
        printf("        %-20s- %s\n", "read [ADDR] [SIZE]","dump registers from address [addr] with [SIZE], default size is 16");
        printf("        %-20s- %s\n\n", "write [ADDR] [VAL]","writw regster at address with value [VAL] aligned");
	//exit(EXIT_FAILURE);      //need to omit 
}

*/

/*

int RegctlMain(int argc, char *argv[])
{
	
	unsigned int longIndex = 0;

    while (longIndex < argc)
    {
        if (strcasecmp(argv[longIndex], "-v") == 0)
        {
            debug_ = verbose_ = 1;
            break;
        }
        longIndex++;
    }
    
	longIndex = 0;
	while (longIndex < iNumCmd_regctl) {
		if (0 == strncmp(argv[1], gCmdHandler_regctl[longIndex].commstr, 16)
		    && gCmdHandler_regctl[longIndex].cmdfunc) {
			gCmdHandler_regctl[longIndex].cmdfunc(argc - 1, &argv[1]);
			return 0;
		}
		longIndex++;
	}

	printf("\n Invalid command \n");	//added
	//display_usage();
#if 0
	else
if (argc == 2) {
	if (argv[1][0] == '-') {
		if (argv[1][1] == 'u') {
			devmem_access_test(0, 0);
		} else if (argv[1][1] == 'r') {
			fprintf(stderr,
				"Please enter phy addr to scan (HEX)\n");
			scanf("%x", &addr);
			fprintf(stderr,
				"Please enter phy range to scan (HEX)\n");
			scanf("%x", &size);
			devmem_access_get_value(addr, size);
		} else if (argv[1][1] == 'w') {
			fprintf(stderr, "Please enter phy addr (HEX)\n");
			scanf("%x", &addr);
			fprintf(stderr, "Please enter value to set (HEX)\n");
			scanf("%x", &size);
			devmem_access_set_value(addr, size);
		} else if (argv[1][1] == 'h') {
			fprintf(stderr, "usage [-u|-h]\n");
		}
	}
} else if (argc == 3) {

	sscanf(argv[1], "%x", &addr);
	sscanf(argv[2], "%x", &size);
	printf("address:%x, size:%x\n", addr, size);
	devmem_access_get_value(addr, size);
} else
	FileRwScan("/dev/mem", 0xf7cc8000, 0x8000, 0);
#endif

return 0;
}

*/
