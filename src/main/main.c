#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include "matrix.h"
#include "bmp.h"
#include "main.h"
#include "random.h"
#include "messages.h"
#include "recovery.h"
#include "distribution.h"

options_st * options;

int 
main(int argc, char* argv[])
{
	printWelcome();
	options = malloc(sizeof(*options));
    char option;
	int valid = 1;
	while(valid && (option = getopt(argc, argv, "d:r:s:m:k:n:i:h")) != -1)
	{
		switch(option)
		{
			// secret image distribution
			case 'd':
				valid = setMode(DISTRIBUTION_MODE);
				break;
			// secret image recovery
			case 'r':
				valid = setMode(RECOVERY_MODE);
				break;
			// In case of -d it must be a B&W and it will be the image to hide
			// In case of -r it will the output image file.
			case 's':
				valid = setImage(optarg);
				break;
			// In case of -d it must be a B&W and same size as secret image (watermark).
			// In case of -r it will contain the transformation of the de watermark image.
			case 'm':
				valid = setWatermark(optarg);
				break;
			// mininum number of necessary shadows
			case 'k':
				valid = setK(atoi(optarg));
				break;
			// total number of shadows to be distributed
			case 'n':
				valid = setN(atoi(optarg));
				break;
			// In case of -d path to where the storing images are
			// In case of -r path to where the images that contain the secret are
			case 'i':
				valid = setDirectory(optarg);
				break;
			case 'h':
				printHelp();
				break;
			default:
				exit(0);
				break;
		}
	}
	if (argv[optind] == NULL) 
	{
		free(options);
		printHelp();
		return EXIT_SUCCESS;
	}
	if (!valid)
	{
		free(options);
		printError(options->error);
		return EXIT_FAILURE;
	}
	if (options->k > options->n)
	{
		printError("k should be less or equal to n");
		free(options);
		return EXIT_FAILURE;
	}
	execute(options);
	free(options);
	return EXIT_SUCCESS;
}

int execute(options_st * options)
{
	switch(options->mode)
	{
		case DISTRIBUTION_MODE: distributeSecret(options->image, options->k, options->n, options->dir, options->watermark); break;
		case RECOVERY_MODE: //recoverSecret(); break;
		default: exit(0);
	}
	return 0;
}

int setImage(const char* image)
{
	if (fileExists(image))
	{
		options->image = image;
		return 1;
	}
	return 0;
}

int setWatermark(const char* watermark)
{
	if (fileExists(watermark))
	{
		options->watermark = watermark;
		return 1;
	}
	return 0;
}

int setDirectory(const char* directory)
{
	if (directoryExists(directory))
	{
		options->dir = directory;
		return 1;
	}
	return 0;
}


int directoryExists(const char* directory)
{
	DIR* dir = opendir(directory);
	if (dir)
	{
    	closedir(dir);
	}
	else if (ENOENT == errno)
	{
		options->error = "Directory does not exist";
	}
	return 0;
}

int fileExists(const char * name)
{
    FILE *file;
    if ((file = fopen(name, "r")))
	{
        fclose(file);
        return 1;
    }
    return 0;
}

int setMode(int mode)
{
	if (mode != DISTRIBUTION_MODE && mode != RECOVERY_MODE)
	{
		options->error = "Error setting mode";
		return 0;
	}
	options->mode = mode;
	return 1; 
}

int setK(int k)
{
	if (k >= 2 && k < 252)
	{
		options->k = k;
		return 1;
	}
	options->error = "K must be a positive integer and less than 252";
	return 0;
}

int setN(int n)
{
	if (n > 0 && n < 252)
	{
		options->n = n;
		return 1;
	}
	options->error = "N must be a positive integer and less than 252";
	return 0;
}
